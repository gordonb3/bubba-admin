$ ->

  $form = $ "#backup-create"

  dialogs = {}

  update_backup_jobs_table = ->
    cb = (jobs) ->
      table = $ "#backup-jobs tbody"
      table.empty()

      row = $ '<tr/>', class: 'backup-job-entry'

      for job in jobs
        data = $.extend {failed: false, running: false}, job
        $cur = row.clone().appendTo table
        $cur.data 'job_info', data
        $cur.append $('<td/>', text: data.label)
        $cur.append $('<td/>', text: data.schedule)

        if data.hasrun and data.status isnt 0
          msg = "Rsync returned error #{data.status}: #{switch data.status
            when 1 then _ "Syntax or usage error"
            when 2 then _ "Protocol incompatibility"
            when 3 then _ "Errors selecting input/output files, dirs"
            when 4 then _ "Requested action not supported: an attempt was made to manipulate
 64-bit files on a platform that cannot support them; or an option was specified
 that is supported by the client and not by the server."
            when 5 then _ "Error starting client-server protocol"
            when 6 then _ "Daemon unable to append to log-file"
            when 10 then _ "Error in socket I/O"
            when 11 then _ "Error in file I/O"
            when 12 then _ "Error in rsync protocol data stream"
            when 13 then _ "Errors with program diagnostics"
            when 14 then _ "Error in IPC code"
            when 20 then _ "Received SIGUSR1 or SIGINT"
            when 21 then _ "Some error returned by waitpid()"
            when 22 then _ "Error allocating core memory buffers"
            when 23 then _ "Partial transfer due to error"
            when 24 then _ "Partial transfer due to vanished source files"
            when 25 then _ "The --max-delete limit stopped deletions"
            when 30 then _ "Timeout in data send/receive"
            when 35 then _ "Timeout waiting for daemon connection"
          }"

          $node = $("<td/>",
            class: "ui-backup-job-failed"
          ).appendTo $cur

          $node.append _("Failed") + " ("
          $("<a/>",
            href: "#"
            html: _("why?")
            click: (e) ->
              e.preventDefault()
              $.alert msg, $.sprintf(_("Backup job for %s on %s failed"), data.username, data.type), null, null,
                width: 500
                height: 300

              false
          ).appendTo $node
          $node.append ")"
        else
          $cur.append $("<td/>",
            text: if data.hasrun then _("Completed") else _("Not yet run")
          )

        $cur.append $('<td/>', html: $('<button/>', class: "submit backup-job-remove",html: _("Remove") ))
      $(".backup-job-entry").first().trigger "click"

    $.post "#{config.prefix}/ajax_backup/get_backup_jobs", {}, cb, 'json'

  update_backup_jobs_table()

  reload_possible_targets = (cur) ->
    cb = (targets) ->
      $('#destinations-table tbody tr').remove()
      for {id, type, username, host} in targets.remote
        html = """
        <tr>
          <td>
          <input type="radio" name="destination" id="destination-remote-#{id}" value="remote-#{id}"/>
          <label for="destination-remote-#{id}">#{if type is 'ssh' then "ssh://#{host}" else type} (#{username})</label>
          </td>
        </tr>
        """
        $('#destinations-table').append(html)

      for {label, uuid} in targets.local
        html = """
        <tr>
          <td>
          <input type="radio" name="destination" id="destination-local-#{uuid}" value="local-#{uuid}"/>
          <label for="destination-local-#{uuid}">#{_("External disk")} (#{label})</label>
          </td>
        </tr>
        """
        $('#destinations-table').append(html)

      if cur
        $("#destination-remote-#{cur}").attr('checked', 'checked')

    $.post "#{config.prefix}/ajax_backup/get_possible_targets", {}, cb, 'json'

  make_create_dialog = ->


    step_show_cb = (e, data) =>
      buttons = dialogs.create.dialog("widget").find(".ui-dialog-buttonset button")
      buttons.button "enable"
      if data.isLastStep
        buttons.eq(0).text $form.formwizard("option", "textSubmit")
      else
        buttons.eq(1).button "disable" if data.isFirstStep
      buttons.eq(0).text $form.formwizard("option", "textNext")

    $form.live('step_shown', step_show_cb)


    open_cb = (event, ui) =>

      reload_possible_targets()

      $form.trigger "reset"
      $form.formwizard "update_steps"
      buttons = dialogs.create.dialog("widget").find(".ui-dialog-buttonset button")
      buttons.eq(1).button "disable"
      buttons.eq(0).click ->
        buttons.button "disable"  if $form.formwizard("option", "validationEnabled") and not $form.validate().numberOfInvalids()

      buttons.eq(1).click ->
        buttons.button "disable"
      $form.find(".primary-field").focus()

    options =
      width: 600
      minWidth: 600
      minHeight: 300
      resizable: true
      position: ["center", 200]
      autoOpen: false
      open: open_cb

    buttons = [
      text: _("Next")
      class: "ui-next-button ui-element-width-50"
      click: -> $form.formwizard "next"
    ,
      text: _("Back")
      class: "ui-prev-button ui-element-width-50"
      click: -> $form.formwizard "back"
    ]

    dialogs['create'] = $.dialog($("#backup-create-dialog"), "", buttons, options)
    $("#backup-create-dialog").submit -> false

  make_create_dialog()

  $form.formwizard
    resetForm: true
    historyEnabled: not true
    validationEnabled: true
    formPluginEnabled: true
    disableUIStyles: true
    next: false
    textNext: _("Next")
    textBack: _("Back")
    textSubmit: _("Complete")
    showBackOnFirstStep: true
    validationOptions:
      rules:
        selection:
          required: true
        destination:
          required: true
        schedule:
          required: true
    formOptions:
      url: "#{config.prefix}/ajax_backup/create"
      type: "post"
      dataType: "json"
      reset: true
      beforeSubmit: ->
        $.throbber.show()
      success: (data) ->
        $.throbber.hide()
        dialogs.create.dialog "close"
        update_backup_jobs_table()

  $('select[name=type]').live 'change', ->
    switch $(@).val()
      when 'ssh'
        $('input[name=host]').closest('tr').show()
      else
        $('input[name=host]').closest('tr').hide()

  $("#backup-job-add").click ->
    $form.formwizard "reset"
    dialogs.create.dialog "open"

  $('#add-new-target').live 'click', ->
    $dialog = $('#create-target').clone().removeAttr 'id'
    open_cb = =>
      $dialog.find('select[name=type]').change()
      $dialog.find('form').ajaxForm
        dataType: 'json'
        beforeSubmit: ->
          $.throbber.show()
        success: (data) ->
          $.throbber.hide()
          if data.error == 1
            alert data.html
            return

          reload_possible_targets(data.key)
          $dia.dialog 'close'
          if data.uuid and data.type is not 'ssh'
            txt = switch data.type
              when 'HiDrive'
                _ """Please click <a href="%s/ajax_settings/get_remote_account_pubkey/%s">here</a> to download the openssh key needed for backup. Upload it to <a target="_blank" href="https://hidrive.strato.com/">HiDrive</a> under Account → Settings → Account management → OpenSSH key"""
              else
                _ """Please click <a href="%s/ajax_settings/get_remote_account_pubkey/%s">here</a> to download the openssh key needed for backup"""
            html = $.sprintf txt, config.prefix, data.uuid
            $.alert html

    options =
      width: 600
      minWidth: 600
      minHeight: 300
      resizable: true
      position: ["center", 200]
      open: open_cb
    $dia = $.dialog($dialog, "", null, options)


  $(".backup-job-remove").live "click", (e) ->
    e.stopPropagation()
    data = $(this).closest("tr").data("job_info")
    $.confirm _("Are you sure you want to permanently remove this backup job?"), _("Remove backup job"), [
      text: _("Remove backup job")
      click: ->
        cb = (data) =>
          if data.error
            update_status false, data.html
          else
            update_status true, _("Backup job was removed from the system")
          $.throbber.hide()
          update_backup_jobs_table()
          $(@).dialog "close"

        $.throbber.show()
        $.post "#{config.prefix}/ajax_backup/remove", type: data.type, target: data.target, schedule: data.schedule, selection: data.selection, cb, "json"
      id: "backup-job-dialog-remove-confirm-button"
    ]
    false
