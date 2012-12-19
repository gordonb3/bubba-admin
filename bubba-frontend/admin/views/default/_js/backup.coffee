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

        if data.failed
          $node = $("<td/>",
            class: "ui-backup-job-failed"
          ).appendTo $cur

          $node.append _("Failed") + " ("
          $("<a/>",
            href: "#"
            html: _("why?")
            click: (e) ->
              e.preventDefault()
              $.alert data.status, $.sprintf(_("Backup job for %s on %s failed"), data.username, data.type), null, null,
                width: 500
                height: 300

              false
          ).appendTo $node
          $node.append ")"
        else
          $cur.append $("<td/>",
            text: data.status
          )

        $cur.append $('<td/>', html: $('<button/>', class: "submit backup-job-remove",html: _("Remove") ))
      $(".backup-job-entry").first().trigger "click"

    $.post "#{config.prefix}/ajax_backup/get_backup_jobs", {}, cb, 'json'

  update_backup_jobs_table()

  reload_possible_targets = (cur) ->
    cb = (targets) ->
      $('#destinations-table tbody tr').remove()
      for {id, type, username} in targets.remote
        html = """
        <tr>
          <td>
          <input type="radio" name="destination" id="destination-remote-#{id}" value="remote-#{id}"/>
          <label for="destination-remote-#{id}">#{type} (#{username})</label>
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
      success: (data) ->
        $.throbber.hide()
        dialogs.create.dialog "close"
        update_backup_jobs_table()

  $("#backup-job-add").click ->
    $form.formwizard "reset"
    dialogs.create.dialog "open"

  $('#add-new-target').live 'click', ->
    $dialog = $('#create-target').clone().removeAttr 'id'
    open_cb = =>
      $dialog.find('form').ajaxForm
        dataType: 'json'
        success: (data) ->
          if data.error == 1
            alert data.html
            return
          reload_possible_targets(data.key)
          $dialog.dialog 'close'

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
