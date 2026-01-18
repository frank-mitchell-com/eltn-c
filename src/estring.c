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

#include <stdlib.h>
#include <string.h>
#include "convert.h"
#include "ealloc.h"
#include "estring.h"


static const char* append_octal(const char* instr, char* buf, size_t* buflenptr) {
    char tmpbuf[4] = { '\0', '\0', '\0', '\0' };
    const char* result = instr;

    for (int i = 0; i < 3; i++) {
        if (ELTN_is_octdigit(*result)) {
            tmpbuf[i] = *result;
            result++;
        } else {
            break;
        }
    }
    buf[*buflenptr] = (char)(0xFF & strtol(tmpbuf, NULL, 8));
    (*buflenptr) += 1;
    return result;
}

static const char* append_hex(const char* instr, char* buf, size_t* buflenptr) {
    char tmpbuf[3];

    if (!ELTN_is_hexdigit(instr[0]) || !ELTN_is_hexdigit(instr[1])) {
        /*
           TODO: raise error 
         */
        return instr;
    }
    tmpbuf[0] = instr[0];
    tmpbuf[1] = instr[1];
    tmpbuf[2] = '\0';
    buf[*buflenptr] = (char)(0xFF & strtol(tmpbuf, NULL, 16));
    (*buflenptr) += 1;
    return instr + 2;
}

static const char* append_unicode(const char* instr,
                                  char* buf, size_t* buflenptr) {
    char32_t cp;
    size_t cplen;
    const char* closebracket = strchr(instr, '}');

    if (*instr != '{' || closebracket == NULL) {
        /*
           TODO: flag error 
         */
        return instr;
    }
    cp = (char32_t) strtol(instr + 1, NULL, 16);

    cplen = C_Conv_codepoint_to_char8(cp, *buflenptr,
                                      (char8_t *) buf + (*buflenptr));
    (*buflenptr) += cplen;
    return closebracket + 1;
}

void ELTN_unescape_quoted_string(ELTN_Pool* h,
                                 const char* instr, const size_t inlen,
                                 char** outstrptr, size_t* outlenptr) {
    char* bufptr = ELTN_alloc(h, inlen);
    size_t buflen = 0;
    const char* index = instr;
    char quotechar = '\0';

    if (bufptr == NULL) {
        *outstrptr = NULL;
        *outlenptr = 0;
        return;
    }

    if (*index == '\'' || *index == '\"') {
        quotechar = *index;
        index++;
    }
    while (index < instr + inlen && *index != quotechar) {
        if (*index != '\\') {
            bufptr[buflen] = *index;
            buflen++;
            index++;
        } else {
            index++;
            switch (*index) {
            case '\n':
                bufptr[buflen] = '\n';
                buflen++;
                index++;
                break;
            case '\r':
                bufptr[buflen] = '\n';
                buflen++;
                index++;
                if (*index == '\n') {
                    index++;
                }
                break;
            case 'a':
                bufptr[buflen] = 0x07;
                buflen++;
                index++;
                break;
            case 'b':
                bufptr[buflen] = 0x08;
                buflen++;
                index++;
                break;
            case 'f':
                bufptr[buflen] = 0x0c;
                buflen++;
                index++;
                break;
            case 'n':
                bufptr[buflen] = 0x0a;
                buflen++;
                index++;
                break;
            case 'r':
                bufptr[buflen] = 0x0d;
                buflen++;
                index++;
                break;
            case 't':
                bufptr[buflen] = 0x09;
                buflen++;
                index++;
                break;
            case 'v':
                bufptr[buflen] = 0x0b;
                buflen++;
                index++;
                break;
            case 'x':
                index++;
                index = append_hex(index, bufptr, &buflen);
                break;
            case 'u':
                index++;
                index = append_unicode(index, bufptr, &buflen);
                break;
            case 'z':
                index++;
                while (ELTN_is_space(*index)) {
                    index++;
                }
                break;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
                index = append_octal(index, bufptr, &buflen);
                break;
            default:
                bufptr[buflen] = *index;
                buflen++;
                index++;
            }
        }
    }

    *outstrptr = bufptr;
    *outlenptr = buflen;
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
