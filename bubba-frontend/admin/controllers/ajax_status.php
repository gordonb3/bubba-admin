<?php
class Ajax_status extends Controller {

    var $json_data=Array(
        'error' => 1,
        'html' => 'Ajax Error: Invalid Request'
    );

    function __construct() {
        parent::Controller();
        require_once(APPPATH."/legacy/defines.php");
        require_once(ADMINFUNCS);
        $this->Auth_model->EnforceAuth('web_admin');

		$this->output->set_header('Last-Modified: '.gmdate('D, d M Y H:i:s', time()).' GMT');
		$this->output->set_header('Expires: '.gmdate('D, d M Y H:i:s', time()).' GMT');
		$this->output->set_header("Cache-Control: no-store, no-cache, must-revalidate, max-age=0, post-check=0, pre-check=0");
		$this->output->set_header("Pragma: no-cache");
	}

    function printers() {
        $this->load->model('system');
        $this->json_data = $this->system->list_printers();
    }

    function disks() {
        $this->load->model('disk_model');
        $disks =  $this->disk_model->list_disks();
        foreach( $disks as &$disk ) {
            $disk['hdtemp'] = $this->disk_model->get_hddtemp($disk['dev']);
            foreach( $disk['partitions'] as &$partition ) {
                if(isset($partition['mountpath']) && $partition['mountpath'] != "") {
                    $partition['free_space'] = disk_free_space($partition['mountpath']);
                }
            }
            unset($partition);
        }

        unset($disk);

        $this->json_data = $disks;
    }

	function free_space() {
        $this->load->model('disk_model');
		$fstab = $this->disk_model->list_fstab();
		$data = array();
		foreach($fstab as $entry) {
			if(
				preg_match("#^/(home.*)?$#", $entry['mount']) &&
				$this->disk_model->is_mounted($entry['mount'])
			) {
				$data[$entry['mount']] = disk_free_space($entry['mount']);
			}
		}
		$this->json_data = $data;
	}

	function total_space() {
        $this->load->model('disk_model');
		$fstab = $this->disk_model->list_fstab();
		$data = array();
		foreach($fstab as $entry) {
			if(
				preg_match("#^/(home.*)?$#", $entry['mount']) &&
				$this->disk_model->is_mounted($entry['mount'])
			) {
				$data[$entry['mount']] = disk_total_space($entry['mount']);
			}
		}
		$this->json_data = $data;
	}

    function vgs() {
        $this->load->model('disk_model');
        $this->json_data = $this->disk_model->list_vgs();
    }

    function mds() {
        $this->load->model('disk_model');
        $this->json_data = $this->disk_model->list_mds();
    }

    function fstab() {
        $this->load->model('disk_model');
        $this->json_data = $this->disk_model->list_fstab();
    }

    function uptime($raw='') {
        $this->load->model('system');
        if($raw == 'raw') {
            $this->json_data = array(
                'uptime' => $this->system->get_raw_uptime()
            );
        } else {
            $this->json_data = array(
                'uptime' => $this->system->get_uptime()->format(_("%a days, %H:%I:%S")),
            );
        }
    }

    function version() {
        $this->load->model('system');
        $this->json_data = array(
            'version' => $this->system->get_system_version()
        );
    }

    function hardware() {
        $this->load->model('system');
        /*
         * hwtype is 10 for B1, 20 for B2, and 30 for B3 and so on.
         */
        $this->json_data = array(
            'hwtype' => $this->system->get_hardware_id()
        );
    }

    function _output($output) {
        echo json_encode($this->json_data);
    }

}
