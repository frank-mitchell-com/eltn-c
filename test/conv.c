/*
 * Copyright 2023 Frank Mitchell
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "minctest.h"
#include "convert.h"

#define STRBUFSIZ   512
#define ESCBUFSIZ   32

typedef struct linkedlist {
    struct linkedlist* tail;
    void* str;
} list_t;

static list_t* _strhead = NULL;

static void* stralloc(void* buf, size_t len, size_t csiz) {
    void* result = calloc(len + 1, csiz);

    memcpy(result, buf, len * csiz);

    list_t* prev = _strhead;

    _strhead = calloc(1, sizeof(list_t));
    _strhead->tail = prev;
    _strhead->str = result;
    return result;
}

static int append_ascii(unsigned int c, char buf[], int j) {
    int len = j;

    if (c <= 0x7F) {
        buf[j] = (char)c;
        len += 1;
    } else {
        sprintf(&(buf[len]), "\\u{%x}", c);
        len = strlen(buf);
    }
    return len;
}

static size_t ucslen(const char32_t* ucs) {
    int len = 0;

    while (ucs[len] != 0) {
        len++;
    }
    return len;
}

static const char* ucs2cstr(const char32_t* ucs) {
    char buf[STRBUFSIZ];
    size_t wlen = ucslen(ucs);
    size_t len = 0;

    memset(buf, 0, STRBUFSIZ);
    for (int i = 0; i < wlen; i++) {
        len = append_ascii(ucs[i], buf, len);
    }
    return (char *)stralloc(buf, len, sizeof(char));
}

static size_t jcslen(const char16_t* jcs) {
    int len = 0;

    while (jcs[len] != 0) {
        len++;
    }
    return len;
}

static const char* jcs2cstr(const char16_t* jcs) {
    char buf[STRBUFSIZ];
    size_t jlen = jcslen(jcs);
    size_t len = 0;

    memset(buf, 0, STRBUFSIZ);
    for (int i = 0; i < jlen; i++) {
        len = append_ascii(jcs[i], buf, len);
    }
    return (char *)stralloc(buf, len, sizeof(char));
}

static const char* utf2cstr(const char* utf8str) {
    char buf[STRBUFSIZ];
    size_t slen = strlen(utf8str);
    size_t len = 0;

    memset(buf, 0, STRBUFSIZ);
    for (int i = 0; i < slen; i++) {
        len = append_ascii(0xFF & utf8str[i], buf, len);
    }
    return (char *)stralloc(buf, len, sizeof(char));
}

static const char32_t* cstr2ucs(const char* s) {
    char32_t buf[STRBUFSIZ];
    char escbuf[ESCBUFSIZ];
    size_t len = strlen(s);
    int i = 0;
    int j = 0;

    memset(buf, 0, STRBUFSIZ);
    while (i < len) {
        if (s[i] != '\\') {
            buf[j++] = (char32_t) s[i++];
        } else {
            memset(escbuf, 0, ESCBUFSIZ);
            for (int k = 0; i < len && k < ESCBUFSIZ - 1; k++) {
                escbuf[k] = s[i++];
                if (escbuf[k] == '}') {
                    unsigned int cp = 0;

                    escbuf[k + 1] = '\0';
                    sscanf(escbuf, "\\u{%x}", &cp);
                    if (cp == 0) {
                        cp = (char32_t) '?';
                    }
                    buf[j++] = (char32_t) cp;
                    break;
                }
            }
        }
    }
    lequal((int)len, i);
    return (char32_t *) stralloc(buf, len, sizeof(char32_t));
}

static const char8_t* ucs2utf8(const char32_t* s) {
    size_t bsz;
    char8_t buf[STRBUFSIZ];

    memset(buf, 0, sizeof(buf));
    bsz = C_Conv_char32_to_8(ucslen(s), s, STRBUFSIZ, buf);

    return (char8_t *) stralloc(buf, bsz, sizeof(char8_t));
}

static int free_strings() {
    int count = 0;
    list_t* head = _strhead;

    _strhead = NULL;

    while (head != NULL) {
        list_t* prev = head;

        head = head->tail;
        free(prev->str);
        free(prev);
        count++;
    }
    return count;
}

static void string_smoke() {
    int count = 0;

    lsequal("alpha", ucs2cstr(U"alpha"));
    lsequal("bravo", ucs2cstr(U"bravo"));
    lsequal("charlie", ucs2cstr(U"charlie"));
    lsequal("delta", ucs2cstr(U"delta"));
    lsequal("echo", ucs2cstr(U"echo"));
    lsequal("tsch\\u{fc}\\u{df}", ucs2cstr(U"tsch\u00fc\u00df"));

    const char* actual = ucs2cstr(cstr2ucs("foxtrot"));

    lsequal("foxtrot", actual);

    const char32_t* actual2 = cstr2ucs("tsch\\u{fc}\\u{df}!");
    const char32_t* expect2 = U"tsch\u00fc\u00df!";

    lequal((int)ucslen(expect2), (int)ucslen(actual2));
    for (int i = 0; i < 7; i++) {
        lequal(expect2[i], actual2[i]);
    }

    const char* actual3 = ucs2cstr(actual2);

    lsequal("tsch\\u{fc}\\u{df}!", actual3);

    for (list_t * head = _strhead; head != NULL; head = head->tail) {
        count++;
    }
    lequal(10, count);

    lequal(count, free_strings());
    lok(_strhead == NULL);
}

static void conv_char32_to_8() {
    // Characters taken from Wikipedia article on UTF-8.
    char* expect = "$ \u00A3 \u0418 \u0939 \u20AC \uD55C \U00010348";
    const char32_t* inbuf = U"$ \u00A3 \u0418 \u0939 \u20AC \uD55C \U00010348";
    const size_t insz = ucslen(inbuf);
    unsigned char outbuf[STRBUFSIZ];
    size_t nread, nwritten;
    int errcode;

    memset(outbuf, 0, STRBUFSIZ);

    C_Conv_to_char8(FMC_UCS_4, insz * sizeof(char32_t), (char8_t *) inbuf,
                    STRBUFSIZ, outbuf, &nread, &nwritten);
    errcode = errno;

    lequal(0, errcode);
    lequal(52, (int)nread);
    lequal(24, (int)nwritten);
    lsequal(expect, (char *)outbuf);

    free_strings();
}

static void conv_char16_to_8() {
    // Characters taken from Wikipedia article on UTF-8.
    char* expect = "$ \u00A3 \u0418 \u0939 \u20AC \uD55C \U00010348";
    const char16_t* inbuf = u"$ \u00A3 \u0418 \u0939 \u20AC \uD55C \U00010348";
    const size_t insz = jcslen(inbuf);
    unsigned char outbuf[STRBUFSIZ];
    size_t nread, nwritten;
    int errcode;

    memset(outbuf, 0, STRBUFSIZ);

    C_Conv_to_char8(FMC_UCS_2, insz * sizeof(char16_t), (char8_t *) inbuf,
                    STRBUFSIZ, outbuf, &nread, &nwritten);
    errcode = errno;

    lequal(0, errcode);
    lequal(28, (int)nread);
    lequal(24, (int)nwritten);
    lsequal(expect, (char *)outbuf);
    lsequal(utf2cstr(expect), utf2cstr((char *)outbuf));

    free_strings();
}

/*
 * TODO: More error conditions and bad output.
 * - run out of output buffer
 * - malformed UTF-8 input (e.g. too many or not enough continuation bytes)
 */

static void conv_is_ascii() {
    const char* test1 = "This is ASCII";
    const char* test2 = "This (\xC2\xA3) is not ASCII";

    lequal(true, C_Conv_is_ascii(strlen(test1), test1));
    lequal(false, C_Conv_is_ascii(strlen(test2), test2));
}


int main(int argc, char* argv[]) {
    lrun("test_code_smoke", string_smoke);
    lrun("cconv_is_ascii", conv_is_ascii);
    lrun("cconv_char32_to_8", conv_char32_to_8);
    lrun("cconv_char16_to_8", conv_char16_to_8);
    lresults();
    return lfails != 0;
}
