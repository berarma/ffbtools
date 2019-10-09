#
# Makefile for ffbtools
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

BUILD_DIR ?= ./build
SRC_DIR ?= ./src

OBJS := $(shell find $(BUILD_DIR) -name *.o 2> /dev/null)
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIR) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CPPFLAGS ?= $(INC_FLAGS) -MMD -MP

LDFLAGS ?= -ldl

all: directories $(BUILD_DIR)/libffblogiocalls.so $(BUILD_DIR)/libffbfixupdate.so

directories: $(BUILD_DIR)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/libffblogiocalls.so: $(SRC_DIR)/libffblogiocalls.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -fPIC -ldl -shared $< -o $@

$(BUILD_DIR)/libffbfixupdate.so: $(SRC_DIR)/libffbfixupdate.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -fPIC -ldl -shared $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

.PHONY: directories clean

clean:
	$(RM) -r $(BUILD_DIR)

-include $(DEPS)
