<?php

require_once "Mycontroller.php";

class Filemanager extends My_CI_Controller{

	var $sortarray=false;
	var $lspath=false;

	function _sort_entries($ta, $tb){
		$a=strtolower($this->sortarray[$ta][3]);
		$b=strtolower($this->sortarray[$tb][3]);

		if($a == $b){
			return 0;
		}
		return ($a < $b) ? -1 : 1;
	}

	function __construct(){

		parent::__construct();

		require_once(APPPATH."/legacy/defines.php");
		require_once(ADMINFUNCS);

		$this->Auth_model->EnforceAuth('web_admin');

	}

	function Filemanager(){
		self::__construct();
	}

	function _renderfull($content, $head=true){
		if(!is_null($head)) {
            $mdata['head'] = $head;
		}
		$navdata["show_level1"] = $this->Auth_model->policy("menu","show_level1");
		$navdata["menu"] = $this->menu->retrieve($this->session->userdata('user'),$this->uri->uri_string());
		$mdata["navbar"]=$this->load->view(THEME.'/nav_view',$navdata,true);
		$mdata["dialog_menu"] = $this->load->view(THEME.'/menu_view',$this->menu->get_dialog_menu(),true);
		$mdata["content"]=$content;
		$this->load->view(THEME.'/main_view',$mdata);
	}

	function delete($strip=""){

		if( $strip == 'json' ) {
			$errors = array();
			$files = $this->input->post('files');
			$user=$this->session->userdata("user");
			if(!is_array($files)) {
				$files = array($files);
			}

			foreach($files as $file){
				if(rm($file,$user)){ // true is false
					$errors[] = $file;
				}
			}
			$data["success"]=empty($errors);

			if( !empty($errors) ) {
				$data['error'] = true;
				$data['html'] = sprintf(_("Failed to delete following files/folders: %s"), implode(', ',$errors));
			}
			header("Content-type: application/json");
			echo json_encode( $data );

		} else {
			echo "error";
		}
	}
	function rename($strip=""){

		if( $strip == 'json' ) {
			$error = false;
			$path = $this->input->post('path');
			$root = $this->input->post('root');
			$newname = $this->input->post('name');
			if( ! file_exists( $path ) ) {
				$error = sprintf(_("Requested file %s to rename doesn't exists"), $path);
			}
			if(!$error){
				$user=$this->session->userdata("user");
				$newpath = "$root/$newname";
				if(mv($path,$newpath,$user)) { // true == false
					$error = sprintf(_("Error renaming file '%s'"), $path);
				}
			}
			$data["success"]=!$error;

			if( $error ) {
				$data['error'] = true;
				$data['html'] = $error;
			}
			header("Content-type: application/json");
			echo json_encode( $data );

		} else {
			echo "error";
		}
	}

	function album($strip=""){

		if( $strip == 'json' ) {
			$error = false;
			if(!$this->Auth_model->policy("album","add")) {
				$error = _("Permission denied");
			} else {
				$files = $this->input->post('files');
				$user=$this->session->userdata("user");
				$this->load->model('album_model');
				$data['files_added'] = $this->album_model->batch_add( $files );
			}

			$data["success"]=!$error;

			if( $error ) {
				$data['error'] = true;
				$data['html'] = $error;
			}
			header("Content-type: application/json");
			echo json_encode( $data );

		} else {
			echo "error";
		}
	}

	function perm($strip="", $mode = 'get' ){

		if( $strip == 'json' ) {
			if( $mode == 'get' ) {
				$files=$this->input->post("files");
				$file_mode = 0000;
				foreach( $files as $file ) {
					$ss = @stat( $file );
					if( $ss ) {
						$file_mode |= $ss['mode'] & 000777;
					}
				}
				if( $file_mode == 0 ) {
					$file_mode = 0775;
				}
				$data = array(
					'permissions' => $file_mode
				);
				header("Content-type: application/json");
				echo json_encode( $data );


			} else {
				$errors = array();

				$files=$this->input->post("files");
				$user=$this->session->userdata("user");

				$mask = 0000;

				if($this->input->post("permission-owner")=="rw") {
					$mask |= 00600;
				}
				if($this->input->post("permission-owner")=="r") {
					$mask |= 00400;
				}

				if($this->input->post("permission-group")=="rw") {
					$mask |= 00060;
				}
				if($this->input->post("permission-group")=="r") {
					$mask |= 00040;
				}

				if($this->input->post("permission-other")=="rw") {
					$mask |= 00006;
				}
				if($this->input->post("permission-other")=="r") {
					$mask |= 00004;
				}

				foreach( $files as $file ) {
					if( changemod($file,$mask,$user) ) { // true == false
						$errors[] = $file;
					}
				}
				$data["success"]=empty($errors);

				if( !empty($errors) ) {
					$data['error'] = true;
					$data['html'] = sprintf(_("Failed to change permission for following files and folders: %s"), implode(', ',$errors));
				}
				header("Content-type: application/json");
				echo json_encode( $data );
			}
		}
	}


	function mkdir($strip=""){

		if( $strip == 'json' ) {
			$error = false;
			$directory=trim($this->input->post("name"));
			$root=trim($this->input->post("root"));
			$user=$this->session->userdata("user");

			if( ! $directory ) {
				$error = _("Error creating folder, no name supplied");
			}

			$realpath = "$root/$directory";
			if( !$error && file_exists( $realpath ) ) {
				$error = _("Error creating folder, name already exists");
			}

			if( ! $error ) {
				$mask = 0000;
				$ss = @stat( $root );
				if( $ss ) {
					$mask = $ss['mode'] & 000777;
				}

				if( !md($root."/".$directory,$mask,$user) ) {
					$error = _("Failed to create folder");
				}

			}
			$data["success"]=!$error;

			if( $error ) {
				$data['error'] = true;
				$data['html'] = $error;
			}
			header("Content-type: application/json");
			echo json_encode( $data );
		}
	}

	function move($strip=""){

		if( $strip == 'json' ) {
			$errors = array();
			$files = $this->input->post('files');
			$target = $this->input->post('path');
			$user=$this->session->userdata("user");

			if(!is_array($files)) {
				$files = array($files);
			}

			foreach($files as $file){
				if(mv($file,$target,$user)){ // true is false
					$errors[] = $file;
				}
			}

			$data["success"]=empty($errors);

			if( !empty($errors) ) {
				$data['error'] = true;
				$data['html'] = sprintf(_("Failed to move the following files and folders: %s"), implode(', ',$errors));
			}
			header("Content-type: application/json");
			echo json_encode( $data );

		} else {
			echo "error";
		}
	}
	function copy($strip=""){

		if( $strip == 'json' ) {
			$errors = array();
			$files = $this->input->post('files');
			$target = $this->input->post('path');
			$user=$this->session->userdata("user");

			if(!is_array($files)) {
				$files = array($files);
			}

			foreach($files as $file){
				if(cp($file,$target,$user)){ // true is false
					$errors[] = $file;
				}
			}

			$data["success"]=empty($errors);

			if( !empty($errors) ) {
				$data['error'] = true;
				$data['html'] = sprintf(_("Failed to copy the following files and folders: %s"), implode(', ',$errors));
			}
			header("Content-type: application/json");
			echo json_encode( $data );

		} else {
			echo "error";
		}
	}
	function _alter(&$item, $key){
		$item=b_dec($item);
	}

	function downloadzip(){
		$this->load->helper('content_disposition');
		$files = $this->input->post('files');
		$user=$this->session->userdata("user");
		$prefix=$this->input->post('path')?$this->input->post('path'):"/";

		$zipname = "download.zip";

		if( $this->input->post('zipname') ){
			$zipname = $this->input->post('zipname');
		}else if( count($files) == 1 ){
			$zipname = basename($files[0]);
		}

		if( strlen($zipname) <= 4 || substr_compare($zipname, '.zip', -4, 4, true ) != 0 ) {
			$zipname .= '.zip';
		}

		$regex = "#^".preg_quote( $prefix, '#' )."/?#";
		foreach( $files as &$file ) {
			$file = preg_replace( $regex, '', $file );
		}
		unset($file);

		header("Cache-Control: must-revalidate, post-check=0, pre-check=0");
		header("Pragma: public");
		mb_internal_encoding('UTF-8');
		header(content_disposition('attachment',$zipname));
		header("Content-type: application/x-zip");
		set_time_limit(3600);
		zip_files($files,$prefix,$user);
	}

	function download(){

		$this->load->helper('content_disposition');
		if(!$this->input->post("path")){
			$get_file="/".join("/",array_slice($this->uri->segment_array(),2));
		}else{
			$get_file=$this->input->post("path");
		}
		$user=$this->session->userdata("user");
		$mime_type=get_mime($get_file);

		$filename=basename($get_file);
		$filesize = get_filesize($get_file,$user);
		header("Cache-Control: must-revalidate, post-check=0, pre-check=0");
		header("Pragma: public");
		header(content_disposition('attachment',$filename));
		header("Content-Length: $filesize");

		if($mime_type) {
			header("Content-type: $mime_type");
		}

		set_time_limit(1800);
		cat_file($get_file,$user);

	}

	function cd($strip=""){
		if(!$this->input->post("path")){
			$path="/".join("/",array_slice($this->uri->segment_array(),2));
			$this->session->set_flashdata('path', $path);
		}
		redirect("/filemanager");
	}

	function index($strip=""){
		if( $strip == 'json' ) {

			$path=$this->input->post('path');
			$user=$this->session->userdata("user");

			$pos=strpos($path,"/home");

			if(($pos===false) || ($pos!=0)){
				$path="/home/$user";
			}
			$out = ls($user,"$path");
			$data['aaData'] = array();
			$data["meta"] = array();

			if($out=="\0\0") {
				// error
				$data["meta"]["permission_denied"]=true;
			}else{
				$typemap = array(
					'F' => 'file',
					'D' => 'dir',
					'L' => 'link',
				);
				foreach($out as $line) {
					if($line[0]=="P"){
						// Permission Hack to avoid more calls than needed to backend
						$perms=explode("\t",$line);
						$data["meta"]["writable"]=$perms[1]=="1";
					}else {
						list( $type, $size, $date, $name ) = explode("\t",$line);
						$data['aaData'][] = array(
							$typemap[$type],
							$name,
							$date,
							$size,
						);
					}
				}
			}
			$data['root'] = $path;

			header("Content-type: application/json");
			echo json_encode( $data );
			return;
		}

		$path = $this->session->flashdata('path');
		$user=$this->session->userdata("user");

		$pos=strpos($path,"/home");

		if(($pos===false) || ($pos!=0)){
			$path="/home/$user";
		}
		$data["path"]=$path;

		if($strip){
			$this->load->view(THEME.'/filemanager/filemanager_index_view',$data);
		}else{
			$this->_renderfull(
				$this->load->view(THEME.'/filemanager/filemanager_index_view',$data,true),
				$this->load->view(THEME.'/filemanager/filemanager_index_head_view',$data,true)
			);
		}
	}

}
