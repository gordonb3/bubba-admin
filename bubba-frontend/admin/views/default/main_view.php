<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
 "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" lang="en" xml:lang="en">
<head>
<meta http-equiv="Expires" content="0" />
<meta http-equiv="Pragma" content="no-cache" />
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
<title>Bubba|TWO - <?=t('title_'.$this->uri->segment(1))?> (<?=php_uname("n")?>)</title>
<link rel="stylesheet" type="text/css" href="<?=FORMPREFIX.'/views/'.THEME?>/_css/jquery.ui.all.css?v='<?=$this->session->userdata('version')?>'" />
<link rel="stylesheet" type="text/css" href="<?=FORMPREFIX.'/views/'.THEME?>/_css/admin.css?v='<?=$this->session->userdata('version')?>'" />
<script type="text/javascript" src="<?=FORMPREFIX.'/views/'.THEME?>/_js/jquery.js?v='<?=$this->session->userdata('version')?>'"></script>
<script type="text/javascript" src="<?=FORMPREFIX.'/views/'.THEME?>/_js/jquery-ui.js?v='<?=$this->session->userdata('version')?>'"></script>
<?if(file_exists(APPPATH.'views/'.THEME.'/_js/i18n/'.LANGUAGE.'/messages.js')):?>
<script type="text/javascript" src="<?=FORMPREFIX.'/views/'.THEME.'/_js/i18n/messages-'.LANGUAGE.'.js'?>?v='<?=$this->session->userdata('version')?>'"></script>
<?else :?>
<script type="text/javascript" src="<?=FORMPREFIX.'/views/'.THEME.'/_js/i18n/messages-en.js'?>?v='<?=$this->session->userdata('version')?>'"></script>
<?endif?>
<?if(false):?>
<script type="text/javascript" src="<?=FORMPREFIX.'/views/'.THEME?>/_js/jquery.lint.js?v='<?=$this->session->userdata('version')?>'"></script>
<?endif?>

<script type="text/javascript" src="<?=FORMPREFIX.'/views/'.THEME?>/_js/main.js?v='<?=$this->session->userdata('version')?>'"></script>


<script type="text/javascript">

config = <?=json_encode(
	array(
		'prefix' => FORMPREFIX,
		'theme' => THEME,
		'version' => $this->session->userdata('version')
)
)?>;
	
$(document).ready(function(){
	
	$('#home_switch').click(function(event) {  
            event.preventDefault();
            $('#home').toggle()
        } );
	$('#sideboard_switch').click(function(event) {  
            event.preventDefault();
            
            if($('#sideboard').is(":visible")) {
                $('#sideboard').hide();
                $('#content').css( 'width', '100%' );
            } else {
                $('#sideboard').show();
                $('#content').css( 'width', '70%' );
            }
            
        } );


<?if(isset($update) && is_array($update)):?>

	update_status(
		<?=( isset($update['success']) && $update['success'] ) ? "true" : "false"?>,
		"<?=isset($update['message']) ? t($update['message']) : ""?>"
	);

<?endif?>

	$("#fn-topnav-help").click( function() {
		<?
		$uri = $this->uri->segment(1,"index");
		if($this->uri->segment(2)) {
			$uri .= "_".$this->uri->segment(2);
		}
		?>
		$.post("<?=FORMPREFIX?>/help/load/html/<?=$uri?>", function(data) {
			$.dialog(
				data,
				"<?=t("help_box_header")?>",
				{},
				{'modal' : false, dialogClass : "ui-help-box", position : ['right','top']});
		});
	});

});
</script>

<?
if(isset($head)) {
	echo $head;
}
?>
</head>
<body id="body_<?=$this->uri->segment($this->uri->total_segments())?>">
<?	
	if( $this->session->userdata("valid") ){
		if(isset($wizard)) {
			if($wizard) {
				print "<div class=\"wizard_bg\"></div>";
				print "<div class=\"wizard\">";
			  echo $wizard;
				print "</div>";
			}
		}
	}
?>
    <div id="wrapper">	
        <div id="header">		
            <div id="topnav">
            		<?if ($this->session->userdata("valid")) { ?>
	                <span id="topnav_status"><?=t("topnav-authorized",$this->session->userdata("user"))?></span>
            		<?} else {?>
	                <span id="topnav_status"><?=t("topnav-not-authorized")?></span>
            		<? } ?>
                <button id="fn-topnav-help" class="ui-button ui-widget ui-state-default ui-corner-all ui-button-icon-only" role="button" title="Help" aria-disabled="false"><span class="ui-button-icon-primary ui-icon ui-icon-lightbulb"></span><span class="ui-button-text">&nbsp;</span></button>
                <button id="home_switch" class="ui-button ui-widget ui-state-default ui-corner-all ui-button-icon-only" role="button" title="Home" aria-disabled="false"><span class="ui-button-icon-primary ui-icon ui-icon-home"></span><span class="ui-button-text">&nbsp;</span></button>
                <button class="ui-button ui-widget ui-state-default ui-corner-all ui-button-icon-only" role="button" title="Log out" aria-disabled="false"><span class="ui-button-icon-primary ui-icon ui-icon-power"></span><span class="ui-button-text">&nbsp;</span></button>                
                <a id="sideboard_switch" href="#" class="ui-state-default" ></a>
            </div>	
            <a href="#" id="a_logo" onclick="location.href='<?=FORMPREFIX?>';"><img id="img_logo" src="<?=FORMPREFIX.'/views/'.THEME?>/_img/logo.png" alt="BUBBA | 2" title="BUBBA | 2" /></a>
            <?=$navbar?>
        </div>		
        <div id="content">
        	<div id="<?=$this->uri->segment(1)?>">
            <?=$content?>
          </div>
        </div>
        <div id="sideboard" >
            <img id="img_sideboard" src="<?=FORMPREFIX.'/views/'.THEME?>/_img/sideboard_tmp.png" alt="tempfil för dashboard" title="tempfil för dashboard" />
        </div>
    </div>
    <div id="update_status"></div>
</body>
</html>
