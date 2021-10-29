/*
 *
 * fftest.c
 *
 * Force feedback test command
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

#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>

#define print_option(option, text, ...) printf("  %c. " text "\n", option, ##__VA_ARGS__)

int device_handle;

int ffbt_set_gain(int gain)
{
    struct input_event event;

    /* Start effect */
    memset(&event, 0, sizeof(event));
    event.type = EV_FF;
    event.code = FF_GAIN;
    event.value = gain;
    if (write(device_handle,&event,sizeof(event)) != sizeof(event)) {
        fprintf(stderr, "ERROR: setting gain failed (%s) [%s:%d]\n",
                strerror(errno), __FILE__, __LINE__);
        return 0;
    }

    return 1;
}

int ffbt_set_autocenter(int level)
{
    struct input_event event;

    /* Start effect */
    memset(&event, 0, sizeof(event));
    event.type = EV_FF;
    event.code = FF_AUTOCENTER;
    event.value = level;
    if (write(device_handle,&event,sizeof(event)) != sizeof(event)) {
        fprintf(stderr, "ERROR: setting gain failed (%s) [%s:%d]\n",
                strerror(errno), __FILE__, __LINE__);
        return 0;
    }

    return 1;
}

int ffbt_upload_effect(struct ff_effect *effect)
{
    /* Upload effect */
    if (ioctl(device_handle, EVIOCSFF, effect) < 0) {
        fprintf(stderr, "ERROR: uploading effect failed (%s) [%s:%d]\n",
                strerror(errno), __FILE__, __LINE__);
        return 0;
    }

    return 1;
}

int ffbt_play_effect(int id, int count)
{
    struct input_event event;

    /* Start effect */
    memset(&event, 0, sizeof(event));
    event.type = EV_FF;
    event.code = id;
    event.value = count;
    if (write(device_handle, &event, sizeof(event)) != sizeof(event)) {
        fprintf(stderr, "ERROR: starting effect failed (%s) [%s:%d]\n",
                strerror(errno), __FILE__, __LINE__);
        return 0;
    }

    return 1;
}

int ffbt_remove_effect(int id)
{
    if (ioctl(device_handle, EVIOCRMFF, id)<0) {
        fprintf(stderr, "ERROR: removing effect failed (%s) [%s:%d]\n",
                strerror(errno), __FILE__, __LINE__);
        return 0;
    }

    return 1;
}

void ffbt_init_effect(struct ff_effect *effect)
{
    effect->id = -1;
    effect->trigger.button = 0;
    effect->trigger.interval = 0;
    effect->replay.length = 0;
    effect->replay.delay = 0;
    effect->direction = 0xC000;
    switch (effect->type) {
        case FF_CONSTANT:
            effect->u.constant.level = 0x6000;
            effect->u.constant.envelope.attack_length = 0;
            effect->u.constant.envelope.attack_level = 0;
            effect->u.constant.envelope.fade_length = 0;
            effect->u.constant.envelope.fade_level = 0;
            break;
        case FF_RAMP:
            effect->u.ramp.start_level = 0x0000;
            effect->u.ramp.end_level = 0x6000;
            effect->u.ramp.envelope.attack_length = 0;
            effect->u.ramp.envelope.attack_level = 0;
            effect->u.ramp.envelope.fade_length = 0;
            effect->u.ramp.envelope.fade_level = 0;
            break;
        case FF_PERIODIC:
            effect->u.periodic.period = 1000;
            effect->u.periodic.magnitude = 0x6000;
            effect->u.periodic.offset = 0;
            effect->u.periodic.phase = 0;
            effect->u.periodic.envelope.attack_length = 0;
            effect->u.periodic.envelope.attack_level = 0;
            effect->u.periodic.envelope.fade_length = 0;
            effect->u.periodic.envelope.fade_level = 0;
            break;
        case FF_SPRING:
        case FF_DAMPER:
        case FF_FRICTION:
        case FF_INERTIA:
            effect->u.condition[0].left_saturation = 0xffff;
            effect->u.condition[0].right_saturation = 0xffff;
            effect->u.condition[0].left_coeff = 0x4000;
            effect->u.condition[0].right_coeff = 0x4000;
            effect->u.condition[0].deadband = 0;
            effect->u.condition[0].center = 0;
            break;
        case FF_RUMBLE:
            effect->u.rumble.strong_magnitude = 0x6000;
            effect->u.rumble.weak_magnitude = 0x2000;
            break;
    }
}

void ffbt_simple_effect(struct ff_effect *effect)
{
    ffbt_init_effect(effect);

    switch (effect->type) {
        case FF_CONSTANT:
            effect->u.constant.level = 0x6000;
            break;
        case FF_RAMP:
            effect->u.ramp.end_level = 0x6000;
            break;
        case FF_PERIODIC:
            effect->u.periodic.period = 1000;
            effect->u.periodic.magnitude = 0x6000;
            break;
        case FF_SPRING:
        case FF_DAMPER:
        case FF_FRICTION:
        case FF_INERTIA:
            effect->u.condition[0].left_coeff = 0x4000;
            effect->u.condition[0].right_coeff = 0x4000;
            break;
        case FF_RUMBLE:
            effect->u.rumble.strong_magnitude = 0x6000;
            effect->u.rumble.weak_magnitude = 0x2000;
            break;
    }
}

char ffbt_read_option(const char *prompt, const char *options)
{
    char input[50];
    char option;

    printf("> %s: ", prompt);

    do {
        if (!fgets(input, sizeof(input), stdin)) {
            return '\n';
        }

        if (strlen(input) <= 2) {
            option = input[0];
            for (size_t i = 0; i < strlen(options); i++) {
                if (option == options[i]) {
                    return option;
                }
            }
        }
    } while (true);
}

int ffbt_read_int(const char *prompt)
{
    char input[50];

    printf("> %s: ", prompt);

    if (!fgets(input, sizeof(input), stdin)) {
        return 0;
    }

    return strtol(input, NULL, 0);
}

void ffbt_menu_effect_parameters(struct ff_effect *effect)
{
    struct ff_envelope *envelope = NULL;
    char option;

    do {
        print_option('a', "id: %d", effect->id);
        print_option('b', "Direction: %u", effect->direction);
        print_option('c', "Replay length: %u", effect->replay.length);
        print_option('d', "Replay delay: %u", effect->replay.delay);
        switch (effect->type) {
            case FF_CONSTANT:
                print_option('e', "Level: %d", effect->u.constant.level);
                envelope = &effect->u.constant.envelope;
                break;
            case FF_RAMP:
                print_option('f', "Start level: %d", effect->u.ramp.start_level);
                print_option('g', "End level: %d", effect->u.ramp.end_level);
                envelope = &effect->u.ramp.envelope;
                break;
            case FF_PERIODIC:
                print_option('h', "Period: %u", effect->u.periodic.period);
                print_option('i', "Magnitude: %d", effect->u.periodic.magnitude);
                print_option('j', "Offset: %d", effect->u.periodic.offset);
                print_option('k', "Phase: %u", effect->u.periodic.phase);
                envelope = &effect->u.periodic.envelope;
                break;
            case FF_SPRING:
            case FF_DAMPER:
            case FF_FRICTION:
            case FF_INERTIA:
                print_option('l', "Right saturation: %u", effect->u.condition[0].right_saturation);
                print_option('m', "Left saturation: %u", effect->u.condition[0].left_saturation);
                print_option('n', "Right coeff: %d", effect->u.condition[0].right_coeff);
                print_option('o', "Left coeff: %d", effect->u.condition[0].left_coeff);
                print_option('p', "Deadband: %u", effect->u.condition[0].deadband);
                print_option('q', "Center: %d", effect->u.condition[0].center);
                break;
            case FF_RUMBLE:
                print_option('r', "Strong magnitude: %u", effect->u.rumble.strong_magnitude);
                print_option('s', "Weak magnitude: %u", effect->u.rumble.weak_magnitude);
                break;
        }
        if (envelope) {
            print_option('t', "Attack length: %u", envelope->attack_length);
            print_option('u', "Attack level: %u", envelope->attack_level);
            print_option('v', "Fade length: %u", envelope->fade_length);
            print_option('w', "Fade level: %u", envelope->fade_level);
        }

        option = ffbt_read_option("Change parameter", "abcdefghijklmnopqrstuvw\n");

        switch (option) {
            case 'a':
                effect->id = ffbt_read_int("New id");
                break;
            case 'b':
                effect->direction = ffbt_read_int("New direction");
                break;
            case 'c':
                effect->replay.length = ffbt_read_int("New replay length");
                break;
            case 'd':
                effect->replay.delay = ffbt_read_int("New replay delay");
                break;
            case 'e':
                effect->u.constant.level = ffbt_read_int("New level");
                break;
            case 'f':
                effect->u.ramp.start_level = ffbt_read_int("New start level");
                break;
            case 'g':
                effect->u.ramp.end_level = ffbt_read_int("New end level");
                break;
            case 'h':
                effect->u.periodic.period = ffbt_read_int("New period");
                break;
            case 'i':
                effect->u.periodic.magnitude = ffbt_read_int("New magnitude");
                break;
            case 'j':
                effect->u.periodic.offset = ffbt_read_int("New offset");
                break;
            case 'k':
                effect->u.periodic.phase = ffbt_read_int("New phase");
                break;
            case 'l':
                effect->u.condition[0].right_saturation = ffbt_read_int("New right saturation");
                break;
            case 'm':
                effect->u.condition[0].left_saturation = ffbt_read_int("New left saturation");
                break;
            case 'n':
                effect->u.condition[0].right_coeff = ffbt_read_int("New right coeff");
                break;
            case 'o':
                effect->u.condition[0].left_coeff = ffbt_read_int("New left coeff");
                break;
            case 'p':
                effect->u.condition[0].deadband = ffbt_read_int("New deadband");
                break;
            case 'q':
                effect->u.condition[0].center = ffbt_read_int("New center");
                break;
            case 'r':
                effect->u.rumble.strong_magnitude = ffbt_read_int("New strong magnitude");
                break;
            case 's':
                effect->u.rumble.weak_magnitude = ffbt_read_int("New weak magnitude");
                break;
            case 't':
                envelope->attack_length = ffbt_read_int("New attack length");
                break;
            case 'u':
                envelope->attack_level = ffbt_read_int("New attack level");
                break;
            case 'v':
                envelope->fade_length = ffbt_read_int("New attack length");
                break;
            case 'w':
                envelope->fade_level = ffbt_read_int("New attack level");
                break;
        }
    } while (option != '\n');
}

void ffbt_menu_upload_effect()
{
    struct ff_effect effect;
    int option;

    printf("Effect types:\n");
    print_option('1', "Constant force");
    print_option('2', "Spring");
    print_option('3', "Damper");
    print_option('4', "Friction");
    print_option('5', "Inertia");
    print_option('6', "Ramp");
    print_option('7', "Sine");
    print_option('8', "Square");
    print_option('9', "Triangle");
    print_option('a', "Saw up");
    print_option('b', "Saw down");
    print_option('c', "Rumble");
    option = ffbt_read_option("Select effect type (1 to return)", "0123456789abc");

    switch (option) {
        case '1':
            effect.type = FF_CONSTANT;
            break;
        case '2':
            effect.type = FF_SPRING;
            break;
        case '3':
            effect.type = FF_DAMPER;
            break;
        case '4':
            effect.type = FF_FRICTION;
            break;
        case '5':
            effect.type = FF_INERTIA;
            break;
        case '6':
            effect.type = FF_RAMP;
            break;
        case '7':
            effect.type = FF_PERIODIC;
            effect.u.periodic.waveform = FF_SINE;
            break;
        case '8':
            effect.type = FF_PERIODIC;
            effect.u.periodic.waveform = FF_SQUARE;
            break;
        case '9':
            effect.type = FF_PERIODIC;
            effect.u.periodic.waveform = FF_TRIANGLE;
            break;
        case 'a':
            effect.type = FF_PERIODIC;
            effect.u.periodic.waveform = FF_SAW_UP;
            break;
        case 'b':
            effect.type = FF_PERIODIC;
            effect.u.periodic.waveform = FF_SAW_DOWN;
            break;
        case 'c':
            effect.type = FF_RUMBLE;
            break;
        case '0':
            return;
    }

    ffbt_simple_effect(&effect);

    ffbt_menu_effect_parameters(&effect);

    if (ffbt_upload_effect(&effect)) {
        printf("* Uploaded effect with id: %d\n", effect.id);
    }
}

void ffbt_menu_play_effect()
{
    int id;
    int count;

    id = ffbt_read_int("Effect id");
    count = ffbt_read_int("Count");

    printf("Playing effect with id %d...\n", id);
    ffbt_play_effect(id, count);
}

void ffbt_menu_stop_effect()
{
    int id;

    id = ffbt_read_int("Effect id");

    printf("Stopping effect with id %d...\n", id);
    ffbt_play_effect(id, 0);
}

void ffbt_menu_remove_effect()
{
    int id;

    id = ffbt_read_int("Effect id");

    if (ffbt_remove_effect(id)) {
        printf("Removed effect with id %d.\n", id);
    }
}

void ffbt_menu_set_gain()
{
    int gain;

    gain = ffbt_read_int("Gain");

    ffbt_set_gain(gain);
}

void menu_set_autocenter()
{
    int level;

    level = ffbt_read_int("Level");

    ffbt_set_autocenter(level);
}

void ffbt_main_menu()
{
    char option;

    do {
        printf("Commands:\n");
        print_option('1', "Upload effect");
        print_option('2', "Play effect");
        print_option('3', "Stop effect");
        print_option('4', "Remove effect");
        print_option('5', "Set gain");
        print_option('6', "Set autocenter");
        option = ffbt_read_option("Select command (q to exit)", "123456q");

        switch (option) {
            case '1':
                ffbt_menu_upload_effect();
                break;
            case '2':
                ffbt_menu_play_effect();
                break;
            case '3':
                ffbt_menu_stop_effect();
                break;
            case '4':
                ffbt_menu_remove_effect();
                break;
            case '5':
                ffbt_menu_set_gain();
                break;
            case '6':
                menu_set_autocenter();
                break;
        }
    } while (option != 'q');
}

void ffbt_new_effect(struct ff_effect *effect, char *params)
{
    char *next_param;
    char *param;
    char *key;
    char *value;
    int nvalue;


    value = strstr(params, "type:") + 5;
    if (strstr(value, "CONSTANT") == value) {
        effect->type = FF_CONSTANT;
    } else if (strstr(value, "RAMP") == value) {
        effect->type = FF_RAMP;
    } else if (strstr(value, "SPRING") == value) {
        effect->type = FF_SPRING;
    } else if (strstr(value, "DAMPER") == value) {
        effect->type = FF_DAMPER;
    } else if (strstr(value, "FRICTION") == value) {
        effect->type = FF_FRICTION;
    } else if (strstr(value, "INERTIA") == value) {
        effect->type = FF_INERTIA;
    } else if (strstr(value, "PERIODIC") == value) {
        effect->type = FF_PERIODIC;
    } else if (strstr(value, "RUMBLE") == value) {
        effect->type = FF_RUMBLE;
    }

    ffbt_init_effect(effect);

    for(; (param = strtok_r(params, " ", &next_param)); params = NULL) {
        key = strtok_r(param, ":", &value);
        if (effect->type == FF_PERIODIC && !strcmp(key, "waveform")) {
            if (!strcmp(value, "SINE")) {
                effect->u.periodic.waveform = FF_SINE;
            } else if (!strcmp(value, "SQUARE")) {
                effect->u.periodic.waveform = FF_SQUARE;
            } else if (!strcmp(value, "TRIANGLE")) {
                effect->u.periodic.waveform = FF_TRIANGLE;
            } else if (!strcmp(value, "SAW_UP")) {
                effect->u.periodic.waveform = FF_SAW_UP;
            } else if (!strcmp(value, "SAW_DOWN")) {
                effect->u.periodic.waveform = FF_SAW_DOWN;
            }
        } else {
            nvalue = strtol(value, NULL, 0);
            if (!strcmp(key, "id")) {
                effect->id = nvalue;
            } else if (!strcmp(key, "length")) {
                effect->replay.length = nvalue;
            } else if (!strcmp(key, "delay")) {
                effect->replay.delay = nvalue;
            } else if (!strcmp(key, "dir")) {
                effect->direction = nvalue;
            } else {
                switch (effect->type) {
                    case FF_CONSTANT:
                        if (!strcmp(key, "level")) {
                            effect->u.constant.level = nvalue;
                        }
                        break;
                    case FF_RAMP:
                        if (!strcmp(key, "start_level")) {
                            effect->u.ramp.start_level = nvalue;
                        } else if (!strcmp(key, "end_level")) {
                            effect->u.ramp.end_level = nvalue;
                        }
                        break;
                    case FF_SPRING:
                        if (!strcmp(key, "deadband")) {
                            effect->u.condition[0].deadband = nvalue;
                        } else if (!strcmp(key, "center")) {
                            effect->u.condition[0].center = nvalue;
                        }
                        // fall through
                    case FF_DAMPER:
                    case FF_FRICTION:
                    case FF_INERTIA:
                        if (!strcmp(key, "left_saturation")) {
                            effect->u.condition[0].left_saturation = nvalue;
                        } else if (!strcmp(key, "right_saturation")) {
                            effect->u.condition[0].right_saturation = nvalue;
                        } else if (!strcmp(key, "left_coeff")) {
                            effect->u.condition[0].left_coeff = nvalue;
                        } else if (!strcmp(key, "right_coeff")) {
                            effect->u.condition[0].right_coeff = nvalue;
                        }
                        break;
                    case FF_RUMBLE:
                        if (!strcmp(key, "strong_rumble")) {
                            effect->u.rumble.strong_magnitude = nvalue;
                        } else if (!strcmp(key, "weak_rumble")) {
                            effect->u.rumble.weak_magnitude = nvalue;
                        }
                        break;
                    case FF_PERIODIC:
                        if (!strcmp(key, "period")) {
                            effect->u.periodic.period = nvalue;
                        } else if (!strcmp(key, "magnitude")) {
                            effect->u.periodic.magnitude = nvalue;
                        } else if (!strcmp(key, "offset")) {
                            effect->u.periodic.offset = nvalue;
                        } else if (!strcmp(key, "phase")) {
                            effect->u.periodic.phase = nvalue;
                        }
                        break;
                }
            }
        }
    }
}

void ffbt_play_file(const char *file_name, int trace_mode)
{
    FILE *file = fopen(file_name, "r");
    struct timespec now;
    struct timespec start_time;
    struct ff_effect effect;
    char line[1024];
    char *token;
    char *next_token;
    char *prefix;
    char *op;
    unsigned long time, delta_time;
    int first = 1;
    int id;
    int count;
    int ids[255] = {-1};
    int save_id = -1;
    int new_id;
    int return_code;

    if (file == NULL) {
        printf("Error: %s", strerror(errno));
        exit(1);
    }

    printf("Playing %s\n\n", file_name);

    while (fgets(line, sizeof(line), file)) {
        if (trace_mode) {
            printf("%s\n", line);
        }
        strtok(line, "\n");
        token = strtok_r(line, " ", &next_token);
        time = strtol(token, NULL, 10);
        if (first) {
            first = 0;
            clock_gettime(CLOCK_MONOTONIC, &start_time);
            start_time.tv_sec -= time / 1000000;
            start_time.tv_nsec -= (time % 1000000) * 1000;
        } else {
            clock_gettime(CLOCK_MONOTONIC, &now);
            delta_time = (now.tv_sec - start_time.tv_sec) * 1000000 +
                (now.tv_nsec - start_time.tv_nsec) / 1000;
            if (time > delta_time) {
                usleep(time - delta_time);
            }
        }
        if (next_token[0] == '#') {
            printf("%s\n", next_token);
            continue;
        } else {
            prefix = strtok_r(NULL, " ", &next_token);
            if (prefix[0] == '<') {
                if (save_id == -1) {
                    continue;
                }
                return_code = strtol(strtok_r(NULL, " ", &next_token), NULL, 10);
                if (return_code != 0) {
                    save_id = -1;
                    continue;
                }
                token = strtok_r(NULL, ":", &next_token);
                new_id = strtol(next_token, NULL, 10);
                ids[new_id] = save_id;
                save_id = -1;
                continue;
            }
            save_id = -1;
            op = strtok_r(NULL, " ", &next_token);
            if (!strcmp(op, "QUERY") || !strcmp(op, "SLOTS")) {
                continue;
            }
        }
        if (!strcmp(op, "GAIN")) {
            ffbt_set_gain(strtol(next_token, NULL, 0));
        } else if (!strcmp(op, "AUTOCENTER")) {
            ffbt_set_autocenter(strtol(next_token, NULL, 0));
        } else if (!strcmp(op, "UPLOAD")) {
            ffbt_new_effect(&effect, next_token);
            if (effect.id == -1) {
                ffbt_upload_effect(&effect);
                save_id = effect.id;
            } else if (ids[effect.id] != -1) {
                effect.id = ids[effect.id];
                ffbt_upload_effect(&effect);
            }
        } else if (!strcmp(op, "PLAY")) {
            id = strtol(strtok_r(NULL, " ", &next_token), NULL, 0);
            count = strtol(strtok_r(NULL, " ", &next_token), NULL, 0);
            if (ids[id] != -1) {
                ffbt_play_effect(ids[id], count);
            }
        } else if (!strcmp(op, "STOP")) {
            id = strtol(strtok_r(NULL, " ", &next_token), NULL, 0);
            if (ids[id] != -1) {
                ffbt_play_effect(ids[id], 0);
            }
        } else if (!strcmp(op, "REMOVE")) {
            id = strtol(strtok_r(NULL, " ", &next_token), NULL, 0);
            ffbt_remove_effect(id);
            ids[id] = -1;
        }
    }
}

int main(int argc, char * argv[])
{
    const char *device_name;
    const char *file_name;
    int interactive_mode = 0;
    int trace_mode = 0;
    int c;

    if (argc == 1) {
        printf("Syntax: %s -d <device> [-i] [-t] [file]\n", argv[0]);
        exit(1);
    }

    opterr = 0;

    while ((c = getopt(argc, argv, "d:it")) != -1) {
        switch (c)
        {
            case 'd':
                device_name = optarg;
                break;
            case 'i':
                interactive_mode = 1;
                break;
            case 't':
                trace_mode = 1;
                break;
            case '?':
                if (optopt == 'd')
                    fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                else if (isprint (optopt))
                    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                else
                    fprintf(stderr,
                            "Unknown option character `\\x%x'.\n",
                            optopt);
                return 1;
            default:
                abort();
        }
    }

    if (interactive_mode == 0) {
        if (optind == argc) {
            fprintf(stderr, "Missing input file for playback.\n");
            return 1;
        }
        if ((optind + 1) < argc) {
            fprintf(stderr, "Too many arguments.\n");
            return 1;
        }
        file_name = argv[optind];
    }

    printf("Force feedback playback tool.\n\n");

    /* Open event device with write permission */
    device_handle = open(device_name, O_RDWR|O_NONBLOCK);
    if (device_handle < 0) {
        fprintf(stderr, "ERROR: can not open %s (%s) [%s:%d]\n",
                device_name, strerror(errno), __FILE__, __LINE__);
        exit(1);
    }

    printf("Using device %s.\n\n", device_name);

    printf("CAUTION: The forces applied might be dangerous.\n\n");

    ffbt_set_gain(0xffff);

    if (interactive_mode) {
        ffbt_main_menu();
    } else {
        ffbt_play_file(file_name, trace_mode);
    }

    close(device_handle);
}
