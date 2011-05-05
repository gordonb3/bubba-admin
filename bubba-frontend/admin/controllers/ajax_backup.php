<?php
class Ajax_backup extends Controller {

    var $json_data=Array(
        'error' => 1,
        'html' => 'Ajax Error: Invalid Request'
    );

    function __construct() {
        parent::Controller();
        $this->load->model("backup");
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
        $data = array();
        foreach( $this->backup->get_jobs() as $job ) {
            try {
                $settings = $this->backup->get_settings($job);

            } catch( NoSettingsException $e ) {
                # as we might have bad data, ignore the job for now
                continue;
            }
            try {
                $schedule = $this->backup->get_schedule($job);
            } catch( NoScheduleException $e ) {
                $schedule = array(
                    "type" => "disabled",
                );
            }
            $status = $this->backup->get_status($job);
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
            case "disabled":
                $date = t("Never");
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
                "status" => "N/A"
            );

            if( $status["running"] ) {
                $cur["running"] = true;
                $cur["status"] = t("Running");
            } else {
                if( $status["error"] ) {
                    $cur["status"] = t("Failed");
                    $cur["failed"] = true;
                } elseif($status["done"]) {
                    $cur["status"] = t("OK");
                } else {
                    $cur["status"] = t("Not run");
                }
            }
            unset($status);
            unset($schedule);
            unset($settings);
            $data[] = $cur;
        }
        $this->json_data = $data;
    }

    function get_backup_job_information() {
        $name = $this->input->post("name");
        $this->json_data = $this->backup->list_backups($name);
    }

    function dirs() {
        function formatBytes($bytes, $precision = 2) {
            $units = array('B', 'KB', 'MB', 'GB', 'TB');

            $bytes = max($bytes, 0);
            $pow = floor(($bytes ? log($bytes) : 0) / log(1024));
            $pow = min($pow, count($units) - 1);

            $bytes /= (1 << (10 * $pow));

            return round($bytes, $precision) . ' ' . $units[$pow];
        }
	    $subpath = $this->input->post('path');
        $modified_subpath = preg_replace("#(^|\/)\.\.?(\/|$)#", '/', $subpath);
		$path = "/home/$modified_subpath";

        $data = array(
            'meta' => array(),
            'root' => $modified_subpath,
            'aaData'  => array()
        );
        if (file_exists($path) && is_dir($path) && is_readable($path)) {
            if ($dh = opendir($path)) {
                while (($file = readdir($dh)) !== false) {
                    if( $file == '.'  || $file == '..' || !is_dir($path) ) {
                        continue;
                    }
                    $filename = $path . '/' . $file;
                    $data['aaData'][] = array(
                        filetype($filename),
                        $file,
                        date ("o-m-d H:i:s", filemtime($filename)),
                        formatBytes(filesize($filename))
                    );
                }
                closedir($dh);
            }
        } else {
            $data["meta"]["permission_denied"]=true;
        }
        $this->json_data = $data;

    }

    function get_available_devices() {

        $this->load->model("Disk_model");

        $disks = $this->Disk_model->list_disks();

        $usable_disks = array();

        foreach($disks as $disk) {
            if(preg_match("#/dev/sda#",$disk["dev"])) {
                continue;
            }
            if(isset($disk["partitions"]) && is_array($disk["partitions"])) {
                foreach($disk["partitions"] as $partition) {
                    if( !strcmp($partition["usage"],"mounted") || !strcmp($partition["usage"],"unused") && $partition["uuid"]) {
                        if($partition["label"]) {
                            $diskdata["label"] = $partition["label"];
                        } else {
                            if(preg_match("#dev/\w+(\d+)#",$disk["dev"],$partition_number)) {
                                $diskdata["label"] = "$disk[model]:$partition_number[1]";
                            } else {
                                $diskdata["label"] = "$disk[model]:1";
                            }
                        }
                        $diskdata["uuid"] = $partition["uuid"];
                        $usable_disks[$disk["model"]][]=$diskdata;
                    } else {

                    }
                }
            } else {
                if( !strcmp($disk["usage"],"mounted") || !strcmp($disk["usage"],"unused") && $disk["uuid"]) {
                    if($disk["label"]) {
                        $diskdata["label"] = $disk["label"];
                    } else {
                        if(preg_match("#dev/\w+(\d+)#",$disk["dev"],$partition_number)) {
                            $diskdata["label"] = "$disk[model]:$partition_number[1]";
                        } else {
                            $diskdata["label"] = "$disk[model]:1";
                        }
                    }
                    $diskdata["uuid"] = $disk["uuid"];
                    $usable_disks[$disk["model"]][]=$diskdata;
                }
            }
        }

        $this->json_data = array( "disks" => $usable_disks );
    }
//http://b3-carl/admin/ajax_backup/create?name=test2&protocol=ftp&schedule-dayhour=1&schedule-monthday=1&schedule-monthhour=1&schedule-timeline=1W&schedule-type=weekly&schedule-weekday=Monday&schedule-weekhour=1&security=yes&security-password=eee&security-password2=eee&selection=email&target-hostname=localhost&target-password=test&target-path=tee&target-username=test
    function create() {
        $name = $this->input->post("name");
        $selection = $this->input->post("selection");
        $protocol = $this->input->post("protocol");
        $schedule_type = $this->input->post("schedule-type");
        $security = $this->input->post("security");

        $target_hostname = $this->input->post("target-hostname");
        $target_username = $this->input->post("target-username");
        $target_password = $this->input->post("target-password");
        $target_device = $this->input->post("target-device");

        $schedule_dayhour = $this->input->post("schedule-dayhour");
        $schedule_weekhour = $this->input->post("schedule-weekhour");
        $schedule_monthhour = $this->input->post("schedule-monthhour");
        $schedule_weekday = $this->input->post("schedule-weekday");
        $schedule_monthday = $this->input->post("schedule-monthday");
        $schedule_timeline = $this->input->post("schedule-timeline");

        $security_password = $this->input->post("security-password");
        $security_password2 = $this->input->post("security-password2");

        $settings = array(
            'local_user' => 'admin'
        );

        /* Basic checks that all data is present and ok, content validation are client side */

        if( !$name || !$selection || !$protocol || !$schedule_type ) {
            throw new Exception("jobname, selection, protocol, or schedule not defined");
        }

        if( !in_array($protocol, array('ftp', 'ssh', 'file')) ) {
            throw new Exception("not valid protocol");
        }

        if( $protocol == 'ftp' || $protocol == 'ssh' ) {
            if( !$target_hostname || !$target_username || !$target_password ) {
                throw new Exception("ftp or ssh without host and/or username/password combo");
            }
        }

        if( $protocol == 'file' && !$target_device ) {
            throw new Exception("file protocol without target disk");
        }

        if( !in_array($selection, array('data', 'email', 'music', 'photo', 'video', 'storage', 'custom')) ) {
            throw new Exception("invalid schedule type");
        }

        if( $schedule_type == 'monthly' && (!$schedule_monthhour || ! $schedule_monthday) ) {
            throw new Exception("monthly schedule without day or hour");
        }

        if( $schedule_type == 'weekly' && (!$schedule_weekhour || ! $schedule_weekday) ) {
            throw new Exception("weekly schedule without day or hour");
        }

        if( $schedule_type == 'daily' && !$schedule_dayhour ) {
            throw new Exception("daily schedule without hour");
        }

        if( !$schedule_timeline ) {
            throw new Exception("missing timeline");
        }

        if( $security && (!$security_password || $security_password != $security_password2) ) {
            throw new Exception("choosen security setting without specifying password, or password missmatch");
        }

        /* Backup job name */
        $this->backup->create_job($name);

        $settins['jobname'] = $name;

        /* Backup file selection */
        $include = array();
        $exclude = array();
        switch( $selection ) {
        case 'data':
            $include[] = '/home/*';
            $exclude[] = '/home/admin';
            $exclude[] = '/home/storage';
            $exclude[] = '/home/web';
            break;
        case 'email':
            $include[] = '/home/*/Mail';
            break;
        case 'music':
            $include[] = '/home/storage/music';
            break;
        case 'photo':
            $include[] = '/home/storage/photo';
            break;
        case 'video':
            $include[] = '/home/storage/video';
            break;
        case 'storage':
            $include[] = '/home/storage';
            break;
        case 'custom':
            $include = $this->input->post('dirs');
            break;
        default:
            $this->err("Wrong selection type: $selection");
            return;
        }

        $settings['include'] = $include;
        $settings['exclude'] = $exclude;
        $settings['selection_type'] = $selection;
        $this->backup->set_backup_files($name, $include, $exclude);

        /* Backup protocol */

        $settings['target_protocol'] = $protocol;
        if( $protocol == 'file' ) {
            $settings['disk_uuid'] = $target_device;
        } else {
            $settings['target_host'] = $target_hostname;
            $settings['target_user'] = $target_username;
            $settings['target_FTPpasswd'] = $target_password;
        }

        /* Backup schedule */

        $schedule['schedule_type'] = $schedule_type;
        $this->backup->set_schedule(
            $name,
            $schedule_type,
            $schedule_monthday,
            $schedule_monthhour,
            $schedule_weekday,
            $schedule_weekhour,
            $schedule_dayhour
        );


        if( $security ) {
            $settings['GPG_key'] = $security_password;
        }

    }

    function err($what) {
        $this->json_data['html'] = $what;
    }
    function _output($output) {
        echo json_encode($this->json_data);
    }

}
