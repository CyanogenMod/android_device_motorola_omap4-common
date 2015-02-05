/*
 * Copyright (c) 2015 The CyanogenMod Project
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

#include <sys/socket.h>
#include <sys/un.h>
#include <inttypes.h>
#include <stdlib.h>
#include <errno.h>

#define LOG_TAG "motorilc"

#include <cutils/log.h>
#include <cutils/properties.h>
#include <cutils/sockets.h>

#include "motoril.h"

int main(int argc, char **argv)
{
	int fd;
	struct motoril_route route;
	uint8_t buf[256];
	unsigned int pos;
	int ret;
	unsigned int i;

	if (argc != 3) {
		fprintf(stderr, "%s if gw\n", argv[0]);
		return EXIT_FAILURE;
	}

	fd = socket_local_client("motorild", ANDROID_SOCKET_NAMESPACE_RESERVED, SOCK_STREAM);
	if (fd < 0) {
		perror("socket_local_client");
		return EXIT_FAILURE;
	}

	memset(&route, 0, sizeof(route));
	strncpy(route.dev, argv[1], sizeof(route.dev)-1);
	strncpy(route.gw, argv[2], sizeof(route.gw)-1);

	pos = 0;
	while (pos < sizeof(route)) {
		ret = write(fd, ((uint8_t*)&route) + pos, sizeof(route) - pos);
		if (ret < 0) {
			perror("write");
			return EXIT_FAILURE;
		}

		pos += ret;
	}

	pos = 0;
	do {
		ret = read(fd, buf + pos, sizeof(buf) - pos);
		if (ret == 0) {
			break;
		} else if (ret < 0) {
			perror("read");
			return EXIT_FAILURE;
		}

		pos += ret;
	} while (1);

	for (i = 0; i < pos; i++)
		printf("%c", buf[i]);
	printf("\n");
	
	return EXIT_SUCCESS;
}
