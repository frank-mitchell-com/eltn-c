/*****************************************************************************
 *
 * Copyright 2025 Frank Mitchell
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
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
#include <ctype.h>
#include "econv.h"
#include "ealloc.h"
#include "etokutil.h"
#include "estrbuf.h"

#define KEYWORDS_SIZE 22

#define BUFFER_SIZE   64

#define LONG_STRING_THRESHOLD   80

/*
 * Keywords in Lua 5.4.  5.5 adds "global", but it's a "soft" keyword
 * and might also be a NAME token.
 */
const char* KEYWORDS[] = {
    "and", "break", "do", "else", "elseif", "end",
    "false", "for", "function", "goto", "if", "in",
    "local", "nil", "not", "or", "repeat", "return",
    "then", "true", "until", "while"
};

bool ELTN_is_keyword_string(const char* wordstr, size_t wordlen) {
    char word[wordlen + 1];

    memset(word, 0, sizeof(word));
    strncpy(word, wordstr, wordlen);
    return ELTN_is_keyword(word);
}

static int strptrcmp(const void* a, const void* b) {
    const char** astrptr = (const char **)a;
    const char** bstrptr = (const char **)b;

    return strcmp(*astrptr, *bstrptr);
}

bool ELTN_is_keyword(const char* word) {
    void* result;

    if (word == NULL) {
        return false;
    }
    result = bsearch(&word, KEYWORDS, KEYWORDS_SIZE, sizeof(char *), strptrcmp);

    return result != NULL;
}

bool ELTN_is_name_string(const char* wordstr, size_t wordlen) {
    char word[wordlen + 1];

    memset(word, 0, sizeof(word));
    strncpy(word, wordstr, wordlen);
    return ELTN_is_name(word);
}

bool ELTN_is_name(const char* word) {
    size_t wordlen = strlen(word);

    if (wordlen == 0) {
        return false;
    }
    if (!ELTN_is_name_start(word[0])) {
        return false;
    }
    for (size_t i = 1; i < wordlen; i++) {
        if (!ELTN_is_name_part(word[i])) {
            return false;
        }
    }
    return true;
}

static const char* strnstr(const char* instr, const size_t inlen,
                           const char* searchstr, const size_t searchlen) {
    const char* result = NULL;

    for (size_t i = 0; i < inlen && result == NULL; i++) {
        if (strncmp(instr + i, searchstr, searchlen) == 0) {
            result = instr + i;
            break;
        }
    }
    return result;
}

static void ELTN_quote_long_string(ELTN_Pool* h,
                                   const char8_t* instr, const size_t inlen,
                                   char8_t** outstrptr, size_t* outlenptr) {
    int quote_level = -1;
    char* last_quote = NULL;
    size_t last_quote_len = 0;
    String_Buffer* lqbuffer = String_Buffer_new_with_pool(h);
    String_Buffer* outbuffer;

    // scan instr for long quote marks
    for (int i = 0; i < BUFFER_SIZE; i++) {
        String_Buffer_clear(lqbuffer);
        String_Buffer_append_char(lqbuffer, ']');
        for (int j = 0; j < i; j++) {
            String_Buffer_append_char(lqbuffer, '=');
        }
        String_Buffer_append_char(lqbuffer, ']');
        String_Buffer_to_string(lqbuffer, &last_quote, &last_quote_len);
        /*
         * Can't use strstr() here because instr may not be null-terminated
         */
        if (strnstr
            ((const char *)instr, inlen, (const char *)last_quote,
             last_quote_len) == NULL) {
            quote_level = i;
            break;
        }
        ELTN_free(h, last_quote);
    }
    String_Buffer_free(lqbuffer);

    // bracket with non-conflicting long quotes
    outbuffer = String_Buffer_new_with_pool(h);
    if (quote_level < 0) {
        (*outstrptr) = NULL;
        (*outlenptr) = 0;
        return;
    }
    String_Buffer_append_char(outbuffer, '[');
    for (int j = 0; j < quote_level; j++) {
        String_Buffer_append_char(outbuffer, '=');
    }
    String_Buffer_append_char(outbuffer, '[');
    String_Buffer_append_string(outbuffer, (const char *)instr, inlen);
    String_Buffer_append_string(outbuffer, last_quote, last_quote_len);
    ELTN_free(h, last_quote);
    String_Buffer_to_string(outbuffer, (char **)outstrptr, outlenptr);
    String_Buffer_free(outbuffer);
}

static void append_hex_escape(String_Buffer* strbuf, char8_t c) {
    char tmpbuf[8];
    size_t written;

    memset(tmpbuf, 0, sizeof(tmpbuf));
    written = snprintf(tmpbuf, sizeof(tmpbuf), "\\x%02x", (int)c);
    String_Buffer_append_string(strbuf, tmpbuf, written);
}

void ELTN_quote_string(ELTN_Pool* h,
                       const char8_t* instr, const size_t inlen,
                       char8_t** outstrptr, size_t* outlenptr) {
    if (inlen > LONG_STRING_THRESHOLD) {
        ELTN_quote_long_string(h, instr, inlen, outstrptr, outlenptr);
    } else {
        String_Buffer* strbuf = String_Buffer_new_with_pool(h);

        String_Buffer_append_char(strbuf, '"');
        for (size_t i = 0; i < inlen; i++) {
            char8_t c = instr[i];

            switch (c) {
            case '\0':
                String_Buffer_append_string(strbuf, "\\0", 2);
                break;
            case '\"':
                String_Buffer_append_string(strbuf, "\\\"", 2);
                break;
            case '\'':
                String_Buffer_append_string(strbuf, "\\\'", 2);
                break;
            case 0x07:
                String_Buffer_append_string(strbuf, "\\a", 2);
                break;
            case 0x08:
                String_Buffer_append_string(strbuf, "\\b", 2);
                break;
            case 0x0c:
                String_Buffer_append_string(strbuf, "\\f", 2);
                break;
            case '\n':
                String_Buffer_append_string(strbuf, "\\n", 2);
                break;
            case '\r':
                String_Buffer_append_string(strbuf, "\\r", 2);
                break;
            case '\t':
                String_Buffer_append_string(strbuf, "\\t", 2);
                break;
            case 0x0b:
                String_Buffer_append_string(strbuf, "\\v", 2);
                break;
            default:
                if (isprint(c) || c > 0x7F) {
                    String_Buffer_append_char(strbuf, (char)c);
                } else {
                    append_hex_escape(strbuf, c);
                    break;
                }
            }
        }
        String_Buffer_append_char(strbuf, '"');
        String_Buffer_to_string(strbuf, (char **)outstrptr, outlenptr);
    }
}

static const char8_t* append_octal(const char8_t* instr, char8_t* buf,
                                   size_t* buflenptr) {
    char8_t tmpbuf[4] = { '\0', '\0', '\0', '\0' };
    const char8_t* result = instr;

    for (int i = 0; i < 3; i++) {
        if (ELTN_is_octdigit(*result)) {
            tmpbuf[i] = *result;
            result++;
        } else {
            break;
        }
    }
    buf[*buflenptr] = (char8_t) (0xFF & strtol((const char *)tmpbuf, NULL, 8));
    (*buflenptr) += 1;
    return result;
}

static const char8_t* append_hex(const char8_t* instr, char8_t* buf,
                                 size_t* buflenptr) {
    char8_t tmpbuf[3];

    if (!ELTN_is_hexdigit(instr[0]) || !ELTN_is_hexdigit(instr[1])) {
        /*
           TODO: raise error 
         */
        return instr;
    }
    tmpbuf[0] = instr[0];
    tmpbuf[1] = instr[1];
    tmpbuf[2] = '\0';
    buf[*buflenptr] = (char8_t) (0xFF & strtol((const char *)tmpbuf, NULL, 16));
    (*buflenptr) += 1;
    return instr + 2;
}

static const char8_t* append_unicode(const char8_t* instr,
                                     char8_t* buf, size_t* buflenptr) {
    char32_t cp;
    size_t cplen;
    const char8_t* closebracket =
        (const char8_t *)strchr((const char *)instr, '}');

    if (*instr != '{' || closebracket == NULL) {
        /*
           TODO: flag error 
         */
        return instr;
    }
    cp = (char32_t) strtol((const char *)(instr + 1), NULL, 16);

    cplen = ELTN_Conv_codepoint_to_char8(cp, *buflenptr, buf + (*buflenptr));
    (*buflenptr) += cplen;
    return closebracket + 1;
}

void ELTN_unescape_quoted_string(ELTN_Pool* h,
                                 const char8_t* instr, const size_t inlen,
                                 char8_t** outstrptr, size_t* outlenptr) {
    char8_t* bufptr = ELTN_alloc(h, inlen);
    size_t buflen = 0;
    const char8_t* index = instr;
    char8_t quotechar = '\0';

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
                              const char8_t* instr, const size_t inlen,
                              char8_t** outstrptr, size_t* outlenptr) {
    size_t new_start;
    size_t new_length;
    size_t quote_length;

    // TODO: merge common code with trim_comment(...), below

    // assert instr[0] == "["
    // assert instr[inlen-1] == "]"
    new_start = 1;
    quote_length = 1;
    new_length = inlen;
    while (new_start < inlen && instr[new_start] != '[') {
        new_start++;
        quote_length++;
    }
    new_start++;
    quote_length++;
    if (quote_length * 2 > inlen) {
        new_start = 0;
        new_length = 0;
    } else {
        new_length -= 2 * quote_length;
        if (instr[new_start] == '\n') {
            new_length--;
            new_start++;
        }
    }
    ELTN_copy_string(h, outstrptr, outlenptr, instr + new_start, new_length);
}


void ELTN_format_comment(ELTN_Pool* h,
                         const char8_t* instr, const size_t inlen,
                         char8_t** outstrptr, size_t* outlenptr) {
    String_Buffer* commentbuf = String_Buffer_new_with_pool(h);

    String_Buffer_append_string(commentbuf, "--", 2);

    if (inlen > LONG_STRING_THRESHOLD
        || strnstr((const char *)instr, inlen, "\n", 1)) {
        char8_t* body;
        size_t bodylen;

        ELTN_quote_long_string(h, instr, inlen, &body, &bodylen);
        String_Buffer_append_string(commentbuf, (const char *)body, bodylen);
        ELTN_free(h, body);
    } else {
        String_Buffer_append_char(commentbuf, ' ');
        String_Buffer_append_string(commentbuf, (const char *)instr, inlen);
        String_Buffer_append_char(commentbuf, '\n');
    }
    String_Buffer_to_string(commentbuf, (char **)outstrptr, outlenptr);
    String_Buffer_free(commentbuf);
}


void ELTN_trim_comment(ELTN_Pool* h, const char8_t* instr, const size_t inlen,
                       char8_t** outstrptr, size_t* outlenptr) {
    size_t new_start;
    size_t new_length;
    size_t quote_length;
    bool notaquote;

    // TODO: merge common code with unquote_long_string(...), above

    //assert instr[0:1] == "--"
    if (instr[2] != '[') {
        new_start = 2;
        new_length = inlen - 3;
        quote_length = 0;
    } else {
        new_start = 3;
        quote_length = 1;
        new_length = inlen - 4;
        notaquote = false;
        while (new_start < inlen && instr[new_start] != '[') {
            if (instr[new_start] != '=') {
                notaquote = true;
                break;
            }
            new_start++;
            quote_length++;
        }
        new_start++;
        quote_length++;
        if (!notaquote && quote_length * 2 < inlen) {
            new_length = inlen - (quote_length * 2 + 2);
            if (instr[new_start] == '\n') {
                new_length--;
                new_start++;
            }
        } else {
            new_start = 2;
            new_length = inlen - 3;
            quote_length = 0;
        }
    }
    ELTN_copy_string(h, outstrptr, outlenptr, instr + new_start, new_length);
}

bool ELTN_is_newline(const char8_t* str, size_t len) {
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
