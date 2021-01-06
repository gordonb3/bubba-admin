<script src="<?=FORMPREFIX.'/views/'.THEME?>/_js/stat.js?v='<?=$this->session->userdata('version')?>" type="text/javascript"></script>

<!-- Wizard -->
<script src="<?=FORMPREFIX.'/views/'.THEME?>/_js/jquery.form.js?v="<?=$this->session->userdata('version')?> type="text/javascript"></script>
<script src="<?=FORMPREFIX.'/views/'.THEME?>/_js/jquery.form.wizard.js?v="<?=$this->session->userdata('version')?> type="text/javascript"></script>
<script type="text/javascript" src="<?=FORMPREFIX.'/views/'.THEME?>/_js/jquery.ba-throttle-debounce.js?v='<?=$this->session->userdata('version')?>'"></script>
<script type="text/javascript" src="<?=FORMPREFIX.'/views/'.THEME?>/_js/wizard.js?v='<?=$this->session->userdata('version')?>'"></script>

<script type="text/javascript">
run_wizard=<?=json_encode(isset($run_wizard) ? $run_wizard : false)?>;

$(function(){
    if(run_wizard) {
        do_run_wizard();
    }
});
</script>
