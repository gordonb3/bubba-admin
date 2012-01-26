<script src="<?=FORMPREFIX.'/views/'.THEME?>/_js/album_users.js?v=<?=$this->session->userdata('version')?>" type="text/javascript"></script>
<script type="text/javascript">
user_accounts=<?=json_encode($accounts)?>;
allowed_to_delete=<?=json_encode($allow_delete)?>;
</script>
