
<table class="disks">
<tr><td colspan="6" class="ui-state-default ui-widghet-header"><?=t('Disk information')?></td></tr>
<tr>
	<th></th>
	<th class="col1"><i><?=t('Disk')?></i></th>
	<th><i><?=t('Device')?></i></th>
	<th><i><?=t('Disk size')?></i></th>
	<th><i><?=t('Type')?></i></th>
	<th><i><?=t('Partitions')?></i></th>
</tr>
<?foreach($disks as $disk):?>
<tr>
	<td>
		<?if($disk['formatable']):?>
		<input
			type="button"
			name="<?=$disk['model']?>"
			rel="<?=$disk['dev']?>"
			class="button format"
			value="<?=t('Format')?>"
		/>
		<?endif?>
	</td>
	<td><?=$disk["model"]?></td>
	<td><?=substr($disk["dev"],5)?></td>
	<td><?=sizetohuman($disk['size']*512,1000)?></td>
	<td><?=$disk["type"]?></td>
	<td class="partitions">
		<table class="partitions">
			<tr>
				<?if( array_key_exists( 'partitions', $disk ) ):?>
				<?foreach( $disk['partitions'] as $partition ):?>
				<?
					if( isset( $partition['system'] ) ) {
						$ccid = 'system';
					} else {
						$ccid = $partition['cid'];
					}
				?>
				<td style="width: <?=max(1,floor($partition['size'] / $disk['size'] * 100))?>%">
					<div class="partition-<?=$ccid?> partition " rel="<?=substr($partition["dev"], 5)?>">
					</div>
				</td>
				<?endforeach?>
				<?else:?>
				<td style="width: 100%">
					<div class="partition-<?=$disk['cid']?> partition " rel="<?=substr($disk["dev"], 5)?>">
					</div>
				</td>
				<?endif?>
			</tr>
		</table>
	</td>
</tr>
<?endforeach?>
</table>

<table class="typetable">
<tr><td colspan="6" class="ui-state-default ui-widghet-header"><?=t('Partition information')?></td></tr>
<tr>
	<th class="colorcode"></th>
	<th><i><?=t('Partition')?></i></th>
	<th><i><?=t('Partition label')?></i></th>
	<th><i><?=t('Partition size')?></i></th>
	<th class="path"><i><?=t('Mount path')?></i></th>
	<th></th>
</tr>

<?foreach( $legends as $legend ):?>
<tr>
	<td><div class="colorcode partition-<?=$legend['cid']?>"></div></td>
	<td><?
		switch($legend['name']) {
			case "/dev/md0":
				print t("Home partition");
				print " (RAID /dev/md0)";
				break;
			case "bubba-storage":
				print t("Home partition");
				print " (LVM)";
				break;
			default:
				print $legend['name'];
				break;
		}	
	?></td>
	<td><?=isset($devices[$legend['name']]['label'])?$devices[$legend['name']]['label']:""?></td> 
	<td><?=sizetohuman($legend['size'],1000)?></td> 
	<td><?if(isset($devices[$legend['name']]['mountpath'])):?>
		<a href="<?=site_url("filemanager/cd".$devices[$legend['name']]['mountpath'])?>"><?=$devices[$legend['name']]['mountpath']?></a>
	<?endif?></td>
	<td>
		<? if( isset($devices[$legend['name']]) ):?>
		<input type="button" rel="<?=$legend['name']?>" class="button mount <?=$devices[$legend['name']]['mounted']?'mounted':''?>" value="<?=$devices[$legend['name']]['mounted']?t('Disconnect'):t('Connect')?>" /></td>
		<?endif?>
</tr>
<?endforeach?>
<tr>
	<td><div class="colorcode partition-system"></div></td>
	<td><?=t("System partitions")?></td>
	<td />
	<td />
	<td />
</tr>
</table>
