<?php

class Upload extends CI_Controller{
	
	function __construct(){
		parent::__construct();

		require_once(APPPATH."/legacy/defines.php");
		require_once(ADMINFUNCS);

		$this->Auth_model->EnforceAuth('web_admin');

	}		

	function Upload(){
		self::__construct();
	}

	function index(){
		$data["path"]="/".join("/",array_slice($this->uri->segment_array(),2));
		$this->load->view(THEME.'/upload/upload_index_view',$data);
	}

}

?>
