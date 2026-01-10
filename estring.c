/*****************************************************************************
 *
 * Copyright 2025 Frank Mitchell
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the “Software”),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 ****************************************************************************/

#include "convert.h"
#include "ealloc.h"
#include "estring.h"

void ELTN_unescape_quoted_string(ELTN_Pool* h,
                                 const char* instr, const size_t inlen,
                                 char** outstrptr, size_t* outlenptr) {
    /*
       TODO: Wrong! 
     */
    ELTN_new_string(outstrptr, outlenptr, instr + 1, inlen - 2);
}

void ELTN_unquote_long_string(ELTN_Pool* h,
                              const char* instr, const size_t inlen,
                              char** outstrptr, size_t* outlenptr) {
    /*
       TODO: Wrong! 
     */
    ELTN_new_string(outstrptr, outlenptr, instr, inlen);
}

void ELTN_trim_comment(ELTN_Pool* h, const char* instr, const size_t inlen,
                       char** outstrptr, size_t* outlenptr) {
    /*
       TODO: Wrong! 
     */
    ELTN_new_string(outstrptr, outlenptr, instr, inlen);
}

bool ELTN_is_newline(const char* str, size_t len) {
    if (len >= 1 && str[0] == '\n') {
        /*
         * Unix-style newline.
         */
        return true;
    }
    if (len >= 2 && str[0] == '\r' && str[1] == '\n') {
        /*
         * Windows-style newline
         */
        return true;
    }
    return false;
}

bool ELTN_is_space(uint32_t c) {
    switch (c) {
    case '\f':
    case '\n':
    case '\r':
    case '\t':
    case '\v':
    case ' ':
        return true;
    default:
        return false;
    }
}

bool ELTN_is_letter(uint32_t c) {
    /*
     * Using a big switch statement because <ctype.h> functions
     * depend on locale, while ELTN's idea of a letter doesn't.
     */
    switch (c) {
    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
    case 'G':
    case 'H':
    case 'I':
    case 'J':
    case 'K':
    case 'L':
    case 'M':
    case 'N':
    case 'O':
    case 'P':
    case 'Q':
    case 'R':
    case 'S':
    case 'T':
    case 'U':
    case 'V':
    case 'W':
    case 'X':
    case 'Y':
    case 'Z':
    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
    case 'g':
    case 'h':
    case 'i':
    case 'j':
    case 'k':
    case 'l':
    case 'm':
    case 'n':
    case 'o':
    case 'p':
    case 'q':
    case 'r':
    case 's':
    case 't':
    case 'u':
    case 'v':
    case 'w':
    case 'x':
    case 'y':
    case 'z':
        return true;
    default:
        return false;
    }
}

bool ELTN_is_digit(uint32_t c) {
    switch (c) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        return true;
    default:
        return false;
    }
}

bool ELTN_is_hexdigit(uint32_t c) {
    switch (c) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
        return true;
    default:
        return false;
    }
}

bool ELTN_is_octdigit(uint32_t c) {
    switch (c) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
        return true;
    default:
        return false;
    }
}

bool ELTN_is_name_start(uint32_t c) {
    return c == '_' || ELTN_is_letter(c);
}

bool ELTN_is_name_part(uint32_t c) {
    return ELTN_is_name_start(c) || ELTN_is_digit(c);
}

bool ELTN_is_number_part(uint32_t c) {
    switch (c) {
    case '.':
    case '+':
    case '-':
    case 'e':
    case 'E':
    case 'p':
    case 'P':
    case 'x':
    case 'X':
        return true;
    default:
        return ELTN_is_hexdigit(c);
    }
}
