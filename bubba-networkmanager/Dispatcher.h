/*

    bubba-networkmanager - http://www.excito.com/

    Dispatcher.h - this file is part of bubba-networkmanager.

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
#ifndef DISPATCHER_H_
#define DISPATCHER_H_

#include <libeutils/NetDaemon.h>
#include <libeutils/json/json.h>

#include <map>

using namespace std;

class Dispatcher: public EUtils::NetDaemon {
public:
	/**
	 * Result of request
	 */
	enum Result{
		/**
		 * Operation failed
		 */
		Failed,
		/**
		 * Operation completed succesfully
		 */
		Done,
		/**
		 * Operation still in progress (Spawned)
		 * Don't decrement usage
		 */
		Spawned
	};
private:
	Json::Reader reader;
	Json::FastWriter writer;
	//typedef (void (Dispatcher::*)(const Json::Value& v)) MemFun;
	map<string,Result (Dispatcher::*)(EUtils::UnixClientSocket* con,const Json::Value& v)> cmds;

	// Actions
	Result test(EUtils::UnixClientSocket* con,const Json::Value& v);
	Result getwanif(EUtils::UnixClientSocket* con,const Json::Value& v);
	Result getlanif(EUtils::UnixClientSocket* con,const Json::Value& v);
	Result setwanif(EUtils::UnixClientSocket* con,const Json::Value& v);
	Result setlanif(EUtils::UnixClientSocket* con,const Json::Value& v);
	Result getinterfaces(EUtils::UnixClientSocket* con,const Json::Value& v);
	Result getwlanif(EUtils::UnixClientSocket* con,const Json::Value& v);


	Result getifcfg(EUtils::UnixClientSocket* con,const Json::Value& v);

	Result getdefaultroute(EUtils::UnixClientSocket* con,const Json::Value& v);
	Result getroutes(EUtils::UnixClientSocket* con,const Json::Value& v);

	Result getnameservers(EUtils::UnixClientSocket* con,const Json::Value& v);
	Result setnameservers(EUtils::UnixClientSocket* con,const Json::Value& v);


	Result getmtu(EUtils::UnixClientSocket* con,const Json::Value& v);
	Result setmtu(EUtils::UnixClientSocket* con,const Json::Value& v);

	// Ethernet
	Result setstatic(EUtils::UnixClientSocket* con,const Json::Value& v);
	Result setdynamic(EUtils::UnixClientSocket* con,const Json::Value& v);
	Result setraw(EUtils::UnixClientSocket* con,const Json::Value& v);

	Result ifup(EUtils::UnixClientSocket* con,const Json::Value& v);
	Result ifdown(EUtils::UnixClientSocket* con,const Json::Value& v);
	Result ifrestart(EUtils::UnixClientSocket* con,const Json::Value& v);

	// AP functions
	Result setapcfg(EUtils::UnixClientSocket* con,const Json::Value& v);
	Result setapif(EUtils::UnixClientSocket* con,const Json::Value& v);

	Result setapssid(EUtils::UnixClientSocket* con,const Json::Value& v);
	Result setapmode(EUtils::UnixClientSocket* con,const Json::Value& v);
	Result setapchannel(EUtils::UnixClientSocket* con,const Json::Value& v);

	Result setapauthnone(EUtils::UnixClientSocket* con,const Json::Value& v);
	Result setapauthwep(EUtils::UnixClientSocket* con,const Json::Value& v);
	Result setapauthwpa(EUtils::UnixClientSocket* con,const Json::Value& v);
	// Todo: this is a kludge...
	Result haswlan(EUtils::UnixClientSocket* con,const Json::Value& v);


	static void sighandler(int sig);
protected:

	Result handle_request(EUtils::UnixClientSocket* con, Json::Value& v);
	void send_jsonvalue(EUtils::UnixClientSocket* con, Json::Value& v);
public:
	Dispatcher(const string& sockpath, int timeout=0);

	virtual void Dispatch(EUtils::UnixClientSocket* con);

	virtual ~Dispatcher();
};

#endif /* DISPATCHER_H_ */
