<?php

class System extends CI_Model {
  private $version;
  public function __construct() {
    parent::__construct();
  }

  public function set_timezone($timezone) {

    $target = "/usr/share/zoneinfo/$timezone";
    if(!file_exists($target)) {
      throw new Exception("Timezone $timezone doesn't exists");
    }
    unlink('/etc/localtime');
    symlink($target, '/etc/localtime');
    file_put_contents('/etc/timezone', $timezone);
  }

  public function get_timezone() {
    return trim(file_get_contents('/etc/timezone'));
  }

  # Lists all timezones with region, UTC has region false
  public function list_timezones() {
    $timezones = array();
    foreach(DateTimeZone::listIdentifiers() as $ts) {
      if(strpos($ts,'/')) {
        list($region, $country) = explode('/', $ts);
        $timezones[$country] = $region;
      } else {
        $timezones[$ts] = false;
      }
    }
    ksort($timezones);
    return $timezones;
  }

  public function get_raw_uptime() {
    $upt=file("/proc/uptime");
    sscanf($upt[0],"%d",$secs_tot);
    return $secs_tot;
  }

  public function get_uptime() {
    $start = new DateTime();
    $start->sub(DateInterval::createFromDateString("{$this->get_raw_uptime()} seconds"));
    return $start->diff(new DateTime());
  }

  public function get_system_version() {
    if(!$this->version) {
      $this->version = file_get_contents(BUBBA_VERSION);
    }
    return $this->version;
  }

  public function get_hardware_id() {
    return getHWType();
  }

  public function list_printers() {
    $json =  _system('cups-list-printers');
    return json_decode(implode($json),true);
  }

  const accounts_file = '/etc/bubba/remote_accounts.yml';
  const remote_jobs_file = '/etc/bubba/remote_backup_jobs.yml';
  const fstab_file = '/etc/fstab';
  const webdav_secrets_file  = '/etc/davfs2/secrets';
  const ssh_keydir = '/etc/bubba/ssh-keys';

  # faithfully stolen from http://stackoverflow.com/questions/2040240/php-function-to-generate-v4-uuid
  private function gen_uuid() {
    return sprintf( '%04x%04x-%04x-%04x-%04x-%04x%04x%04x',
      // 32 bits for "time_low"
      mt_rand( 0, 0xffff ), mt_rand( 0, 0xffff ),

      // 16 bits for "time_mid"
      mt_rand( 0, 0xffff ),

      // 16 bits for "time_hi_and_version",
      // four most significant bits holds version number 4
      mt_rand( 0, 0x0fff ) | 0x4000,

      // 16 bits, 8 bits for "clk_seq_hi_res",
      // 8 bits for "clk_seq_low",
      // two most significant bits holds zero and one for variant DCE1.1
      mt_rand( 0, 0x3fff ) | 0x8000,

      // 48 bits for "node"
      mt_rand( 0, 0xffff ), mt_rand( 0, 0xffff ), mt_rand( 0, 0xffff )
    );
  }
  private function create_ssh_key($uuid) {
    $priv_path = implode(DIRECTORY_SEPARATOR, array(self::ssh_keydir, $uuid));
    if(file_exists($priv_path)) {
      # XXX forcefully removing old key
      @unlink($priv_path);
      @unlink($priv_path.".pub");
    }
    _system("ssh-keygen", '-f', $priv_path, '-N', '', '-q');
    $pubkey = file_get_contents($priv_path.'.pub');
    return $pubkey;
  }

  public function get_pubkey($uuid) {
    $priv_path = implode(DIRECTORY_SEPARATOR, array(self::ssh_keydir, $uuid));
    $pub_path = $priv_path.'.pub';
    $pubkey = file_get_contents($pub_path);
    return $pubkey;
  }

  private function upload_sshkey($host, $username, $password, $uuid) {
    $priv_path = implode(DIRECTORY_SEPARATOR, array(self::ssh_keydir, $uuid));
    $pub_path = $priv_path.'.pub';
    $descriptorspec = array(
      13 => array('pipe', 'r')
    );

    $process = proc_open("sshpass-copy-id -i $pub_path $username@$host", $descriptorspec, $pipes);
    fwrite($pipes[13], $password);
    fclose($pipes[13]);
    $ret = proc_close($process);
    error_log("sshpass-copy-id returned $ret!");
  }

  private $hidrive_token = null;
  private $hidrive_api = 'https://api.hidrive.strato.com/1.0/';

  public function get_hidrive_token($username, $password, $force = false) {
    if($this->hidrive_token && !$force) {
      return $this->hidrive_token;
    }

    $request = new HTTP_Request2($this->hidrive_api."auth.getToken", HTTP_Request2::METHOD_GET, array('ssl_verify_peer' => false));
    $request->setAuth($username, $password);

    $response = $request->send();
    if($response->getStatus() == 200) {
      $data = json_decode($response->getBody(), true);
      if(isset($data['status']['code']) && $data['status']['code'] == 0) {
        $this->hidrive_token =  $data['data']['token'];
        return $this->hidrive_token;
      } else {
        return false;
      }
    }
  }

  public function verify_hidrive_protocols($token) {
    $request = new HTTP_Request2($this->hidrive_api."space.getFeatures", HTTP_Request2::METHOD_GET, array('ssl_verify_peer' => false));
    $request->setHeader('X-Auth-Token', $token);

    $response = $request->send();
    if($response->getStatus() == 200) {
      $data = json_decode($response->getBody(), true);
      if(isset($data['status']['code']) && $data['status']['code'] == 0) {
        $protocols = $data['data']['protocols'];
        return $protocols['rsync'] && $protocols['webdav'];
      }
    }
  }

  public function verify_hidrive_space($token, $required) {
    $request = new HTTP_Request2($this->hidrive_api."quota.list", HTTP_Request2::METHOD_GET, array('ssl_verify_peer' => false));
    $request->setHeader('X-Auth-Token', $token);

    $response = $request->send();
    if($response->getStatus() == 200) {
      $data = json_decode($response->getBody(), true);
      if(isset($data['status']['code']) && $data['status']['code'] == 0) {
        $root = $data['data']['root'];
        return $required < $root['available'];
      }
    }
  }


  public function add_remote_account($type, $username, $password, $host) {

    # Check HiDrive for permissions
    # https://api.freemium.stg.rzone.de/ contains incomplete api spec
    if($type == 'HiDrive') {
      $token = $this->get_hidrive_token($username, $password);
      if(!$token) {
        throw new Exception(_("Unable to verify credentials"));
      }
      if(!$this->verify_hidrive_protocols($token)) {
        throw new Exception(_("Selected HiDrive account doesn't have the stuff required for this system"));
      }
    }

    $accounts = array();
    if(file_exists(self::accounts_file)) {
      $accounts = spyc_load_file(self::accounts_file);
    }


    $arr = array(
      'type' => $type,
      'username' => $username,
      'password' => $password
    );
    if($host) {
      $arr['host'] = $host;
      $key = "$type|$host|$username";
    } else {
      $key = "$type|$username";
    }

    if(isset($accounts[$key])) {
      throw new Exception(_('Account allready defined'));
    }
    $uuid = $this->gen_uuid();
    $pubkey = $this->create_ssh_key($uuid);
    $arr['uuid'] = $uuid;

    $accounts[$key] = $arr;
    file_put_contents(self::accounts_file,Spyc::YAMLDump($accounts));
    if($type == 'ssh') {
      $this->upload_sshkey($host, $username, $password, $uuid);
    }
    return array('key' => $key, 'uuid' => $uuid, 'pubkey' => $pubkey);
  }

  public function remove_remote_account($type, $username, $host) {
    if($host) {
      $target = "$type|$host|$username";
    } else {
      $target = "$type|$username";
    }

    $jobs = array();
    if(file_exists(self::remote_jobs_file)) {
      $jobs = spyc_load_file(self::remote_jobs_file);
    }
    unset($jobs[$target]);
    file_put_contents(self::remote_jobs_file,Spyc::YAMLDump($jobs));

    $accounts = array();
    if(file_exists(self::accounts_file)) {
      $accounts = spyc_load_file(self::accounts_file);
    }
    unset($accounts[$target]);
    file_put_contents(self::accounts_file,Spyc::YAMLDump($accounts));
    return true;
  }


  public function ssh_edit_remote_account($key, $username, $host) {

    $jobs = array();
    if(file_exists(self::remote_jobs_file)) {
      $jobs = spyc_load_file(self::remote_jobs_file);
    }
    $job = $jobs[$key];
    unset($jobs[$key]);


    $accounts = array();
    if(file_exists(self::accounts_file)) {
      $accounts = spyc_load_file(self::accounts_file);
    }
    $account = $accounts[$key];
    unset($accounts[$key]);

    $account['username'] = $username;
    $account['host'] = $host;

    $newkey = "ssh|$host|$username";
    $accounts[$newkey] = $account;

    $jobs[$newkey] = $job;

    file_put_contents(self::accounts_file,Spyc::YAMLDump($accounts));

    file_put_contents(self::remote_jobs_file,Spyc::YAMLDump($jobs));

    return $newkey;

  }

  public function get_remote_accounts() {
    $targets = array();
    if(file_exists(self::accounts_file)) {
      $accounts = spyc_load_file(self::accounts_file);
      foreach($accounts as $id => $account) {
        $target = array(
          'id' => $id,
          'uuid' => $account['uuid'],
          'type' => $account['type'],
          'username' => $account['username'],
        );
        if(isset($account['host'])) {
          $target['host'] = $account['host'];
        }
        $targets[] = $target;
      }
    }
    return $targets;
  }

  public function get_remote_account($key) {
    $target = array();
    if(file_exists(self::accounts_file)) {
      $accounts = spyc_load_file(self::accounts_file);
      foreach($accounts as $id => $account) {
        if($id != $key) {
          continue;
        }
        $target = array(
          'id' => $id,
          'type' => $account['type'],
          'username' => $account['username'],
          'uuid' => $account['uuid'],
        );
        if(isset($account['host'])) {
          $target['host'] = $account['host'];
        }
      }
    }
    return $target;
  }

  public function get_webdav_path($type, $username) {
    return "/home/admin/$type/$username";
  }

  public function create_webdav_path($type, $username) {
    $path = "/home/admin/$type";
    if(! file_exists($path) ) {
      mkdir($path, 0700);
      chown($path, 'admin');
      chgrp($path, 'admin');
    }
    $path = "/home/admin/$type/$username";
    if(! file_exists($path) ) {
      mkdir($path, 0700);
      chown($path, 'admin');
      chgrp($path, 'admin');
    }
  }

  public function get_webdav_url($type) {
    switch($type) {
    case 'HiDrive':
      return 'http://webdav.hidrive.strato.com';
    }
  }

  private function is_mounted($path) {
    exec("mountpoint -q " . escapeshellarg($path), $out, $ret);
    return $ret == 0;
  }
  public function add_webdav($type, $username, $password) {
    $url = $this->get_webdav_url($type);
    $path = $this->get_webdav_path($type, $username);

    if($this->is_mounted($path)) {
      _system("umount", '-f', $path);
    }
    $oldsecrets = file_get_contents(self::webdav_secrets_file);

    # Remove old path if allready there
    $secrets = preg_replace("#^".preg_quote($path).".*#m", "", $oldsecrets);

    $secrets .= sprintf("\n%s\t\"%s\"\t\"%s\"\n", addslashes($path), addslashes($username), addslashes($password));

    if($oldsecrets != $secrets) {
      file_put_contents(self::webdav_secrets_file, $secrets);
    }

    $oldfstab = file_get_contents(self::fstab_file);
    $fstab = preg_replace("#^".preg_quote($url)."\s+".preg_quote($path).".*#m", "", $oldfstab);

    $fstab .= "$url $path davfs defaults,gid=users,dir_mode=775,file_mode=664,_netdev 0 0\n";

    if(! file_exists($path) ) {
      $this->create_webdav_path($type, $username);
    }

    if($fstab != $oldfstab) {
      file_put_contents(self::fstab_file, $fstab);
      _system("mount", "-a");
    }

  }

  public function remove_webdav($type, $username) {
    $url = $this->get_webdav_url($type);
    $path = $this->get_webdav_path($type, $username);
    if($this->is_mounted($path)) {
      _system("umount", '-f', $path);
    }
    $oldsecrets = file_get_contents(self::webdav_secrets_file);

    # Remove old path if allready there
    $secrets = preg_replace("#^".preg_quote($path).".*#m", "", $oldsecrets);
    file_put_contents(self::webdav_secrets_file, $secrets);

    $oldfstab = file_get_contents(self::fstab_file);
    $fstab = preg_replace("#^".preg_quote($url)."\s+".preg_quote($path).".*#m", "", $oldfstab);
    if($fstab != $oldfstab) {
      file_put_contents(self::fstab_file, $fstab);
      _system("mount", "-a");
    }

    if( file_exists($path) ) {
      @rmdir($path);
    }
  }

  public function edit_webdav($type, $username, $password) {
    $this->remove_webdav($type, $username);
    $this->add_webdav($type, $username, $password);
  }


  public function get_sshfs_path($host, $username) {
    return "/home/admin/ssh/${username}@${host}";
  }
    public function get_sshfs_remotepath() {
    return "backup/" . gethostname();
  }

  public function create_sshfs_path($host, $username) {
    $path = "/home/admin/ssh";
    if(! file_exists($path) ) {
      @mkdir($path, 0700);
      chown($path, 'admin');
      chgrp($path, 'admin');
    }
    $path = $this->get_sshfs_path($host,$username);
    if(! file_exists($path) ) {
      @mkdir($path, 0700);
      chown($path, 'admin');
      chgrp($path, 'admin');
    }
  }

  public function move_sshfs($old_host, $old_username, $new_host, $new_username, $uuid) {
    if( $old_host == $new_host && $old_username == $new_username ) {
      return;
    }
    $this->remove_sshfs($old_hostm, $old_username);
    $this->add_sshfs($new_username, $new_host, $uuid);
  }

  public function add_sshfs($host, $username, $uuid) {
    $sshkey = implode(DIRECTORY_SEPARATOR, array(self::ssh_keydir, $uuid));

    $oldfstab = file_get_contents(self::fstab_file);
    $fstab = preg_replace("#^sshfs\#".preg_quote($username)."@".preg_quote($host).".*#m", "", $oldfstab);
    $remotepath = $this->get_sshfs_remotepath();
    $path = $this->get_sshfs_path($host, $username);
    if($this->is_mounted($path)) {
      _system("umount", '-f', $path);
    }

    $fstab .= "sshfs#$username@$host: $path fuse identityfile=$sshkey,defaults,allow_other,_netdev,gid=users 0 0\n";

    if(! file_exists($path) ) {
      $this->create_sshfs_path($host, $username);
    }

    if($fstab != $oldfstab) {
      file_put_contents(self::fstab_file, $fstab);
      sleep(5);
      _system("mount", "-a");
    }

  }

  public function remove_sshfs($host, $username) {
    $path = $this->get_sshfs_path($host, $username);
    if($this->is_mounted($path)) {
      _system("umount", '-f', $path);
    }

    $oldfstab = file_get_contents(self::fstab_file);
    $fstab = preg_replace("#^sshfs\#".preg_quote($username)."@".preg_quote($host).".*#m", "", $oldfstab);
    if($fstab != $oldfstab) {
      file_put_contents(self::fstab_file, $fstab);
      _system("mount", "-a");
    }
    if( file_exists($path) ) {
      @rmdir($path);
    }
  }
}
