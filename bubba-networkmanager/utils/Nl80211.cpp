/*
 * =====================================================================================
 *
 *       Filename:  Nl80211.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  12/22/2009 05:26:28 PM CET
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:   (), 
 *        Company:  
 *
 * =====================================================================================
 */

#include <string>
#include <list>
#include <map>

#include "Nl80211.h"
#include <cerrno>

#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>
#include <netlink/msg.h>
#include <netlink/attr.h>
#include <net/if.h>
#include <sstream>
#include <ifstream>

#include "include/nl80211.h"

using namespace NetworkManager;

Nl80211::Nl80211( std::string phy ) {
    this->init( phy );
}
static int ieee80211_frequency_to_channel(int freq)
{
   if (freq == 2484)
       return 14;

   if (freq < 2484)
       return (freq - 2407) / 5;

   /* FIXME: dot11ChannelStartingFactor (802.11-2007 17.3.8.3.2) */
   return freq/5 - 1000;
}

static int Nl80211::error_handler( struct sockaddr_nl *nla, struct nlmsgerr *err, void *arg ) {
    int *ret = static_cast<int *>(arg);
    *ret = err->error;
    return NL_STOP;
}

static int Nl80211::finish_handler(struct nl_msg *msg, void *arg) {
    int *ret = static_cast<int *>(arg);
    *ret = 0;
    return NL_SKIP;
}

static int Nl80211::ack_handler(struct nl_msg *msg, void *arg) {
    int *ret = static_cast<int *>(arg);
    *ret = 0;
    return NL_STOP;
}

int Nl80211::valid_handler(struct nl_msg *msg, void *arg) {
    struct nlattr *tb_msg[NL80211_ATTR_MAX + 1];
    struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));

    struct nlattr *tb_band[NL80211_BAND_ATTR_MAX + 1];

    struct nlattr *tb_freq[NL80211_FREQUENCY_ATTR_MAX + 1];
    static struct nla_policy freq_policy[NL80211_FREQUENCY_ATTR_MAX + 1] = 
    {
        [NL80211_FREQUENCY_ATTR_FREQ] = { .type = NLA_U32 },
        [NL80211_FREQUENCY_ATTR_DISABLED] = { .type = NLA_FLAG },
        [NL80211_FREQUENCY_ATTR_PASSIVE_SCAN] = { .type = NLA_FLAG },
        [NL80211_FREQUENCY_ATTR_NO_IBSS] = { .type = NLA_FLAG },
        [NL80211_FREQUENCY_ATTR_RADAR] = { .type = NLA_FLAG },
        [NL80211_FREQUENCY_ATTR_MAX_TX_POWER] = { .type = NLA_U32 },
    };

    struct nlattr *nl_band;
    struct nlattr *nl_freq;

    uint8_t bandidx = 1;
    int rem_band, rem_freq;

    nla_parse(
            tb_msg,
            NL80211_ATTR_MAX,
            genlmsg_attrdata(gnlh, 0),
            genlmsg_attrlen(gnlh, 0),
            NULL
            );
    if (!tb_msg[NL80211_ATTR_WIPHY_BANDS])  {
        return NL_SKIP;
    }

    nla_for_each_nested(nl_band, tb_msg[NL80211_ATTR_WIPHY_BANDS], rem_band) {

        nla_parse(
                tb_band, 
                NL80211_BAND_ATTR_MAX, 
                nla_data(nl_band),
                nla_len(nl_band),
                NULL
                );

#ifdef NL80211_BAND_ATTR_HT_CAPA
        if (tb_band[NL80211_BAND_ATTR_HT_CAPA]) {
            _cap = nla_get_u16(tb_band[NL80211_BAND_ATTR_HT_CAPA]);
        }
#endif
        Channels channels;

        nla_for_each_nested(nl_freq, tb_band[NL80211_BAND_ATTR_FREQS], rem_freq) {
            Channel channel;
            uint32_t freq;
            nla_parse(
                    tb_freq,
                    NL80211_FREQUENCY_ATTR_MAX,
                    nla_data(nl_freq),
                    nla_len(nl_freq),
                    freq_policy
                    );

            if (!tb_freq[NL80211_FREQUENCY_ATTR_FREQ]) {
                continue;
            }
            freq = nla_get_u32( tb_freq[NL80211_FREQUENCY_ATTR_FREQ] );

            {
                std::ostringstream oss;
                oss << freq;

                channel["freq"] = freq.str();
            }
            {
                std::ostringstream oss;
                oss << ieee80211_frequency_to_channel(freq);

                channel["channel"] = oss.str();
            }

            if (
                    tb_freq[NL80211_FREQUENCY_ATTR_MAX_TX_POWER] 
                    && !tb_freq[NL80211_FREQUENCY_ATTR_DISABLED]
               ) {
                {
                    std::ostringstream oss;
                    oss << 0.01 * nla_get_u32(tb_freq[NL80211_FREQUENCY_ATTR_MAX_TX_POWER]);

                    channel["dBm"] = oss.str();
                }
            }

            if (tb_freq[NL80211_FREQUENCY_ATTR_DISABLED]) {
                channel["disabled"] = "true";
            }

            if (tb_freq[NL80211_FREQUENCY_ATTR_PASSIVE_SCAN]) {
                channel["passive_scanning"] = "true";
            }
            if (tb_freq[NL80211_FREQUENCY_ATTR_NO_IBSS]) {
                channel["no_IBBS"] = "true";
            }
            if (tb_freq[NL80211_FREQUENCY_ATTR_RADAR]) {
                channel["radar_detection"] = "true";
            }
            channels.push_back( channel );
        }
        _bands[bandidx] = channels;
        bandidx++;
    }
    return 0;
}


static int Nl80211::phy_lookup( std::string phy ) {
    std::ostringstream pathss;
    pathss << "/sys/class/ieee80211/" << phy << "/index";
    std::string path = pathss.str();

    std::ifstream ifs ( path.c_str() );

    if( ifs.fail() ) {
        throw runtime_error( "Failed to lookup phy " + phy );
    }

    int retval;
    ifs >> retval;

    if( ifs.fail() ) {
        throw runtime_error( "Error in input from sysfs for phy " + phy );
    }
    ifs.close();
    return retval;
}

void Nl80211::init( std::string phy ) {
    struct nl_handle *sock;
    struct nl_msg *msg;
    struct nl_cb *cb;

    int family;
    int devidx = 0;
    int err = 0;

    sock = nl_handle_alloc();
    if( !sock ) {
        throw runtime_error( "Failed to allocate netlink socket. " + string(strerror(ENOMEM)) );
    }

    if( genl_connect(sock) ) {
        nl_socket_free( sock );
        throw runtime_error( "Failed to connect to generic netlink. " + string(strerror(ENOLINK)) );
    }

    family = genl_ctrl_resolve(sock, "nl80211");
    if( !family ) {
        nl_socket_free( sock );
        throw runtime_error( "nl80211 not found. " + string(strerror(ENOENT)) );
    }
    devidx = phy_lookup( phy );

    msg = nlmsg_alloc();
    if( !msg ) {
        throw runtime_error( "Failed to allocate netlink message. " + string(strerror(ENOMEM)) );
    }

    cb = nl_cb_alloc(NL_CB_DEFAULT);
    if( !cb ) {
        throw runtime_error( "Failed to allocate netlink callbacks. " + string(strerror(ENOMEM)) );
    }

    genlmsg_put(msg, 0, 0, family, 0, NLM_F_DUMP, NL80211_CMD_GET_WIPHY, 0);
    NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, devidx);
    nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, this->handler, NULL);
    err = nl_send_auto_complete(sock, msg);

    if (err < 0) {
        nl_cb_put(cb);
        nlmsg_free(msg);
        return err;
    }
    err = 1;

    nl_cb_err(cb, NL_CB_CUSTOM, error_handler, &err);
    nl_cb_set(cb, NL_CB_FINISH, NL_CB_CUSTOM, finish_handler, &err);
    nl_cb_set(cb, NL_CB_ACK, NL_CB_CUSTOM, ack_handler, &err);

    while (err > 0)
        nl_recvmsgs(sock, cb);

}

const Nl80211::Bands Nl80211::bands() const {
    return _bands;
}
const uint16_t Nl80211::capabilities() const {
    return _cap;
}

