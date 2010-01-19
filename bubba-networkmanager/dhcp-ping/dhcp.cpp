
#include <cstdio>

#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>
#include <linux/if_ether.h>
#include <netinet/in.h>
#include <net/if.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include <syslog.h>

#include "dhcp.h"

bool read_packet(int sock,dhcp_packet* dp){
	ip_udp_dhcp_packet packet;
	
	
	memset(&packet,0,sizeof(packet));
	
	int nread=read(sock, &packet, sizeof(packet));
	if(nread<0){
		syslog(LOG_ERR,"Failed to read packet: %m");
		return false;
	}
	
	if(nread<(int)(sizeof(packet.ip) + sizeof(packet.udp))){
		//syslog(LOG_DEBUG,"Packet to small");
		return false;
	}
	
	if(nread<(ntohs(packet.ip.tot_len))){
		//syslog(LOG_DEBUG,"Partial read");
		return false;
	}
	
	// Is this "our" packet?
	if(packet.ip.protocol != IPPROTO_UDP){
		//syslog(LOG_DEBUG,"No udp packet");
		return false;
	}
	
	// IPV4?
	if(packet.ip.version != IPVERSION){
		//syslog(LOG_DEBUG,"Not ipv4");
		return false;
	}
	
	if(packet.udp.dest != htons(DHCP_CLIENT_PORT)){
		// syslog(LOG_DEBUG,"Wrong port");
		return false;
	}
	
	// Fix packet real length
	nread=ntohs(packet.ip.tot_len);
	memcpy(dp,&packet.dhcp,nread-(sizeof(packet.ip)+sizeof(packet.udp)));
	
	// Verify cookie as last check
	if( !(dp->options[0]==0x63 && dp->options[1]==0x82 && dp->options[2]==0x53 && dp->options[3]==0x63) ){
		syslog(LOG_DEBUG, "Wrong magic: [%2x] [%2x] [%2x] [%2x]",dp->options[0],dp->options[1],dp->options[2],dp->options[3]);
		return false;
	}
	
	return true;

}

bool wait_offer(int sock, int timeout){
	dhcp_packet dp;

	struct timeval tv;
	tv.tv_sec=timeout;
	tv.tv_usec=0;
		
	while(tv.tv_sec>0 || tv.tv_usec>0){
		//syslog(LOG_DEBUG, "Waiting %d s %d us for offer",(int)tv.tv_sec,(int)tv.tv_usec);
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

		if(read_packet(sock,&dp)){
			syslog(LOG_DEBUG,"Got answer");
			return true;
		}
	}
		
	return false;
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

static bool get_hw_address(int sock, const char* ifnam, char hw[6]){
	struct ifreq interface;
	int ret;
	
	strncpy(interface.ifr_ifrn.ifrn_name,ifnam,IFNAMSIZ-1);
	interface.ifr_ifrn.ifrn_name[IFNAMSIZ-1]=0;

	if((ret=ioctl(sock,SIOCGIFHWADDR,&interface))<0){
		perror("Unable to get hw-address from socket");
	}else{
		memcpy(hw,&interface.ifr_hwaddr.sa_data,6);
	}
	
	return ret==0;
}


bool send_discover(int sock, const char* iface){
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
	if(get_hw_address(sock,iface,hwaddr)<0){
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
		perror("Failed to send dhcp discover packet");
		return false;
	}
	
	return true;
}


static bool bind_to_interface(int sock, const char* iface){

	struct ifreq interface;
	strncpy(interface.ifr_ifrn.ifrn_name,iface,IFNAMSIZ-1);
	interface.ifr_ifrn.ifrn_name[IFNAMSIZ-1]=0;
	if(setsockopt(sock,SOL_SOCKET,SO_BINDTODEVICE,(char *)&interface,sizeof(interface))<0){
		perror("Failed to bind socket to interface");
		return false;
	}
	return true;
}

int create_broadcast_socket(const char* iface){
	int sock;
	
	// Create socket
	if((sock=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP))<0){
		perror("Failed to create socket");
		return sock;
	}

	int val=1;
	if(setsockopt(sock,SOL_SOCKET,SO_BROADCAST,(const char*)&val,sizeof(val))<0){
		perror("Failed to set broadcast flag");
		close(sock);
		return -1;
	}

	if(setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,(const char*)&val,sizeof(val))<0){
		perror("Failed to set reuse flag");
		close(sock);
		return -1;
	}

	if(!bind_to_interface(sock,iface)){
		close(sock);
		return -1;
	}
		
	return sock;
}

int create_raw_socket(const char* iface){
	int sock;
	
	if((sock=socket(AF_PACKET,SOCK_DGRAM,htons(ETH_P_ALL)))<0){
		perror("Failed to create socket");
		return sock;
	}

	if(!bind_to_interface(sock,iface)){
		close(sock);
		return -1;
	}

	
	return sock;
}

