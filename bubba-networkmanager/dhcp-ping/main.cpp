#include <iostream>
#include <string>
#include <cstdio>
#include <stdexcept>

#include <string.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/types.h>
#include <stdlib.h>
#include <time.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>
#include <sys/select.h>
#include <syslog.h>

#include <libeutils/POpt.h>
#include <libeutils/Thread.h>

#include "dhcp.h"

using namespace std;
using namespace EUtils;

static const char* appversion=PACKAGE_VERSION;
static const char* builddate=__DATE__ " " __TIME__;

#define DHCP_SERVER_PORT 67
#define DHCP_CLIENT_PORT 68
/*
struct config{
	string interface;
	int timeout;
};
static struct config cfg;



bool get_hw_address(int sock, const string& ifnam, char hw[6]){
	struct ifreq interface;
	int ret;
	
	strncpy(interface.ifr_ifrn.ifrn_name,ifnam.c_str(),IFNAMSIZ-1);
	interface.ifr_ifrn.ifrn_name[IFNAMSIZ-1]=0;

	if((ret=ioctl(sock,SIOCGIFHWADDR,&interface))<0){
		syslog(LOG_ERR,"Unable to get hw-address from socket: %m");
	}else{
		memcpy(hw,&interface.ifr_hwaddr.sa_data,6);
	}
	
	return ret==0;
}


u_int32_t get_xid(){
	static u_int32_t xid=0;
	
	if(xid==0){
		srandom(time(NULL));
		xid=random();
		if(xid==0){
			xid++;
		}		
	}
	
	return xid;
}

bool wait_offer(int sock){
	dhcp_packet dp;

	struct timeval tv;
	tv.tv_sec=cfg.timeout;
	tv.tv_usec=0;
		
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(sock, &fds);

	int f=select(sock+1,&fds,NULL,NULL,&tv);
	if(f<0){
		syslog(LOG_ERR,"Failed to select: %m");
		return false;
	}
	
	if(f==0){
		syslog(LOG_DEBUG,"Timeout waiting for reply");
		return false;
	}	

	if(!FD_ISSET(sock,&fds)){
		syslog(LOG_ERR,"Failed waiting for reply: %m");
		return false;
	}

	struct sockaddr_in source;
	socklen_t ssize=sizeof(source);
	memset(&source,0,ssize);
	
	int rec=recvfrom(sock,(char *)&dp,sizeof(dp),0, (struct sockaddr*)&source,&ssize);
	if(rec<0){
		syslog(LOG_ERR,"Failed to read reply: %m");
		return false;
	}
	
	return true;
}

bool send_discover(int sock){
	dhcp_packet dp;
	
	memset(&dp,0,sizeof(dp));
	dp.op=BOOTREQUEST;				// Request 
	dp.htype=1;						// Ethernet 10Mb
	dp.hlen=6;						// Ethernet address size 
	dp.hops=0;						// Not really nessecery
	dp.xid=get_xid();				// random cookie
	dp.secs=100;					// how long has transaction taken
	dp.flags=htons(DHCP_BROADCAST_FLAG);	// How do we want our answer
	
	// Set our hw address
	char hwaddr[6];
	if(get_hw_address(sock,cfg.interface,hwaddr)<0){
		return false;
	}
	memcpy(dp.chaddr,hwaddr,6);
	
	// Set magic option cookie
	dp.options[0]=0x63;
	dp.options[1]=0x82;
	dp.options[2]=0x53;
	dp.options[3]=0x63;
	
	// DHCP discover option
	dp.options[4]=DHCP_OPTION_MESSAGE_TYPE;	// Type
	dp.options[5]=0x01;						// Length
	dp.options[6]=DHCPDISCOVER;				// Data

	// No more options
	dp.options[7]=DHCP_OPTION_END;
	
	// Send packet
	struct sockaddr_in addr;
	addr.sin_family=AF_INET;
	addr.sin_port=htons(DHCP_SERVER_PORT);
	addr.sin_addr.s_addr=INADDR_BROADCAST;
	memset(&addr.sin_zero,0,sizeof(&addr.sin_zero));
	
	if(sendto(sock,(const char*)&dp,sizeof(dp),0,(const struct sockaddr *) &addr,sizeof(addr))<0){
		syslog(LOG_ERR,"Failed to send dhcp discover packet: %m");
		return false;
	}
	
	return true;
}
	
	

int create_socket(void){
	int sock;
	
	// Create socket
	if((sock=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP))<0){
		syslog(LOG_ERR,"Failed to create socket: %m");
		return sock;
	}

	int val=1;
	if(setsockopt(sock,SOL_SOCKET,SO_BROADCAST,(const char*)&val,sizeof(val))<0){
		syslog(LOG_ERR,"Failed to set broadcast flag: %m");
		close(sock);
		return -1;
	}

	if(setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,(const char*)&val,sizeof(val))<0){
		syslog(LOG_ERR,"Failed to set reuse flag: %m");
		close(sock);
		return -1;
	}

	// Bind socket to named interface	
	struct ifreq interface;
	strncpy(interface.ifr_ifrn.ifrn_name,cfg.interface.c_str(),IFNAMSIZ-1);
	interface.ifr_ifrn.ifrn_name[IFNAMSIZ-1]=0;
	if(setsockopt(sock,SOL_SOCKET,SO_BINDTODEVICE,(char *)&interface,sizeof(interface))<0){
		syslog(LOG_ERR,"Failed to bind socket to interface: %m");
		close(sock);
		return -1;
	}

	// Setup address
	struct sockaddr_in addr;
	memset(&addr,0,sizeof(addr));
	memset(&addr.sin_zero,0,sizeof(addr.sin_zero));
	addr.sin_family=AF_INET;
	addr.sin_port=htons(DHCP_CLIENT_PORT);
	addr.sin_addr.s_addr= INADDR_ANY;

	
	// Bind socket to address
	if(bind(sock,(struct sockaddr*)&addr,sizeof(addr))<0){
		syslog(LOG_ERR,"Failed to bind socket to address: %m");
		close(sock);
		return -1;
	}
	
	
	return sock;
}

void destroy_socket(int sock){
	if(close(sock)<0){
		syslog(LOG_ERR,"Failed to close dhcp socket: %m");
	}
}
*/
class DhcpPing: public Thread{
private:
	int l_sock,s_sock,;
	int timeout;
	string interface;
	char hw[6];
	bool done;
	bool result;

	bool create_socket(int *sock){

		// Create socket
		if((*sock=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP))<0){
			syslog(LOG_ERR,"Failed to create socket: %m");
			return false;
		}

		int val=1;
		if(setsockopt(*sock,SOL_SOCKET,SO_BROADCAST,(const char*)&val,sizeof(val))<0){
			syslog(LOG_ERR,"Failed to set broadcast flag: %m");
			return false;
		}

		if(setsockopt(*sock,SOL_SOCKET,SO_REUSEADDR,(const char*)&val,sizeof(val))<0){
			syslog(LOG_ERR,"Failed to set reuse flag: %m");
			return false;
		}

		// Bind socket to named interface
		struct ifreq interface;
		strncpy(interface.ifr_ifrn.ifrn_name,this->interface.c_str(),IFNAMSIZ-1);
		interface.ifr_ifrn.ifrn_name[IFNAMSIZ-1]=0;
		if(setsockopt(*sock,SOL_SOCKET,SO_BINDTODEVICE,(char *)&interface,sizeof(interface))<0){
			syslog(LOG_ERR,"Failed to bind socket to interface: %m");
			return false;
		}

		// Setup address
		struct sockaddr_in addr;
		memset(&addr,0,sizeof(addr));
		memset(&addr.sin_zero,0,sizeof(addr.sin_zero));
		addr.sin_family=AF_INET;
		addr.sin_port=htons(DHCP_CLIENT_PORT);
		addr.sin_addr.s_addr= INADDR_ANY;


		// Bind socket to address
		if(bind(*sock,(struct sockaddr*)&addr,sizeof(addr))<0){
			syslog(LOG_ERR,"Failed to bind socket to address: %m");
			return false;
		}


		return true;
	}

	void destroy_socket(int *sock){
		if(close(*sock)<0){
			syslog(LOG_ERR,"Failed to close dhcp socket: %m");
		}
		*sock=-1;
	}

	static u_int32_t get_xid(){
		static u_int32_t xid=0;

		if(xid==0){
			srandom(time(NULL));
			xid=random();
			if(xid==0){
				xid++;
			}
		}

		return xid;
	}

	bool get_hw_address(){
		struct ifreq interface;
		int ret;

		strncpy(interface.ifr_ifrn.ifrn_name,this->interface.c_str(),IFNAMSIZ-1);
		interface.ifr_ifrn.ifrn_name[IFNAMSIZ-1]=0;

		if((ret=ioctl(this->l_sock,SIOCGIFHWADDR,&interface))<0){
			syslog(LOG_ERR,"Unable to get hw-address from socket: %m");
		}else{
			memcpy(hw,&interface.ifr_hwaddr.sa_data,6);
		}

		return ret==0;
	}


	bool send_discover(){
		dhcp_packet dp;

		memset(&dp,0,sizeof(dp));
		dp.op=BOOTREQUEST;				// Request
		dp.htype=1;						// Ethernet 10Mb
		dp.hlen=6;						// Ethernet address size
		dp.hops=0;						// Not really nessecery
		dp.xid=get_xid();				// random cookie
		dp.secs=100;					// how long has transaction taken
		dp.flags=htons(DHCP_BROADCAST_FLAG);	// How do we want our answer

		// Set our hw address
		if(!get_hw_address()){
			return false;
		}
		memcpy(dp.chaddr,this->hw,6);

		// Set magic option cookie
		dp.options[0]=0x63;
		dp.options[1]=0x82;
		dp.options[2]=0x53;
		dp.options[3]=0x63;

		// DHCP discover option
		dp.options[4]=DHCP_OPTION_MESSAGE_TYPE;	// Type
		dp.options[5]=0x01;						// Length
		dp.options[6]=DHCPDISCOVER;				// Data

		// No more options
		dp.options[7]=DHCP_OPTION_END;

		// Send packet
		struct sockaddr_in addr;
		addr.sin_family=AF_INET;
		addr.sin_port=htons(DHCP_SERVER_PORT);
		addr.sin_addr.s_addr=INADDR_BROADCAST;
		memset(&addr.sin_zero,0,sizeof(&addr.sin_zero));

		if(sendto(s_sock,(const char*)&dp,sizeof(dp),0,(const struct sockaddr *) &addr,sizeof(addr))<0){
			syslog(LOG_ERR,"Failed to send dhcp discover packet: %m");
			return false;
		}

		return true;
	}

	bool wait_offer(){
		dhcp_packet dp;

		struct timeval tv;
		tv.tv_sec=this->timeout;
		tv.tv_usec=0;

		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(l_sock, &fds);

		int f=select(l_sock+1,&fds,NULL,NULL,&tv);
		if(f<0){
			syslog(LOG_ERR,"Failed to select: %m");
			return false;
		}

		if(f==0){
			syslog(LOG_DEBUG,"Timeout waiting for reply");
			return false;
		}

		if(!FD_ISSET(l_sock,&fds)){
			syslog(LOG_ERR,"Failed waiting for reply: %m");
			return false;
		}

		struct sockaddr_in source;
		socklen_t ssize=sizeof(source);
		memset(&source,0,ssize);

		int rec=recvfrom(l_sock,(char *)&dp,sizeof(dp),0, (struct sockaddr*)&source,&ssize);
		if(rec<0){
			syslog(LOG_ERR,"Failed to read reply: %m");
			return false;
		}

		return true;
	}


public:
	DhcpPing(string interface, int timeout):
			Thread(),l_sock(-1),s_sock(-1),timeout(timeout),interface(interface),
			done(false),result(false){
		if(!this->create_socket(&l_sock)){
			throw runtime_error("Failed to create listen socket");
		}
		if(!this->create_socket(&s_sock)){
			throw runtime_error("Failed to create send socket");
		}
	}

	virtual void Run(){
		this->result=this->wait_offer();
		this->done=true;

	}

	bool Ping(){
		this->Start();
		this->send_discover();
		while(!this->done){
			usleep(1000);
		}
		return result;
	}

	virtual ~DhcpPing(){
		if(l_sock>=0){
			this->destroy_socket(&l_sock);
		}

		if(s_sock>=0){
			this->destroy_socket(&s_sock);
		}

	}
};


int main(int argc, char** argv){
	//int result=0;
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

/*
	int fd=create_socket();
	
	if(fd<0){
		return -1;
	}
	
	if(send_discover(fd)){
		syslog(LOG_DEBUG,"Sent discover");
	}
	
	if(!wait_offer(fd)){
		syslog(LOG_DEBUG,"wait for offer failed");
		result=1;
	}else{
		syslog(LOG_DEBUG,"Dhcp server found");
	}
	
	destroy_socket(fd);
	return result;
*/
}
