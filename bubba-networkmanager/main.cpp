/*

    bubba-networkmanager - http://www.excito.com/

    main.cpp - this file is part of bubba-networkmanager.

    Copyright (C) 2009 Tor Krill <tor@excito.com>

    bubba-networkmanager is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2
    as published by the Free Software Foundation.

    bubba-networkmanager is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    version 2 along with bubba-networkmanager if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.

    $Id$
*/

#include <iostream>
#include <fstream>
#include <libeutils/POpt.h>
#include <libeutils/Services.h>

#include <syslog.h>
#include <stdlib.h>
#include <unistd.h>

using namespace std;
using namespace EUtils;

static const char* appversion=PACKAGE_VERSION;
static const char* builddate=__DATE__ " " __TIME__;

#include "Dispatcher.h"
#include "datamodel/EthernetInterface.h"
#include "datamodel/BridgeInterface.h"
#include "datamodel/WlanInterface.h"
#include "utils/Sockios.h"
#include "utils/SysConfig.h"

using namespace NetworkManager;

int main( int argc, char** argv ) {

    openlog( "bubba-networkmanager", LOG_PERROR,LOG_DAEMON );

    POpt p;

    p.AddOption( Option( "fg",'f',Option::None,"Run in foreground, dont daemonize","","false" ) );
    p.AddOption( Option( "debug",'d',Option::Int,"Set debug level","value 0-7 (default is 5 and 7 is max)","5" ) );
    p.AddOption( Option( "version",'v',Option::None,"Show version","","false" ) );
    p.AddOption( Option( "ttl",'t',Option::Int,"time to live in seconds","60","60" ) );
    p.AddOption( Option( "pidfile", '\0', Option::String, "PID file", "/var/run/bubba-networkmanager.pid", "/var/run/bubba-networkmanager.pid" ) );
    p.AddOption( Option( "socket", '\0', Option::String, "Socket to communicate on", "/tmp/bubba-networkmanager.sock", "/tmp/bubba-networkmanager.sock" ) );
    p.AddOption( Option( "config", '\0', Option::String, "Configuration file to read from", "/etc/bubba-networkmanager.conf", "/etc/bubba-networkmanager.conf" ) );

    if ( !p.Parse( argc,argv ) ) {
        syslog( LOG_ERR,"Failed to parse arguments use %s -? for info",argv[0] );
        return 1;
    }

    if ( p["version"]=="true" ) {
        cerr << "Version: "<< appversion<<endl;
        cerr << "Built  : "<< builddate<<endl;
        return 0;
    }

    if ( Services::IsRunning( "bubba-networkmanager" ) ) {
        cerr << "An instance of bubba-networkmanager is already running"<<endl;
        cerr << "Terminating"<<endl;
        return 1;
    }

    SysConfig::ConfigFile = p["config"];
    SysConfig& cfg=SysConfig::Instance();
    cfg.Writeback();

    if ( geteuid() != 0 ) {
        syslog( LOG_ERR, "Started as non root terminating" );
        cerr << "You must be root to run application" << endl;
        return 1;
    }

    int loglevel=atoi( p["debug"].c_str() );
    setlogmask( LOG_UPTO( loglevel ) );

    if ( p["fg"]=="false" ) {
        daemon( 1,0 );
        syslog( LOG_INFO,"Daemonizing" );
        ofstream pidfile( p["pidfile"].c_str() );
        pidfile<<getpid()<<endl;
        pidfile.close();
    }

    //Remove any stale sockets
    unlink( p["socket"].c_str() );

    syslog( LOG_NOTICE,"Starting up" );
    Dispatcher d( p["socket"].c_str(),atoi(p["ttl"].c_str()) );

    d.Run();

    syslog( LOG_NOTICE,"Shutting down" );
    return 0;
}

