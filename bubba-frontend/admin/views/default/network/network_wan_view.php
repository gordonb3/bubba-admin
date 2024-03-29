<? if($this->session->userdata("network_profile") == "auto" || $this->session->userdata("network_profile") == "custom"): ?>
	<div class="ui-network-information-panel">
	<?=sprintf(_("These settings are locked")." ("._("%s is using automatic network settings"), NAME).")"?>&nbsp;.&nbsp;<br />
	<?=_("To unlock, select Router or Server profile under the ")?><a href="<?=FORMPREFIX?>/network/profile"><?=_("Profile")?></a> tab
		</div>
<? endif ?>

<form id="WANCFG" action="<?=FORMPREFIX?>/network/wanupdate" method="post">
<table id="table-network-wan" class="ui-table-outline">
  <tr><td colspan="4" class="ui-state-default ui-widget-header"><?=_("WAN")?></td></tr>
	<tr>
		<td >
		<input type="radio" class="checkbox_radio" name='netcfg' value='dhcp' onclick="dhcp_onclick()" <?=$dhcp?"checked=\"checked\"":""?>/>
		</td>
		<td colspan="3">
			<label for=""><label for=""><?=_('Obtain IP-address automatically')?> (DHCP)</label>
		</td>
	</tr>
	<tr>
		<td>
			<input type="radio" class="checkbox_radio" name='netcfg' value='static' onclick="static_onclick(<?=$disable_gw?>)" <?=$dhcp?"":"checked=\"checked\""?>/>
		</td>
		<td  colspan="3">
			<label for=""><label for=""><?=_('Use static IP address settings')?>:</label>
		</td>
	</tr>
	<tr id="tr-network-ip">
		<td></td>
		<td><label for=""><?=_("IP")?></label>:</td>
		<td><input <?=$dhcp?"disabled=\"disabled\"":""?> value='<?=$oip[0]?>' class='ip' name='IP[0]' type='text' size='3' maxlength='3'/>&nbsp;.&nbsp;<input <?=$dhcp?"disabled=\"disabled\"":""?> value='<?=$oip[1]?>' class='ip' name='IP[1]' type='text' size='3' maxlength='3'/>&nbsp;.&nbsp;<input <?=$dhcp?"disabled=\"disabled\"":""?> value='<?=$oip[2]?>' class='ip' name='IP[2]' type='text' size='3' maxlength='3'/>&nbsp;.&nbsp;<input <?=$dhcp?"disabled=\"disabled\"":""?> value='<?=$oip[3]?>' class='ip' name='IP[3]' type='text' size='3' maxlength='3'/></td><td><?=$err_ip?"* " . _("Invalid IP"):""?></td>
	</tr>
	<tr id="tr-network-netmask">
		<td></td>
		<td><label for=""><?=_("Netmask")?></label>:</td>
		<td><input <?=$dhcp?"disabled=\"disabled\"":""?> value='<?=$omask[0]?>' class='ip' name='mask[0]' type='text' size='3' maxlength='3'/>&nbsp;.&nbsp;<input <?=$dhcp?"disabled=\"disabled\"":""?> value='<?=$omask[1]?>' class='ip' name='mask[1]' type='text' size='3' maxlength='3'/>&nbsp;.&nbsp;<input <?=$dhcp?"disabled=\"disabled\"":""?> value='<?=$omask[2]?>' class='ip' name='mask[2]' type='text' size='3' maxlength='3'/>&nbsp;.&nbsp;<input <?=$dhcp?"disabled=\"disabled\"":""?> value='<?=$omask[3]?>' class='ip' name='mask[3]' type='text' size='3' maxlength='3'/></td><td><?=$err_mask?"* " . _("Invalid netmask"):""?></td>
	</tr>
	<tr id="tr-network-gateway">
		<td></td>
		<td><label for=""><?=_('Default gateway')?></label>:</td>
		<td>
			<input
        <?if($disable_gw || $dhcp):?>disabled="disabled"<?endif?>
				value='<?=$ogw[0]?>'
				class='ip'
				name='gw[0]'
				type='text'
				size='3'
				maxlength='3'
				/>&nbsp;.&nbsp;<input
        <?if($disable_gw || $dhcp):?>disabled="disabled"<?endif?>
				value='<?=$ogw[1]?>'
				class='ip'
				name='gw[1]'
				type='text'
				size='3'
				maxlength='3'
			/>&nbsp;.&nbsp;<input
        <?if($disable_gw || $dhcp):?>disabled="disabled"<?endif?>
				value='<?=$ogw[2]?>'
				class='ip'
				name='gw[2]'
				type='text'
				size='3'
				maxlength='3'
			/>&nbsp;.&nbsp;<input
        <?if($disable_gw || $dhcp):?>disabled="disabled"<?endif?>
				value='<?=$ogw[3]?>'
				class='ip'
				name='gw[3]'
				type='text'
				size='3'
				maxlength='3'
			/>
		</td>
		<td><?=$err_gw?"* " . _("Invalid gateway"):""?></td>
	</tr>
	<tr id="tr-network-dns">
		<td></td>
		<td><label for=""><?=_('Primary DNS')?></label>:</td>
		<td>
			<input
        <?if($disable_gw || $dhcp):?>disabled="disabled"<?endif?>
				value='<?=$odns[0]?>'
				class='ip'
				name='dns[0]'
				type='text'
				size='3'
				maxlength='3'
			/>&nbsp;.&nbsp;<input
        <?if($disable_gw || $dhcp):?>disabled="disabled"<?endif?>
				value='<?=$odns[1]?>'
				class='ip'
				name='dns[1]'
				type='text'
				size='3'
				maxlength='3'
			/>&nbsp;.&nbsp;<input
        <?if($disable_gw || $dhcp):?>disabled="disabled"<?endif?>
				value='<?=$odns[2]?>'
				class='ip'
				name='dns[2]'
				type='text'
				size='3'
				maxlength='3'
			/>&nbsp;.&nbsp;<input
        <?if($disable_gw || $dhcp):?>disabled="disabled"<?endif?>
				value='<?=$odns[3]?>'
				class='ip'
				name='dns[3]'
				type='text'
				size='3'
				maxlength='3'
			/>
		</td>
		<td><?=$err_dns?"* " . _("Invalid DNS setting"):""?></td>
	</tr>
	<tr>
		<td colspan="4">
			<input type="hidden" name='update'/>
			<button class="submit .fn-network-button_submit"><?=_("Update")?></button>
		</td>
	</tr>

</table>
</form>
