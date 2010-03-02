<?php
// $lang['']="";
$lang['Login']="Login";
$lang['Home']="Home";
$lang['Users']="Users";
$lang['Services']="Services";
$lang['Mail']="Mail";
$lang['Network']="Network";
$lang['Printing']="Printing";
$lang['Settings']="Settings";
$lang['Filemanager']="File manager";
$lang['Album']="Photo album";
$lang['Stat']="Home";
$lang['Mail']="Mail";
$lang['Downloads']="Downloads";
$lang['Disk']="Disk";
$lang['Userinfo']="User info";
$lang['Shutdown']="Confirm Shutdown";

$lang['title_']=$lang['Home'];
$lang['title_login']=$lang['Login'];
$lang['title_users']=$lang['Users'];
$lang['title_services']=$lang['Services'];
$lang['title_mail']=$lang['Mail'];
$lang['title_network']=$lang['Network'];
$lang['title_printing']=$lang['Printing'];
$lang['title_settings']=$lang['Settings'];
$lang['title_filemanager']=$lang['Filemanager'];
$lang['title_album']=$lang['Album'];
$lang['title_stat']=$lang['Stat'];
$lang['title_usermail']=$lang['Mail'];
$lang['title_downloads']=$lang['Downloads'];
$lang['title_disk']=$lang['Disk'];
$lang['title_userinfo']=$lang['Userinfo'];
$lang['title_shutdown']=$lang['Shutdown'];


/* Generic button labels and texts */

$lang['button_label_continue']='Continue';
$lang['button_label_delete']='Delete';
$lang['button_label_cancel']='Cancel';
$lang['generic_dialog_text_please_wait'] = "Please wait...";
$lang['generic_dialog_text_warning'] = "Warning";


// backup field translations
$lang['current_job'] = "Job name";
$lang['target_protocol'] = "Target";

// disk
$lang['disk_action_title_extend_lvm'] = 'Extending user storage space';
$lang['disk_action_title_create_raid'] = 'Converting system to RAID';
$lang['disk_action_title_restore_raid'] = 'Recovering RAID';
$lang['disk_action_title_format'] = 'Formating disk';
$lang['in_sync'] = 'In sync';
$lang['faulty'] = 'Disk error';
$lang['active'] = 'Active';
$lang['clean'] = 'Clean';

$lang['disk_format_title'] = "Format disk";
$lang['disk_format_error_mounts_exists_message'] = "There seems to be disks mounted, please unmount these and try again";
$lang['disk_format_message'] = "Please specify label for your new partition";
$lang['disk_format_format_button_label'] = "Format disk";
$lang['disk_format_label_label'] = "Label";
$lang['disk_format_warning_1'] = "Formating disk will destroy all data on disk";
$lang['disk_format_warning_2'] = "Continue with formatting the disk?";
$lang['disk_format_format_progress_title'] = "Formating disk";
$lang['disk_format'] = "";

$lang['disk_lvm_extend_dialog_warning_message'] = "<p>This will erase all the data on the external device. Continue?</p> <p>Note: Removal of the new disk from the system will require a full reinstall.</p>";
$lang['disk_lvm_extend_dialog_warning_title'] = "Extend default data partition";
$lang['disk_lvm_extend_dialog_warning_button_label'] = "Extend partition";
$lang['disk_lvm_extend_dialog_title'] = "Extending disk";

/* RAID */
$lang['disk_raid_setup_title'] = "Setup RAID array";
$lang['disk_raid_create_label'] = "Create RAID array";
$lang['disk_raid_create_message'] = "Set up internal disk and one external disk into a RAID mirror solution (RAID 1)";
$lang['disk_raid_recover_label'] = "Recover RAID array";
$lang['disk_raid_recover_message'] = "Recover internal disk or add a new external disk to existing RAID array";
$lang['disk_raid_status_title'] = "RAID Status";
$lang['disk_raid_degraded_recover_status_message'] = "Recovering RAID array '%s'";
$lang['disk_raid_degraded_recover_status_message_eta_hours'] = "Current recover progress is %d%% and is estimated to finish in %d hours %d minutes";
$lang['disk_raid_degraded_recover_status_message_eta_minutes'] = "Current recover progress is %d%% and is estimated to finish in %d minutes";
$lang['disk_raid_degraded_message'] = "RAID array degraded";
$lang['disk_raid_degraded_missing_disk_message'] = "Disk missing in RAID array '%s'";
$lang['disk_raid_external_failure_title'] = "Error: External disk has malfunctioned";
$lang['disk_raid_external_failure_message_1'] = "The external RAID disk (<strong>%s</strong>) in the RAID array has malfunctioned";
$lang['disk_raid_external_failure_message_2'] = "Please replace the disk (also press \"Remove\" below to acknowledge the removal of the disk)";
$lang['disk_raid_external_failure_message_3'] = "When the disk has been replaced, press \"Recover RAID array\" to add the new disk to the array";
$lang['disk_raid_normal_op_message'] = "Normal operation";
$lang['disk_raid_not_activated_message'] = "RAID not activated";
$lang['disk_raid_detailed_info_title'] = "Detailed information";
$lang['disk_raid_list_of_arrays_title'] = "List of RAID arrays";
$lang['disk_raid_table_list_of_arrays_array_name_title'] = "Array name";
$lang['disk_raid_table_list_of_arrays_level_title'] = "Level";
$lang['disk_raid_table_list_of_arrays_state_title'] = "State";
$lang['disk_raid_table_list_of_arrays_label_title'] = "Label";
$lang['disk_raid_table_list_of_arrays_size_title'] = "Size";
$lang['disk_raid_list_of_disks_title'] = "List of RAID disks";
$lang['disk_raid_table_list_of_disks_disk_title'] = "Disk";
$lang['disk_raid_table_list_of_disks_parent_title'] = "Parent";
$lang['disk_raid_table_list_of_disks_state_title'] = "State";
$lang['disk_raid_table_list_of_disks_size_title'] = "Size";
$lang['disk_raid_disk_faulty_remove_button_label'] = "Remove";

# Create
$lang['disk_raid_create_progress_title'] = "Recovering RAID array";
$lang['disk_raid_create_title'] = "Create RAID array";
$lang['disk_raid_create_error_mounts_exists_message'] = "There seems to be disks mounted, please unmount these and try again";
$lang['disk_raid_create_select_disk_message'] = "Select which external disk to include in the array. For best usage an external disk with the same size is recommended";
$lang['disk_raid_create_warning_1'] = "Creating the RAID array will <strong>destroy all content</strong> on your internal disk (/home&nbsp;-&nbsp;including&nbsp;'storage') and erase the selected external disk";
$lang['disk_raid_create_warning_2'] = "Please make certain that you have a backup of all files";
$lang['disk_raid_create_warning_3'] = "Continue to create RAID?";
$lang['disk_raid_create_error_no_disks_found_message'] = "No usable disk found";
$lang['disk_raid_create_button_label'] = "Create RAID";

# Recover
$lang['disk_raid_recover_title'] = "Recover RAID array";
$lang['disk_raid_recover_broken_external_progress_title'] = "Recovering external disk in RAID array";
$lang['disk_raid_recover_broken_external_message'] = "Select external disk to add to RAID array";
$lang['disk_raid_recover_broken_external_warning_1'] = "Recovering the RAID array will <strong>destroy all content</strong> on the selected extenal disk";
$lang['disk_raid_recover_broken_external_warning_2'] = "Continue to recover RAID?";
$lang['disk_raid_recover_broken_external_button_label'] = "Add disk to RAID array";
$lang['disk_raid_recover_broken_external_no_disks_message'] = "There are no usable external disks attached, please add an external e-SATA disk and try again";
$lang['disk_raid_recover_broken_internal_progress_title'] = "Recovering internal disk in RAID array";
$lang['disk_raid_recover_broken_internal_mount_exists_message'] = "There seems to be disks mounted, please unmount these and try again";
$lang['disk_raid_recover_broken_internal_message'] = "Select which external disk to recover RAID data from";
$lang['disk_raid_recover_broken_internal_button_label'] = "Recover internal disk";
$lang['disk_raid_recover_broken_internal_warning_1'] = "Recovering the RAID array will <strong>destroy all content</strong> on your internal disk (/home&nbsp;-&nbsp;including&nbsp;'storage')";
$lang['disk_raid_recover_broken_internal_warning_2'] = "Continue to recover RAID?";
$lang['disk_raid_recover_broken_internal_button_label'] = "Recover internal disk";
$lang['disk_raid_recover_broken_internal_no_raid_message'] = "No disks with RAID data found";

// Network
$lang['wlan_title'] = 'Wireless';
$lang['wlan_title_ssid'] = 'Network name (SSID)';
$lang['wlan_title_ssid_popup'] = 'The network name is used to connect to the Bubba|TWO via a wireless network, often called SSID';
$lang['wlan_title_enable'] = 'Enable wireless';
$lang['wlan_title_enable_popup'] = 'Check this checkbox to enable wireless functionallity for your Bubba|TWO';

$lang['wlan_title_advanced'] = 'Advanced wireless settings';

$lang['wlan_title_band'] = 'Band';
$lang['wlan_title_band_1'] = '2.4GHz band used by 802.11g';
$lang['wlan_title_band_2'] = '5GHz band used by 802.11a';

$lang['wlan_title_mode'] = 'Mode';
$lang['wlan_title_mode_popup'] = 'The operation mode for selected band';
$lang['wlan_title_legacy_mode_2'] = 'Legacy mode (802.11a)';
$lang['wlan_title_legacy_mode_1'] = 'Legacy mode (802.11g)';
$lang['wlan_title_mixed_mode_2'] = 'Mixed mode (802.11n + 802.11a)';
$lang['wlan_title_mixed_mode_1'] = 'Mixed mode (802.11n + 802.11g)';
$lang['wlan_title_greenfield_mode'] = 'N only mode (802.11n only)';

$lang['wlan_title_encryption'] = 'Encryption';
$lang['wlan_title_encryption_popup'] = 'The encryption to use';
$lang['wlan_title_encryption_wpa2'] = 'WPA2';
$lang['wlan_title_encryption_wpa12'] = 'WPA1 or WPA2';
$lang['wlan_title_encryption_wpa1'] = 'WPA1';
$lang['wlan_title_encryption_wep'] = 'WEP';
$lang['wlan_title_encryption_none'] = 'None';

$lang['wlan_title_width'] = 'Channel width';
$lang['wlan_title_width_popup'] = 'The targeted width of the channel in MHz';
$lang['wlan_title_width_20MHz'] = '20MHz';
$lang['wlan_title_width_40MHz'] = '40MHz';

$lang['wlan_title_password'] = 'Password';
$lang['wlan_title_password_popup'] = 'The WEP or WPA password that should be required to connect to the AP';

$lang['wlan_title_channel'] = 'Channel';
$lang['wlan_title_channel_popup'] = 'The main channel to use';

$lang['wlan_title_broadcast'] = 'Broadcast SSID';
$lang['wlan_title_broadcast_popup'] = 'Whenever to broadcast the SSID';

# Printing
$lang['printing_add_error_invalid_characters'] = "Invalid characters in share name, only <strong>A-Z</strong>,<strong>a-z</strong> and <strong>_</strong> is allowed";
$lang['printing_add_error_no_name'] = "No name was provided";
$lang['printing_add_error_no_printer_name'] = "No printer name was provided";
$lang['printing_add_error_no_printer_path'] = "No printer path was provided";
$lang['printing_add_operation_fail'] = "Adding printer failed";
$lang['printing_add_success'] = "Printer <strong>%s</strong> was added successfully";
$lang['printing_delete_success'] = "Printer <strong>%s</strong> was deleted successfully";

# Services
$lang['service_update_success'] = "Services updated";

# Settings
$lang['settings_traffic_success'] = "Traffic limit updated";
$lang['settings_traffic_error_service_unavailable'] = "Traffic service is unavailable";
$lang['settings_traffic_error_set_dl_throttle'] = "Failed to set download throttle";
$lang['settings_traffic_error_set_ul_throttle'] = "Failed to set upload throttle";

$lang['settings_backup_error_no_path'] = "Failed to set up mount point for backup";
$lang['settings_backup_error_failed'] = "The system was unable to create an backup";
$lang['settings_backup_success'] = "System backup was sucessfully created";

$lang['settings_restore_error_no_path'] = "Failed to set up mount point for restore";
$lang['settings_restore_error_failed'] = "The system was unable to restore the system from an backup";
$lang['settings_restore_success'] = "System was sucessfully restored";

$lang['settings_datetime_success'] = "Timezone, date and/or time was successfully updated";
$lang['settings_datetime_error_set_timezone'] = "Failed to set timezone <strong>%s</strong>";
$lang['settings_datetime_error_set_date_time'] = "Failed to set date <strong>%s</strong> and time <strong>%s</strong>";

$lang['settings_software_install_package'] = "Install %s";
$lang['settings_software_update_software'] = "Update software";
$lang['settings_software_update_system'] = "Update system";
$lang['settings_software_include_hotfixes'] = "Include hotfixes and system specific updates";

$lang['settings_identity_error_change_hostname'] = "Failed to change hostname";
$lang['settings_identity_error_invalid_hostname'] = "Hostname <strong>%s</strong> is invalid, only character <strong>A-Za-z0-9-</strong> is valid";
$lang['settings_identity_easyfind_error_fail_set_name'] = "Failed to set easyfind name <strong>%s</strong>, probably this name is taken allready. Please try an other one";
$lang['settings_identity_easyfind_error_invalid_name'] = "Easyfind name <strong>%s</strong> is invalid, only character <strong>A-Za-z0-9-</strong> is valid";
$lang['settings_identity_easyfind_error_fail_enable'] = "Failed to enable easyfind";
$lang['settings_identity_easyfind_error_fail_disable'] = "Failed to disable easyfind";
$lang['settings_identity_title'] = "Windows share options"; # XXX Erm? WTF???
$lang['settings_identity_hostname_label'] = "Hostname";
$lang['settings_identity_workgroup_label'] = "Workgroup";
$lang['settings_identity_update_hostname_workgroup_label'] = "Update hostname and workgroup";
$lang['settings_identity_easyfind_title'] = "Easyfind options";
$lang['settings_identity_easyfind_message'] = "Use 'Easyfind' to locate your Bubba";
$lang['settings_identity_update_easyfind_label'] = "Update easyfind";

//  ---------- Users  -----
$lang['realname'] = 'Real name';
$lang['username'] = 'User name';
$lang['shell_login'] = 'Shell login';
$lang['allow_ssh'] = 'Allow SSH login';
$lang['allow_remote'] = 'Allow remote access to config interface';
$lang['users_pwd1'] = 'New password';
$lang['users_pwd2'] = 'Verify password';
$lang['illegal'] = 'Illegal characters in password';
$lang["mismatch"]='Password do not match';
$lang["sambafail"]='Failed to update password';
$lang["passwdfail"]=$lang["sambafail"];

$lang["usr_caseerr"] = "No uppercase letters allowed in username";
$lang["usr_existerr"] = "User already exists or is an administrational account";
$lang["usr_nonameerr"] = "No username entered";
$lang["usr_spacerr"] = "White space not allowed in username";
$lang["pwd_charerr"] = "Illegal characters in password";
$lang["usr_charerr"] = "Illegal characters in username"; 
$lang["usr_longerr"] = "Username to long. Max 32 characters";
$lang["usr_createerr"] = "Error creating user";
$lang["usr_addok"] = "User added";
$lang["pwd_mismatcherr"] = "Passwords do not match or password empty";

//  ---------- Admin Mail-----
$lang["usrinvalid"] = "Not authorized to add accounts for selected user.";
$lang["infoincomp"] = "Account information incomplete. Account not added.";
$lang["mail_addok"] = "Account added.";
$lang["mail_err_usrinvalid"] = "User not allowed to update account";
$lang["mail_editok"] = "Account updated.";

/*  ------------------- Texts to locate help pages.  -------------------*/
//General pages
$lang['help_stat']="";
$lang['help_login']="?page=quickstart.html";
$lang['help_filemanager']="?page=fileserver.html#WEB_BASED";


// Administrator pages
$lang['help_users']="?page=administrator.html#USERS";
$lang['help_services']="?page=administrator.html#SERVICES";
$lang['help_mail']="?page=administrator.html#MAIL";
$lang['help_network']="?page=administrator.html#NETWORK";
$lang['help_wan']="?page=administrator.html#NETWORK_WAN";
$lang['help_lan']="?page=administrator.html#NETWORK_LAN";
$lang['help_other']="?page=administrator.html#NETWORK_identity";
$lang['help_fw']="?page=administrator.html#NETWORK_Firewall";
$lang['help_disk']="?page=administrator.html#DISK";
$lang['help_lvm']="?page=administrator.html#DISK_LVM";
$lang['help_raid']="?page=administrator.html#DISK_RAID";
$lang['help_printing']="?page=administrator.html#PRINTING";
$lang['help_settings']="?page=administrator.html#SETTINGS";
$lang['help_backup']="?page=backup.html";
$lang['help_restore']="?page=backup.html#RESTORE";
$lang['help_trafficsettings']="?page=administrator.html#traffic";
$lang['help_datetime']="?page=administrator.html#dateandtime";
$lang['help_backuprestore']="?page=administrator.html#backuprestore";
$lang['help_software']="?page=sw_upgrade.html";
$lang['help_hotfix']="?page=sw_upgrade.html#hotfix";
$lang['help_logs']="?page=administrator.html#logs";


// User pages
$lang['help_usermail']="?page=users.html#MAIL";
$lang['help_downloads']="?page=users.html#DOWNLOADS";
$lang['help_userinfo']="?page=users.html#USERINFO";
$lang['help_album']="?page=users.html#PHOTOALBUM";

// Help pages
$lang['help_network']=$lang['help_network_profile']="
this is the helptext
";
$lang['help_network_wan']="this is the helptext for wan";