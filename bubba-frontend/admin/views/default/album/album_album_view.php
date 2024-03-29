<script type="text/javascript">
createFileTree = function(obj) {
	obj.fileTree({
		root: '/',
		fakeTopRoot: true,
		script: '<?=site_url("ajax_album/get_album_list")?>',
		fileCallback: function(file) {
			$('#album_list li a').removeClass('selected' );
			$('#album_list li#image_' + file + '> a').addClass( 'selected' );
			$('#album_edit_area').html( '<div id="loader" class="loading"></div>' );


			$.throbber.show();
			$.post( '<?=site_url("ajax_album/get_image_metadata")?>',
				{ 'image': file }, function(data) {
					$.throbber.hide();

					$('<img />').load(function() {
						$(this).hide();
						$('#loader')
							.removeClass('loading')
							.html(this);

						$(this).fadeIn();
					})
						.error(function(e,v,d) {alert('error: ' + e.data)} )
						.attr('src', data.image );
					form = $('<form />');
					form.submit(function() {
						post = {
							caption: this.caption.value,
							name: this.name.value,
							id: file
						};
						$.throbber.show();
						$.post( '<?=site_url("ajax_album/update_image_metadata")?>',
							post, function(data) {
								$.throbber.hide();
								$('#image_' + file + ' > a' ).text( post.name );
								if(data.error) {
									update_status(false,"<?=_("Error updating image")?>");
								} else {
									update_status(true,"<?=_("Image updated")?>");
								}
								//$('#tmp').html( data.html );
							}, 'json' );
						return false;
					});
					$('#album_edit_area').append("<div class='filename'>" + data.path + "</div>");
					field1 = $('<fieldset />');
					field1.appendTo($('#album_edit_area'));
					form.appendTo(field1);

					image_metadata = $("<table class='metadata' />");
					data_row = $("<tr />");
					data_cell = $('<td />');
					data_cell.append('<label for="name"><?=_("Image name")?>: </label>');
					data_row.append(data_cell);
					data_cell = $('<td />');
					data_cell.append($('<input type="text" id="name"/>').attr('value', data.name));
					data_row.append(data_cell);
					image_metadata.append(data_row);


					data_row = $("<tr />");
					data_cell = $('<td />');
					data_cell.append('<label for="caption"><?=_("Description")?>: </label>');
					data_row.append(data_cell);
					data_cell = $('<td />');
					if(data.caption) {
						img_cap = data.caption;
					} else {
						img_cap = "";
					}
					data_cell.append($('<textarea id="caption" />').attr('value', img_cap));
					data_row.append(data_cell);
					image_metadata.append(data_row);

					form.append(image_metadata);

					del = $('<input type="button" value="<?=_("Remove from album")?>" />');
					del.click(function() {
						$.confirm( '<?=_("Remove selected image from album?")?>', '',[
					{
						'label': 'Remove',
						'callback': function() {
							var self = this;
							$.throbber.show();
							$.post( '<?=site_url("ajax_album/delete_image")?>',
					{id: file}, function(data) {
								$.throbber.hide();
								$(self).dialog("close");
								$('#image_' + file ).remove();
								$('#album_edit_area').html("");
								if(data.error) {
									update_status(false,"<?=_("Error removing image")?>");
								} else {
									update_status(true,"<?=_("Image removed from album")?>");
								}
							}, 'json' );

						}
					}]);
						return false;
					});
					form.append(del);
					form.append($('<input type="submit" value="<?=_("Update")?>" />'));

				}, 'json' );

		},
		dirCollapseCallback: function(dir) {
			if( $('#album_list li#album_' + dir + '> a').hasClass( 'selected' ) ) {
				return true;
			}
			$('#album_list').data('selected_elem', this );
			this.dirExpandCallback( dir );
			return false;
		},
		dirExpandCallback: function(dir) {
			$('#album_list li a').removeClass('selected' );
			$('#album_list li#album_' + dir + '> a').addClass( 'selected' );
			$('#album_edit_area').html( '' );

			field1 = $('<fieldset />');
			field2 = field1.clone();
			field2.prependTo($('#album_edit_area'));
			field1.prependTo($('#album_edit_area'));

			$.throbber.show();
			$.post( '<?=site_url("ajax_album/get_album_metadata")?>',
				{ 'album': dir }, function(data) {
					$.throbber.hide();


					form = $('<form />');
					form.submit(function() {
						post = {
							caption: this.caption.value,
							name: this.name.value,
							id: dir
						};
						$.throbber.show();
						$.post( '<?=site_url("ajax_album/update_album_metadata")?>',
							post, function(data) {
								$.throbber.hide();
								$('#album_' + dir + ' > a' ).text( post.name );
								if(data.error) {
									update_status(false,"<?=_("Error updating album")?>");
								} else {
									update_status(true,"<?=_("Album updated")?>");
								}
							}, 'json' );
						return false;
					});
					form.appendTo(field1);
					album_metadata = $("<table class='metadata' />");
					data_row = $("<tr />");
					data_cell = $('<td />');
					data_cell.append('<label for="name"><?=_("Album name")?>: </label>');
					data_row.append(data_cell);
					data_cell = $('<td />');
					data_cell.append($('<input type="text" id="name"/>').attr('value', data.name));
					data_row.append(data_cell);
					album_metadata.append(data_row);


					data_row = $("<tr />");
					data_cell = $('<td />');
					data_cell.append('<label for="caption"><?=_("Description")?>: </label>');
					data_row.append(data_cell);
					data_cell = $('<td />');
					if(data.caption) {
						album_cap = data.caption;
					} else {
						album_cap = "";
					}
					data_cell.append($('<textarea id="caption" />').attr('value', album_cap));
					data_row.append(data_cell);
					album_metadata.append(data_row);

					form.append(album_metadata);
					del = $('<input type="button" value="<?=_("Delete album")?>" />');
					form.append(del);
					form.append($('<input type="submit" value="<?=_("Update")?>" />'));
					del.click(function() {
						$.confirm(
							'Delete album "' + data.name +'"?',
							"<?=_('Delete album')?>",
							[
								{
								label : 'Delete album',
									callback : function() {
									var self = this;
									$.throbber.show();
									$.post( '<?=site_url("ajax_album/delete_album")?>',
									{id: dir},
									function(data) {
										$.throbber.hide();
										$(self).dialog("close");
										$('#album_edit_area').html("");
										$('#album_' + dir ).remove();
										if(data.error) {
											update_status(false,"<?=_("Error deleting album")?>");
										} else {
										update_status(true,"<?=_("Album deleted")?>");
										}
									},
									'json'
									);
								}
							}
						]
					);
					}
				);
				}, 'json' );

			$.throbber.show();
			$.post( '<?=site_url("ajax_album/get_album_access_list")?>',
				{ 'album': dir }, function(data) {
					$.throbber.hide();


					form = $('<form />');
					form.submit(function() {
						post = {
							caption: this.caption.value,
							name: this.name.value,
							id: dir
						};
						$.throbber.show();
						$.post( '<?=site_url("ajax_album/update_album_access_list")?>',
							post, function(data) {
								$.throbber.hide();
								//$('#tmp').html( data.html );
							}, 'json' );
						return false;
					});
					form.appendTo(field2);

					if( ! (data.public.child && ! data.public.public) ) {
						form.append(
							$('<div class="public_access"/>')
								.append(
									$('<label />')
										.attr('for', 'public')
										.text('<?=_("Allow anonymous access")?>: ')
								)
								.append(
									$("<input type='checkbox'/>")
									.attr({
										id:'public',
										checked: data.public.public || data.public.selfpublic,
										disabled: !data.public.public && data.public.child
									})
									.toggleClass("checkbox_radio",<?=preg_match("/Opera/i",$_SERVER['HTTP_USER_AGENT'])?"false":"true"?>)
									.click(function() {
										if( this.checked ) {
											$('#user-mod input:checkbox').attr('disabled', true);
											$('#user-mod').addClass('disabled');
										} else {
											$('#user-mod input:checkbox').attr('disabled', false);
											$('#user-mod').removeClass('disabled');
										}
									} )
								)
						);
					} else {
						form.append(
							$('<div class="public_access"/>')
								.append(
									$('<label />')
										.text('<?=_("Anonymous access not granted to parent album.")?>')
										.addClass("comment")
								)
							);
					}

					box = $('<table id="user-mod"/>');
					box.toggleClass('disabled',$("#public").is(':checked') && $("#public").length);
					form.append( box );

					tr = $('<tr />');
					tr.append(
						$('<td />')
						.addClass('username')
						.text("<?=_("Viewer")?>")
					);
					tr.append(
						$('<td />')
						.text( "<?=_("Access allowed")?>" )
					);
					box.append( tr );

					for( i = 0; i < data.list.length; ++i ) {
						cur = data.list[i];

						input = $("<input type='checkbox'/>")
						.attr({
							checked: cur.has_access == 1,
							disabled: data.public.public,
							rel: cur.userid
						})
						.toggleClass("checkbox_radio",<?=preg_match("/Opera/i",$_SERVER['HTTP_USER_AGENT'])?"false":"true"?>)
						tr = $('<tr />');

						tr.append(
							$('<td />')
							.addClass('username')
							.append(cur.username)
						);
						tr.append(
							$('<td />')
							.append( input )
						);
						box.append( tr );
					}
					buttons = $('<div />');
					input = $('<input />')
					.attr({
						type:'button',
						value: "Update"
					})
					.click(function() {

						$.throbber.show();
						$.post( '<?=site_url("ajax_album/set_public")?>',
							{
								album: dir,
								public: $("#public").attr("checked")
							}, function(data) {
								$.throbber.hide();
								if(data.error) {
									update_status(false,"<?=_("Error updating user access to public")?>");
								} else {
									update_status(true,"<?=_("Album access rights updated")?>");
								}
							},
							'json'
						);

						uids = $("#user-mod input:checked")
							.map(function() { return $(this).attr("rel"); })
							.get();


						n_uid = "[";
						jQuery.each( uids,function() {
							n_uid += "\""+this+"\", ";
						});
						n_uid = n_uid.replace(/, $/,""); // remove trailing ","
						n_uid += "]";
						$.throbber.show();
						$.post( '<?=site_url("ajax_album/modify_user_access")?>',
							{
								album: dir,
								uid: n_uid
							}, function(data) {
								$.throbber.hide();
								if(data.error) {
									update_status(false,"<?=_("Error updating user access")?>");
								} else {
									update_status(true,"<?=_("Album access rights updated")?>");
								}
							},
							'json'
						);
					});

					buttons.append(input);
					form.append(buttons);

			}, 'json' );
		},
		moveCallback: function( from, to, directory ) {
			$.throbber.show();
			$.post( '<?=site_url("ajax_album/move")?>',
				{ id: from, target: to, album: directory }, function(data) {
					$.throbber.hide();
					//$('#tmp').html( data.html );
				}, 'json' );
		},
		readyCallback: function(root) {
			id = $('#album_list').data('last_id');
			if( id && $(root).attr('id') == '' ) {
				$('#album_' + id + ' > a').click()
			}
		}
   	});
};

$(document).ready( function() {
	createFileTree( $('#album_list') );


	$("#add").click(function(e) {
	$.throbber.show();
	$.post( '<?=site_url("ajax_album/create_album")?>',
				{}, function(data) {
					$.throbber.hide();
					$('#album_list').html();
					$('#album_list').data('last_id', data.id);
					createFileTree( $('#album_list') );
			//$('#tmp').html( data.html );
		}, 'json' );
	});
});
</script>

<table id="album" class="ui-table-outline ui-no-hover">
<tr>
	<th colspan="2" class="ui-state-default ui-widget-header"><?=_("Existing albums")?></th>
</tr>
<tr>
	<td colspan="2"><div><?=_("Adding images is done using the")?> <a href="<?=FORMPREFIX?>/filemanager/cd/home/storage/pictures"><?=_("filemanager")?></a></div></td>
</tr>
<tr>
	<td><div id="album_list" /></td>
	<td><div id="album_edit_area" /></td>
</tr>
<tr></tr>
<tr>
	<td class="buttons" colspan="2">
		<input id="add" type="button" value="<?=_('Create empty album')?>" />
		<span class='user-group-comment' colspan='2'>(<?=_('Drag and drop to move images/albums')?>)</span>
	</td>
</tr>

</table>
<div id="tmp"></div>
