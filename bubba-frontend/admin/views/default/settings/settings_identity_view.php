
	<form action="<?=FORMPREFIX?>/settings/identity" method="post">
	<table class="networksettings ui-table-two-col ui-table-outline">
		<thead>
		<tr><td colspan="2" class="ui-state-default ui-widget-header"><?=_("System identity")?></td></tr>
		</thead>
		<tbody>
		<tr>

			<td><?=_("Hostname")?>:</td>
			<td>
				<input
					type="text"
					name="hostname"
					value="<?=$hostname?>"
				/>
			</td>
		</tr>
		<tr>

			<td><?=_("Workgroup")?>:</td>
			<td>
				<input
					type="text"
					name="workgroup"
					value="<?=$workgroup?>"
				/>
			</td>
		</tr>
		</tbody>
		<tfoot>
		<tr>
		<td colspan="2">
			<input
					type="submit"
					value='<?=_("Update hostname and workgroup")?>'
					name='samba_update'
			/>
		</td>
		</tr>
		</tfoot>
	</table>
	</form>

