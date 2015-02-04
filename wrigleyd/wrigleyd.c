/*
 * Copyright (c) 2014 The CyanogenMod Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <netlink/netlink.h>
#include <netlink/socket.h>
#include <netlink/msg.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <linux/fib_rules.h>
#include <inttypes.h>
#include <stdlib.h>
#include <errno.h>

#define LOG_TAG "wrigleyd"

#include <cutils/log.h>
#include <cutils/properties.h>

static void add_rule(void)
{
	ALOGI("adding rule for table 9999");
	system("/system/bin/ip rule add pref 9999 lookup main");
}

static int keep_rule(struct nl_msg *msg, struct nlmsghdr *hdr)
{
	uint8_t *data;

	if (hdr->nlmsg_len < 44)
		return 0;

	data = nlmsg_data(hdr);
	if (!data)
		return 0;

	/* 9999 */
	if ((data[24] != 0x0f) || (data[25] != 0x27))
		return 0;

	ALOGI("rule for table 9999 was deleted...");
	add_rule();

	return 0;
}

static void add_gw(char *gw, char *iface)
{
	char cmd[256];

	ALOGI("adding interface route to %s on interface %s", gw, iface);

	memset(cmd, 0, sizeof(cmd));
	snprintf(cmd, sizeof(cmd)-1, "/system/bin/ip route add %s dev %s table local", gw, iface);
	ALOGI(cmd);
	system(cmd);
	memset(cmd, 0, sizeof(cmd));
	snprintf(cmd, sizeof(cmd)-1, "/system/bin/ip route add default via %s table %s", gw, iface);
	ALOGI(cmd);
	system(cmd);
}

static int force_gateway(struct nl_msg *msg, struct nlmsghdr *hdr, int up)
{
	char value[PROPERTY_VALUE_MAX];
	static char prop[128];
	struct ifaddrmsg *ifa;
	struct rtattr *rth;
	int rtl;
	int count;
	static int last_devidx = -1;

	if (hdr->nlmsg_len < sizeof(struct ifaddrmsg))
		return 0;

	ifa = nlmsg_data(hdr);
	if (!ifa)
		return 0;

	rth = IFA_RTA(ifa);
	rtl = IFA_PAYLOAD(hdr);

	while (rtl && RTA_OK(rth, rtl)) {
		if (rth->rta_type == IFA_LOCAL) {
			uint32_t ipaddr;
			char name[IFNAMSIZ];

			memset(name, 0, IFNAMSIZ);
			if (up) {
				if (if_indextoname(ifa->ifa_index, name) == NULL) {
					ALOGE("Can't resolve interface index %d, errno: %d!", ifa->ifa_index, errno);
					rth = RTA_NEXT(rth, rtl);
					continue;
				}

				if (strncmp(name, "rmnet", 5)) {
					rth = RTA_NEXT(rth, rtl);
					continue;
				}
			} else {
				if (ifa->ifa_index != last_devidx) {
					rth = RTA_NEXT(rth, rtl);
					continue;
				}
			}

			if (up) {
				ipaddr = htonl(*((uint32_t *)RTA_DATA(rth)));
				ALOGI("%s (%d) is now %d.%d.%d.%d",
						name, ifa->ifa_index,
						(ipaddr >> 24) & 0xff,
						(ipaddr >> 16) & 0xff,
						(ipaddr >> 8) & 0xff,
						ipaddr & 0xff);

				memset(prop, 0, sizeof(prop));
				snprintf(prop, sizeof(prop) - 1, "net.%s.gw", name);

				memset(value, 0, sizeof(value));
				count = 30000;
				while((property_get(prop, value, NULL) == 0) && count) {
					usleep(1000);
					count--;
				}

				if (strlen(value) == 0) {
					ALOGI("%s not set yet!", prop);
					rth = RTA_NEXT(rth, rtl);
					continue;
				}

				add_gw(value, name);
				last_devidx = ifa->ifa_index;
			} else {
				ALOGI("dev %d is now down, clearing %s", last_devidx, prop);
				property_set(prop, "");
				last_devidx = -1;
			}
		}
		rth = RTA_NEXT(rth, rtl);
	}

	return 0;
}

static int watcher(struct nl_msg *msg, void *arg)
{
	struct nlmsghdr *hdr;
	hdr = nlmsg_hdr(msg);

	if (!hdr)
		return 0;

	switch (hdr->nlmsg_type) {
		case RTM_DELRULE:
			return keep_rule(msg, hdr);
			break;
		case RTM_NEWADDR:
			return force_gateway(msg, hdr, 1);
			break;
		case RTM_DELADDR:
			return force_gateway(msg, hdr, 0);
			break;
		case RTM_NEWRULE:
			break;
		default:
			ALOGI("unknown message %02x", hdr->nlmsg_type);
			break;
	}

	return 0;
}

int main(__attribute__((unused))int argc, __attribute__((unused))char **argv)
{
	struct nl_sock *sk;

	ALOGI("wrigleyd starting");

	add_rule();

	sk = nl_socket_alloc();

	nl_socket_disable_seq_check(sk);
	nl_socket_modify_cb(sk, NL_CB_VALID, NL_CB_CUSTOM, watcher, NULL);

	nl_connect(sk, NETLINK_ROUTE);

	nl_socket_add_memberships(sk, RTNLGRP_IPV4_RULE, RTNLGRP_IPV4_IFADDR, 0);

	while (1)
		nl_recvmsgs_default(sk);

	return EXIT_SUCCESS;
}
