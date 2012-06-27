$(function(){
    var dialogs = {};

    var dialog_options = {
        'create': {
            'width': 600,
            //            'height': 500,
            'minWidth': 600,
            'minHeight': 300,
            'resizable': true,
			'position': ['center',200]
		},
        'edit': {
            'width': 600,
            //            'height': 500,
            'minWidth': 600,
            'minHeight': 300,
            'resizable': true,
			'position': ['center',200]
        }
    };

    var dialog_buttons = {
        'create': [
            {
                'text': _("Next"),
				'class': 'ui-next-button ui-element-width-50',
				'click': function() {
					if($("#fn-backup-create")) {
						$("#fn-backup-create").formwizard('next');
					}
				}
            },
            {
                'text': _("Back"),
				'class': 'ui-prev-button ui-element-width-50',
				'click': function() {
					if($("#fn-backup-create")) {
						$("#fn-backup-create").formwizard('back');
					}
				}
            }
        ],
        'edit': [
            {
                'text': _("Next"),
				'class': 'ui-next-button ui-element-width-50',
				'click': function() {
					if($("#fn-backup-edit")) {
						$("#fn-backup-edit").formwizard('next');
					}
				}
            },
            {
                'text': _("Back"),
				'class': 'ui-prev-button ui-element-width-50',
				'click': function() {
					if($("#fn-backup-edit")) {
						$("#fn-backup-edit").formwizard('back');
					}
				}
            }
        ]

    };

    button_labels = {
        'create': pgettext('button', 'Create'),
        'edit': pgettext('button', 'edit')
    };

    var dialog_onclose = {
    };

	var dialog_pre_open_callbacks = {
		create : function(){ // when the dialog is opened
			var buttons = dialogs.create.dialog('widget').find(".ui-dialog-buttonset button"); // cache the buttons
			buttons.eq(1).button("disable"); // disable the back button (on the first step)
			buttons.eq(0).click(function(){ // when Next is clicked
				if($("#fn-backup-create").formwizard("option", "validationEnabled") && !$("#fn-backup-create").validate().numberOfInvalids()){ // if statement needed if validation is enabled
					buttons.button("disable"); // disable the buttons to prevent double click
				}
			});

			buttons.eq(1).click(function(){ // when Back is clicked
				buttons.button("disable"); // disable the buttons to prevent double click
			});

			$("#fn-backup-create").bind("step_shown", function(e,data){ // when a step is shown..
				buttons.button("enable"); // enable the dialog buttons

				if(data.isLastStep){ // if last step
					buttons.eq(0).text($("#fn-backup-create").formwizard("option","textSubmit")); // change text of the button to 'Submit' and return
				}else if(data.isFirstStep){ // if first step
					buttons.eq(1).button("disable"); // disable the Back button
				}
				buttons.eq(0).text($("#fn-backup-create").formwizard("option","textNext")); // set the text of the Next button to 'Next'
			});
		},
		edit : function(){ // when the dialog is opened
			var buttons = dialogs.edit.dialog('widget').find(".ui-dialog-buttonset button"); // cache the buttons
			buttons.eq(1).button("disable"); // disable the back button (on the first step)
			buttons.eq(0).click(function(){ // when Next is clicked
				if($("#fn-backup-edit").formwizard("option", "validationEnabled") && !$("#fn-backup-edit").validate().numberOfInvalids()){ // if statement needed if validation is enabled
					buttons.button("disable"); // disable the buttons to prevent double click
				}
			});

			buttons.eq(1).click(function(){ // when Back is clicked
				buttons.button("disable"); // disable the buttons to prevent double click
			});

			$("#fn-backup-edit").bind("step_shown", function(e,data){ // when a step is shown..
				buttons.button("enable"); // enable the dialog buttons

				if(data.isLastStep){ // if last step
					buttons.eq(0).text($("#fn-backup-edit").formwizard("option","textSubmit")); // change text of the button to 'Submit' and return
				}else if(data.isFirstStep){ // if first step
					buttons.eq(1).button("disable"); // disable the Back button
				}
				buttons.eq(0).text($("#fn-backup-edit").formwizard("option","textNext")); // set the text of the Next button to 'Next'
			});
		}
	};

    var dialog_callbacks = {
        'create': function() {
        },
        'edit': function() {
        }
    };

    var update_available_devices = function() {
        $.throbber.show();
        var $select = $('.fn-backup-target-device');
        $select.empty();
        $.post(
            config.prefix + "/ajax_backup/get_available_devices",
            {},
            function(data) {
                $.each(data.disks, function(label, partitions) {
                    var $group = $('<optgroup/>', {'text': label}).appendTo($select);
                    $.each(partitions, function() {
                        var partition = this;
                        $('<option/>', {'value': partition.uuid, 'html': partition.label}).appendTo($group);
                    });

                });
                $.throbber.hide();
            },
            "json"
        );

    };

    var update_backup_job_information = function(job, runs) {
        var table = $("#fn-backup-job-runs tbody");
        table.empty();
        var row = $("<tr/>");
        $.each(runs, function() {
            var data = $.extend({failed: false, running: false},this);
            var $row = row.clone();
            $row.appendTo(table);
            $row.append($('<td/>',{text: data.date}));
            $row.append($('<td/>').append( $('<button/>', {
                'class' : "submit fn-job-restore",
                html: _("Restore")
            }).
            attr('disabled', data.failed ? 'disabled' : '').
            toggleClass('disabled', data.failed)));
            $row.data('job',job);
            $row.data('date', data.date);
        });
    };

    var update_backup_jobs_table = function() {
        $.post( config.prefix + "/ajax_backup/get_backup_jobs", {},
            function(jobs) {
                var table = $("#fn-backup-jobs tbody");
                table.empty();
                var row = $("<tr/>", { 'class': 'fn-backup-job-entry'});
                $.each( jobs, function() {
                    var data = $.extend({failed: false, running: false},this);
                    var $cur = row.clone();
                    $cur.data('job', data.name);
                    $cur.appendTo(table);
                    $cur.append($('<td/>',{text: data.name}));
                    $cur.append($('<td/>',{text: data.target}));
                    $cur.append($('<td/>',{text: data.schedule}));
					if( data.failed ) {
						$node = $('<td/>',{'class': 'ui-backup-job-failed'}).appendTo($cur);

						$node.append(_("Failed") + ' (');
						$('<a/>', {
							'href': '#',
							'html': _("why?"),
							'click': function(e){
								e.preventDefault();
								$.alert(
									data.status,
									$.sprintf(_("Backup job %s failed"), data.name),
									null,
									null,
									{'width': 500, 'height': 300}
								);
								return false;
							}
						}).appendTo($node);
						$node.append(')');
					} else {
						$cur.append($('<td/>',{text: data.status}));
					}

                    $cur.append($('<td/>').append($("<div/>", {'class': 'ui-inline'}).append(
                        $('<button/>', {
                            'class' : "submit fn-job-remove",
                            html: _("Remove")
                        }),
                        $('<button/>', {
                            'class' : "submit fn-job-edit",
                            html: _("Edit")
                        }),
                        $('<button/>', {
                            'class' : "submit fn-job-run",
                            html: _("Run now")
                        }))
                        )
                        );

                    $cur.find("button").attr('disabled', data.running ? 'disabled' : '').
                    toggleClass('disabled', data.running);

                }
			);

			// Trigger first entry in list. Bug #1698
			$('.fn-backup-job-entry').first().trigger('click');
            },
            "json"
        );
    };

    update_backup_jobs_table();

    $.each(['create', 'edit'], function(index, value) {

        if (typeof dialog_options[value] == "undefined") {
            dialog_options[value] = {};
        }

        var options = $.extend({},
            dialog_options[value], {
                "autoOpen": false,
                "open": function(event, ui) {
                    var current = $("#fn-backup-" + value + "");
                    current.trigger("reset");
                    if (typeof dialog_pre_open_callbacks[value] != "undefined") {
                        dialog_pre_open_callbacks[value].apply(this, arguments);
                    }
                    $(".fn-primary-field", current).focus();
                }
            }
        );
        var buttons;
        if (dialog_buttons[value]) {
            buttons = dialog_buttons[value];
        } else {
            buttons = [{
                'text': button_labels[value],
                'click': function() {
                    dialog_callbacks[value].apply(dialogs[value], arguments);
                },
				id: 'fn-' + value + '-dialog-button',
				'class': 'ui-element-width-100'
            }];
        }
        if( dialog_onclose[value] ) {
            options.close = dialog_onclose[value];
        }
        dialogs[value] = $.dialog(
            $("#fn-backup-" + value + "-dialog"), "", buttons, options
        );

        $("#fn-backup-" + value + "-dialog").submit(function() {
            dialog_callbacks[value].apply(dialogs[value]);

            return false;
        });
    });


	jQuery.validator.addMethod("alnum", function(value, element, params) {
		return this.optional(element) || /^[a-z0-9]+$/i.test(value);
	}, _("Only alphanumreric values are allowed for the name"));

	jQuery.validator.addMethod("hostname", function(value, element, params) {
        return this.optional(element) ||
        /^([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\-]{0,61}[a-zA-Z0-9])(\.([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\-]{0,61}[a-zA-Z0-9]))*$/i.test(value) ||
        /^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$/.test(value);
    }, _("Please enter a valid hostname or IPv4 address"));

    $("#fn-backup-create").formwizard(
		{
			resetForm: true,
			historyEnabled: !true,
			validationEnabled: true,
			formPluginEnabled: true,
			disableUIStyles: true,
			next: false,
			textNext: _("Next"),
			textBack: _("Back"),
			textSubmit: _("Complete"),
			showBackOnFirstStep: true,
			validationOptions: {
				'rules': {
					'name': {
						'alnum': true,
						'required': true,
						'remote': {
							'type': 'POST',
							'url': config.prefix + "/ajax_backup/validate"
						}
					},
					'target-device': {
						'required': function(element) {
							return $('#fn-backup-create-protocol option:selected').val() == 'file';
						}
					},
					'target-hostname': {
						'hostname': true,
						'required': function(element) {
							return $('#fn-backup-create-protocol option:selected').val() != 'file';
						}
					},
					'target-username': {
						'required': function(element) {
							return $('#fn-backup-create-protocol option:selected').val() != 'file';
						}
					},
					'target-password': {
						'required': function(element) {
							return $('#fn-backup-create-protocol option:selected').val() != 'file';
						}
					},
					'selection': {
						'required': true
					},
					'protocol': {
						'required': true
					},
					'schedule-type': {
						'required': true
					},
					'security-password': {
						'required': function(element) {
							return $('#fn-backup-create-security-enable').is(':checked');
						}
					},
					'security-password2': {
						'equalTo': '#fn-backup-create-security-password'
					}

				},
				'messages': {
					'name': {
						'remote': jQuery.format("{0} is already in use")
					}
				}
			},
			formOptions: {
				'url': config.prefix + "/ajax_backup/create",
				'type': 'post',
				'dataType': 'json',
				'beforeSubmit': function(arr, $form, options) {
					var $custom = $('#fn-backup-create-selection-custom-selection');
					$.each(
						$custom.data('selection'),
						function() {
							arr.push({'name': 'dirs[]', 'value': this});
						}
					);
					console.log(arr);
					//               $.throbber.show();

					return true;
				},
				'reset': true,
				'success': function( data ) {
					$.throbber.hide();
					dialogs.create.dialog('close');
					update_backup_jobs_table();
				}
			}
		}
		).bind('step_shown', function(event, data) {
			if(data.isBackNavigation) {
				if( data.currentStep === "fn-backup-create-form-step-4" ) {
					$('.fn-backup-schedule').change();
				}
			} else {
				switch( data.currentStep ) {
				case "fn-backup-create-form-step-2":
					$("#fn-backup-create-selection-custom-browse").button('disable');
					break;
				case "fn-backup-create-form-step-3":
					$('#fn-backup-create-protocol').change();
					break;
				case "fn-backup-create-form-step-4":
					$('.fn-backup-schedule').change();
					break;
				case "fn-backup-create-form-step-5":
					$("#fn-backup-create-security-enable").change();
					break;
				}
			}
		}
	);

    $("#fn-backup-edit").formwizard(
		{
			resetForm: !true,
			historyEnabled: !true,
			validationEnabled: true,
			formPluginEnabled: true,
			disableUIStyles: true,
			next: false,
			textNext: _("Next"),
			textBack: _("Back"),
			textSubmit: _("Complete"),
			showBackOnFirstStep: true,
			validationOptions: {
				'rules': {
					'target-device': {
						'required': function(element) {
							return $('#fn-backup-edit-protocol option:selected').val() == 'file';
						}
					},
					'target-hostname': {
						'hostname': true,
						'required': function(element) {
							return $('#fn-backup-edit-protocol option:selected').val() != 'file';
						}
					},
					'target-username': {
						'required': function(element) {
							return $('#fn-backup-edit-protocol option:selected').val() != 'file';
						}
					},
					'target-password': {
						'required': function(element) {
							return $('#fn-backup-edit-protocol option:selected').val() != 'file';
						}
					},
					'selection': {
						'required': true
					},
					'protocol': {
						'required': true
					},
					'schedule-type': {
						'required': true
					},
					'security-password': {
						'required': function(element) {
							return $('#fn-backup-edit-security-enable').is(':checked');
						}
					},
					'security-password2': {
						'equalTo': '#fn-backup-edit-security-password'
					}

				},
				'messages': {
					'name': {
						'remote': jQuery.format(_("{0} is already in use"))
					}
				}
			},
			formOptions: {
				'url': config.prefix + "/ajax_backup/edit",
				'type': 'post',
				'dataType': 'json',
				'beforeSubmit': function(arr, $form, options) {
					var $custom = $('#fn-backup-edit-selection-custom-selection');
					$.each(
						$custom.data('selection'),
						function() {
							arr.push({'name': 'dirs[]', 'value': this});
						}
					);
					console.log(arr);
					//               $.throbber.show();

					return true;
				},
				'reset': !true,
				'success': function( data ) {
					$.throbber.hide();
					dialogs.edit.dialog('close');
					update_backup_jobs_table();
				}
			}
		}
		).bind('step_shown', function(event, data) {
			if(data.isBackNavigation) {
				if( data.currentStep ===  "fn-backup-edit-form-step-4" ) {
					$('.fn-backup-schedule').change();
				}
			} else {
				switch( data.currentStep ) {
				case "fn-backup-edit-form-step-2":
					$("#fn-backup-edit-selection-custom-browse").button('disable');
					break;
				case "fn-backup-edit-form-step-3":
					$('#fn-backup-edit-protocol').change();
					break;
				case "fn-backup-edit-form-step-4":
					$('.fn-backup-schedule').change();
					break;
				case "fn-backup-edit-form-step-5":
					$("#fn-backup-edit-security-enable").change();
					break;
				}
			}
		}
	);


    $("#fn-backup-job-add").click(function(){
        $("#fn-backup-create").formwizard('reset');

        $('#fn-backup-create-selection-custom-selection').data('selection', []);
        dialogs.create.dialog("open");
    });

    $(".fn-job-edit").live('click', function(e){
        $.post(config.prefix + '/ajax_backup/get_job_info', { 'name': $(this).closest('tr').data('job') },
        function(data){
            e.stopPropagation();
            data = $.extend({
                'schedule_type': 'weekly',
                'selection_type': 'custom',
                'target_protocol': 'ftp',
                'target_device': '',
                'target_host': '',
                'target_user': '',
                'target_FTPpasswd': '',
                'schedule_monthday': 1,
                'schedule_monthhour': 1,
                'schedule_weekday': 'Monday',
                'schedule_weekhour': 1,
                'schedule_dayhour': 1,
                'full_expiretime': '1M',
                'files': [],
                'GPG_key': ''

            }, data);
            // XXX retrieve parseable data?
            var name = data.jobname;

            $("#fn-backup-edit").formwizard('reset');
            dialogs.edit.dialog("open");

            $('#fn-backup-edit-name').val(name);

            $('#fn-backup-edit-selection-'+data.selection_type).attr('checked', 'checked');
            if(data.selection_type == 'custom') {
                $('#fn-backup-edit-selection-custom-browse').removeAttr('disabled');
            }
            $('#fn-backup-edit-selection-custom-selection').data('selection', data.files).html(data.files.join(', '));

            $('#fn-backup-edit-protocol option[value='+data.target_protocol+']').attr('selected', 'selected');
            $('#fn-backup-edit-target-device').val(data.dist_uuid);
            $('#fn-backup-edit-target-server-hostname').val(data.target_host);
            $('#fn-backup-edit-target-server-username').val(data.target_user);
            $('#fn-backup-edit-target-server-password').val(data.target_FTPpasswd);
            $('#fn-backup-edit-target-path').val(data.target_path);

            $('#fn-backup-edit-schedule-'+data.schedule_type).attr('checked', 'checked');
            $('#fn-backup-edit-schedule-monthday').val(data.schedule_monthday);
            $('#fn-backup-edit-schedule-monthhour').val(data.schedule_monthhour);
            $('#fn-backup-edit-schedule-weekday').val(data.schedule_weekday);
            $('#fn-backup-edit-schedule-weekhour').val(data.schedule_weekhour);
            $('#fn-backup-edit-schedule-dayhour').val(data.schedule_dayhour);
            $('#fn-backup-edit-schedule-timeline').val(data.full_expiretime);

            $('#fn-backup-edit-security-enable').attr('checked', data.GPG_key !== '' ? 'checked' : '');
            $('#fn-backup-edit-security-password, #fn-backup-edit-security-password2').val(data.GPG_key);
        }, 'json');

        return false;
    });

    $('.fn-job-remove').live('click', function(e) {
        e.stopPropagation();
        job = $(this).closest('tr').data('job');
        $.confirm(
            _("Are you sure you want to permanently remove this backup job?"),
            _("Remove backup job"),
            [
                {
                    'text': _("Remove backup job"),
                    'click': function(){
                        var confirm_dialog = $(this);
                        $.throbber.show();
                        $.post(
                            config.prefix + "/ajax_backup/remove",
                            { 'name': job },
                            function(data){
                                if( data.error ) {
                                    update_status( false, data.html );
                                } else {
                                    update_status(
                                        true,
                                        _("Backup job was removed from the system")
                                    );
                                }
                                $.throbber.hide();
                                update_backup_jobs_table();
                                confirm_dialog.dialog('close');
                            }, 'json'
                        );

                    },
                    id: 'fn-backup-job-dialog-remove-confirm-button'
                }
            ]
        );
        return false;
    });

    $('.fn-job-run').live('click', function(e) {
        e.stopPropagation();
        job = $(this).closest('tr').data('job');
        $.confirm(
            _("Are you sure you want to run this backup job now?"),
            _("Run backup job now"),
            [
                {
                    'text': _("Run backup job"),
                    'click': function(){
                        var confirm_dialog = $(this);
                        $.throbber.show();
                        $.post(
                            config.prefix + "/ajax_backup/run",
                            { 'name': job },
                            function(data){
                                if( data.error ) {
                                    update_status( false, data.html );
                                } else {
                                    update_status(
                                        true,
                                        _("Backup job has been initialized to be executed at this particlular moment in time.")
                                    );
                                }
                                setTimeout(function(){
                                    $.throbber.hide();
                                    update_backup_jobs_table();
                                    confirm_dialog.dialog('close');
                                }, 2000);
                            }, 'json'
                        );

                    },
                    id: 'fn-backup-job-dialog-run-confirm-button'
                }
            ]
        );
        return false;
    });

    $('.fn-backup-restore-action').live('click', function(e) {
		if( $(this).val() == 'newdir' ) {
			$('#fn-backup-restore-target').removeAttr('disabled');
		} else {
			$('#fn-backup-restore-target').attr('disabled', 'disabled');
		}
	});

    $('.fn-job-restore').live('click', function(e) {
        e.stopPropagation();
        var job = $(this).closest('tr').data('job');
        var date = $(this).closest('tr').data('date');
		var $obj = $("#fn-backup-restore");
		$obj[0].reset();
		var $filemanager = $obj.find('.fn-restore-filemanager');
		var $validator = $obj.validate({
			'rules': {
				'target': {
					'required': '#fn-backup-restore-action-newdir:checked'
				},
				'selection': {
					'required': true
				}
			}
		});

        $.dialog(
            $obj,
            _("Restore backed up data"),
            [
                {
                    'text': _("Restore selected files and directories"),
					'click': function(e) {
						if( !$validator.form() ) {
							$(e.target).closest('button').button('enable');
							return false;
						}

                        var selected = $obj.find('.fn-backup-restore-selection').val();
                        var action = $obj.find('.fn-backup-restore-action:checked').val();
                        var target = $obj.find('.fn-backup-restore-target').val();
                        $(this).dialog('close');

                        $.confirm(
                            _("Are you sure? Any existing files will be overwritten."),
                            _("Confirm restore"),
                            [
                                {
                                    'text': _("Yes"),
                                    'click': function(e) {
                                        $.throbber.show();
										var self = this;
                                        $.post(config.prefix + "/ajax_backup/restore",
                                            {
                                                'name': job,
                                                'date': date,
                                                'action': action,
                                                'target': target,
                                                'selection': selected
                                            },
                                            function(data) {
                                                update_status(true, "done");
                                                $.throbber.hide();
                                                $(self).dialog('close');
                                            }, 'json');
                                    }
                                },
                                {
                                    'text': _("No"),
                                    'click': function(e) {
                                        $(this).dialog('close');
                                    }
                                }
                            ]
                        );
                    },
                    'class': 'ui-element-width-100'
                }
            ],
            {
                'width': 600,
                'Height': 400,
                'resizable': false,
                'position': ['center',200],
                modal: true,
                autoOpen: true,
                open: function() {

                    $filemanager.filemanager({
                        root: '/home',
                        animate: false,
                        dirPostOpenCallback: function(){},
                        ajaxSource: config.prefix + "/ajax_backup/get_restore_data",
                        ajaxExtraData: {'name': job, 'date': date},
						multiSelect: false,
						mouseDownCallback: function() {
							$obj.find('.fn-backup-restore-selection').val($filemanager.filemanager('getSelected')[0]);
						},
                        columns: [
                            { "sWidth": "0px", "bSortable": false, "aaSorting": [ "asc" ], "sClass": "ui-filemanager-column-type" },
                            { "sWidth": "auto", "aaSorting": [ "asc", "desc" ], "sClass": "ui-filemanager-column-name" },
                            { "sWidth": "200px", "sClass": "ui-filemanager-column-date" },
                            { "sWidth": "30px", "bSortable": false, "sClass": "ui-filemanager-column-next" }
                        ]
                    });


                    return true;
                },
                close: function() {
					$filemanager.filemanager('destroy');
                    return true;
                }
            }
        );

        return false;
    });

    $('.fn-backup-job-entry').live('click', function(){
		$('.fn-backup-job-entry').removeClass('ui-filemanager-state-selected');
		$(this).addClass('ui-filemanager-state-selected');
        var name = $(this).data('job');
        $.throbber.show(_("Retrieving data for selected backup job. Please stand by"));
        $.post(
            config.prefix + "/ajax_backup/get_backup_job_information",
            {'name': name},
            function(data) {
                update_backup_job_information(name, data);
                $.throbber.hide();
            },
            "json"
        );

    });

    $.each(['create', 'edit'], function(key,value){


        $('#fn-backup-'+value+'-selection-custom-browse').click(function(){
            $filemanager = $(".ui-custom-select-filemanager");
            $.dialog(
                $filemanager,
                _("File/Directory selector"),
                [
                    {
                        'text': pgettext("button","Select"),
                        'click': function() {
                            $(this).dialog('close');
                        },
                        'class': 'ui-element-width-100'
                    }
                ],
                {
                    'width': 600,
                    'Height': 400,
                    'resizable': false,
                    'position': ['center',600],
                    modal: !true,
                    autoOpen: true,
                    open: function() {

                        $filemanager.filemanager({
                            root: '/home/',
                            animate: false,
                            dirPostOpenCallback: function(){},
                            ajaxSource: config.prefix + "/ajax_backup/dirs"
                        });


                        dialogs[value].dialog('disable');
                       dialogs[value].dialog('widget').hide();
                        return true;
                    },
                    close: function() {
                        var selected = $filemanager.filemanager('getSelected');
                        $filemanager.filemanager("destroy");
                        $('#fn-backup-'+value+'-selection-custom-selection').text(selected.join(', ')).data('selection', selected);
                        dialogs[value].dialog('enable');
                        dialogs[value].dialog('widget').show();
                        return true;
                    }
                }
            );
	return false;
        });
        // Custom browse for selection button
        $('#fn-backup-'+value+'-selection-custom-browse').button({'disabled': true});

        $('#fn-backup-'+value+' .fn-backup-selection').change(function(){
            if( $(this).is('#fn-backup-'+value+'-selection-custom') ) {
                $('#fn-backup-'+value+'-selection-custom-browse').button('enable');
            } else {
                $('#fn-backup-'+value+'-selection-custom-browse').button('disable');
            }
        });

        $('#fn-backup-'+value+' .fn-backup-schedule').change(function(){
            var self = $('#fn-backup-'+value+' .fn-backup-schedule:checked');
            var $timeline = $('#fn-backup-'+value+'-schedule-timeline');
            var val = $('#fn-backup-'+value+'-schedule-timeline option:selected').val();
            $timeline.find('option').removeAttr('disabled');
            switch(self.attr('id')) {
            case 'fn-backup-'+value+'-schedule-hourly':
            case 'fn-backup-'+value+'-schedule-daily':
                break;
            case 'fn-backup-'+value+'-schedule-weekly':
                $timeline.find('option[value=1D]').attr('disabled', 'disabled');
                if( val == '1D' ) {
                    $timeline.val('1W');
                }
                break;
            case 'fn-backup-'+value+'-schedule-monthly':
                $timeline.find('option[value=1D]').attr('disabled', 'disabled');
                $timeline.find('option[value=1W]').attr('disabled', 'disabled');
                if( val == '1D' || val == '1W' ) {
                    $timeline.val('1M');
                }
                break;
            case 'fn-backup-'+value+'-schedule-disabled':
                $timeline.find('option').attr('disabled', 'disabled');
                break;
            }

            if( self.is('#fn-backup-'+value+'-schedule-monthly') ) {
                $('#fn-backup-'+value+'-schedule-monthday, #fn-backup-'+value+'-schedule-monthhour').removeAttr('disabled');
            } else {
                $('#fn-backup-'+value+'-schedule-monthday, #fn-backup-'+value+'-schedule-monthhour').attr('disabled', 'disabled');
            }

            if( self.is('#fn-backup-'+value+'-schedule-weekly') ) {
                $('#fn-backup-'+value+'-schedule-weekday, #fn-backup-'+value+'-schedule-weekhour').removeAttr('disabled');
            } else {
                $('#fn-backup-'+value+'-schedule-weekday, #fn-backup-'+value+'-schedule-weekhour').attr('disabled', 'disabled');
            }

            if( self.is('#fn-backup-'+value+'-schedule-daily') ) {
                $('#fn-backup-'+value+'-schedule-dayhour').removeAttr('disabled');
            } else {
                $('#fn-backup-'+value+'-schedule-dayhour').attr('disabled', 'disabled');
            }

        });

        $('#fn-backup-'+value+'-security-enable').change(function(){
            if($(this).is(':checked')) {
                $('#fn-backup-'+value+'-security-password, #fn-backup-'+value+'-security-password2').removeAttr('disabled');
            } else {
                $('#fn-backup-'+value+'-security-password, #fn-backup-'+value+'-security-password2').attr('disabled', 'disabled');
            }
        });

        $('#fn-backup-'+value+'-protocol').change(function(){
            switch( $(this).val() ) {
            case 'ftp':
            case 'ssh':
                $('#fn-backup-'+value+'-target-server-hostname').removeAttr('disabled').closest('tr').show();
                $('#fn-backup-'+value+'-target-server-username').removeAttr('disabled').closest('tr').show();
                $('#fn-backup-'+value+'-target-server-password').removeAttr('disabled').closest('tr').show();
                $('#fn-backup-'+value+'-target-device').attr('disabled', 'disabled').closest('tr').hide();
                break;
            case 'file':
                $('#fn-backup-'+value+'-target-server-hostname').attr('disabled', 'disabled').closest('tr').hide();
                $('#fn-backup-'+value+'-target-server-username').attr('disabled', 'disabled').closest('tr').hide();
                $('#fn-backup-'+value+'-target-server-password').attr('disabled', 'disabled').closest('tr').hide();
                $('#fn-backup-'+value+'-target-device').removeAttr('disabled').closest('tr').show();
                update_available_devices();
                break;
            }
        });
    });

});
