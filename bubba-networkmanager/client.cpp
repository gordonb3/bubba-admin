/*
 * =====================================================================================
 *
 *       Filename:  client.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  11/09/2009 02:52:13 PM CET
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:   (), 
 *        Company:  
 *
 * =====================================================================================
 */

#include <libeutils/NetClient.h>
#include <string>
#include <iostream>
#include <libeutils/json/json.h>

int main( int argc, char** argv ) {

    if ( geteuid() != 0 ) {
		std::cerr << "You must be root to run application" << std::endl;
        return 1;
    }

	if( argc <= 1 ) {
		std::cerr << "Requires a command" << std::endl;
		return 1;
	}
	std::string command("/usr/sbin/bubba-networkmanager --socket /tmp/bubba-networkmanager.sock --config /etc/bubba-networkmanager.conf");
	std::string socket("/tmp/bubba-networkmanager.sock");
	EUtils::NetClient client( command, socket );
	Json::Reader reader;
	Json::FastWriter writer;
	std::string cmd, value;
	cmd = argv[1];

	Json::Value query(Json::objectValue);
	if( argc == 2 ) {
		if( cmd == "getlanif" ) {
			query["cmd"] = "getlanif";
		} else if( cmd == "getwanif" ) {
			query["cmd"] = "getwanif";
		} else {
			std::cerr << "wrong command: " << cmd << std::endl;
			return 1;
		}
	} else if( argc == 3 ) {
		value = argv[2];
		if( cmd == "setlanif" ) {
			query["cmd"] = "setlanif";
			query["lanif"] = value;
		} else if( cmd == "setwanif" ) {
			query["cmd"] = "setwanif";
			query["wanif"] = value;
		} else {
			std::cerr << "wrong command: " << cmd << std::endl;
			return 1;
		}
	}

	client.WriteLine(writer.write(query));
	std::string returned = client.ReadLine();
	Json::Value json;
	if( reader.parse( returned, json ) ) {
		if( json["status"].asBool() ) {
			if( cmd == "getlanif" ) {
				std::cout << json["lanif"].asString() << std::endl;
			} else if( cmd == "getwanif" ) {
				std::cout << json["wanif"].asString() << std::endl;
			}
			return 0;
		} else {
			std::cerr << json.asString() << std::endl;
			return 1;
		}
	} else {
		std::cerr << reader.getFormatedErrorMessages() << std::endl;
		return 2;
	}
}
