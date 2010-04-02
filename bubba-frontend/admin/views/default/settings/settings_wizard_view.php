<script type="text/javascript">
<!--

$(document).ready(function(){

	$("#ntp").click(function() {
		if($(this).attr('checked')) {
			$(".timedate").attr("disabled","true");
		} else {
			$(".timedate").removeAttr("disabled");
		}	
	})

});

// -->
</script>

<?
if($this->session->userdata("run_wizard")): // wizard is running
?>
	<h1 class="wizard-header"><?=t('Step 1/3: Date and time')?></h1>

		<form action="<?=FORMPREFIX?>/settings/wizard" method="post">
		<div id="ui-wizard-timezone">
			<table>
				<thead>
				  <tr><th colspan="4"><h2><?=t('Timezone')?></h2></th></tr>
			  </thead>
			  <tbody>
				<tr class="ui-header"><td><?=t('Current timezone is')?>:</td><td><?=$wiz_data['t_zone']?></td></tr>
				<tr class="ui-header"><td>Select timezone:</td><td>
					<select name="wiz_data[user_tz]">
					<option value=""> --- <?=t('Change timezone')?> --- </option>
					<?
					foreach($wiz_data['t_zoneinfo'] as $region => $a_name) {
						if($region != "other" && $region != "Etc" && $region != "US") {
							print "\n";
							foreach($a_name as $name) {
								print "\t\t<option value=\"$region/$name\">$region - $name</option>\n";
							}
						}
					}
					?>
					</select>
					<?=isset($err['timezone'])?"<tr><td></td><td><div class=\"highlight\">".t($err['timezone'])."</div></td></tr>\n":""?>
				</td></tr>
			</tbody>		
			</table>
		</div>
		<div id="ui-wizard-datetime">
			<table>
				<thead>
				  <tr><th colspan="4"><h2><?=t('title_settings-date')?></h2></th></tr>
				</thead>
				<tbody>
					<tr class="ui-header"><td><?=t('Auto adjust date and time')?></td><td><input id="ntp" type="checkbox" class="slide" name="wiz_data[use_ntp]" <?=isset($wiz_data['use_ntp'])?"CHECKED":""?> /></td></tr>
					<tr class="ui-header"><td>Date:</td><td><input type="text" class="timedate" name="wiz_data[date]" value="<?=$wiz_data['date']?>" <?=isset($wiz_data['use_ntp'])?"DISABLED":""?> /> <span>(<?=t('YYYYMMDD')?>)</span></td></tr>
					<tr class="ui-header"><td>Time:</td><td><input type="text" class="timedate" name="wiz_data[time]" value="<?=$wiz_data['time']?>" <?=isset($wiz_data['use_ntp'])?"DISABLED":""?> /> <span>(<?=t('HHmm')?>)</span></td></tr>
					<?=isset($err['timedate'])?"<tr><td></td><td><div class=\"highlight\">".t($err['timedate'])."</div></td></tr>\n":""?>
					<tr ><td colspan="2"><span class="wiz_spacer">&nbsp;</span><input class='submitbutton' type='submit' id="wizard_next" name="wiz_data[postingpage]" value='<?=t('Next')?>'/></td></tr>
				</tbody>
			</table>
		</div>
		</form>
<?
  else : // show start wizard page
?>
		<form action="<?=FORMPREFIX?>/settings/wizard" method="post">
			<table class="ui-table-outline">
				<thead>
			    <tr><td colspan="4" class="ui-state-default ui-widget-header"><?=t('Setup wizard')?></td></tr>
				</thead>
			</table>
			<table>
				<tbody>
				<tr><td>
					<?=t('To configure basic functionality of Bubba|2, press the button to start the setup wizard.')?>					
				</td></tr>
				</tbody>
				<tfoot>
				<tr><td>
					<input type="submit" class="submitbutton"  name='wiz_data[start]' value="<?=t('Start setup wizard')?>"/>
				</td></tr>
				</tfoot>
			</table>
		</form>
<? endif ?>
