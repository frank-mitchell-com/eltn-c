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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "minctest.h"
#include "convert.h"
#include "esource.h"

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

static void read_all_source(ELTN_Source* source, char8_t* outbuf, size_t outlen) {
    int i = 0;

    for (; i < outlen; i++) {
        int32_t curr = ELTN_Source_next_char(source, true);

        if (curr < 0) {
            break;
        }
        outbuf[i] = (char8_t) curr;
    }
    if (i < outlen) {
        outbuf[i] = (char8_t) '\0';
    }
}

void source_smoke() {
    ELTN_Source* source = ELTN_Source_new_with_pool(NULL);

    lok(source != NULL);
    lok(0 == ELTN_Source_length(source));
    lok(0 < ELTN_Source_capacity(source));
    lok(ELTN_Source_is_empty(source));
    lok(!ELTN_Source_is_closed(source));

    ELTN_Source_free(source);
}

void source_write() {
    ELTN_Source* source = ELTN_Source_new_with_pool(NULL);
    const char* testdata = "hello world";
    const int testdatalen = strlen(testdata);
    char8_t buffer[BUFFER_SIZE];

    lok(source != NULL);

    memset(buffer, 0, sizeof(buffer));

    lequal(0, (int)ELTN_Source_length(source));
    lequal(testdatalen, (int)ELTN_Source_write(source, testdata, testdatalen));
    lequal(testdatalen, (int)ELTN_Source_length(source));
    lok(!ELTN_Source_is_empty(source));
    lok(!ELTN_Source_is_closed(source));

    ELTN_Source_close(source);
    lok(ELTN_Source_is_closed(source));

    lok(!ELTN_Source_is_empty(source));
    read_all_source(source, buffer, sizeof(buffer));

    lequal(testdatalen, (int)strlen((const char *)buffer));
    lsequal(testdata, (const char *)buffer);

    lok(ELTN_Source_is_empty(source));

    ELTN_Source_free(source);
}


void source_read() {
    ELTN_Source* source = ELTN_Source_new_with_pool(NULL);
    const char* data = "this is a medium sized string.";
    Test_Buffer testbuf;
    char8_t outbuf[BUFFER_SIZE];

    Test_Buffer_init(&testbuf, (char8_t *) data, 0);
    lok(0 < ELTN_Source_read(source, Test_Reader, &testbuf));
    memset(outbuf, 0, sizeof(outbuf));
    read_all_source(source, outbuf, sizeof(outbuf));
    lsequal(data, (const char *)outbuf);

    ELTN_Source_free(source);
}

void source_buffer_cycle() {
    ELTN_Source* source = ELTN_Source_new_with_pool(NULL);
    char8_t buffer[BUFFER_SIZE];

    lok(source != NULL);

    memset(buffer, 0, sizeof(buffer));

    lok(ELTN_Source_set_capacity(source, 5));
    lequal(5, (int)ELTN_Source_capacity(source));

    lequal(3, (int)ELTN_Source_write(source, "abc", 3));
    memset(buffer, 0, sizeof(buffer));
    read_all_source(source, buffer, sizeof(buffer));
    lsequal("abc", (char *)buffer);
    lequal(5, (int)ELTN_Source_capacity(source));

    lequal(3, (int)ELTN_Source_write(source, "def", 3));
    memset(buffer, 0, sizeof(buffer));
    read_all_source(source, buffer, sizeof(buffer));
    lsequal("def", (char *)buffer);
    lequal(5, (int)ELTN_Source_capacity(source));

    lequal(3, (int)ELTN_Source_write(source, "ghi", 3));
    memset(buffer, 0, sizeof(buffer));
    read_all_source(source, buffer, sizeof(buffer));
    lsequal("ghi", (char *)buffer);
    lequal(5, (int)ELTN_Source_capacity(source));

    lequal(3, (int)ELTN_Source_write(source, "jkl", 3));
    memset(buffer, 0, sizeof(buffer));
    read_all_source(source, buffer, sizeof(buffer));
    lsequal("jkl", (char *)buffer);
    lequal(5, (int)ELTN_Source_capacity(source));

    lequal(3, (int)ELTN_Source_write(source, "mno", 3));
    memset(buffer, 0, sizeof(buffer));
    read_all_source(source, buffer, sizeof(buffer));
    lsequal("mno", (char *)buffer);
    lequal(5, (int)ELTN_Source_capacity(source));

    ELTN_Source_free(source);
}

void source_buffer_resize() {
    ELTN_Source* source = ELTN_Source_new_with_pool(NULL);
    char8_t buffer[BUFFER_SIZE];

    lok(source != NULL);

    memset(buffer, 0, sizeof(buffer));

    lok(ELTN_Source_set_capacity(source, 5));
    lequal(5, (int)ELTN_Source_capacity(source));

    lequal(3, (int)ELTN_Source_write(source, "abc", 3));
    memset(buffer, 0, sizeof(buffer));
    read_all_source(source, buffer, sizeof(buffer));
    lsequal("abc", (char *)buffer);
    lequal(5, (int)ELTN_Source_capacity(source));

    lequal(3, (int)ELTN_Source_write(source, "def", 3));
    lequal(3, (int)ELTN_Source_write(source, "ghi", 3));
    lequal(3, (int)ELTN_Source_write(source, "jkl", 3));
    lequal(3, (int)ELTN_Source_write(source, "mno", 3));
    memset(buffer, 0, sizeof(buffer));
    read_all_source(source, buffer, sizeof(buffer));
    lsequal("defghijklmno", (char *)buffer);

    ELTN_Source_free(source);
}

int main(int argc, char* argv[]) {
    lrun("test_source_smoke", source_smoke);
    lrun("test_source_read", source_read);
    lrun("test_source_write", source_write);
    lrun("test_source_buffer_cycle", source_buffer_cycle);
    lrun("test_source_buffer_resize", source_buffer_resize);
    lresults();
    return lfails != 0;
}
