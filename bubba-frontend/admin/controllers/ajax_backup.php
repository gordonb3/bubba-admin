<?php
class Ajax_backup extends Controller {

    var $json_data=Array(
        'error' => 1,
        'html' => 'Ajax Error: Invalid Request'
    );

    function __construct() {
        parent::Controller();
        require_once(APPPATH."/legacy/defines.php");
        require_once(ADMINFUNCS);

        $this->Auth_model->EnforceAuth('web_admin');
        $this->Auth_model->enforce_policy('web_admin','administer', 'admin');
        load_lang("bubba",THEME.'/i18n/'.LANGUAGE);

        $this->output->set_header('Last-Modified: '.gmdate('D, d M Y H:i:s', time()).' GMT');
        $this->output->set_header('Expires: '.gmdate('D, d M Y H:i:s', time()).' GMT');
        $this->output->set_header("Cache-Control: no-store, no-cache, must-revalidate, max-age=0, post-check=0, pre-check=0");
        $this->output->set_header("Pragma: no-cache");
    }
    function get_backup_jobs() {
        $this->load->model("backup");
        $data = array();
        foreach( $this->backup->get_jobs() as $job ) {
            try {
                $settings = $this->backup->get_settings($job);
                $schedule = $this->backup->get_schedule($job);
                $status = $this->backup->get_status($job);

            } catch( NoSettingsException $e ) {
                # as we might have bad data, ignore the job for now
                continue;
            } catch( NoScheduleException $e ) {
                $schedule = array(
                    "type" => "disabled",
                );
            }
            $date = "";
            switch($schedule["type"]) {
            case "hourly":
                $date = t("Hourly");
                break;
            case "daily":
                $date = t("Each day");
                break;
            case "weekly":
                $date = t("Once a week");
                break;
            case "monthly":
                $date = t("Every month");
                break;
            default:
                $date = t("Once in a while");
            }

            $target = $settings["target_protocol"];

            switch($target) {
            case "file":
                $target = "USB";
                break;
            case "FTP":
            case "SSH":
                break;
            default:
                $target = "???";
            }
            $cur = array(
                "name" => $job,
                "target" => $target,
                "schedule" => $date,
                "status" => "N/A",
                #"a" => $settings,
                #"b" => $schedule
            );

            if( $status["running"] ) {
                $cur["running"] = true;
                $cur["status"] = t("Running");
            }
            $data[] = $cur;
        }
        $this->json_data = $data;
        /*$this->json_data =
        array(
        array(
        "name" => "My Music",
        "target" => "FTP",
        "schedule" => "Daily",
        "status" => "OK"
        ),
        array(
        "name" => "My email",
        "target" => "USB",
        "schedule" => "Hourly",
        "status" => "Failed",
        "failed" => true
        ),
        array(
        "name" => "Testing",
        "target" => "USB",
        "schedule" => "Every monday",
        "status" => "Not run"
        ),
        array(
        "name" => "Carl's stuff",
        "target" => "SSH",
        "schedule" => "Every century",
        "status" => "Running",
        "running" => true,
        'jobs' => $this->backup->get_jobs(),
        'settings' => $this->backup->get_settings('aaa'),
        'schedule' => $this->backup->get_schedule('aaa')
        ),
        );*/
    }

    function get_backup_job_information() {
        $name = $this->input->post("name");
        $this->json_data = array(
            array(
                "date" => "Mon, 18 Apr 2011 12:16:42 +0200",
                "type" => "Full",
                "status" => "OK"
            ),
            array(
                "date" => "Sun, 17 Apr 2011 12:13:32 +0200",
                "type" => "Incremental",
                "status" => "OK"
            ),
            array(
                "date" => "Sat, 16 Apr 2011 12:21:13 +0200",
                "type" => "Incremental",
                "status" => "Failed [Why?]",
                "failed" => true  
            ),
            array(
                "date" => "Fri, 15 Apr 2011 12:32:01 +0200",
                "type" => "Full",
                "status" => "OK"
            ),
        );
    }

    function _output($output) {
        echo json_encode($this->json_data);
    }

}
