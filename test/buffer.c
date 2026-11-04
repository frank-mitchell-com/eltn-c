/*
 * Copyright 2025 Frank Mitchell
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

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "minctest.h"
#include "econv.h"
#include "ebuffer.h"

#define BUFFER_SIZE     64
#define BUFFER_INCR     3

typedef struct Test_Buffer {
    const char8_t* buf;
    size_t len;
    size_t idx;
    int error;
} Test_Buffer;

static void Test_Buffer_init(Test_Buffer* self, const char8_t* data, int error) {
    self->buf = data;
    self->len = strlen((const char *)data);
    self->idx = 0;
    self->error = error;
}

static int Test_Reader(void* ud, char** strptr, size_t* sizeptr) {
    Test_Buffer* self = (Test_Buffer *) ud;;
    size_t old_idx = self->idx;
    size_t size = 0;
    char* str = NULL;

    if (self->idx >= self->len) {
        (*sizeptr) = 0;
        (*strptr) = NULL;
        return self->error;
    }
    self->idx += BUFFER_INCR;
    if (self->idx > self->len) {
        self->idx = self->len;
    }
    size = self->idx - old_idx;
    str = (char *)calloc(size + 1, sizeof(char));
    memcpy(str, self->buf + old_idx, size);

    (*sizeptr) = size;
    (*strptr) = str;
    return 0;
}

static void read_all_buffer(ELTN_Buffer* buffer, char8_t* outbuf, size_t outlen) {
    int i = 0;

    for (; i < outlen; i++) {
        int32_t curr = ELTN_Buffer_next_char(buffer, true);

        if (curr < 0) {
            break;
        }
        outbuf[i] = (char8_t) curr;
    }
    if (i < outlen) {
        outbuf[i] = (char8_t) '\0';
    }
}

static void print_string(const char* label, const char8_t* str, size_t len,
                         size_t charsize) {
    char outbuf[4096];
    int outind = 0;

    memset(outbuf, 0, sizeof(outbuf));
    for (int i = 0; i < len * charsize; i += charsize) {
        char32_t chr = 0;

        switch (charsize) {
        case 4:
            chr = *(char32_t *) (str + i);
            break;
        case 2:
            chr = *(char16_t *) (str + i);
            break;
        default:
            chr = *(char8_t *) (str + i);
            break;
        }

        switch (chr) {
        case 0:
            sprintf(outbuf + outind, "\\0");
            outind += 2;
            break;
        case ' ':
            sprintf(outbuf + outind, " ");
            outind += 1;
            break;
        default:
            if (isgraph(chr) && chr < 0xFF) {
                outbuf[outind] = (char)chr;
                outind++;
            } else {
                int nprint = sprintf(outbuf + outind, "\\u{%x}", chr);

                outind += nprint;
            }
            break;
        }
    }
    printf("*** DEBUG %s: %s\n", label, outbuf);
}

void buffer_smoke() {
    ELTN_Buffer* buffer = ELTN_Buffer_new_with_pool(NULL);

    lok(buffer != NULL);
    lok(0 == ELTN_Buffer_length(buffer));
    lok(0 < ELTN_Buffer_capacity(buffer));
    lok(ELTN_Buffer_is_empty(buffer));
    lok(!ELTN_Buffer_is_closed(buffer));

    ELTN_Buffer_free(buffer);
}

void buffer_write() {
    ELTN_Buffer* buffer = ELTN_Buffer_new_with_pool(NULL);
    const char* testdata = "hello world";
    const int testdatalen = strlen(testdata);
    char8_t strbuf[BUFFER_SIZE];
    size_t written;

    lok(buffer != NULL);

    memset(strbuf, 0, sizeof(strbuf));

    lequal(0, (int)ELTN_Buffer_length(buffer));
    written = ELTN_Buffer_write(buffer, (char *)testdata, testdatalen);
    lequal(testdatalen, (int)written);
    lequal(testdatalen, (int)ELTN_Buffer_length(buffer));
    lok(!ELTN_Buffer_is_empty(buffer));
    lok(!ELTN_Buffer_is_closed(buffer));

    ELTN_Buffer_close(buffer);
    lok(ELTN_Buffer_is_closed(buffer));

    lok(!ELTN_Buffer_is_empty(buffer));
    read_all_buffer(buffer, strbuf, sizeof(strbuf));

    lequal(testdatalen, (int)strlen((const char *)strbuf));
    lsequal(testdata, (const char *)strbuf);

    lok(ELTN_Buffer_is_empty(buffer));

    ELTN_Buffer_free(buffer);
}


void buffer_read() {
    ELTN_Buffer* buffer = ELTN_Buffer_new_with_pool(NULL);
    const char* data = "this is a medium sized string.";
    Test_Buffer testbuf;
    char8_t outbuf[BUFFER_SIZE];

    Test_Buffer_init(&testbuf, (char8_t *) data, 0);
    lok(0 < ELTN_Buffer_read(buffer, Test_Reader, &testbuf));
    memset(outbuf, 0, sizeof(outbuf));
    read_all_buffer(buffer, outbuf, sizeof(outbuf));
    lsequal(data, (const char *)outbuf);

    ELTN_Buffer_free(buffer);
}

void buffer_ring_cycle() {
    ELTN_Buffer* buffer = ELTN_Buffer_new_with_pool(NULL);
    char8_t outbuf[BUFFER_SIZE];

    lok(buffer != NULL);

    memset(outbuf, 0, sizeof(outbuf));

    lok(ELTN_Buffer_set_capacity(buffer, 5));
    lequal(5, (int)ELTN_Buffer_capacity(buffer));

    lequal(3, (int)ELTN_Buffer_write(buffer, "abc", 3));
    memset(outbuf, 0, sizeof(outbuf));
    read_all_buffer(buffer, outbuf, sizeof(outbuf));
    lsequal("abc", (char *)outbuf);
    lequal(5, (int)ELTN_Buffer_capacity(buffer));

    lequal(3, (int)ELTN_Buffer_write(buffer, "def", 3));
    memset(outbuf, 0, sizeof(outbuf));
    read_all_buffer(buffer, outbuf, sizeof(outbuf));
    lsequal("def", (char *)outbuf);
    lequal(5, (int)ELTN_Buffer_capacity(buffer));

    lequal(3, (int)ELTN_Buffer_write(buffer, "ghi", 3));
    memset(outbuf, 0, sizeof(outbuf));
    read_all_buffer(buffer, outbuf, sizeof(outbuf));
    lsequal("ghi", (char *)outbuf);
    lequal(5, (int)ELTN_Buffer_capacity(buffer));

    lequal(3, (int)ELTN_Buffer_write(buffer, "jkl", 3));
    memset(outbuf, 0, sizeof(outbuf));
    read_all_buffer(buffer, outbuf, sizeof(outbuf));
    lsequal("jkl", (char *)outbuf);
    lequal(5, (int)ELTN_Buffer_capacity(buffer));

    lequal(3, (int)ELTN_Buffer_write(buffer, "mno", 3));
    memset(outbuf, 0, sizeof(outbuf));
    read_all_buffer(buffer, outbuf, sizeof(outbuf));
    lsequal("mno", (char *)outbuf);
    lequal(5, (int)ELTN_Buffer_capacity(buffer));

    ELTN_Buffer_free(buffer);
}

void buffer_ring_resize() {
    ELTN_Buffer* buffer = ELTN_Buffer_new_with_pool(NULL);
    char8_t outbuf[BUFFER_SIZE];

    lok(buffer != NULL);

    memset(outbuf, 0, sizeof(outbuf));

    lok(ELTN_Buffer_set_capacity(buffer, 5));
    lequal(5, (int)ELTN_Buffer_capacity(buffer));

    lequal(3, (int)ELTN_Buffer_write(buffer, "abc", 3));
    memset(outbuf, 0, sizeof(outbuf));
    read_all_buffer(buffer, outbuf, sizeof(outbuf));
    lsequal("abc", (char *)outbuf);
    lequal(5, (int)ELTN_Buffer_capacity(buffer));

    lequal(3, (int)ELTN_Buffer_write(buffer, "def", 3));
    lequal(3, (int)ELTN_Buffer_write(buffer, "ghi", 3));
    lequal(3, (int)ELTN_Buffer_write(buffer, "jkl", 3));
    lequal(3, (int)ELTN_Buffer_write(buffer, "mno", 3));
    memset(outbuf, 0, sizeof(outbuf));
    read_all_buffer(buffer, outbuf, sizeof(outbuf));
    lsequal("defghijklmno", (char *)outbuf);

    ELTN_Buffer_free(buffer);
}

void buffer_write_bom32() {
    ELTN_Buffer* buffer = ELTN_Buffer_new_with_pool(NULL);
    const char32_t* testdata = U"\ufeffhello world";
    const char* expect = "hello world";
    const int testdatalen = (strlen(expect) + 1) * 4;
    const int expectlen = strlen(expect);
    char8_t strbuf[BUFFER_SIZE];
    size_t written;

    lok(buffer != NULL);

    memset(strbuf, 0, sizeof(strbuf));

    lequal(0, (int)ELTN_Buffer_length(buffer));

    written = ELTN_Buffer_write(buffer, (char *)testdata, testdatalen);
    lequal(expectlen, (int)written);
    lequal(expectlen, (int)ELTN_Buffer_length(buffer));

    ELTN_Buffer_close(buffer);

    read_all_buffer(buffer, strbuf, sizeof(strbuf));

    lequal(expectlen, (int)strlen((const char *)strbuf));
    lsequal(expect, (const char *)strbuf);

    lok(ELTN_Buffer_is_empty(buffer));

    ELTN_Buffer_free(buffer);
}

void buffer_write_bom16() {
    ELTN_Buffer* buffer = ELTN_Buffer_new_with_pool(NULL);
    const char16_t* testdata = u"\ufeffhello world";
    const char* expect = "hello world";
    const int testdatalen = (strlen(expect) + 1) * 2;
    const int expectlen = strlen(expect);
    char8_t strbuf[BUFFER_SIZE];
    size_t written;

    lok(buffer != NULL);

    memset(strbuf, 0, sizeof(strbuf));

    lequal(0, (int)ELTN_Buffer_length(buffer));

    written = ELTN_Buffer_write(buffer, (char *)testdata, testdatalen);
    lequal(expectlen, (int)written);
    lequal(expectlen, (int)ELTN_Buffer_length(buffer));

    ELTN_Buffer_close(buffer);

    read_all_buffer(buffer, strbuf, sizeof(strbuf));

    lequal(expectlen, (int)strlen((const char *)strbuf));
    lsequal(expect, (const char *)strbuf);

    lok(ELTN_Buffer_is_empty(buffer));

    ELTN_Buffer_free(buffer);
}

void buffer_write_bom8() {
    ELTN_Buffer* buffer = ELTN_Buffer_new_with_pool(NULL);
    const char* testdata = "\ufeffhello world";
    const char* expect = "hello world";
    const int testdatalen = strlen(testdata);
    const int expectlen = strlen(expect);
    char8_t strbuf[BUFFER_SIZE];
    size_t written;

    lok(buffer != NULL);

    memset(strbuf, 0, sizeof(strbuf));

    lequal(0, (int)ELTN_Buffer_length(buffer));

    written = ELTN_Buffer_write(buffer, (char *)testdata, testdatalen);
    lequal(expectlen, (int)written);
    lequal(expectlen, (int)ELTN_Buffer_length(buffer));

    ELTN_Buffer_close(buffer);

    read_all_buffer(buffer, strbuf, sizeof(strbuf));

    lequal(expectlen, (int)strlen((const char *)strbuf));
    lsequal(expect, (const char *)strbuf);

    lok(ELTN_Buffer_is_empty(buffer));

    ELTN_Buffer_free(buffer);
}


int main(int argc, char* argv[]) {
    lrun("test_buffer_smoke", buffer_smoke);
    lrun("test_buffer_read", buffer_read);
    lrun("test_buffer_write", buffer_write);
    lrun("test_buffer_ring_cycle", buffer_ring_cycle);
    lrun("test_buffer_ring_resize", buffer_ring_resize);
    lrun("test_buffer_write_bom32", buffer_write_bom32);
    lrun("test_buffer_write_bom16", buffer_write_bom16);
    lrun("test_buffer_write_bom8", buffer_write_bom8);
    lresults();
    return lfails != 0;
}
