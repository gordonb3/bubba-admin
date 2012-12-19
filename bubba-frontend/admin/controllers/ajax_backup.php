<?php
class Ajax_backup extends Controller {
  const accounts_file = '/etc/bubba/remote_accounts.yml';
  const local_jobs_file = '/etc/bubba/local_backup_jobs.yml';
  const remote_jobs_file = '/etc/bubba/remote_backup_jobs.yml';
  const status_file = '/var/lib/bubba/backup_status.yml';

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

    $this->output->set_header('Last-Modified: '.gmdate('D, d M Y H:i:s', time()).' GMT');
    $this->output->set_header('Expires: '.gmdate('D, d M Y H:i:s', time()).' GMT');
    $this->output->set_header("Cache-Control: no-store, no-cache, must-revalidate, max-age=0, post-check=0, pre-check=0");
    $this->output->set_header("Pragma: no-cache");
  }

  function get_possible_targets() {
    $targets = array();
    if(file_exists(self::accounts_file)) {
      $accounts = spyc_load_file(self::accounts_file);
      foreach($accounts as $id => $account) {
        $targets[] = array(
          'id' => $id,
          'type' => $account['type'],
          'username' => $account['username'],
        );
      }
    }
    $devs = $this->get_available_devices();
    $ret = array(
      'remote' => $targets,
      'local' => $devs
    );
    $this->json_data = $ret;
  }

  function get_backup_jobs() {

    $accounts = spyc_load_file(self::accounts_file);
    $local_jobs = spyc_load_file(self::local_jobs_file);
    $remote_jobs = spyc_load_file(self::remote_jobs_file);
    $status = spyc_load_file(self::status_file);
    $data = array();
    foreach($remote_jobs as $job => $schedules) {
      if($job == '') {
        continue;
      }
      $account = $accounts[$job];
      if(!is_array($schedules)) {
        continue;
      }
      foreach($schedules as $schedule => $selections) {
        if(!is_array($selections)) {
          continue;
        }
        foreach($selections as $selection => $enabled) {
          $cur = array(
            'target' => $job,
            'schedule' => $schedule,
            'selection' => $selection,
            'type' => $account['type'],
            'username' => $account['username'],
          );

          if(isset($status[$job][$schedule][$selection])) {
            $cur['status'] = $status[$job][$schedule][$selection];
          } else {
            $cur['status'] = _('Unknown');
          }
          $cur['label'] = sprintf(_("Backup of %s made %s for %s on %s"), $selection, $schedule, $account['username'], $account['type']);
          $data[] = $cur;
        }
      }
    }

    foreach($local_jobs as $job => $schedules) {
      if(!is_array($schedules)) {
        continue;
      }
      foreach($schedules as $schedule => $selections) {
        if(!is_array($selections)) {
          continue;
        }
        foreach($selections as $selection => $enabled) {
          $cur = array(
            'target' => $job,
            'schedule' => $schedule,
            'selection' => $selection,
            'type' => 'local',
          );

          if(isset($status[$job][$schedule][$selection])) {
            $cur['status'] = $status[$job][$schedule][$selection];
          } else {
            $cur['status'] = _('Unknown');
          }
          $cur['label'] = sprintf(_("Backup of %s made %s to External disk %s"), $selection, $schedule, $job);
          $data[] = $cur;
        }
      }
    }

    $this->json_data = $data;
  }

  public function create() {
    $target = $this->input->post('destination');
    list($where, $target) = explode('-', $target, 2);
    $schedule = $this->input->post('schedule');
    $selection = $this->input->post('selection');
    $jobs = array();
    if( $where == 'remote' ) {
      if(file_exists(self::remote_jobs_file)) {
        $jobs = spyc_load_file(self::remote_jobs_file);
      }

      $jobs[$target][$schedule][$selection] = true;

      $yaml = Spyc::YAMLDump($jobs);
      file_put_contents(self::remote_jobs_file,$yaml);

    } elseif ($where == 'local' ) {
      if(file_exists(self::local_jobs_file)) {
        $jobs = spyc_load_file(self::local_jobs_file);
      }
      $jobs[$target][$schedule][$selection] = true;

      $yaml = Spyc::YAMLDump($jobs);
      file_put_contents(self::local_jobs_file,$yaml);

    }

    # TODO reload backups?
    $this->json_data = true;
  }

  public function remove() {
    $target = $this->input->post('target');
    $type = $this->input->post('type');
    $schedule = $this->input->post('schedule');
    $selection = $this->input->post('selection');
    if($type == 'local') {
      $jobs = spyc_load_file(self::local_jobs_file);
      unset($jobs[$target][$schedule][$selection]);

      file_put_contents(self::local_jobs_file,Spyc::YAMLDump($jobs));
    } else {
      $jobs = spyc_load_file(self::remote_jobs_file);
      unset($jobs[$target][$schedule][$selection]);

      file_put_contents(self::remote_jobs_file,Spyc::YAMLDump($jobs));
    }

    # TODO reload backups?
    $this->json_data = true;
  }


  private function get_available_devices() {

    $this->load->model("Disk_model");

    $disks = $this->Disk_model->list_disks();

    $usable_disks = array();

    foreach($disks as $disk) {
      if(preg_match("#/dev/sda#",$disk["dev"])) {
        continue;
      }
      if(isset($disk["partitions"]) && is_array($disk["partitions"])) {
        foreach($disk["partitions"] as $partition) {
          $diskdata = array();
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
            $usable_disks[]=$diskdata;
          } else {

          }
        }
      } else {
        if( !strcmp($disk["usage"],"mounted") || !strcmp($disk["usage"],"unused") && $disk["uuid"]) {
          $diskdata = array();
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
          $usable_disks[]=$diskdata;
        }
      }
    }

    return $usable_disks;
  }



  function err($what) {
    $this->json_data['html'] = $what;
  }
  function _output($output) {
    echo json_encode($this->json_data);
  }

}
