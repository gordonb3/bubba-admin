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
        define("MIN",60);
        define("HOUR",3600);
        define("DAY",86400);

        $secs_tot = $this->get_raw_uptime();
        $days = intval($secs_tot/DAY);
        $hours = intval(($secs_tot%DAY)/HOUR);
        $minutes = intval(($secs_tot%HOUR)/MIN);
        $secs = intval($secs_tot%MIN);

        return new DateInterval("P{$days}DT{$hours}H{$minutes}M{$secs}S");
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

}
