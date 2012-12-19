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
 }
