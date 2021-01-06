<?$bgwhite=true?>
<table class="ui-table-outline">
<thead>
    <tr><td colspan="2" class="ui-state-default ui-widget-header"><?=_('Show logs')?></td></tr>
</thead>
</table>
<div class="ui-inset">
	<form action="<?=FORMPREFIX?>/settings/logs" method="post" id="settings_logs">
	<select name="log" id="settings_log">
	<?foreach( $logs as $name => $path ):?>
		<?if(is_array($path)):?>
		<?$bgwhite=!$bgwhite?>
		<optgroup label="<?=$name?>" style="color:red;<?if(!$bgwhite)echo "background:#eeeeee;"?>">
		<?foreach( $path as $inner_name => $inner_path ):?>
			<option style="color:#333;<?if(!$bgwhite)echo "background:#eeeeee;"?>" <?if(isset($log_name)&& $log_name == $inner_path){echo "selected=\"selected\"";}?> label="<?=$inner_name?>" value="<?=$inner_path?>"> <?=$inner_name?> </option>
		<?endforeach?>
		</optgroup>
		<?else:?>
		<option style="color:#333;" <?if(isset($log_name)&& $log_name == $path){echo "selected=\"selected\"";}?> label="<?=$name?>" value="<?=$path?>"> <?=$name?> </option>
		<?endif?>
	<?endforeach?>
	</select>
	<input type="submit" value="<?=_("Show")?>"/>
	</form>
<?if(isset($log_name)):?>
	<h3><?=$log_name?></h3>
	<div class="ui-widget-content ui-log ui-corner-all">
		<?$n=0;foreach( $content as $line):?>
		<div class="ui-log-line<?=$n++%2?>"><?=htmlspecialchars($line)?></div>
		<?endforeach?>
	</div>
<?endif?>
</div>
