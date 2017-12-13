#include "awoxl_client.h"
#include "awoxl_protocol.h"
#include "awoxl_util.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static const char* COMMAND_USAGE =
    "\nAllowed commands: \n"
    "- on\n"
    "- off\n"
    "- brightness (increase|decrease|2-11)\n"
    "- white (increase|decrease|2-11)\n"
    "- rgb (random|(0-255),(0-255),(0-255))\n";

int invalid_command() {
    fprintf(stderr, "Error: Invalid command usage!\n");
    fprintf(stderr, COMMAND_USAGE);
    return 1;
}

void parse_brightness_white(char** args, int args_count, int* value,
        int increase, int decrease) {
    const char* arg = args_count >= 1 ? args[0] : "";
    if (strcmp(arg, "increase") == 0)
        *value = increase;
    else if (strcmp(arg, "decrease") == 0)
        *value = decrease;
    else if (sscanf(arg, "%d", value) != 1) {
        exit(invalid_command());
    }
}

int main(int argc, char** argv) {
    srand_milliseconds();

    char* mac = NULL;
    char* command = NULL;
    char** arguments = NULL;
    int arguments_count = 0;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-b") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: -b option: You have to specify a valid mac adress!\n");
                return 1;
            }
            i++;
            mac = argv[i];
        } else {
            command = argv[i];
            arguments = &argv[i + 1];
            arguments_count = argc - i - 1;
            break;
        }
    }

    if (command == NULL) {
        fprintf(stderr, "Usage: %s [-b mac-adress] command args...\n", argv[0]);
        fprintf(stderr, COMMAND_USAGE);
        return 1;
    }
    
    bdaddr_t dst;
    if (mac != NULL && parse_mac(mac, &dst) != 0) {
        fprintf(stderr, "Error: -b option: You have to specify a valid mac adress!\n");
        return 1;
    }

    int c = -1;
    int value = 0;
    unsigned char r, g, b;
    if (strcmp(command, "on") == 0) {
        c = 1;
    } else if (strcmp(command, "off") == 0) {
        c = 2;
    } else if (strcmp(command, "brightness") == 0) {
        c = 3;
        parse_brightness_white(arguments, arguments_count, &value,
                AWOXL_BRIGHTNESS_INCREASE, AWOXL_BRIGHTNESS_DECREASE);
        if (value < 0 || value > 11)
            return invalid_command();
    } else if (strcmp(command, "white") == 0) {
        c = 4;
        parse_brightness_white(arguments, arguments_count, &value,
                AWOXL_TEMPERATURE_INCREASE, AWOXL_TEMPERATURE_DECREASE);
        if (value < 0 || value > 11)
            return invalid_command();
    } else if (strcmp(command, "rgb") == 0) {
        c = 5;
        if (parse_rgb(arguments, arguments_count, &r, &g, &b) != 0)
            return invalid_command();
    } else {
        return invalid_command();
    }

    int sock = awoxl_connect(dst);
    if (sock < 0) {
        fprintf(stderr, "Error: Unable to connect! Errno: %d\n", errno);
        return 1;
    }

    awoxl_on(sock);

    if (c == 1) {
        awoxl_on(sock);
        // printf("on\n");
    } else if (c == 2) {
        awoxl_off(sock);
        // printf("off\n");
    } else if (c == 3) {
        awoxl_brightness(sock, value);
        //printf("brightness %d\n", value);
    } else if (c == 4) {
        awoxl_white(sock, value);
        // printf("white %d\n", value);
    } else if (c == 5) {
        awoxl_rgb(sock, r, g, b);
        // printf("rgb %d,%d,%d\n", r, g, b);
    }

    awoxl_disconnect(sock);
}

