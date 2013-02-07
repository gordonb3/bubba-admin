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


  public function add_remote_account($type, $username, $password, $host) {
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

  public function add_webdav($type, $username, $password) {
    $url = $this->get_webdav_url($type);
    $path = $this->get_webdav_path($type, $username);
    _system("umount", $path);
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
    _system("umount", $path);

    _system('umount', '-f', $path);
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

  public function add_sshfs($host, $username, $uuid) {
    $sshkey = implode(DIRECTORY_SEPARATOR, array(self::ssh_keydir, $uuid));

    $oldfstab = file_get_contents(self::fstab_file);
    $fstab = preg_replace("#^sshfs\#".preg_quote($username)."@".preg_quote($host).".*#m", "", $oldfstab);
    $remotepath = $this->get_sshfs_remotepath();
    $path = $this->get_sshfs_path($host, $username);
    _system("umount", $path);

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
    _system("umount", $path);

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
