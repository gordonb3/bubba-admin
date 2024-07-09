<?php

require_once "Mycontroller.php";

class Logout extends My_CI_Controller{

	function __construct(){
		parent::__construct();
	}

	function Logout(){
		self::__construct();
	}

	function index(){
		$this->Auth_model->Logout();
		$this->session->unset_userdata("caller");
		redirect("login");
	}

}
?>
