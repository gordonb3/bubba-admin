<?php
function _system( /* $command, $args... */ ) {
    $shell_cmd = escapeshellargs( func_get_args() );
	exec( $shell_cmd , $output, $retval );
	if( $retval == 0 ) {
		return $output;
	} else {
		return null;
	}
}

function escapeshellargs( $cmd ) {
    if(!is_array($cmd)) {
        $cmd = func_get_args();
    }
	$command = array_shift( $cmd );
	$shell_cmd = implode(
		' ',
		array(
			$command,
			implode( 
				' ',
				array_map( 
					'escapeshellarg', 
					$cmd 
				)
			)
		)
    );
    return $shell_cmd;
}

function invoke_rc_d( $name, $action ) {
    $cmd = array(
        "/sbin/rc-service", 
        "-q", 
        $name, 
        $action
    );
    exec( escapeshellargs( $cmd ), $output, $retval );
    return $retval == 0;
}

function update_rc_d( $name, $action="defaults", $priority=0, $runlevel=0) {
	if($action == "enable") {
		$cmd = array(
			"/sbin/rc-update",
			"-q",
			"add",
			$name,
			"default"
		);
	} elseif($action == "disable") {
		$cmd = array(
			"/sbin/rc-update",
			"-q",
			"del",
			$name,
			"default"
		);
	} else {
		$cmd = array(
			"/sbin/rc-update",
			"-q",
			$action,
			$name,
			"default"
		);
	}
	exec( escapeshellargs( $cmd ), $output, $retval );
	return $retval == 0;
}
