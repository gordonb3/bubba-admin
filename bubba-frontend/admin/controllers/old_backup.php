<?php

class Old_Backup extends CI_Controller {
    public function __construct() {
        parent::__construct();
        require_once(APPPATH."/legacy/defines.php");
        require_once(ADMINFUNCS);

        $this->Auth_model->EnforceAuth('web_admin');


    }
    function _renderfull($content, $head=true){
        if(!is_null($head)) {
            if( $head === true ) {
                $mdata["head"] = $this->load->view(THEME.'/old_backup/backup_head_view','',true);
            } else {
                $mdata['head'] = $head;
            }
        }
        $navdata["menu"] = $this->menu->retrieve($this->session->userdata('user'),$this->uri->uri_string());
        $mdata["navbar"]=$this->load->view(THEME.'/nav_view',$navdata,true);
        $mdata["dialog_menu"] = $this->load->view(THEME.'/menu_view',$this->menu->get_dialog_menu(),true);
        $mdata["content"]=$content;
        $this->load->view(THEME.'/main_view',$mdata);
    }
    public function index($strip="") {
        if( $strip == 'json' ) {
        }
        $data = array();
        if($strip){
            $this->load->view(THEME.'/old_backup/backup_view',$data);
        }else{
            $this->_renderfull($this->load->view(THEME.'/old_backup/backup_view',$data,true));
        }
    }
}
