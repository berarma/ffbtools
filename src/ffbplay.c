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

#define _GNU_SOURCE
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

#define print_option(option, text, ...) printf("  %c. " text "\n", option, ##__VA_ARGS__)

int device_handle;

int set_gain(int gain)
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

int set_autocenter(int level)
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

int upload_effect(struct ff_effect *effect)
{
    /* Upload effect */
    if (ioctl(device_handle, EVIOCSFF, effect) < 0) {
        fprintf(stderr, "ERROR: uploading effect failed (%s) [%s:%d]\n",
                strerror(errno), __FILE__, __LINE__);
        return 0;
    }

    return 1;
}

int play_effect(int id, int count)
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

int remove_effect(int id)
{
    if (ioctl(device_handle, EVIOCRMFF, id)<0) {
        fprintf(stderr, "ERROR: removing effect failed (%s) [%s:%d]\n",
                strerror(errno), __FILE__, __LINE__);
        return 0;
    }

    return 1;
}

void init_effect(struct ff_effect *effect)
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
            effect->u.rumble.weak_magnitude = 0x6000;
            break;
    }
}

char read_option(const char *prompt, const char *options)
{
    char input[50];
    char option;

    printf("> %s: ", prompt);

    do {
        fgets(input, sizeof(input), stdin);

        if (strlen(input) <= 2) {
            option = input[0];
            for (int i = 0; i < strlen(options); i++) {
                if (option == options[i]) {
                    return option;
                }
            }
        }
    } while (true);
}

int read_int(const char *prompt)
{
    char input[50];

    printf("> %s: ", prompt);

    fgets(input, sizeof(input), stdin);

    return strtol(input, NULL, 0);
}

void menu_effect_parameters(struct ff_effect *effect)
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
                print_option('m', "Left saturation: %u", effect->u.condition[0].right_saturation);
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

        option = read_option("Change parameter", "abcdefghijklmnopqrstuvw\n");

        switch (option) {
            case 'a':
                effect->id = read_int("New id:");
                break;
            case 'b':
                effect->direction = read_int("New direction");
                break;
            case 'c':
                effect->replay.length = read_int("New replay length");
                break;
            case 'd':
                effect->replay.delay = read_int("New replay delay");
                break;
            case 'e':
                effect->u.constant.level = read_int("New level");
                break;
            case 'f':
                effect->u.ramp.start_level = read_int("New start level");
                break;
            case 'g':
                effect->u.ramp.end_level = read_int("New end level");
                break;
            case 'h':
                effect->u.periodic.period = read_int("New period");
                break;
            case 'i':
                effect->u.periodic.magnitude = read_int("New magnitude");
                break;
            case 'j':
                effect->u.periodic.offset = read_int("New offset");
                break;
            case 'k':
                effect->u.periodic.phase = read_int("New phase");
                break;
            case 'l':
                effect->u.condition[0].right_saturation = read_int("New right saturation");
                break;
            case 'm':
                effect->u.condition[0].left_saturation = read_int("New left saturation");
                break;
            case 'n':
                effect->u.condition[0].right_coeff = read_int("New right coeff");
                break;
            case 'o':
                effect->u.condition[0].left_coeff = read_int("New left coeff");
                break;
            case 'p':
                effect->u.condition[0].deadband = read_int("New deadband");
                break;
            case 'q':
                effect->u.condition[0].center = read_int("New center");
                break;
            case 'r':
                effect->u.rumble.strong_magnitude = read_int("New strong magnitude");
                break;
            case 's':
                effect->u.rumble.weak_magnitude = read_int("New weak magnitude");
                break;
            case 't':
                envelope->attack_length = read_int("New attack length");
                break;
            case 'u':
                envelope->attack_level = read_int("New attack level");
                break;
            case 'v':
                envelope->fade_length = read_int("New attack length");
                break;
            case 'w':
                envelope->fade_level = read_int("New attack level");
                break;
        }
    } while (option != '\n');
}

void menu_upload_effect()
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
    option = read_option("Select effect type (0 to return)", "0123456789abc");

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

    init_effect(&effect);

    menu_effect_parameters(&effect);

    if (upload_effect(&effect)) {
        printf("* Uploaded effect with id: %d\n", effect.id);
    }
}

void menu_play_effect()
{
    int id;
    int count;

    id = read_int("Effect id");
    count = read_int("Count");

    printf("Playing effect with id %d...\n", id);
    play_effect(id, count);
}

void menu_stop_effect()
{
    int id;

    id = read_int("Effect id");

    printf("Stopping effect with id %d...\n", id);
    play_effect(id, 0);
}

void menu_remove_effect()
{
    int id;

    id = read_int("Effect id");

    if (remove_effect(id)) {
        printf("Removed effect with id %d.\n", id);
    }
}

void menu_set_gain()
{
    int gain;

    gain = read_int("Gain");

    set_gain(gain);
}

void menu_set_autocenter()
{
    int level;

    level = read_int("Level");

    set_autocenter(level);
}

void main_menu()
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
        option = read_option("Select command (q to exit)", "123456q");

        switch (option) {
            case '1':
                menu_upload_effect();
                break;
            case '2':
                menu_play_effect();
                break;
            case '3':
                menu_stop_effect();
                break;
            case '4':
                menu_remove_effect();
                break;
            case '5':
                menu_set_gain();
                break;
            case '6':
                menu_set_autocenter();
                break;
        }
    } while (option != 'q');
}

int main(int argc, char * argv[])
{
    const char *device_name;

    if (argc == 2) {
        device_name = argv[1];
    } else {
        printf("Syntax: %s <device>\n", argv[0]);
        exit(1);
    }

    printf("Force feedback test tool.\n\n");

    /* Open event device with write permission */
    device_handle = open(device_name, O_RDWR|O_NONBLOCK);
    if (device_handle < 0) {
        fprintf(stderr, "ERROR: can not open %s (%s) [%s:%d]\n",
                device_name, strerror(errno), __FILE__, __LINE__);
        exit(1);
    }

    printf("Using device %s.\n\n", device_name);

    printf("CAUTION: The forces applied might be dangerous.\n");

    set_gain(0xffff);

    main_menu();

    close(device_handle);
}
