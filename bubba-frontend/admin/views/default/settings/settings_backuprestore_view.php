<table class="ui-table-outline">
<thead>
    <tr><td colspan="2" class="ui-state-default ui-widget-header"><?=_('Backup and restore settings')?></td></tr>
</thead>
<tbody>
<tr>
	<td><?=_("Source")?>/<?=_("Destination")?></td>
	<td>
		<select name="destination" id="unitselector">
<?if(!empty($disks)):?>
<?foreach($disks as $disk):?>
			<option value="<?=$disk?>"><?=$disk?></option>
<?endforeach?>
<?else :?>
			<option><?=_('No backup medium found')?></option>
<?endif?>
		</select>
	</td>
</tr>
</tbody>
<tfoot>
<tr>
	<td colspan="2">
<form action="<?=FORMPREFIX?>/settings/restore" method="post" id="restoreform">
			<input class="unitclass" type="hidden" name="unit"/>
			<input type="submit" name="restore" value="<?=_("Restore")?>" <?if(empty($disks)):?>disabled="disabled"<?endif?>/>
		</form>

<form action="<?=FORMPREFIX?>/settings/backup" method="post" id="backupform">
			<input class="unitclass" type="hidden" name="unit"/>
			<input type="submit" name="backup" value="<?=_("Backup")?>" <?if(empty($disks)):?>disabled="disabled"<?endif?>/>
		</form>
	</td>
</tr>
</tfoot>

</table>

