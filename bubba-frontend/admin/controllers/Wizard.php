<?php
require_once "Mycontroller.php";

class wizard extends My_CI_Controller {

    function __construct() {
        parent::__construct();
        require_once(APPPATH."/legacy/defines.php");
        require_once(ADMINFUNCS);

        $this->Auth_model->EnforceAuth('web_admin');
        $this->Auth_model->enforce_policy('web_admin','administer', 'admin');
    }

    public function get_languages() {

        $languages = $this->gettext->get_languages();
        $official_languages= array();
        $user_languages = array();


        foreach( $languages as $language ) {
            if($language['status'] == 'official') {
                $official_languages[] = $language;
            }
            if($language['status'] == 'user') {
                $user_languages[] = $language;
            }

        }
        $data['official_languages'] = $official_languages;
        $data['user_languages'] = $user_languages;
        $this->load->view(THEME.'/wizard_lang_view', $data);
    }

    public function get_wizard() {
        $this->load->model('system');
        $this->load->model('networkmanager');

        $language = $this->input->post('language');
        $languages = $this->gettext->get_languages();
        if(isset($languages[$language])) {
            $locale = $languages[$language]['locale'];
        } else {
            $locale = "en_US";
        }
        setlocale(LC_MESSAGES, $locale.".UTF8");
        setlocale(LC_TIME, $locale.".UTF8");

        # Timezones

        $data['timezones'] = $this->system->list_timezones();
        $data['current_timezone'] = $this->system->get_timezone();

        $this->load->view(THEME.'/wizard_view', $data);
    }

    public function update() {
        $this->load->model("auth_model");
        $this->load->model("system");
        $this->load->model("networkmanager");
        $language = trim($this->input->post('language'));
        $admin_password1 = trim($this->input->post('admin_password1'));
        $admin_password2 = trim($this->input->post('admin_password2'));
        $password1 = trim($this->input->post('password1'));
        $password2 = trim($this->input->post('password2'));
        $realname = trim($this->input->post('realname'));
        $timezone = $this->input->post('timezone');
        $username = trim($this->input->post('username'));


        $this->mark_dirty();

        $errors = array();

        try {
            if($language) {
                $languages = $this->gettext->get_languages();
                if(isset($languages[$language])) {
                    $locale = $languages[$language]['locale'];
                    update_bubbacfg("admin","default_lang",$language);
                    update_bubbacfg("admin","default_locale",$locale);
                    $conf = parse_ini_file(ADMINCONFIG);
                    if(! (isset($conf['language']) && $conf['language'])) {
                        $this->session->set_userdata('language',$language);
                        $this->session->set_userdata('locale',$locale);
                    }
                } else {
                    throw new Exception("Unavailable language");
                }
            }
        } catch( Exception $e ) {
            $errors[] = $e->getMessage();
        }

        try {
            if($timezone !== $this->system->get_timezone()) {
                $this->system->set_timezone($timezone);
            }
        } catch( Exception $e ) {
            $errors[] = $e->getMessage();
        }

        try {
            if( $admin_password1 && $admin_password1 == $admin_password2 ) {
                _system(BACKEND, "set_unix_password", 'admin', $admin_password1);
            }
        } catch( Exception $e ) {
            $errors[] = $e->getMessage();
        }

        try {
            if( $username && $password1 && $password1 == $password2 ) {
                $shell = '/sbin/nologin';
                $group = 'users'; // Static group for em all

                if (
                    $this->auth_model->user_exists($username)
                    || $username == "root"
                    || $username == "storage"
                    || $username == "web"
                    || $username == ""
                    || strpos($username, ' ') !== false
                    || !preg_match('/^[a-z0-9 _-]+$/',$username)
                    || strlen($username) > 32
                    || $username[0] == '-'
                    || $password1 == ""
                    || $password1 != $password2
                ) {
                    throw new Exception(_('User name validation failed'));
                } else {
                    _system(BACKEND, "add_user", $realname, $group, $shell, $password1, $username);
                }
            }
        } catch( Exception $e ) {
            $errors[] = $e->getMessage();
        }

        if(!empty($errors)) {
            $this->output->set_output(json_encode(array('error' => "true", 'messages' => $errors)));
        } else {
            $this->output->set_output(json_encode(array('error' => "false",)));
        }
    }


    public function username_is_available() {
        $this->load->model("auth_model");
        $username=strtolower(trim($this->input->post('username')));


        header("Content-type: application/json");
        $this->output->set_output(json_encode( !$this->auth_model->user_exists($username) ));

    }

    public function mark_dirty() {
        unset($_SESSION['run_wizard']);
        update_bubbacfg($_SESSION['user'],'run_wizard',"no");
    }
}
