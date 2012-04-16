<?php
function write_ini_file( $filename, $array )
{
    function enquote($key, $val) {
        if(is_string($val)) {
            return "$key = \"" . addslashes($val) . "\"";
        } else {
            return "$key = $val";
        }
    }
    $res = array();
    foreach( $array as $key => $val )
    {
        if( is_array($val) ) {
            $res[] = "[$key]";
            foreach($val as $skey => $sval) {
                if( is_array( $sval ) ) {
                    foreach($sval as $sskey => $ssval) {
                        $res[] = "  " . enquote("$$sskey\[\]", $$ssval);
                    }
                } else {
                    $res[] = "  " . enquote("$$skey", $$sval);
                }
            }
        } else {
            $res[] = enquote($key, $val);
        }
    }
    file_put_contents( $filename, implode("\n",$res), LOCK_EX );
}
