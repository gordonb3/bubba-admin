<script type="text/javascript" src="<?=FORMPREFIX.'/views/'.THEME?>/_js/jquery.validate.js?v='<?=$this->session->userdata('version')?>'"></script>

<script  type="text/javascript">

wlan_configurable=<?=$wlan_configurable?"true":"false"?>;
bands=<?=json_encode($bands)?>;
current_mode=<?=json_encode($current_mode)?>;
current_band=<?=json_encode($current_band)?>;
current_channel=<?=json_encode($current_channel)?>;
capabilities=<?=json_encode($capabilities)?>;
labels = {
	legacy_2: <?=json_encode(_("Legacy mode (802.11a)"))?>,
	legacy_1: <?=json_encode(_("Legacy mode (802.11g)"))?>,
	mixed_2: <?=json_encode(_("Mixed mode (802.11n + 802.11a)"))?>,
	mixed_1: <?=json_encode(_("Mixed mode (802.11n + 802.11g)"))?>

};

</script>

<script  type="text/javascript">
$(document).ready( function() {

  if( wlan_configurable ) {
      $("#wLANCFG :disabled").removeAttr("disabled");
      $("#wLANCFG .ui-icon-bubba-switch").removeClass("ui-state-disabled");
	  if( ! capabilities["RX_GF"] ) {
		  $("#mode_greenfield").attr("disabled","disabled");
	  }
  } else {
	  // we dont want to do anything
	  return;
  }

	// hide advanced settings
	$("#wlan_adv_mark").html("+");
	$("#wlan_adv").css("display","none");


	$.validator.addMethod('wep', function(value, element, params) {
		return value.length == 5 || value.length == 13;
	});
	$.validator.addMethod('gf_mode', function(value, element) {
		if( value == "greenfield" && ! capabilities["RX_GF"] ) {
			return false;
		}
		return true;
	}, "Current hardware doesn't support Greenfield mode");

	validator = $('#wLANCFG').validate({
		rules: {
			'ssid' : {
				required: true,
				maxlength: 32
			},
			'mode' : {
				gf_mode: true
			}
		},
		messages: {
			'ssid': {
				required: "Please enter an SSID (Service Set Identifier)",
				maxlength: jQuery.format("SSID has a maximum length of 32 characters")
			},
			'mode': {
				gf_mode: jQuery.format("Current hardware doesn't support Greenfield mode")
			}
		}
	});

    $("select#channel").change(function() {
        current_channel = $(this).val();
    });

	$("input[name='band']").change(function() {
        band = $(this).val();
        $("option#mode_legacy").empty().html(labels["legacy_" + band])
        $("option#mode_mixed").empty().html(labels["mixed_" + band])
		current_selected_channel = current_channel;
		sel=$("select#channel");
		sel.empty();

        for( i = 0; i < bands[band].length; ++i ) {
            cur = bands[band][i];
            if( cur["disabled"] == "true" || cur["radar_detection"] == "true" || cur["passive_scanning"] == "true" ) {
                continue;
            }
            opt = $("<option/>");
            opt.val(cur["channel"]);
            opt.html("<?=_("Channel")?> " + cur["channel"] + " (" + cur["freq"] + " MHz)");
            if( cur["channel"] == current_channel ) {
                opt.attr("selected", "selected");
            }
            sel.append(opt);
        }
	});
	$("input#band"+current_band+"").change();

	$('select#encryption').change(function() {
		$('#password').rules("remove");
		if($(this).val() == 'none') {
			$('#password').attr('disabled', 'disabled');
		} else {
			$('#password').removeAttr('disabled');
			if($(this).val() == 'wep') {
				$('#password').rules("add", {
					required: true,
					wep: true,
					messages: {
						required: "Please enter an WEP key",
						wep: jQuery.format("WEP key needs to be 5 or 13 characters")
					}

				});
			} else {
				$('#password').rules("add", {
					required: true,
					rangelength: [8, 63],
					messages: {
						required: "Please enter an WPA key",
						rangelength: jQuery.format("WPA key needs to be between {0} and {1} characters")
					}
				});
			}
		}
	});
	$('select#mode').change(function() {
		switch( $(this).val() ) {
		case "legacy":
			$("select#width").val("20");
			$("select#width").attr("disabled", "disabled");
			break;
		case "mixed":
			$("select#width").removeAttr("disabled");
			break;
		case "greenfield":
			$("select#width").val("40");
			$("select#width").attr("disabled", "disabled");
			break;
		}
	});

	$('select#encryption').trigger('change');
	$('select#mode').trigger('change');
});
</script>
