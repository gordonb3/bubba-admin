/*
    
    bubba-networkmanager - http://www.excito.com/
    
    Sockios.cpp - this file is part of bubba-networkmanager.
    
    Copyright (C) 2009 Tor Krill <tor@excito.com>
    
    bubba-networkmanager is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2
    as published by the Free Software Foundation.
    
    bubba-networkmanager is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    version 2 along with libeutils; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
    
    $Id$
*/

#include "Sockios.h"

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <strings.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#include <libeutils/FileUtils.h>

using namespace EUtils;

#include <stdexcept>
#include <algorithm>

using namespace std;

namespace NetworkManager{

namespace Sockios{

static string sockaddrtostring(struct sockaddr* address){
	string ret;
	switch(address->sa_family){
	case AF_UNIX :
		throw runtime_error("Currently unsupported network class AF_UNIX");
		break;
	case AF_INET:
	{
		struct sockaddr_in *addr=(struct sockaddr_in*)address;
		return inet_ntoa(addr->sin_addr);
		break;
	}
	case AF_INET6:
		throw runtime_error("Currently unsupported network class AF_INET6");
		break;
	case AF_UNSPEC:
		throw runtime_error("Currently unsupported network class AF_UNSPEC");
		break;
	default:
		throw runtime_error("Unknown network class");
	}
	return ret;
}

static bool stringtoipv4(const string& src, struct in_addr *dst){
	return inet_pton(AF_INET,src.c_str(),dst)>0;
}

static bool stringtoipv6(const string& src, struct in6_addr *dst){
	return inet_pton(AF_INET6,src.c_str(),dst)>0;
}

static bool set_address(int sock, const string& ifname, const string& address){
	struct ifreq req;
	bool ret=true;
	struct sockaddr_in *ipaddr=(struct sockaddr_in *)&req.ifr_addr;

	bzero(&req,sizeof(struct ifreq));
	sprintf(req.ifr_name,"%s",ifname.c_str());
	req.ifr_addr.sa_family=AF_INET;

	if((ret=stringtoipv4(address,&(ipaddr->sin_addr)))){
		if(ioctl(sock,SIOCSIFADDR,&req)<0){
			throw runtime_error("Ioctl failed "+string(strerror(errno)));
		}
	}

	return ret;
}

static string get_address(int sock, const string& ifname){
	int ret;
	struct ifreq req;

	bzero(&req,sizeof(struct ifreq));
	sprintf(req.ifr_name,"%s",ifname.c_str());

	if((ret=ioctl(sock,SIOCGIFADDR,&req))<0){
		return "";
	}

	return sockaddrtostring(&req.ifr_addr);
}

static bool set_broadcast(int sock, const string& ifname, const string& address){
	struct ifreq req;
	bool ret=true;
	struct sockaddr_in *ipaddr=(struct sockaddr_in *)&req.ifr_broadaddr;

	bzero(&req,sizeof(struct ifreq));
	sprintf(req.ifr_name,"%s",ifname.c_str());
	req.ifr_addr.sa_family=AF_INET;

	if((ret=stringtoipv4(address,&(ipaddr->sin_addr)))){
		if(ioctl(sock,SIOCSIFBRDADDR,&req)<0){
			throw runtime_error("Ioctl failed "+string(strerror(errno)));
		}
	}

	return ret;
}



static string get_broadcast(int sock, const string& ifname){
	int ret;
	struct ifreq req;

	bzero(&req,sizeof(struct ifreq));
	sprintf(req.ifr_name,"%s",ifname.c_str());

	if((ret=ioctl(sock,SIOCGIFBRDADDR,&req))<0){
		return "";
	}

	return sockaddrtostring(&req.ifr_broadaddr);
}

static bool set_netmask(int sock, const string& ifname, const string& address){
	struct ifreq req;
	bool ret=true;
	struct sockaddr_in *ipaddr=(struct sockaddr_in *)&req.ifr_netmask;

	bzero(&req,sizeof(struct ifreq));
	sprintf(req.ifr_name,"%s",ifname.c_str());
	req.ifr_addr.sa_family=AF_INET;

	if((ret=stringtoipv4(address,&(ipaddr->sin_addr)))){
		if(ioctl(sock,SIOCSIFNETMASK,&req)<0){
			throw runtime_error("Ioctl failed "+string(strerror(errno)));
		}
	}

	return ret;
}


static string get_netmask(int sock, const string& ifname){
	int ret;
	struct ifreq req;

	bzero(&req,sizeof(struct ifreq));
	sprintf(req.ifr_name,"%s",ifname.c_str());

	if((ret=ioctl(sock,SIOCGIFNETMASK,&req))<0){
		return "";
	}

	return sockaddrtostring(&req.ifr_netmask);
}

static short get_flags(int sock, const string& ifname){
	int ret;
	struct ifreq req;

	bzero(&req,sizeof(struct ifreq));
	sprintf(req.ifr_name,"%s",ifname.c_str());

	if((ret=ioctl(sock,SIOCGIFFLAGS,&req))<0){
		throw runtime_error("Ioctl failed "+string(strerror(errno)));
	}

	return req.ifr_flags;

}

static bool set_mtu(int sock, const string& ifname, int mtu){
	struct ifreq req;
	bool ret=true;

	bzero(&req,sizeof(struct ifreq));
	sprintf(req.ifr_name,"%s",ifname.c_str());
	req.ifr_mtu=mtu;

	if(ioctl(sock,SIOCSIFMTU,&req)<0){
		throw runtime_error("Ioctl failed "+string(strerror(errno)));
	}

	return ret;
}

static int get_mtu(int sock, const string& ifname){
	int ret;
	struct ifreq req;

	bzero(&req,sizeof(struct ifreq));
	sprintf(req.ifr_name,"%s",ifname.c_str());

	if((ret=ioctl(sock,SIOCGIFMTU,&req))<0){
		throw runtime_error("Ioctl failed "+string(strerror(errno)));
	}

	return req.ifr_mtu;
}

static bool set_metric(int sock, const string& ifname, int metric){
	struct ifreq req;
	bool ret=true;

	bzero(&req,sizeof(struct ifreq));
	sprintf(req.ifr_name,"%s",ifname.c_str());
	req.ifr_metric=metric;

	if(ioctl(sock,SIOCSIFMETRIC,&req)<0){
		cout << "Failed to set metric: "<<strerror(errno)<<endl;
		// Might not be supported, thus dont fail
	}

	return ret;
}

static int get_metric(int sock, const string& ifname){
	int ret;
	struct ifreq req;

	bzero(&req,sizeof(struct ifreq));
	sprintf(req.ifr_name,"%s",ifname.c_str());

	if((ret=ioctl(sock,SIOCGIFMETRIC,&req))<0){
		throw runtime_error("Ioctl failed "+string(strerror(errno)));
	}

	return req.ifr_metric;
}

static bool set_flags(int sock,const string& ifname, const Json::Value& value){

	bool ret=true;

	// Read out current setting
	short flags=get_flags(sock,ifname);

	if(value.isMember("up")){
		flags=value["up"].asBool()?(flags|IFF_UP):(flags&~IFF_UP);
	}

	if(value.isMember("running")){
		flags=value["running"].asBool()?(flags|IFF_RUNNING):(flags&~IFF_RUNNING);
	}

	if(value.isMember("broadcast")){
		flags=value["broadcast"].asBool()?(flags|IFF_BROADCAST):(flags&~IFF_BROADCAST);
	}

	if(value.isMember("loopback")){
		flags=value["loopback"].asBool()?(flags|IFF_LOOPBACK):(flags&~IFF_LOOPBACK);
	}

	if(value.isMember("p2p")){
		flags=value["p2p"].asBool()?(flags|IFF_POINTOPOINT):(flags&~IFF_POINTOPOINT);
	}

	if(value.isMember("promisc")){
		flags=value["promisc"].asBool()?(flags|IFF_PROMISC):(flags&~IFF_PROMISC);
	}

	if(value.isMember("allmulti")){
		flags=value["allmulti"].asBool()?(flags|IFF_ALLMULTI):(flags&~IFF_ALLMULTI);
	}

	if(value.isMember("multicast")){
		flags=value["multicast"].asBool()?(flags|IFF_MULTICAST):(flags&~IFF_MULTICAST);
	}

	struct ifreq req;

	bzero(&req,sizeof(struct ifreq));
	sprintf(req.ifr_name,"%s",ifname.c_str());
	req.ifr_flags=flags;

	if(ioctl(sock,SIOCSIFFLAGS,&req)<0){
		throw runtime_error("Ioctl failed "+string(strerror(errno)));
	}

	return ret;
}


Json::Value GetConfig(const string& device){
	Json::Value ret(Json::objectValue);

	int sock=socket(AF_INET,SOCK_DGRAM,0);
	try{

		ret["name"]=device;

		short flags=get_flags(sock,device);
		ret["flags"]["up"]=(flags&IFF_UP)?true:false;
		ret["flags"]["running"]=(flags&IFF_RUNNING)?true:false;
		ret["flags"]["broadcast"]=(flags & IFF_BROADCAST )?true:false;
		ret["flags"]["loopback"]=(flags & IFF_LOOPBACK )?true:false;
		ret["flags"]["p2p"]=(flags & IFF_POINTOPOINT )?true:false;
		ret["flags"]["promisc"]=(flags & IFF_PROMISC )?true:false;
		ret["flags"]["allmulti"]=(flags & IFF_ALLMULTI )?true:false;
		ret["flags"]["multicast"]=(flags & IFF_MULTICAST )?true:false;
		ret["mtu"]=get_mtu(sock,device);
		ret["metric"]=get_metric(sock,device);

		if(ret["flags"]["running"].asBool()){
			ret["address"]=get_address(sock,device);
			ret["broadcast"]=get_broadcast(sock,device);
			ret["netmask"]=get_netmask(sock,device);
		}
	}catch(runtime_error& err){
		ret["error"]=err.what();
	}
	return ret;
}

bool SetConfig(const string& device, const Json::Value& value){
	bool ret=true;
	int sock=socket(AF_INET,SOCK_DGRAM,0);

	try {

		if(value.isMember("address")){
			ret=set_address(sock,device,value["address"].asString());
		}

		if(ret && value.isMember("broadcast")){
			ret=set_broadcast(sock,device,value["broadcast"].asString());
		}

		if(ret && value.isMember("netmask")){
			ret=set_netmask(sock,device,value["netmask"].asString());
		}

		if(ret && value.isMember("mtu")){
			ret=set_mtu(sock,device,value["mtu"].asInt());
		}

		if(ret && value.isMember("metric")){
			ret=set_metric(sock,device,value["metric"].asInt());
		}

		if(ret && value.isMember("flags")){
			ret=set_flags(sock,device,value["flags"]);
		}


	}catch(runtime_error& err){
		cerr << "Failed to set: "<<err.what()<<endl;
		ret=false;
	}
	return ret;
}

static string trimsys(const string& in){
	return in.substr(15);
}

list<string> GetInterfaceNames(){
	list<string> ret=FileUtils::Glob("/sys/class/net/*");

	transform(ret.begin(),ret.end(),ret.begin(),trimsys);

	return ret;
}

}


}
