// Generated by CoffeeScript 1.10.0
$(function() {
  var reload;
  reload = function() {
    var cb;
    cb = function(accounts) {
      var $node, account, html, i, len, results;
      $('#accounts tbody tr').remove();
      if (!accounts) {
        return;
      }
      results = [];
      for (i = 0, len = accounts.length; i < len; i++) {
        account = accounts[i];
        html = "<tr>\n  <td>" + (account.type === 'ssh' ? "ssh://" + account.host : account.type) + "</td>\n  <td>" + account.username + "</td>\n  <td><a href=\"" + config.prefix + "/ajax_settings/get_remote_account_pubkey/" + account.uuid + "\" class=\"pubkey\">" + (_("public key")) + "</a></td>\n  <td>\n    <button class=\"submit account-remove\">" + (_("Remove")) + "</button>\n    <button class=\"submit account-edit\">" + (_("Edit")) + "</button>\n  </td>\n</tr>";
        $node = $(html);
        $node.data('account_data', account);
        results.push($('#accounts tbody').append($node));
      }
      return results;
    };
    return $.post(config.prefix + "/ajax_settings/get_remote_accounts", {}, cb, 'json');
  };
  reload();
  $("#add-new").click(function(e) {
    var $dia, $dialog, open_cb, options;
    $dialog = $('#create-account').clone().removeAttr('id');
    open_cb = (function(_this) {
      return function() {
        var form;
        $dialog.find('select[name=type]').change();
        form = $dialog.find('form');
        form.validate({
          rules: {
            username: {
              required: true
            },
            password: {
              required: true
            }
          }
        });
        return form.ajaxForm({
          dataType: 'json',
          beforeSubmit: function(arr, $form, options) {
            return $.throbber.show();
          },
          success: function(data) {
            var html, txt;
            $.throbber.hide();
            if (data.error === 1) {
              alert(data.html);
              return;
            }
            reload();
            $dia.dialog('close');
            if (data.uuid && data.type !== 'ssh') {
              txt = (function() {
                switch (data.type) {
                  default:
                    return _("Please click <a href=\"%s/ajax_settings/get_remote_account_pubkey/%s\">here</a> to download the openssh key needed for backup");
                }
              })();
              html = $.sprintf(txt, config.prefix, data.uuid);
              return $.alert(html);
            }
          }
        });
      };
    })(this);
    options = {
      width: 600,
      minWidth: 600,
      minHeight: 300,
      resizable: true,
      position: ["center", 200],
      open: open_cb
    };
    return $dia = $.dialog($dialog, "", null, options);
  });
  $('.account-edit').live('click', function(e) {
    var $dia, $dialog, account, open_cb, options;
    e.stopPropagation();
    account = $(this).closest("tr").data("account_data");
    $dialog = $('#edit-account').clone().removeAttr('id');
    open_cb = (function(_this) {
      return function() {
        var form;
        form = $dialog.find('form');
        form.find('input.username').val(account.username);
        if (account.type === 'ssh') {
          form.find('input[name=host]').val(account.host);
          form.find('input[name=username]').val(account.username);
          form.find('input[name=password]').attr('disabled', 'disabled').closest('tr').hide();
          form.validate({
            rules: {
              username: {
                required: true
              },
              host: {
                required: true
              }
            }
          });
        }
        return form.ajaxForm({
          data: {
            id: account.id
          },
          dataType: 'json',
          beforeSubmit: function(arr, $form, options) {
            if (!form.valid()) {
              return false;
            }
            return $.throbber.show();
          },
          success: function(data) {
            $.throbber.hide();
            if (data.error === 1) {
              alert(data.html);
              return;
            }
            reload();
            return $dia.dialog('close');
          }
        });
      };
    })(this);
    options = {
      width: 600,
      minWidth: 600,
      minHeight: 300,
      resizable: true,
      position: ["center", 200],
      open: open_cb
    };
    return $dia = $.dialog($dialog, "", null, options);
  });
  $('.account-remove').live('click', function(e) {
    var account;
    e.stopPropagation();
    account = $(this).closest("tr").data("account_data");
    $.confirm(_("Are you sure you want to permanently remove this remote account?"), _("Remove remote account"), [
      {
        text: _("Remove remote account"),
        click: function() {
          var cb;
          cb = (function(_this) {
            return function(data) {
              $.throbber.hide();
              reload();
              return $(_this).dialog("close");
            };
          })(this);
          $.throbber.show();
          return $.post(config.prefix + "/ajax_settings/remove_remote_account", {
            host: account.host,
            type: account.type,
            username: account.username
          }, cb, "json");
        }
      }
    ]);
    return false;
  });
  return $('select[name=type]').live('change', function() {
    switch ($(this).val()) {
      case 'ssh':
        return $('input[name=host]').closest('tr').show();
      default:
        return $('input[name=host]').closest('tr').hide();
    }
  });
});
