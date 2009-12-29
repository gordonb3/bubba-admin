/*
 * =====================================================================================
 * 
 *       Filename:  Nl80211.h
 * 
 *    Description:  
 * 
 *        Version:  1.0
 *        Created:  12/22/2009 05:22:42 PM CET
 *       Revision:  none
 *       Compiler:  gcc
 * 
 *         Author:   (), 
 *        Company:  
 * 
 * =====================================================================================
 */

#ifndef NL80211_H
#define NL80211_H
#include <string>
#include <net/if.h>
#include <netlink/msg.h>

#include <stdint.h>
#include <map>
#include <list>

namespace NetworkManager {

    class Nl80211 {
    public:
        typedef std::map<std::string, std::string> Channel;
        typedef std::list<Channel> Channels;
        typedef std::map<uint8_t,Channels> Bands;
    private:
        Bands _bands;
        uint16_t _cap;

        Nl80211( std::string phy ) {
            this->init( phy );
        }
        Nl80211(Nl80211& r);
        Nl80211& operator=(const Nl80211& r);
        int init( std::string phy );
        static int phy_lookup( std::string phy );
        inline static int ieee80211_frequency_to_channel(int freq);



    public:
        static int error_handler(struct sockaddr_nl *nla, struct nlmsgerr *err, void *arg);
        static int finish_handler(struct nl_msg *msg, void *arg);
        static int ack_handler(struct nl_msg *msg, void *arg);
        int valid_handler(struct nl_msg *msg);

        static Nl80211& Instance( std::string phy ) {
            static Nl80211 nl(phy);
            return nl;
        }
        static int call_valid_handler(struct nl_msg *msg, void *arg) {
            return static_cast<Nl80211*>(arg)->valid_handler( msg );
        }
        const Bands& bands() const;
        const uint16_t capabilities() const;
        virtual ~Nl80211(void) {}

    };
}
#endif //NL80211_H
