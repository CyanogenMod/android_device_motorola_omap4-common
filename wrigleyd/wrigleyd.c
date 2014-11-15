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
#include <linux/fib_rules.h>
#include <inttypes.h>
#include <stdlib.h>

static void add_rule(void)
{
	system("/system/bin/ip rule add pref 9999 lookup main");
}

static int watcher(struct nl_msg *msg, void *arg)
{
	struct nlmsghdr *hdr;
	uint8_t *data;

	hdr = nlmsg_hdr(msg);

	if (!hdr)
		return 0;

	/* Delete rule */
	if (hdr->nlmsg_type != 0x21)
		return 0;

	if (hdr->nlmsg_len < 44)
		return 0;

	data = nlmsg_data(hdr);
	if (!data)
		return 0;

	/* 9999 */
	if ((data[24] != 0x0f) || (data[25] != 0x27))
		return 0;

	printf("Re-adding rule...\n");
	add_rule();

	return 0;
}

int main(__attribute__((unused))int argc, __attribute__((unused))char **argv)
{
	struct nl_sock *sk;

	add_rule();

	sk = nl_socket_alloc();

	nl_socket_disable_seq_check(sk);
	nl_socket_modify_cb(sk, NL_CB_VALID, NL_CB_CUSTOM, watcher, NULL);

	nl_connect(sk, NETLINK_ROUTE);

	nl_socket_add_memberships(sk, RTNLGRP_IPV4_RULE, 0);

	while (1)
		nl_recvmsgs_default(sk);

	return EXIT_SUCCESS;
}
