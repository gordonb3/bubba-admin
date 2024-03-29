#!/bin/sh
# Converted from Gentoo iptables initd script
# Copyright 1999-2013 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# Bubba specific entries by Gordon Bos
# $Header: $

extra_commands="check save panic"
extra_started_commands="reload"

iptables_bin="/sbin/iptables"
iptables_proc="/proc/net/ip_tables_names"
iptables_save="/etc/bubba/firewall.conf"
iptables_down="/etc/bubba/firewall_down.conf"


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
		policy="ACCEPT"
	fi
	for table in $(cat ${iptables_proc}) ; do
		local chains
		case ${table} in
			nat)    chains="PREROUTING POSTROUTING OUTPUT";;
			mangle) chains="PREROUTING INPUT FORWARD OUTPUT POSTROUTING";;
			filter) chains="INPUT FORWARD OUTPUT";;
			*)      chains="";;
		esac
		local chain
		for chain in ${chains} ; do
			${iptables_bin} -t ${table} -P ${chain} ${policy}
		done

		${iptables_bin} -F -t ${table}
		${iptables_bin} -X -t ${table}
	done
}

checkkernel() {
	if [ ! -e ${iptables_proc} ] ; then
		eerror "Your kernel lacks iptables support, please load"
		eerror "appropriate modules and try again."
		return 1
	fi
	return 0
}

checkconfig() {
	if [ ! -f ${iptables_save} ] ; then
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
	${iptables_bin}-restore ${SAVE_RESTORE_OPTIONS} < "${iptables_save}"
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
	if [ -r  "${iptables_down}" ]; then
		ebegin "Loading 'stopped' rules"
		${iptables_bin}-restore "${iptables_down}"
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
	${iptables_bin}-restore --test ${SAVE_RESTORE_OPTIONS} < "${iptables_save}"
	eend $?
}

check() {
	# Short name for users of init.d script.
	checkrules
}

save() {
	ebegin "Saving firewall state"
	checkpath -q -d "$(dirname "${iptables_save}")"
	checkpath -q -m 0600 -f "${iptables_save}"
	${iptables_bin}-save ${SAVE_RESTORE_OPTIONS} > "${iptables_save}"
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
	${iptables_bin} -A INPUT -i ${LAN_if} -j ACCEPT
	${iptables_bin} -A OUTPUT -d ${LAN_net} -j ACCEPT
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
