#!/bin/sh
# Converted from Gentoo iptables initd script
# Copyright 1999-2013 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# Bubba and netfilter specific entries by Gordon Bos
# $Header: $

extra_commands="check save panic"
extra_started_commands="reload"

nftables_bin="/sbin/nft"
nftables_proc="/proc/net/netfilter"
nftables_save="/etc/bubba/firewall.nft"
nftables_stopped="/etc/bubba/firewall.nft.stopped"


length=0
ebegin() {
	line=$1
	length=${#line}
	echo -n "$1"
}

eend() {
	printf '%*s' $((77-$length)) "["
	if [ "$1" == "0" ]; then
		echo -e "\e[32mOK\e[0m]"
	else
		echo -e "\e[31mKO\e[0m]"
	fi
}

eerror() {
	echo -e "\e[31m$1\e[0m"
}

flush_rules() {
	local table policy=$1
	if [ -z "$policy" ]; then
		policy="accept"
	fi

	nft flush ruleset
	tables="filter nat"
	for table in ${tables} ; do
		${nftables_bin} add table ip ${table}
		local chains
		case ${table} in
			nat)    chains="PREROUTING POSTROUTING INPUT OUTPUT";;
			filter) chains="INPUT FORWARD OUTPUT";;
			*)      chains="";;
		esac
		local chain
		for chain in ${chains} ; do
			${nftables_bin} add chain ip ${table} ${chain} \{ type nat hook prerouting priority 0\; policy ${policy}\; \}
		done
	done
}

checkkernel() {
	if [ ! -e ${nftables_proc} ] ; then
		eerror "Your kernel lacks nftables support, please load"
		eerror "appropriate modules and try again."
		return 1
	fi
	return 0
}

checkconfig() {
	if [ ! -f ${nftables_save} ] ; then
		eerror "Not starting firewall.  First create some rules then run:"
		eerror "/opt/bubba/sbin/bubba-firewall.sh save"
		return 1
	fi
	return 0
}

start() {
	checkconfig || return 1
	ebegin "Setting up firewall"
	if [ ! -z "${MODULES}" ]; then
		local module
		for module in "${MODULES}" ; do
			modprobe ${module}
		done
	fi
	echo "1" > /proc/sys/net/ipv4/ip_forward
	${nftables_bin} flush ruleset
	${nftables_bin} -f "${nftables_save}"
	eend $?
}

stop() {
	if [ "${SAVE_ON_STOP}" = "yes" ] ; then
		save || return 1
	fi
	checkkernel || return 1
	ebegin "Stopping firewall"
	flush_rules
	eend $?
	if [ -r  "${nftables_stopped}" ]; then
		ebegin "Loading 'stopped' rules"
		${nftables_bin} -f "${nftables_stopped}"
		eend $?
	else
		echo "0" > /proc/sys/net/ipv4/ip_forward
	fi
}

reload() {
	checkkernel || return 1
	checkrules || return 1
	ebegin "Flushing firewall"
	flush_rules
	eend $?

	start
}

checkrules() {
	ebegin "Checking rules"
	${nftables_bin} -c -f "${nftables_save}"
	eend $?
}

check() {
	# Short name for users of init.d script.
	checkrules
}

save() {
	ebegin "Saving firewall state"
	checkpath -q -d "$(dirname "${nftables_save}")"
	checkpath -q -m 0600 -f "${nftables_save}"
	${nftables_bin} -nn list ruleset > "${nftables_save}"
	eend $?
}

panic() {
	checkkernel || return 1
	if systemctl is-started bubba-firewall; then
		systemctl stop bubba-firewall
	fi

	ebegin "Dropping all packets"
	flush_rules DROP
	local LAN_if=$(/opt/bubba/bin/bubba-networkmanager-cli getlanif)
	local LAN_net=$(ip route show dev ${LAN_if} scope link | cut -d ' ' -f1)
	${nftables_bin} add rule ip filter INPUT iifname ${LAN_if} accept
	${nftables_bin} add rule ip filter OUTPUT ip daddr ${LAN_net} accept
	eend $?
}

if [ "$1" == "" ];then
	echo "Usage: $0 <start|stop|reload|check|save|panic>"
else
	$1 2>/dev/null
	if [ $length == 0 ]; then
		echo "Usage: $0 <start|stop|reload|check|save|panic>"
	fi
fi
