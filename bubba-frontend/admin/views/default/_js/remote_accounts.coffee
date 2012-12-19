$ ->
  reload = ->
    cb = (accounts) ->
      $('#accounts tbody tr').remove()
      return unless accounts
      for account in accounts
        html = """
        <tr>
          <td>#{account.type}</td>
          <td>#{account.username}</td>
          <td>
            <button class="submit account-remove">#{_("Remove")}</button>
          </td>
        </tr>
        """
        $node = $(html)
        $node.data('account_data', account)
        $('#accounts tbody').append($node)


    $.post "#{config.prefix}/ajax_settings/get_remote_accounts", {}, cb, 'json'

  reload()

  $("#add-new").click (e) ->
    $dialog = $('#create-account').clone().removeAttr 'id'
    open_cb = =>
      $dialog.find('form').ajaxForm
        dataType: 'json'
        success: (data) ->
          if data.error == 1
            alert data.html
            return
          reload()
          $dia.dialog 'close'

    options =
      width: 600
      minWidth: 600
      minHeight: 300
      resizable: true
      position: ["center", 200]
      open: open_cb
    $dia = $.dialog($dialog, "", null, options)

  $('.account-remove').live 'click', (e) ->
    e.stopPropagation()
    account = $(@).closest("tr").data("account_data")
    $.confirm _("Are you sure you want to permanently remove this remote account?"), _("Remove remote account"), [
      text: _("Remove remote account")
      click: ->
        cb = (data) =>
          $.throbber.hide()
          reload()
          $(@).dialog "close"

        $.throbber.show()
        $.post "#{config.prefix}/ajax_settings/remove_remote_account", type: account.type, username: account.username, cb, "json"
    ]
    false
