<?php

class Stat extends CI_Controller{

	function stat(){
		parent::__construct();

		require_once(APPPATH."/legacy/defines.php");
		require_once(ADMINFUNCS);

		$this->Auth_model->enforce_policy('web_admin','administer', 'admin');
		$this->Auth_model->EnforceAuth('web_admin');

	}

	function _renderfull($content){
		$navdata["menu"] = $this->menu->retrieve($this->session->userdata('user'),$this->uri->uri_string());
		$mdata["navbar"]=$this->load->view(THEME.'/nav_view',$navdata,true);
		$mdata["head"] = $this->load->view(THEME.'/stat/stat_head_view',$navdata,true);;
        $mdata["dialog_menu"] = $this->load->view(THEME.'/menu_view',$this->menu->get_dialog_menu(),true);
        $mdata["content"]=$content;
		$this->load->view(THEME.'/main_view',$mdata);
	}	

	function _getvolume($path){
		$res=array();
		$res["size"]=disk_total_space($path);
		$res["free"]=disk_free_space($path);
		return $res;
	}	

	function _getdisk($dev){
		$res=array();
		$res["temp"]=get_hdtemp($dev);
		return $res;
    }

    function _getprinters() {
        $json =  _system('cups-list-printers');
        return json_decode(implode($json),true);
    }

	function info(){

		$sdata["version"] = get_package_version("bubba-frontend");
		$sdata['uptime']=uptime();
		$sdata['partitions']["home"]=$this->_getvolume("/home/");
		$sdata['partitions']["system"]=$this->_getvolume("/");
		$sdata['disks']["sda"]=$this->_getdisk("/dev/sda");
        $sdata['printers'] = $this->_getprinters();

		header("Content-type: application/json");
		print json_encode($sdata);
	}

	function index($strip=""){

		$this->load->model( 'notify' );
		$this->load->model( 'disk_model' );

		if($strip=="json"){
			$this->info();
			return;
		}

		if( file_exists( BUBBA_VERSION ) ) {
			$sdata["version"] = file_get_contents( BUBBA_VERSION );
		} else {
			$sdata["version"] = 'N/A';
		}
		$sdata['uptime']=uptime();

		$freespace=intval(disk_free_space("/home/")/(1048576));
		$totalspace=intval(disk_total_space("/home/")/(1048576));


		$sdata['freespace']=number_format($freespace,0,' ',' ');
		$sdata['totalspace']=number_format($totalspace,0,' ',' ');
		$sdata['percentused']=intval(100*(($totalspace-$freespace)/$totalspace));
		$sdata['notifications'] = $this->notify->list_all();
        $sdata['printers'] = $this->_getprinters();
		$sdata['temperature'] = $this->disk_model->get_hddtemp( '/dev/sda' );

		if($strip){
			$this->load->view(THEME.'/stat/stat_view',$sdata);
		} else {
			if( $this->session->userdata("run_wizard") ) {
                $sdata['run_wizard'] = true;
			}
            $this->_renderfull($this->load->view(THEME.'/stat/stat_view',$sdata,true));
		}
	}
}

?>
