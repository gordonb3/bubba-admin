$.validator.addMethod('valid_username', function(value, element, params) {
    return value.length === 0 || (/^[^\-][a-z0-9 _\-]+$/.test(value) && value != 'web' && value != 'storage' && value != 'root');
},
jQuery.format("not a valid username"));

wizard = null;
button_spec = [{
    'text': _("Next"),
    'class': 'ui-next-button ui-element-width-50',
    'click': function() {
        if (wizard) {
            wizard.formwizard('next');
        }
    }
},
{
    'text': _("Back"),
    'class': 'ui-prev-button ui-element-width-50',
    'click': function() {
        if (wizard) {
            wizard.formwizard('back');
        }
    }
}];

function do_run_wizard() {
    wizard_element = $('<div/>');

    wizard_dialog = $.dialog(wizard_element, "", button_spec, {
        'width': 600,
        'height': 400,
        'resizable': false,
        'position': 'center',
        'close': function(event, ui) {
            $.post(config.prefix + "/wizard/mark_dirty");
        }
    });
    var buttons = $(".ui-dialog-buttonset button"); // cache the buttons
    buttons.eq(1).button("disable"); // disable the back button (on the first step)
    wizard_dialog.dialog('open');
    buttonpane = wizard_dialog.dialog('widget').children('.ui-dialog-buttonpane');
    //buttonpane.find('.ui-prev-button').hide();
    wizard_element.load(config.prefix + "/wizard/get_languages", function() {

        buttonpane.find('.ui-next-button').one('click', function() {

            selected_language = $('#fn-wizard-language option:selected').val();
            wizard_element.load(config.prefix + "/wizard/get_wizard", {
                language: selected_language
            },
            function() {
                wizard = wizard_element.children("form");

                // FIXME should be global option, taken from main
                var iCheckbox_options = {
                    switch_container_src: config.prefix + '/views/' + config.theme + '/_img/bubba_switch_container.png',
                    class_container: 'ui-icon-bubba-switch-container',
                    class_switch: 'ui-icon-bubba-switch',
                    switch_speed: 50,
                    switch_swing: -65,
                    checkbox_hide: true,
                    switch_height: 21,
                    switch_width: 127
                };

                wizard_element.find('.slide').iCheckbox(iCheckbox_options);

                wizard_dialog.bind('dialogclose', function(event, ui) {
                    wizard_dialog.remove();
                });

                buttons.eq(0).click(function() { // when Next is clicked
                    if (wizard.formwizard("option", "validationEnabled") && !wizard.validate().numberOfInvalids()) { // if statement needed if validation is enabled
                        buttons.button("disable"); // disable the buttons to prevent double click
                    }
                });

                buttons.eq(1).click(function() { // when Back is clicked
                    buttons.button("disable"); // disable the buttons to prevent double click
                });

                wizard.bind("step_shown", function(e, data) { // when a step is shown..
                    buttons.button("enable"); // enable the dialog buttons
                    if (data.isLastStep) { // if last step
                        buttons.eq(0).text(wizard.formwizard("option", "textSubmit")); // change text of the button to 'Submit' and return
                    } else if (data.isFirstStep) { // if first step
                        buttons.eq(1).button("disable"); // disable the Back button
                    }
                    buttons.eq(0).text(wizard.formwizard("option", "textNext")); // set the text of the Next button to 'Next'
                });

            });

        });

    });

}
