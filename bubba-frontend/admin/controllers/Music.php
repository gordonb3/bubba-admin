<?php

require_once "Mycontroller.php";

class Music extends My_CI_Controller{

	function __construct(){
		parent::__construct();

		require_once(APPPATH."/legacy/defines.php");
		require_once(ADMINFUNCS);

		$this->Auth_model->EnforceAuth('web_admin');

	}

	function Music(){
		self::__construct();
	}

	function index($strip=""){
		if($strip == "") {
			$host = explode(":",$_SERVER["HTTP_HOST"]);
			$data["host"] = $host[0];
			if(sizeof($host) > 1) {
				$data["port"] = $host[1];
			} else {
				$data["port"] = "";
			}
			$this->load->view(THEME.'/music/music_view',$data);
		} else {
			echo "No data available";
		}
	}
}

?>