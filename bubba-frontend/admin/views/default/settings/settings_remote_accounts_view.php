<table id="accounts" class="ui-table-outline">
  <thead>
    <tr>
      <th colspan="4" class="ui-state-default ui-widget-header"><?=_("Remote accounts")?></th>
    </tr>
    <tr class="ui-header">
      <th><?=_("Type")?></th>
      <th><?=_("Username")?></th>
      <th>&nbsp;</th>
    </tr>
  </thead>
  <tbody>
  </tbody>
  <tfoot>
    <tr>
      <td colspan="3">
        <button class="submit" id="add-new"><?=_("Add new remote account")?></button>
      </td>
    </tr>
  </tfoot>
</table>

<div id="templates" class="ui-helper-hidden">

  <div id="create-account">
    <h3><?=_("Define a new remote target")?></h3>
    <form action="<?=FORMPREFIX?>/ajax_settings/new_remote_account" method="post">
      <table>
        <tr>
          <td>
            <select id="type" name="type">
              <option value="HiDrive">HiDrive</option>
              <option value="Google Drive">Google Drive</option>
            </select>
          </td>
        </tr>
        <tr>
          <td>
            <label for="username"><?=_("Username)")?></label>
            <input type="text" id="username" name="username"/>
          </td>
        </tr>
        <tr>
          <td>
            <label for="password"><?=_("Password)")?></label>
            <input type="password" id="password" name="password"/>
          </td>
        </tr>
        <tr>
          <td>
            <label for="sshkey"><?=_("OpenSSH key)")?></label>
            <textarea id="sshkey" name="sshkey"></textarea>
          </td>
        </tr>
        <tr>
          <td>
            <input type="submit" value="<?=_("Create target")?>" />
          </td>
        </tr>
      </table>
    </form>
  </div>
 </div>
