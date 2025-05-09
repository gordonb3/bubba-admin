<?php

require_once "Mycontroller.php";

class Settings extends My_CI_Controller{

  function __construct(){
    parent::__construct();

    require_once(APPPATH."/legacy/defines.php");
    require_once(ADMINFUNCS);


    $this->Auth_model->enforce_policy('web_admin','administer', 'admin');
    $this->load->model('system');

  }

  function Settings(){
    self::__construct();
  }

  function _renderfull($content,$head=null, $mdata=array()){

    $navdata["menu"] = $this->menu->retrieve($this->session->userdata('user'),$this->uri->uri_string());
    $mdata["navbar"]=$this->load->view(THEME.'/nav_view',$navdata,true);
    $mdata["dialog_menu"] = $this->load->view(THEME.'/menu_view',$this->menu->get_dialog_menu(),true);
    $mdata["content"]=$content;
    if(!is_null($head)) {
      $mdata['head'] = $head;
    }

    $this->load->view(THEME.'/main_view',$mdata);
  }

  function _datetime($data = array()) {

    if(service_running("ntpd")) {
      $data['use_ntp']= true;
    }

    $data['t_zone'] = $this->system->get_timezone();
    date_default_timezone_set($data['t_zone']);

    $data['t_zoneinfo'] = get_timezone_info();
    if(isset($data['t_zoneinfo']['US'])) {
      unset($data['t_zoneinfo']['US']);
    }
    if(isset($data['t_zoneinfo']['Etc'])) {
      unset($data['t_zoneinfo']['Etc']);
    }
    if(isset($data['t_zoneinfo']['other'])) {
      unset($data['t_zoneinfo']['other']);
    }
    $data['date'] = date("Ymd");
    $data['time'] = date("Hi");

    return $data;
  }
  function netupdate($strip=""){

    $hostname = $this->input->post('hostname');
    $workgroup = $this->input->post('workgroup');


    $orig_workgroup=get_workgroup();

    $data["success"]=true;
    $data["err_hostinvalid"]=false;
    $data["err_ip"]=false;
    $data["err_gw"]=false;
    $data["err_dns"]=false;
    $data["err_netmask"]=false;

    $namechange=false;
    $restart_network=false;

    // Check hostname change
    if ($hostname!=php_uname('n')) {
      if(preg_match("/^[A-Za-z0-9-]+$/",$hostname)){
        //change_hostname($hostname);
        $namechange=true;
      }else{
        $data["err_hostinvalid"]=true;
        $data["success"]=false;
      }
    }

    // Check if workgroup changed
    if ($orig_workgroup!=$workgroup) {
      //set_workgroup($workgroup);
      $namechange=true;
    }


    if($data["success"]){
      if($namechange){
        restart_samba();
      }

      if ($restart_network) {
        restart_network("eth0");
      }
    }

    $data["namechange"]=$namechange;
    $data["networkrestart"]=$restart_network;

    if($strip){
      $this->load->view(THEME.'/settings/settings_netupdate_view',$data);
    }else{
      $this->_renderfull($this->load->view(THEME.'/settings/settings_netupdate_view',$data,true));
    }
  }

  function backup($strip = false){

    $this->load->model('disk_model');

    $part=$this->input->post("unit");
    if(!$part) {
      redirect('/settings/backuprestore');
    }
    if($this->disk_model->is_mounted($part)){
      $info = $this->disk_model->partition_info(substr($part,5));
      $path = $info['mountpath'];
      $allready_mounted = true;
    }else{
      $path = $this->disk_model->mount_partition( $part );
    }
    if(!isset($path) || !$path) {
      $data['update'] = array(
        'success' => false,
        'message' => _("Failed to set up mount point for backup"),
      );
    } else {
      set_time_limit(5*60);
      if(!backup_config($path)) {
        $data['update'] = array(
          'success' => true,
          'message' => _("System settings backup was sucessfully created"),
        );
      } else {
        $data['update'] = array(
          'success' => false,
          'message' => _("The system was unable to create a backup"),
        );
      }
      if(!isset($allready_mounted) || !$allready_mounted) {
        // only unmount if not mounted to start with.
        $this->disk_model->umount_partition( $part );
      }
    }
    $this->session->set_flashdata('update', $data['update']);
    redirect('/settings/backuprestore');
  }

  function restore($strip=""){

    $this->load->model('disk_model');

    $data["success"]=false;
    $data["err_nomedia"]=false;
    $data["err_opfailed"]=false;

    $part=$this->input->post("unit");
    if(!$part) {
      redirect('/settings/backuprestore');
    }

    if($this->disk_model->is_mounted($part)){
      $info = $this->disk_model->partition_info(substr($part,5));
      $path = $info['mountpath'];
      $mounted = true;
    }else{
      $path = $this->disk_model->mount_partition( $part );
    }
    if(!isset($path) || !$path) {
      $data['update'] = array(
        'success' => false,
        'message' => _("Failed to set up mount point for restore"),
      );
    } else {
      set_time_limit(5*60);
      if(!restore_config($path)) {
        $data['update'] = array(
          'success' => true,
          'message' => _("System settings was sucessfully restored"),
        );
      } else {
        $data['update'] = array(
          'success' => false,
          'message' => _("The system was unable to restore the system from an backup"),
        );
      }

    }

    if(!isset($mounted)) {
      $this->disk_model->umount_partition($part);
    }
    $this->session->set_flashdata('update', $data['update']);
    redirect('/settings/backuprestore');
  }

  /* XXX This function can't utilize a format parameter due to clients might use current setting still during update */
  function fwupdate(){
    $this->load->helper('bubba_socket');
    $output = '';
    switch( $this->input->post( 'action' ) ) {
    case 'hotfix_run':
      $enabled = $this->input->post('hotfix_enabled') == 'true';
      update_bubbacfg( 'admin', 'hotfix', $enabled );
      if( $enabled ) {
        if( file_exists('/tmp/bubba-apt.pid') ) {
          $pid = intval(file_get_contents( '/tmp/bubba-apt.pid' ));
          $running=posix_kill($pid, 0);
          if(posix_get_last_error()==1) /* EPERM */
            $running=true;
          if( $running ) {
            $this->output->set_output( json_encode( array( 'done' => 1, 'stop' => 0, 'statusMessage' => 'Upgrade is running' ) ) );
            return;
          }
        }
      } else {
        $this->output->set_output( json_encode( array( 'done' => 1, 'stop' => 0, 'statusMessage' => 'Hotfix check disabled per request' ) ) );
        return;
      }
      old_hotfix_run();
      $output = old_hotfix_query_progress();
      break;
    case 'hotfix_progress':
      $output = old_hotfix_query_progress();
      break;
    case 'upgrade':
      old_apt_upgrade_packages();
      $output = old_apt_query_progress();
      break;
    case 'install':
      old_apt_install_package( $this->input->post( 'package' ) );
      $output = old_apt_query_progress();
      break;
    case 'progress':
      $output = old_apt_query_progress();
      break;
    case 'get_versions':
      $versions = get_package_version(array("bubbagen","bubba3-kernel","bubba-admin","singapore","filetransferdaemon","lyrionmusicserver"));
      $this->session->set_userdata("version",$versions['bubbagen']);
      $output = str_replace("null","not installed",json_encode($versions));
      break;
    }
    syslog( LOG_DEBUG, $output );
    $this->output->set_output($output);
  }

  function settraffic($strip=""){

    $ftd_enabled=query_service("filetransferdaemon");
    if($ftd_enabled){

      require_once(IPCFUNCS);

      $btdl_throttle=(int)$this->input->post('torrent_download');
      $btul_throttle=(int)$this->input->post('torrent_upload');

      if($btdl_throttle<-1){
        $btdl_throttle=-1;
      }

      if($btul_throttle<-1){
        $btul_throttle=-1;
      }

      $data['update'] = array(
        'success' => true,
        'message' => 'settings_traffic_success'
      );
      $dl=new Downloader;
      if(!$dl->set_download_throttle("torrent",$btdl_throttle==-1?-1:$btdl_throttle*1024)){
        /* Failed */
        $data['update'] = array(
          'success' => false,
          'message' => 'settings_traffic_error_set_dl_throttle'
        );
      }
      if(!$dl->set_upload_throttle("torrent",$btul_throttle==-1?-1:$btul_throttle*1024)){
        /* Failed */
        $data['update'] = array(
          'success' => false,
          'message' => 'settings_traffic_error_set_ul_throttle'
        );
      }


    }else{
      //Service not running
      $data['update'] = array(
        'success' => false,
        'message' => 'settings_traffic_error_service_unavailable'
      );
    }
    $data["btul_throttle"]=$btul_throttle;
    $data["btdl_throttle"]=$btdl_throttle;
    $data["ftd_enabled"]=$ftd_enabled;

    if($strip){
      $this->load->view(THEME.'/settings/settings_traffic_view',$data);
    }else{
      $this->_renderfull($this->load->view(THEME.'/settings/settings_traffic_view',$data,true));
    }
  }

  function setdate($strip=""){
    // $strip = -1 will not load any view, instead return "$data"
    $uri = THEME.'/settings/settings_setdate_view';

    $error = false;
    // -- Get input data
    if($strip == "-1") {
      $wiz_data = $this->input->post('wiz_data');

      $user_tz = $wiz_data['user_tz']; // always exists, but might be ""
      $ntp = isset($wiz_data['use_ntp'])?true:false;
      $date = isset($wiz_data['date'])?$wiz_data['date']:"";
      $time = isset($wiz_data['time'])?$wiz_data['time']:"";
    } else {
      $ntp = $this->input->post("use_ntp");
      $date=$this->input->post("date");
      $time=$this->input->post("time");
      $user_tz = $this->input->post("user_tz");
    }


    //  -- timezone --
    if($user_tz) {
      $ret = set_timezone($user_tz);
      if($ret != 0) {
        $error = true;
        $data['update'] = array(
          'success' => false,
          'message' => sprintf(_("Failed to set timezone <strong>%s</strong>"), $user_tz),
        );
      }
    }

    //  -- date and time --
    if(!$error && !$ntp) {
      if(service_running("ntpd")) {
        // turn off ntp
        stop_service("ntpd");
        remove_service("ntpd");
      }

      // -- Set date and time
      $res = set_time($time,$date,$ntp);
      if($res <> 0) {
        $error = true;
        $data['update'] = array(
          'success' => false,
          'message' => sprintf(_("Failed to set date <strong>%s</strong> and time <strong>%s</strong>"), $date, $time),
        );
        $data['date'] = $date;
        $data['time'] = $time;
        $data['t_zone'] = $this->system->get_timezone();
        if($ntp) $data['use_ntp'] = true;
      }
    } else { // use ntp
      if(!service_running("ntpd")) {
        // turn on ntp
        start_service("ntpd");
        add_service("ntpd");
      }
    }

    usleep(1000000); # 1s
    if(query_service("dovecot") && !service_running("dovecot")) {
      // restart dovecot
      d_print_r("Restart dovecot\n");
      start_service("dovecot");
    }
    if(!$error) {
      $data['update'] = array(
        'success' => true,
        'message' => _("Timezone, date and/or time was successfully updated"),
      );
    }

    if($strip == -1) {
      return $data;
    }else{
      $this->datetime( $strip, $data );
    }
  }
  function set_lang($strip=""){
    $lang = $this->input->post("lang");
    if($lang) {
      $languages = $this->gettext->get_languages();
      if(isset($languages[$lang])) {
        $locale = $languages[$lang]['locale'];
        update_bubbacfg("admin","default_lang",$lang);
        update_bubbacfg("admin","default_locale",$locale);
        $conf = parse_ini_file(ADMINCONFIG);
        if(! (isset($conf['language']) && $conf['language'])) {
          $this->session->set_userdata('language',$lang);
          $this->session->set_userdata('locale',$locale);
          redirect('settings/datetime');
        }
        $data['update'] = array(
          'success' => true,
          'message' => _("Default system language updated"),
        );
      }
    } else {
      // no update
      $data = array();
    }
    $this->datetime("", $data );
  }

  function trafficsettings($strip=""){
    $trafdata["ftd_enabled"]=query_service("filetransferdaemon");
    if($trafdata["ftd_enabled"]){
      require_once(IPCFUNCS);
      $dl=new Downloader;

      $trafdata["btul_throttle"]=$dl->get_upload_throttle("torrent");
      if($trafdata["btul_throttle"]>-1){
        $trafdata["btul_throttle"]=$trafdata["btul_throttle"]/1024;
      }
      $trafdata["btdl_throttle"]=$dl->get_download_throttle("torrent");
      if($trafdata["btdl_throttle"]>-1){
        $trafdata["btdl_throttle"]=$trafdata["btdl_throttle"]/1024;
      }
    }else{
      $trafdata["btul_throttle"]=_('n/a');
      $trafdata["btdl_throttle"]=_('n/a');
    }

    if($strip){
      $this->load->view(THEME.'/settings/settings_traffic_view',$trafdata);
    }else{
      $this->_renderfull($this->load->view(THEME.'/settings/settings_traffic_view',$trafdata,true));
    }
  }

  function datetime($strip="", $data = array()){

    $data = $this->_datetime($data);

    $data["available_languages"] = $this->gettext->get_languages();

    if($strip){
      $this->load->view(THEME.'/settings/settings_datetime_view');
    }else{
      $this->_renderfull(
        $this->load->view(THEME.'/settings/settings_datetime_view',$data,true),
        $this->load->view(THEME.'/settings/settings_datetime_head_view',$data,true)
      );
    }
  }

  function backuprestore($strip="",$data=array()){
    $update =  $this->session->flashdata('update');
    if( $update ) {
      $data['update'] = $update;
    }
    $this->load->model('disk_model');
    $data['disks'] = array();
    $disks=$this->disk_model->list_external_disks(false,true,false,true); // allow removable, not RAID, allow USB, list partitions
    if(sizeof($disks)) {
      foreach($disks as $disk) {
        foreach($disk as $part) {
          $data["disks"][] = $part;
        }
      }
      sort($data["disks"]);
    }
    if($strip){
      $this->load->view(THEME.'/settings/settings_backuprestore_view',$data);
    }else{
      $this->_renderfull(
        $this->load->view(THEME.'/settings/settings_backuprestore_view',$data,true),
        $this->load->view(THEME.'/settings/settings_backuprestore_head_view',$data,true)
      );
    }
  }

  function software($action = 'upgrade', $package = null ){
    $data['action'] = $action;
    $data['package'] = $package;
    $data['version'] = file_get_contents(BUBBA_VERSION);

    $this->_renderfull(
      $this->load->view(THEME.'/settings/settings_software_view',$data,true),
      $this->load->view(THEME.'/settings/settings_software_head_view',$data,true)
    );
  }

  function logs($strip=""){
    $base_path = '/var/log/';
    # mapping of log file name and path
	$logs = array("messages" => "messages");

	foreach (glob($base_path."*",GLOB_MARK) as $node) {
		if (preg_match("(/$|gz$|tallylog|wtmp|messages|lastlog)",$node)) continue;
		$nodename = substr($node,9);
		$logs[$nodename] = $nodename;
	}

	foreach (glob($base_path."*",GLOB_ONLYDIR) as $dir) {
		$basename = substr($dir,9);
		switch ($basename) {
			case "sandbox":
				continue 2;
				break;
			case "lyrionmusicserver":
				$dirname = "Lyrion Music Server";
				break;
			case "apache2":
				$dirname = "Apache webserver";
				break;
			case "nginx":
				$dirname = "Nginx webserver";
				break;
			case "cups":
				$dirname = "Cups printserver";
				break;
			case "samba":
				$dirname = "Samba fileserver";
				break;
			default:
				$dirname = $basename;
				break;
		}
		$logs[$dirname] = Array();
		$path_len = strlen($dir."/");
		$numlogfiles = 0;
		foreach (glob($dir."/*",GLOB_MARK) as $node) {
			if (preg_match("(/$|gz$)",$node)) continue;
			$nodename = substr($node,$path_len);
			$logs[$dirname][$nodename] = $basename."/".$nodename;
			$numlogfiles++;
		}
		if (!$numlogfiles) unset($logs[$dirname]);
	}

    function array_values_recursive($array) {
      $flat = array();

      foreach ($array as $value) {
        if (is_array($value)){
          $flat = array_merge($flat, array_values_recursive($value));
        }else{
          $flat[] = $value;
        }
      }
      return $flat;
    }

    if($this->input->post('log') ){
      $current_log = $this->input->post('log');
      if( in_array( $current_log, array_values_recursive($logs) )  && file_exists($base_path.$current_log) ) {
        $data['log_name'] = $current_log;
        $code = file_get_contents("/var/log/$current_log");
        $data['content'] = explode("\n", $code);
      }
    }
    $data['logs']=$logs;
    if($strip){
      $this->load->view(THEME.'/settings/settings_logs_view',$data);
    }else{
      $this->_renderfull($this->load->view(THEME.'/settings/settings_logs_view',$data,true));
    }
  }


  function index($strip=""){
    $this->_renderfull(
      $this->load->view(THEME.'/settings/settings_view','',true),
      $this->load->view(THEME.'/settings/settings_head_view','',true)
    );
  }

  function startwizard() {
    $this->index();
  }

  function identity( $strip = "" ) {
    $this->load->model('networkmanager');

    $update = false;
    $data['workgroup'] = $current_workgroup = get_workgroup();
    $data['hostname'] = $current_hostname = php_uname('n');

    try {

      if( $this->input->post("samba_update") ) {
        $update = true;
        # we update samba

        # Get input hostname and workgroup
        $hostname=$this->input->post("hostname");
        $workgroup=$this->input->post('workgroup');

        $restart_services = false;

        if( $hostname != $current_hostname ) {
          # Hostname is updated
          if(preg_match("/^[A-Za-z0-9-]+$/",$hostname)){
            # Valid hostname given
            if($this->networkmanager->set_hostname($hostname)){
              # we failed to update hostname
              throw new Exception(_("Failed to change hostname"));
            }
            $restart_services=true;
            $data['hostname'] = $hostname;
          }else{
            # invalid hostname
            throw new Exception(sprintf(_("Hostname <strong>%s</strong> is invalid, only character <strong>A-Za-z0-9-</strong> is valid"),$hostname));
          }
        }

        if( $workgroup != $current_workgroup ) {
          // TODO : Add errorchecking
          set_workgroup($workgroup);
          $restart_services=true;
          $data['workgroup'] = $workgroup;

        }

        if ($restart_services){
          if(query_service("samba")){
            restart_samba();
          }
        } else {
          # We just clicked on the update button
          $update = false;
        }

      }
      $update_msg = array(
        'success' => true,
        'message' => _('Update successful')
      );

    } catch( Exception $e ) {
      $update_msg = array(
        'success' => false,
        'message' => $e->getMessage()
      );
    }

    if( $update ) {
      $data["update"] = $update_msg;
    }

    if($strip){
      $this->load->view(THEME.'/settings/settings_identity_view',$data);
    }else{
      $this->_renderfull(
        $this->load->view(THEME.'/settings/settings_identity_view',$data,true),
        null,
        $data
      );
    }

  }

  public function remote_accounts() {
    $data = array();
    $this->_renderfull(
      $this->load->view(THEME.'/settings/settings_remote_accounts_view',$data,true),
      $this->load->view(THEME.'/settings/settings_remote_accounts_head_view',$data,true),
      $data
    );

  }

}

