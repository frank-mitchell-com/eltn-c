/*
 * Copyright 2026 Frank Mitchell
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

#include <stdio.h>
#include <stdlib.h>
#include "minctest.h"
#include "eltn.h"

void write_data(FILE* file, const char* data, size_t datalen) {
    size_t writelen;

    writelen = fwrite(data, sizeof(char), datalen, file);
    lequal(0, ferror(file));
    lequal((int)datalen, (int)writelen);

    rewind(file);
    lequal(0, ferror(file));
    lequal(0, feof(file));
}

void read_data(FILE* file, char* buffer, size_t bufsize, size_t* readlenptr) {
    rewind(file);
    lequal(0, ferror(file));
    lequal(0, feof(file));

    memset(buffer, 0, bufsize);
    (*readlenptr) = fread(buffer, sizeof(char), bufsize - 1, file);
    lequal(0, ferror(file));
}

void file_smoke(void) {
    FILE* file = tmpfile();
    const char* data = "this is a test.\nnothing to see here, move along.\n";
    size_t datalen = strlen(data);
    size_t readlen = 0;
    char buffer[1024];

    write_data(file, data, datalen);

    read_data(file, buffer, sizeof(buffer), &readlen);

    lequal((int)datalen, (int)readlen);
    lsequal(data, buffer);

    fclose(file);
}

void file_read_eltn(void) {
    FILE* file = tmpfile();
    const char* data = "{hello=\"world\"}\n";
    size_t datalen = strlen(data);
    ELTN_Parser* parser = ELTN_Parser_new();
    ssize_t readlen;
    char* value = NULL;
    size_t vallen = 0;

    write_data(file, data, datalen);

    readlen = ELTN_Parser_read_file(parser, file);
    lok(readlen > 0);

    lequal(ELTN_STREAM_START, ELTN_Parser_event(parser));
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_TABLE_START, ELTN_Parser_event(parser));
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_KEY_STRING, ELTN_Parser_event(parser));
    ELTN_Parser_string(parser, &value, &vallen);
    lsequal("hello", value);
    lequal(5, (int)vallen);
    free(value);
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_VALUE_STRING, ELTN_Parser_event(parser));
    ELTN_Parser_string(parser, &value, &vallen);
    lsequal("world", value);
    lequal(5, (int)vallen);
    free(value);
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_TABLE_END, ELTN_Parser_event(parser));
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_STREAM_END, ELTN_Parser_event(parser));
    lok(!ELTN_Parser_has_next(parser));

    ELTN_Parser_free(parser);
    fclose(file);
}

void file_write_eltn() {
    FILE* file = tmpfile();
    const char* expected = "{\n    hello = \"world\",\n}\n";
    size_t expectlen = strlen(expected);
    size_t readlen;
    char buffer[1024];

    ELTN_Emitter* emitter = ELTN_Emitter_new();

    ELTN_Emitter_table_start(emitter);
    ELTN_Emitter_key_string(emitter, "hello", 5);
    ELTN_Emitter_value_string(emitter, "world", 5);
    ELTN_Emitter_table_end(emitter);

    ELTN_Emitter_write_file(emitter, file);

    read_data(file, buffer, sizeof(buffer), &readlen);

    lequal((int)expectlen, (int)readlen);
    lsequal(expected, buffer);

    ELTN_Emitter_free(emitter);
    fclose(file);
}

int main(void) {
    lrun("test_file_smoke", file_smoke);
    lrun("test_file_read_eltn", file_read_eltn);
    lrun("test_file_write_eltn", file_write_eltn);
    lresults();
    return lfails != 0;
}
