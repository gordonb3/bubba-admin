<?php

class Album extends Controller {
	function __construct() {
		parent::Controller();
		require_once(APPPATH."/legacy/defines.php");
		require_once(ADMINFUNCS);

		$this->Auth_model->EnforceAuth();
		$this->Auth_model->DenyUser('admin');
		load_lang("bubba",THEME.'/i18n/'.LANGUAGE);
	}

	function _output($content){
		$navdata["menu"] = $this->menu->retrieve($this->session->userdata('user'),$this->uri->uri_string());
		$navdata["show_level1"] = $this->Auth_model->policy("menu","show_level1");
		$mdata["navbar"]=$this->load->view(THEME.'/nav_view',$navdata,true);
		$mdata["dialog_menu"] = $this->load->view(THEME.'/menu_view','',true);
		$mdata["head"] = $this->load->view(THEME.'/album/album_head_view','',true);
		$mdata["content"]=$content;
		echo $this->load->view(THEME.'/main_view',$mdata, true);
	}

	function index() {
		redirect( 'album/albums' );
	}

	function albums() {
		$db = $this->load->database('album', true);
		$data['free_images'] = $db->select('id, name')->from('image')->where( array( 'album' => null ) )->get()->result_array();
		$data['albums'] = $db->select('id, name, parent')->from('album')->get()->result_array();
		$data['images'] = array();

		$imgs = $db->select('id, name, album,width,height')->from('image')->get()->result_array();
		foreach( $imgs as $img ) {
			$data['images'][$img['album']][] = $img ;
		}

		$this->load->view(THEME.'/album/album_album_view',$data);
	}

	function users() {
		$data = array();
		$this->load->view(THEME.'/album/album_users_view',$data);
	}
}
