<?php
$CI =& get_instance();
define("SCRIPTDIR","/opt/bubba/bin");
define("BACKEND",SCRIPTDIR."/backend.pl");
define("BACKUP",SCRIPTDIR."/backup.pl");
define("RESTORE_LOCKFILE","/var/lock/restore.lock");
define("BACKUP_LOCKFILE","/var/lock/backup.lock");

define("UPDATER",SCRIPTDIR."/updater.pl");
define("PRINTFS",SCRIPTDIR."/print.pl");
define("FIREWALL",SCRIPTDIR."/firewall.pl");
define("DISK",SCRIPTDIR."/disk.pl");
define("ADMINFUNCS",SCRIPTDIR."/adminfunctions.php");
define("IPCFUNCS","/opt/bubba/web-admin/ftd/ipc.php");
define("PWFILE","/etc/shadow");
define("UINFOFILE","/etc/passwd");
define("VERSION","BUBBA_VERSION");
define("FORMPREFIX",preg_replace("|^(/[^/]*)[/$].*$|","\\1",$_SERVER["REQUEST_URI"]));
define("FALLBACKIP","192.168.10.1");
define("BUBBA_VERSION","/etc/bubba/bubba.version");
define("USER_CONFIG",".bubbacfg");
define("ADMINCONFIG","/home/admin/".USER_CONFIG);

if(isB3()) {
	define("NAME","B3");
	define("DEFAULT_HOST", "b3");
} else {
	define("NAME","Bubba|2");
	define("DEFAULT_HOST", "bubba");
}

if($CI->session->userdata("language")){
	define("LANGUAGE",$CI->session->userdata("language"));
	define("CURRENT_LOCALE",$CI->session->userdata("locale"));
}else{
	if(file_exists(ADMINCONFIG)) {
		$conf = parse_ini_file(ADMINCONFIG);
		if(isset($conf['default_lang'])) {
			define("LANGUAGE",$conf['default_lang']);
		} else {
			// Default, make a guess?
			define("LANGUAGE","en");
        }
		if(isset($conf['default_locale'])) {
			define("CURRENT_LOCALE",$conf['default_locale']);
		} else {
			// Default, make a guess?
			define("CURRENT_LOCALE","en_US");
		}

	} else {
		// Default, make a guess?
		define("LANGUAGE","en");
        define("CURRENT_LOCALE","en_US");
	}
}

if($CI->session->userdata("theme")){
	define("THEME",$CI->session->userdata("theme"));
}else{
	// Default
	define("THEME","default");
}

if($CI->session->userdata("user")){
	define("USER",$CI->session->userdata("user"));
}else{
	// Default - should not be possible
	define("USER","none");
}


?>
