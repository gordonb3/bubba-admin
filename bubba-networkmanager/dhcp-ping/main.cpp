#include <iostream>
#include <stdexcept>

#include <syslog.h>
#include <stdlib.h>

#include <libeutils/POpt.h>
#include <libeutils/Thread.h>

#include "dhcp.h"

static const char* appversion=PACKAGE_VERSION;
static const char* builddate=__DATE__ " " __TIME__;

using namespace std;
using namespace EUtils;


class DhcpPing: public Thread{
private:
	string iface;
	int timeout;
	bool done;
	bool result;
public:
	DhcpPing(string iface, int timeout):Thread(),iface(iface),timeout(timeout),done(false){
	}

	virtual void Run(){
		int sock=create_raw_socket(iface.c_str());
		if(sock<0){
			result=false;
			done=true;
			return;
		}
		if(wait_offer(sock,timeout)){
			syslog(LOG_DEBUG,"Found server");
			result=true;
		}else{
			syslog(LOG_DEBUG,"No server found");
			result=false;
		}
		close(sock);
		done=true;
	}
	
	bool Ping(){
		this->Start();
		this->Yield();
		int sock=create_broadcast_socket(iface.c_str());
		if(sock>=0){
			send_discover(sock,iface.c_str());
			close(sock);
		}
		while(!this->done){
			usleep(1000);
		}
	
		return result;
	}
	
	virtual ~DhcpPing(){
	}
		
	
};


int main(int argc, char** argv){

    openlog( "dhcpping", LOG_PERROR,LOG_USER);

	POpt p;

	p.AddOption(Option("interface",'i',Option::String,"Interface to use","eth0","eth0"));
	p.AddOption(Option("timeout",'t',Option::Int,"Timeout waiting for reply (Seconds)","1","1"));
    p.AddOption( Option( "version",'v',Option::None,"Show version","","false" ) );
    p.AddOption( Option( "debug",'d',Option::Int,"Set debug level, (0-7)","5","5" ) );

	if ( !p.Parse( argc,argv ) ) {
    	syslog( LOG_ERR,"Failed to parse arguments use %s -? for info",argv[0] );
        return 1;
	}

	if ( p["version"]=="true" ) {
    	cerr << "Version: "<< appversion<<endl;
        cerr << "Built  : "<< builddate<<endl;
        return 0;
	}

	int loglevel=atoi( p["debug"].c_str() );
	setlogmask( LOG_UPTO( loglevel ) );

	string interface=p["interface"];
	int timeout=atoi(p["timeout"].c_str());
	bool result=false;
	try{
		DhcpPing dping(interface,timeout);

		result=dping.Ping();
	}catch(runtime_error& err){

	}

	if(!result){
		syslog(LOG_DEBUG,"wait for offer failed");
	}else{
		syslog(LOG_DEBUG,"Dhcp server found");
	}

	return result?0:1;
}
