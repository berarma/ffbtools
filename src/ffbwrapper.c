/*
 *
 * libffbdebug.c
 *
 * Logs IOCTL and write calls to the FFB subsystem
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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#define ioctl ioctl_trash_function
#include <linux/input.h>
#undef ioctl

#define ioctlRequestCode(request) (request & ((_IOC_DIRMASK << _IOC_DIRSHIFT) | (_IOC_TYPEMASK << _IOC_TYPESHIFT) | (_IOC_NRMASK << _IOC_NRSHIFT)))

#define testBit(bit, array) ((array[bit/8] >> bit%8) & 1)

#define report(...) snprintf(report_string, sizeof(report_string), __VA_ARGS__); output(report_string);

static void init() __attribute__((constructor));

static int dev_major = 0;
static int dev_minor = 0;
static int enable_logger = 0;
static int enable_update_fix = 0;
static int enable_direction_fix = 0;
static int enable_features_hack = 0;
static int enable_force_inversion = 0;
static int ignore_set_gain = 0;
static int enable_offset_fix = 0;
static FILE *log_file = NULL;
static char report_string[1024];
static short last_effect_used = 16;

static void output(char *message)
{
    static struct timespec t0 = {0, 0};
    struct timespec now;
    unsigned long reltime;

    if (!enable_logger) {
        return;
    }

    clock_gettime(CLOCK_MONOTONIC, &now);

    if (t0.tv_sec == 0 && t0.tv_nsec == 0) {
        t0.tv_sec = now.tv_sec;
        t0.tv_nsec = now.tv_nsec;
    }

    reltime = (now.tv_sec - t0.tv_sec) * 1.0e6 + (now.tv_nsec - t0.tv_nsec) / 1.0e3;

    fprintf(log_file, "%012lu %s\n", reltime, message);
    fflush(log_file);
}

static void init()
{
    const char *str_dev_major = getenv("FFBTOOLS_DEV_MAJOR");
    const char *str_dev_minor = getenv("FFBTOOLS_DEV_MINOR");

    if (str_dev_major != NULL && str_dev_minor != NULL) {
        dev_major = strtol(str_dev_major, NULL, 0);
        dev_minor = strtol(str_dev_minor, NULL, 0);
    }

    const char *str_logger = getenv("FFBTOOLS_LOGGER");
    if (str_logger != NULL && strcmp(str_logger, "1") == 0) {
        const char *filename = getenv("FFBTOOLS_LOG_FILE");
        if (filename != NULL) {
            log_file = fopen(filename, "a");
            if (log_file == NULL) {
                printf("Cannot create log file.\n");
            } else {
                enable_logger = 1;
            }
        }
    }

    const char *str_update_fix = getenv("FFBTOOLS_UPDATE_FIX");
    if (str_update_fix != NULL && strcmp(str_update_fix, "1") == 0) {
        enable_update_fix = 1;
    }

    const char *str_direction_fix = getenv("FFBTOOLS_DIRECTION_FIX");
    if (str_direction_fix != NULL && strcmp(str_direction_fix, "1") == 0) {
        enable_direction_fix = 1;
    }

    const char *str_features_hack = getenv("FFBTOOLS_FEATURES_HACK");
    if (str_features_hack != NULL && strcmp(str_features_hack, "1") == 0) {
        enable_features_hack = 1;
    }

    const char *str_force_inversion = getenv("FFBTOOLS_FORCE_INVERSION");
    if (str_force_inversion != NULL && strcmp(str_force_inversion, "1") == 0) {
        enable_force_inversion = 1;
    }

    const char *str_ignore_set_gain = getenv("FFBTOOLS_IGNORE_SET_GAIN");
    if (str_ignore_set_gain != NULL && strcmp(str_ignore_set_gain, "1") == 0) {
        ignore_set_gain = 1;
    }

    const char *str_offset_fix = getenv("FFBTOOLS_OFFSET_FIX");
    if (str_offset_fix != NULL && strcmp(str_offset_fix, "1") == 0) {
        enable_offset_fix = 1;
    }

    if (enable_logger && ftell(log_file) == 0) {
        report("# DEVICE_NAME=%s, UPDATE_FIX=%d,"
                "DIRECTION_FIX=%d, FEATURES_HACK=%d,"
                "FORCE_INVERSION=%d, IGNORE_SET_GAIN=%d, OFFSET_FIX=%d",
                getenv("FFBTOOLS_DEVICE_NAME"), enable_update_fix,
                enable_direction_fix, enable_features_hack,
                enable_force_inversion, ignore_set_gain, enable_offset_fix);
    }
}

static int checkDescriptor(int fd)
{
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
    static char string[256];
    static char effect_params[256];
    struct ff_effect *effect = NULL;
    char *waveform = "UNKNOWN";

    if (!_ioctl) {
        _ioctl = dlsym(RTLD_NEXT, "ioctl");
    }

    if (!checkDescriptor(fd)) {
        return _ioctl(fd, request, argp);
    }

    switch (ioctlRequestCode(request)) {
        case ioctlRequestCode(EVIOCGBIT(EV_FF, 0)):
            report("> QUERY # Query force feedback features.");
            break;
        case ioctlRequestCode(EVIOCGEFFECTS):
            report("> SLOTS # Get maximum number of simultaneous effects in memory.");
            break;
        case ioctlRequestCode(EVIOCRMFF):
            report("> REMOVE %d # Remove effect from memory.", (int)((intptr_t)argp));
            break;
        case ioctlRequestCode(EVIOCSFF):
            effect = (struct ff_effect*) argp;
            effect->replay.length = 0x4e20;
            char *type = "UNKNOWN";
            switch (effect->type) {
                case FF_RUMBLE:
                    type = "RUMBLE";
                    snprintf(effect_params, sizeof(effect_params),
                            "strong:%u, weak:%u",
                            effect->u.rumble.strong_magnitude,
                            effect->u.rumble.weak_magnitude);
                    break;
                case FF_CONSTANT:
                    type = "CONSTANT";
                    snprintf(effect_params, sizeof(effect_params),
                            "level:%d attack_length:%u attack_level:%u "
                            "fade_length:%u fade_level:%u",
                            effect->u.constant.level,
                            effect->u.constant.envelope.attack_length,
                            effect->u.constant.envelope.attack_level,
                            effect->u.constant.envelope.fade_length,
                            effect->u.constant.envelope.fade_level);
                    break;
                case FF_RAMP:
                    type = "RAMP";
                    snprintf(effect_params, sizeof(effect_params),
                            "start_level:%d end_level:%d attack_length:%u "
                            "attack_level:%u fade_length:%u fade_level:%u",
                            effect->u.ramp.start_level,
                            effect->u.ramp.end_level,
                            effect->u.ramp.envelope.attack_length,
                            effect->u.ramp.envelope.attack_level,
                            effect->u.ramp.envelope.fade_length,
                            effect->u.ramp.envelope.fade_level);
                    break;
                case FF_PERIODIC:
                    type = "PERIODIC";
                    switch (effect->u.periodic.waveform) {
                        case FF_SQUARE:
                            waveform = "SQUARE";
                            break;
                        case FF_TRIANGLE:
                            waveform = "TRIANGLE";
                            break;
                        case FF_SINE:
                            waveform = "SINE";
                            break;
                        case FF_SAW_UP:
                            waveform = "SAW_UP";
                            break;
                        case FF_SAW_DOWN:
                            waveform = "SAW_DOWN";
                            break;
                        case FF_CUSTOM:
                            waveform = "CUSTOM";
                            break;
                    }
                    snprintf(effect_params, sizeof(effect_params),
                            "waveform:%s period:%u magnitude:%d offset:%d "
                            "phase:%u attack_length:%u attack_level:%u "
                            "fade_length:%u fade_level:%u",
                            waveform, effect->u.periodic.period,
                            effect->u.periodic.magnitude,
                            effect->u.periodic.offset,
                            effect->u.periodic.phase,
                            effect->u.periodic.envelope.attack_length,
                            effect->u.periodic.envelope.attack_level,
                            effect->u.periodic.envelope.fade_length,
                            effect->u.periodic.envelope.fade_level);
                    break;
                case FF_SPRING:
                    type = "SPRING";
                    break;
                case FF_FRICTION:
                    type = "FRICTION";
                    break;
                case FF_DAMPER:
                    type = "DAMPER";
                    break;
                case FF_INERTIA:
                    type = "INERTIA";
                    break;
            }

            if (effect->type == FF_SPRING || effect->type == FF_FRICTION || effect->type == FF_DAMPER || effect->type == FF_INERTIA) {
                snprintf(effect_params, sizeof(effect_params),
                        "right_saturation:%u left_saturation:%u right_coeff:%d "
                        "left_coeff:%d deadband:%u center:%d",
                        effect->u.condition[0].right_saturation,
                        effect->u.condition[0].left_saturation,
                        effect->u.condition[0].right_coeff,
                        effect->u.condition[0].left_coeff,
                        effect->u.condition[0].deadband,
                        effect->u.condition[0].center);
            }

            int modified = enable_direction_fix | enable_force_inversion;

            report("%s> UPLOAD id:%d dir:%d length:%d delay:%d type:%s %s",
                    modified ? "#" : "", effect->id,
                    effect->direction, effect->replay.length,
                    effect->replay.delay, type, effect_params);

            if (enable_direction_fix && (effect->direction == 0 || effect->direction == 0x8000)) {
                effect->direction -= 0x4000;
                report("> UPLOAD id:%d dir:%d type:%s length:%d delay:%d %s "
                        "# direction fix", effect->id, effect->direction, type,
                        effect->replay.length, effect->replay.delay,
                        effect_params);
            }

            if (enable_force_inversion) {
                effect->direction -= 0x8000;
                report("> UPLOAD id:%d dir:%d type:%s length:%d delay:%d %s "
                        "# force inversion fix", effect->id, effect->direction,
                        type, effect->replay.length, effect->replay.delay,
                        effect_params);
            }

            if (effect->type == FF_PERIODIC && enable_offset_fix) {
                effect->u.periodic.offset = (int)effect->u.periodic.offset * 0x7fff / 10000;
                effect->u.periodic.phase = (int)effect->u.periodic.phase * 0xffff / 35999;
                snprintf(effect_params, sizeof(effect_params),
                        "waveform:%s, period:%u, magnitude:%d, offset:%d, phase:%u, attack_length:%u, attack_level:%u, fade_length:%u, fade_level:%u",
                        waveform, effect->u.periodic.period,
                        effect->u.periodic.magnitude,
                        effect->u.periodic.offset,
                        effect->u.periodic.phase,
                        effect->u.periodic.envelope.attack_length,
                        effect->u.periodic.envelope.attack_level,
                        effect->u.periodic.envelope.fade_length,
                        effect->u.periodic.envelope.fade_level);
                report("%s> UPLOAD id:%d dir:%d length:%d delay:%d type:%s %s",
                        modified ? "#" : "", effect->id,
                        effect->direction, effect->replay.length,
                        effect->replay.delay, type, effect_params);
            }
            break;
    }

    int result = _ioctl(fd, request, argp);

    switch (ioctlRequestCode(request)) {
        case ioctlRequestCode(EVIOCGBIT(EV_FF, 0)):
            {
                int passes = 0;
                do {
                    strcpy(string, "");
                    if (testBit(FF_CONSTANT, argp)) strcat(string, " Constant");
                    if (testBit(FF_PERIODIC, argp)) {
                        strcat(string, " Periodic (");
                        if (testBit(FF_SQUARE, argp)) strcat(string, " Square");
                        if (testBit(FF_TRIANGLE, argp)) strcat(string, " Triangle");
                        if (testBit(FF_SINE, argp)) strcat(string, " Sine");
                        if (testBit(FF_SAW_UP, argp)) strcat(string, " Saw up");
                        if (testBit(FF_SAW_DOWN, argp)) strcat(string, " Saw down");
                        if (testBit(FF_CUSTOM, argp)) strcat(string, " Custom");
                        strcat(string, " )");
                    }
                    if (testBit(FF_RAMP, argp)) strcat(string, " Ramp");
                    if (testBit(FF_SPRING, argp)) strcat(string, " Spring");
                    if (testBit(FF_FRICTION, argp)) strcat(string, " Friction");
                    if (testBit(FF_DAMPER, argp)) strcat(string, " Damper");
                    if (testBit(FF_RUMBLE, argp)) strcat(string, " Rumble");
                    if (testBit(FF_INERTIA, argp)) strcat(string, " Inertia");
                    if (testBit(FF_GAIN, argp)) strcat(string, " Gain");
                    if (testBit(FF_AUTOCENTER, argp)) strcat(string, " Autocenter");
                    if (passes) {
                        report("< %d, %s # features hack", result, string);
                        break;
                    } else {
                        if (enable_features_hack) {
                            report("#< %d, %s", result, string);
                        } else {
                            report("< %d, %s", result, string);
                        }
                    }
                    if (enable_features_hack) {
                        memset(argp, 255, _IOC_SIZE(request));
                        passes++;
                    }
                } while (enable_features_hack);
            }
            break;
        case ioctlRequestCode(EVIOCRMFF):
            if (enable_features_hack) {
                report("#< %d", result);
            } else {
                report("< %d", result);
            }
            if (enable_features_hack && result != 0) {
                result = 0;
                report("< %d # features hack", result);
            }
            break;
        case ioctlRequestCode(EVIOCGEFFECTS):
            if (enable_features_hack) {
                report("#< %d, effects: %d", result, *((int*)argp));
            } else {
                report("< %d, effects: %d", result, *((int*)argp));
            }
            break;
        case ioctlRequestCode(EVIOCSFF):
            effect = (struct ff_effect*) argp;

            if (enable_update_fix && result < 0 && errno == EINVAL && effect->id >= 0) {
                report("#< %d id:%d", result, effect->id);
                effect->id = -1;
                result = _ioctl(fd, request, argp);
                report("< %d id:%d # update fix", result, effect->id);
            } else if (enable_features_hack && result != 0) {
                report("#< %d id:%d", result, effect->id);
                if (effect->id == -1) {
                    effect->id = last_effect_used++;
                }
                result = 0;
                report("< %d id:%d # features hack", result, effect->id);
            } else {
                report("< %d id:%d", result, effect->id);
            }
            break;
    }

    return result;
}

ssize_t write(int fd, const void *buf, size_t num)
{
    static ssize_t (*_write)(int fd, const void *buf, size_t num) = NULL;
    struct input_event *event = NULL;
    int result;

    if (!_write) {
        _write = dlsym(RTLD_NEXT, "write");
    }

    event = (struct input_event*) buf;

    if (!checkDescriptor(fd) || event->type != EV_FF) {
        return _write(fd, buf, num);
    }

    switch (event->code) {
        case FF_GAIN:
            if (ignore_set_gain) {
                report("#> GAIN %d (ignored)", event->value);
            } else {
                report("> GAIN %d", event->value);
            }
            break;
        case FF_AUTOCENTER:
            report("> AUTOCENTER %d", event->value);
            break;
        default:
            if (event->value) {
                report("> PLAY %u %d", event->code, event->value);
            } else {
                report("> STOP %u", event->code);
            }
            break;
    }

    if (!ignore_set_gain || event->code != FF_GAIN) {
        result = _write(fd, buf, num);
    } else {
        result = 0;
    }

    report("< %d", result);

    if (enable_features_hack && result != 0 && event->code < FF_MAX_EFFECTS) {
        result = 0;
        report("< %d # features hack", result);
    }

    return result;
}
