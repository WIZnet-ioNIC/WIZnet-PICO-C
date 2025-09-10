#ifndef _CAN_PARSER_H
#define _CAN_PARSER_H
/**
    ----------------------------------------------------------------------------------------------------
    Includes
    ----------------------------------------------------------------------------------------------------
*/
#include <stdio.h>

#include "port_common.h"

#include "pico/stdlib.h"
#include "pico/binary_info.h"

int getInputString(char *buffP);
char* custom_strtok(char* str, const char* delim);

inline int hex_char_to_int(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    } else if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    } else if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    return -1;
}

inline uint32_t string_to_hex32(const char* str) {
    uint32_t result = 0;

    while (*str) {
        int value = hex_char_to_int(*str);
        if (value == -1) {
            printf("Invalid hex character: %c\n", *str);
            return 0;
        }

        if (result > (0xFFFFFFFF >> 4)) {
            printf("Overflow detected: input too large for 32-bit\n");
            return 0;
        }

        result = (result << 4) | value;
        str++;
    }

    return result;
}

inline uint64_t string_to_hex64(const char* str) {
    uint64_t result = 0;

    while (*str) {
        int value = hex_char_to_int(*str);
        if (value == -1) {
            printf("Invalid hex character: %c\n", *str);
            return 0;
        }
        if (result > (0xFFFFFFFFFFFFFFFF >> 4)) {
            printf("Overflow detected: input too large for 64-bit\n");
            return 0;
        }

        result = (result << 4) | value;
        str++;
    }

    return result;
}
#endif