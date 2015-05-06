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
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#define LOG_TAG "motorild"

#include <cutils/log.h>
#include <cutils/properties.h>
#include <cutils/sockets.h>

#include "motoril.h"

#define FREQUENCIES_PATH "/sys/devices/system/cpu/cpu0/cpufreq/scaling_available_frequencies"
#define MAXFREQ_PATH "/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq"
#define CPU1_ONLINE_PATH "/sys/devices/system/cpu/cpu1/online"
#define BOOSTPULSE_PATH "/sys/devices/system/cpu/cpufreq/interactive/boostpulse"

static int set_gw(char *iface, char *gw)
{
	int ret;
	pid_t child;

	child = fork();

	if (child < 0) {
		ALOGE("Can't fork");
		return -1;
	} else if (child == 0) {
		execl("/system/bin/ndc","ndc","route","add","dst","v4",iface,"0",gw, NULL);
		exit(EXIT_FAILURE);
	} else {
		while (waitpid(child, &ret, 0) != child);
		ret = WEXITSTATUS(ret);
	}
	ALOGI("ndc returned %d", ret);

	return ret;
}

static int ring(void)
{
	int fd;
	char buf[1024] = {0};
	char *max_freq = NULL;
	int r;

	fd = open(FREQUENCIES_PATH, O_RDONLY);
	if (fd != -1) {
		r = read(fd, buf, sizeof(buf));
		if (r > 0) {
			char *tok = NULL;
			char *s = buf;
			while((tok = strtok(s, " \r\n"))) {
				s = NULL;
				max_freq = tok;
			}
		}

		close(fd);
	} else {
		ALOGW("Can't read supported frequencies");
	}

	if (max_freq) {
		ALOGI("Setting maximum frequency to %s", max_freq);
		fd = open(MAXFREQ_PATH, O_WRONLY);
		if (fd != -1) {
			write(fd, max_freq, strlen(max_freq));
			close(fd);
		} else {
			ALOGW("Can't set maximum frequency");
		}
	}

	fd = open(BOOSTPULSE_PATH, O_WRONLY);
	if (fd != -1) {
		ALOGI("Boosting CPU");
		write(fd, "1", strlen("1"));
		close(fd);
	} else {
		ALOGW("Can't send boost-pulse");
	}

	fd = open(CPU1_ONLINE_PATH, O_RDWR);
	if (fd != -1) {
		r = read(fd, buf, sizeof(buf));
		if (r > 0) {
			ALOGI("CPU1 online: %c", *buf);
			if (buf[0] != '1') {
				ALOGI("Setting CPU1 to online\r\n");
				write(fd, "1", strlen("1"));
			}
		}
		close(fd);
	} else {
		ALOGW("Can't read CPU1 online state.");
	}

	return 0;
}

int main(__attribute__((unused))int argc, __attribute__((unused))char **argv)
{
	int fd, client;
	struct sockaddr_un clientaddr;
	socklen_t clientlen = sizeof(clientaddr);
	struct motoril_call call;
	unsigned int pos;
	int ret;

	ALOGI("motorild starting");

	fd = android_get_control_socket("motorild");
	if (fd < 0) {
		ALOGE("Can't get socket!");
	}

	ret = listen(fd, 10);

	while (1) {
		client = accept(fd, (struct sockaddr*)&clientaddr, &clientlen);
		if (client == -1) {
			ALOGE("accept");
			continue;
		}

		ALOGI("accepted connection");

		pos = 0;
		do {
			ret = read(client, ((uint8_t*)&call) + pos, sizeof(call) - pos);
			if (ret == 0) {
				ALOGE("connection closed");
				break;
			} else if (ret < 0) {
				ALOGE("read");
				break;
			}

			pos += ret;
		} while (pos < sizeof(call));

		if (pos < sizeof(call)) {
			close(client);
			continue;
		}

		ALOGI("CMD: %d", call.cmd);
		switch (call.cmd) {
			case MOTORIL_CMD_ROUTE:
				ALOGI("IF: %s, GW: %s", call.dev, call.gw);
				ret = set_gw(call.dev, call.gw);
				break;
			case MOTORIL_CMD_RING:
				ALOGI("RING");
				ret = ring();
				break;
			default:
				ret = 1;
				break;
		}

		if (ret == 0) {
			write(client, "OK", 2);
		} else {
			write(client, "KO", 2);
		}

		close(client);
	}

	return EXIT_SUCCESS;
}
