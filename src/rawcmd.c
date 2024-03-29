/*
 *
 * rawcmd.c
 *
 * Send raw data to a hidraw device.
 *
 * Copyright 2019 Bernat Arlandis <bernat@hotmail.com>
 */

/*
 * This file is part of ffbtools.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <linux/hidraw.h>

#define REPORT_SIZE 8

int main(int argc, char *argv[])
{
    unsigned char report[REPORT_SIZE] = {0};
    char *device_name;
    static struct timespec t0;
    struct timespec t1;
    unsigned long reltime;
    int res;

    if (argc < 3) {
        printf("Usage: %s <raw device> [--id n] <B0> [B1] [B2] [B3] [B4] [B5] [B6] ...\n", argv[0]);
        printf("Example: %s /dev/hidraw8 0xF3\n", argv[0]);
        exit(1);
    }

    device_name = argv[1];

    argv += 2;
    argc -= 2;

    if (!strcmp(argv[0], "--id")) {
        report[0] = strtol(argv[1], NULL, 0);
        printf("report id: %d\n", report[0]);
        argv += 2;
        argc -= 2;
    }

    if (argc >= REPORT_SIZE) {
        printf("Error: too many arguments.");
        exit(1);
    }

    for (int i=0; i < argc; i++) {
        report[i + 1] = strtol(argv[i], NULL, 0);
    }

    int fd = open(device_name, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "ERROR: can not open %s (%s) [%s:%d]\n",
                device_name, strerror(errno), __FILE__, __LINE__);
        exit(1);
    }

    clock_gettime(CLOCK_MONOTONIC, &t0);

    res = write(fd, report, REPORT_SIZE);

    clock_gettime(CLOCK_MONOTONIC, &t1);
    reltime = (t1.tv_sec - t0.tv_sec) * 1.0e9 + (t1.tv_nsec - t0.tv_nsec);
    printf("Time: %012lu\n", reltime);

    printf("Return value: %d, errno:%d, strerror: %s\n", res, errno, strerror(errno));

    close(fd);
}
