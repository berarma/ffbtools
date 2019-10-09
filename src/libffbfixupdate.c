/*
 *
 * libfixffupdate.c
 *
 * Reuploads and effect whenever it fails updating
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
#include <dlfcn.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#define ioctl ioctl_trash_function
#include <linux/input.h>
#undef ioctl

#define IOCTL_REQUEST_CODE(request) (request & ((_IOC_DIRMASK << _IOC_DIRSHIFT) | (_IOC_TYPEMASK << _IOC_TYPESHIFT) | (_IOC_NRMASK << _IOC_NRSHIFT)))

static int dev_major = 0;

static int dev_minor = 0;

static int checkDescriptor(int fd)
{
    if (dev_major == 0 && dev_minor == 0) {
        const char *str_dev_major = getenv("FFBTOOLS_DEV_MAJOR");
        const char *str_dev_minor = getenv("FFBTOOLS_DEV_MINOR");

        if (str_dev_major != NULL && str_dev_minor != NULL) {
            dev_major = strtol(str_dev_major, NULL, 0);
            dev_minor = strtol(str_dev_minor, NULL, 0);
        }
    }

    if (dev_major != 0 && dev_minor != 0) {
        struct stat sb;

        fstat(fd, &sb);

        if (major(sb.st_rdev) == dev_major && minor(sb.st_rdev) == dev_minor) {
            return 1;
        }
    }

    return 0;
}

int ioctl(int fd, unsigned long request, char *argp)
{
    static int (*_ioctl)(int fd, unsigned long request, char *argp) = NULL;
    struct ff_effect *effect = NULL;

    if (!_ioctl) {
        _ioctl = dlsym(RTLD_NEXT, "ioctl");
    }

    if (!checkDescriptor(fd)) {
        return _ioctl(fd, request, argp);
    }

    int result = _ioctl(fd, request, argp);

    if (result == -1 && errno == EINVAL && IOCTL_REQUEST_CODE(request) == IOCTL_REQUEST_CODE(EVIOCSFF)) {
        effect = (struct ff_effect*) argp;
        effect->id = -1;
        result = _ioctl(fd, request, argp);
    }

    return result;
}
