#ifndef MY_DHCP_H
#define MY_DHCP_H

#include <sys/types.h>



#define BOOTREQUEST		1
#define BOOTREPLY		2

#define DHCPDISCOVER	1
#define DHCPOFFER		2
#define DHCPREQUEST		3
#define DHCPDECLINE		4
#define DHCPACK			5
#define DHCPNACK		6
#define DHCPRELEASE		7

#define DHCP_OPTION_MESSAGE_TYPE		53
#define DHCP_OPTION_HOST_NAME			12
#define DHCP_OPTION_BROADCAST_ADDRESS	28
#define DHCP_OPTION_REQUESTED_ADDRESS	50
#define DHCP_OPTION_LEASE_TIME			51
#define DHCP_OPTION_SERVER_IDENTIFIER	54
#define DHCP_OPTION_RENEWAL_TIME		58
#define DHCP_OPTION_REBINDING_TIME		59
#define DHCP_OPTION_END					255

#define MAX_DHCP_CHADDR_LENGTH	16
#define MAX_DHCP_SNAME_LENGTH	64
#define MAX_DHCP_FILE_LENGTH	128
#define MAX_DHCP_OPTIONS_LENGTH	312

#define DHCP_BROADCAST_FLAG 32768

typedef struct {
	u_int8_t	op;
	u_int8_t	htype;
	u_int8_t  	hlen;
	u_int8_t  	hops;
	u_int32_t 	xid;
	u_int16_t 	secs;
	u_int16_t 	flags;
	struct in_addr	ciaddr;
	struct in_addr	yiaddr;
	struct in_addr	siaddr;
	struct in_addr	giaddr;
	unsigned char 	chaddr[MAX_DHCP_CHADDR_LENGTH];
	char	sname[MAX_DHCP_SNAME_LENGTH];
	char	file[MAX_DHCP_FILE_LENGTH];
	char	options[MAX_DHCP_OPTIONS_LENGTH];

} __attribute__((__packed__)) dhcp_packet;


#endif
