<h2 class="fn-dialog-header"><?=_("Setup wizard")?></h2>
<form id="my-form">

    <div id="fn-wizard-step-2" class="step">
        <h3><?=_("Set timezone (step 2 of 5)")?></h3>
        <select name="timezone" id="fn-wizard-timezone">
            <?foreach($timezones as $country => $region):?>
            <option
            id="timezone-option-<?=$country?>"
            value="<?=$region?"$region/$country":"$country"?>"
            <?if($current_timezone == $country || $current_timezone == "$region/$country"):?>
            selected="selected"
            <?endif?>
            >
            <?if($region):?>
            <?=sprintf("%s (%s)", str_replace('_',' ',$country), str_replace('_',' ',$region))?>
            <?else:?>
            <?=str_replace('_',' ',$country)?>
            <?endif?>
            </option>
            <?endforeach?>
        </select>
    </div>

    <div id="fn-wizard-step-3" class="step">
        <h3><?=_("Change Administrator password (step 3 of 5)")?></h3>
        <p>
            <em>
                <?=_("It's highly recommended to change the admin password for security reasons")?>
            </em>
        </p>
        <table>

            <tr class="ui-header">
                <td>
                    <label for="fn-wizard-admin-password1"><?=_("Password")?>:</label>
                </td>
                <td>
                    <input type="password" name="admin_password1" id="fn-wizard-admin-password1"/>
                </td>
            </tr>

            <tr class="ui-header">
                <td>
                    <label for="fn-wizard-admin-password2"><?=_("Confirm password")?>:</label>
                </td>
                <td>
                    <input type="password" name="admin_password2" id="fn-wizard-admin-password2"/>
                </td>
            </tr>

        </table>

    </div>

    <div id="fn-wizard-step-4" class="step">
        <h3><?=_("Create user (step 4 of 5)")?></h3>
        <p>
            <em>
                <?=sprintf(_("For day-to-day operations it's recommended to use a regular user. You can later add more users for every person using the %s"), NAME)?>
            </em>
        </p>

        <table>

            <tr class="ui-header">
                <td>
                    <label for="fn-wizard-user-username"><?=_("User name")?>:</label>
                </td>
                <td>
                    <input type="text" name="username" id="fn-wizard-user-username"/>
                </td>
            </tr>

            <tr class="ui-header">
                <td>
                    <label for="fn-wizard-user-realname"><?=_("Real name")?>:</label>
                </td>
                <td>
                    <input type="text" name="realname" id="fn-wizard-user-realname"/>
                </td>
            </tr>

            <tr class="ui-header">
                <td>
                    <label for="fn-wizard-user-password1"><?=_("Password")?>:</label>
                </td>
                <td>
                    <input type="password" name="password1" id="fn-wizard-user-password1"/>
                </td>
            </tr>

            <tr class="ui-header">
                <td>
                    <label for="fn-wizard-user-password2"><?=_("Confirm password")?>:</label>
                </td>
                <td>
                    <input type="password" name="password2" id="fn-wizard-user-password2"/>
                </td>
            </tr>

        </table>

    </div>

    </form>
