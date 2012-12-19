<?php

class System extends Model {
  private $version;
  public function __construct() {
    parent::Model();
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
  const fstab_file = '/etc/fstab';
  const webdav_secrets_file  = '/etc/davfs2/secrets';

  public function add_remote_account($type, $username, $password, $sshkey) {
    $accounts = spyc_load_file(self::accounts_file);

    $arr = array(
      'type' => $type,
      'username' => $username,
      'password' => $password,
      'openssh' => $sshkey
    );

    $key = "$type|$username";
    if(isset($accounts[$key])) {
      throw new Exception('Account allready defined');
    } else {
      $accounts[$key] = $arr;
      file_put_contents(self::accounts_file,Spyc::YAMLDump($accounts));
      return $key;
    }
  }

  public function remove_remote_account($type, $username) {
    $accounts = spyc_load_file(self::accounts_file);
    unset($accounts["$type|$username"]);
    file_put_contents(self::accounts_file,Spyc::YAMLDump($accounts));
    return true;
  }

  public function get_remote_accounts() {
    $targets = array();
    if(file_exists(self::accounts_file)) {
      $accounts = spyc_load_file(self::accounts_file);
      foreach($accounts as $id => $account) {
        $targets[] = array(
          'id' => $id,
          'type' => $account['type'],
          'username' => $account['username'],
        );
      }
    }
    return $targets;
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
    $oldsecrets = file_get_contents(self::webdav_secrets_file);

    # Remove old path if allready there
    $secrets = preg_replace("#^".preg_quote($path).".*#m", "", $oldsecrets);

    $secrets .= sprintf("\n%s\t\"%s\"\t\"%s\"\n", addslashes($path), addslashes($username), addslashes($password));

    if($oldsecrets != $secrets) {
      file_put_contents(self::webdav_secrets_file, $secrets);
    }

    $oldfstab = file_get_contents(self::fstab_file);
    $fstab = preg_replace("#^".preg_quote($url)."\s+".preg_quote($path).".*#m", "", $oldfstab);

    $fstab .= "\n$url $path davfs defaults,gid=users,dir_mode=775,file_mode=664 0 0\n";

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

    _system('umount', '-f', $path);
    $oldsecrets = file_get_contents(self::webdav_secrets_file);

    # Remove old path if allready there
    $secrets = preg_replace("#^".preg_quote($path)."#m", "", $oldsecrets);
    file_put_contents(self::webdav_secrets_file, $secrets);

    $oldfstab = file_get_contents(self::fstab_file);
    $fstab = preg_replace("#^".preg_quote($url)."\s+".preg_quote($path)."#m", "", $oldfstab);
    file_put_contents(self::webdav_secrets_file, $secrets);
    if( file_exists($path) ) {
      @rmdir($path);
    }
  }
}
