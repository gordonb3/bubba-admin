<?php

class Logout extends CI_Controller{

	function Logout(){
		parent::__construct();
	}
	
	function index(){
		$this->Auth_model->Logout();
		$this->session->unset_userdata("caller");
		redirect("login");
	}

}
?>
