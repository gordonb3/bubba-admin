<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

class BubbaSocketException extends Exception {
    const NO_SOCKET = 1;
	public function __construct($message, $code = 0) {
		$message .= " (" . socket_strerror($code) . ")";
		parent::__construct($message, $code);
	}
}

class BubbaSocket {
	public static $cout = array();
	public static $count = 0;
	private $_SOCKET;
	private $_ERROR;
	private $_OUTPUT;
	private $_RETVAL;
	const READ_MAX = 4096;

	public function __construct( $cmd, $args, $socket ) {
		set_time_limit(120);
		BubbaSocket::$count++;
		BubbaSocket::$cout[] = &$this->_OUTPUT;

		$this->_SOCKET = socket_create( AF_UNIX, SOCK_STREAM, 1);

		socket_set_nonblock( $this->_SOCKET );
		if(!@socket_connect( $this->_SOCKET, $socket )){
            if( $cmd === false ) {
                throw new BubbaSocketException("Failed to connect to socket and no shell command was given", BubbaSocketException::NO_SOCKET);
            }

			$shell_cmd = implode(
				' ',
				array(
					$cmd,
					implode(
						' ',
						array_map(
							'escapeshellarg',
							$args
						)
					)
				)
			);


			exec( $shell_cmd, $this->_OUTPUT, $this->_RETVAL );

			if( $this->_RETVAL != 0 ) {
				throw new Exception("failed to execute $shell_cmd", $this->_RETVAL );
			}

			$error = NULL;

			while(!@socket_connect( $this->_SOCKET, $socket )) {
				$error = socket_last_error();

				if (
					$error != SOCKET_ECONNREFUSED &&
					$error != SOCKET_EINPROGRESS &&
					$error != SOCKET_EALREADY &&
					$error !=  SOCKET_ENOENT
				) {
					$this->_ERROR = "Error Connecting Socket: ".socket_strerror($error);
					syslog( LOG_ERR, $this->_ERROR );
					socket_close($this->_SOCKET);
					return NULL;
				}

				usleep(4000);
			}

			socket_set_block($this->_SOCKET);
		}
	}

	public function say( $msg ) {
		$msg .= "\r\n";
		if( false === @socket_write( $this->_SOCKET, $msg ) ) {
			throw new BubbaSocketException( "Failed to write to socket", socket_last_error( $this->_SOCKET ) );
		}
	}

	public function getline( $max = BubbaSocket::READ_MAX ) {
		return socket_read( $this->_SOCKET, $max, PHP_NORMAL_READ );
	}

	public function close() {
		socket_set_block($this->_SOCKET);
		socket_set_option($this->_SOCKET, SOL_SOCKET, SO_LINGER, array('l_onoff' => 1, 'l_linger' => 1) );
		socket_close( $this->_SOCKET );
	}

	public function get_output() {
		return $this->_OUTPUT;
	}

	public static function dump() {
		$str = '';
		foreach( BubbaSocket::$cout as $c ) {
			if( is_array( $c ) ) {
				$str .= join( "<br/>\n", $c );
			} else {
				$str .= $c;
			}
		}
		print("<pre>Count: ".BubbaSocket::$count."</pre>");
		print("<pre>$str</pre>");
	}
}
class BubbaAptSocket extends BubbaSocket {
	function __construct() {
		parent::__construct( false, false, '/var/run/bubba-aptd.sock' );
	}
}

class OldBubbaAptSocket extends BubbaSocket {
	function __construct() {
		parent::__construct(
			'/opt/bubba/bin/updatebackend.pl',
			array(
				'--mode=ithreads',
				'--daemonize'
			),
			'/run/bubba-apt.socket'
		);
	}
}

class OldBubbaHotfixSocket extends BubbaSocket {
	function __construct() {
		parent::__construct( '/opt/bubba/bin/hotfix.pl', array( '--mode=ithreads', '--daemonize' ), '/run/bubba-hotfix.socket' );
	}
}

class BubbaNetworkManagerSocket extends BubbaSocket {
	function __construct() {
		parent::__construct(
			'/opt/bubba/sbin/bubba-networkmanager',
			array(
				'--socket', '/run/bubba-networkmanager.sock',
				'--config', '/etc/bubba/networkmanager.conf'
			),
			'/run/bubba-networkmanager.sock'
		);
	}
}

class BubbaAlbumSocket extends BubbaSocket {
	function __construct() {
		parent::__construct( '/opt/bubba/bin/album_import.pl', array( '--debug', '--mode=ithreads', '--daemonize' ), '/run/bubba-album.socket' );
	}
}

class BubbaDiskSocket extends BubbaSocket {
	function __construct() {
		parent::__construct( '/opt/bubba/bin/diskdaemon.pl', array( '--debug', '--mode=ithreads', '--daemonize' ), '/run/bubba-disk.socket' );
	}
}
function apt_query_progress() {
	$sock = new BubbaAptSocket();
	$sock->say( json_encode( array( 'action' => 'query_progress' ) ) );
	$ret = $sock->getline();
	$sock->close();
	return $ret;
}

function apt_upgrade_packages() {
	$sock = new BubbaAptSocket();
    $sock->say( json_encode( array( 'action' => 'upgrade_packages') ) );
	$ret = $sock->getline();
	$sock->close();
	return $ret;
}

function apt_install_package( $package ) {
	$sock = new BubbaAptSocket();
	$sock->say( json_encode( array( 'action' => 'install_package', 'package' => $package ) ) );
	$ret = $sock->getline();
	$sock->close();
	return $ret;
}

function old_apt_query_progress() {
	$sock = new OldBubbaAptSocket();
	$sock->say( json_encode( array( 'action' => 'query_progress' ) ) );
	$ret = $sock->getline();
	$sock->close();
	return $ret;
}

function old_apt_upgrade_packages() {
	$sock = new OldBubbaAptSocket();
	$sock->say( json_encode( array( 'action' => 'upgrade_packages' ) ) );
	$ret = $sock->getline();
	$sock->close();
	return $ret;
}

function old_apt_install_package( $package ) {
	$sock = new OldBubbaAptSocket();
	$sock->say( json_encode( array( 'action' => 'install_package', 'package' => $package ) ) );
	$ret = $sock->getline();
	$sock->close();
	return $ret;
}

function old_hotfix_query_progress() {
	$sock = new OldBubbaHotfixSocket();
	$sock->say( json_encode( array( 'action' => 'query_progress' ) ) );
	$ret = $sock->getline();
	$sock->close();
	return $ret;
}

function old_hotfix_run( ) {
	$sock = new OldBubbaHotfixSocket();
	$sock->say( json_encode( array( 'action' => 'run' ) ) );
	$ret = $sock->getline();
	$sock->close();
	return $ret;
}

function query_network_manager( array $what ) {
		$socket = new BubbaNetworkManagerSocket();
		$socket->say(json_encode( $what ));
		$data = json_decode( $socket->getline(), true );
		$socket->close();
		return $data;
}
