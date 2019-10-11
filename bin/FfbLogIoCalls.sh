#!/bin/sh
#
# Runs an application preloading FfbLogIoCalls.
# 
# Copyright 2019 Bernat Arlandis <bernat@hotmail.com>
#
# This file is part of ffbtools.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

CURRENT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)

if [ -f "${CURRENT_DIR}/../build/libffblogiocalls.so" ]
then
    OUR_LD_PRELOAD="${CURRENT_DIR}/../build/libffblogiocalls.so"
else
    OUR_LD_PRELOAD="/usr/local/lib/libffblogiocalls.so"
fi

LD_PRELOAD="${OUR_LD_PRELOAD} ${LD_PRELOAD}"

DEVICE_FILE=$(readlink -f "$1")

if [ -n "$DEVICE_FILE" ]
then
    FFBTOOLS_DEV_MAJOR=0x$(stat --format="%t" "${DEVICE_FILE}")
    FFBTOOLS_DEV_MINOR=0x$(stat --format="%T" "${DEVICE_FILE}")
    shift
fi

if [ -n "$1" ]
then
    if [ "$1" != "--" ]
    then
        FFBTOOLS_LOG_FILE="$1-$(date +%Y%m%d%H%M%S).log"
        shift
    fi
fi

if [ "$1" = "--" ]
then
    COMMAND="$2"
    shift 2
fi

if [ -z "${FFBTOOLS_DEV_MAJOR}" -o -z "${FFBTOOLS_DEV_MINOR}" -o -z "${COMMAND}" ]
then
    echo "Missing arguments: $(basename $0) <device path> [<log file>] -- <command> ..."
    exit 1
fi

export LD_PRELOAD FFBTOOLS_DEV_MAJOR FFBTOOLS_DEV_MINOR FFBTOOLS_LOG_FILE

"${COMMAND}" "$@"
