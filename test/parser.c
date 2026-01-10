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

#include <stdlib.h>
#include <string.h>
#include "minctest.h"
#include "eltn.h"

void read_string(ELTN_Parser* parser, const char* data) {
    size_t datalen = strlen(data);
    ssize_t actual;

    actual = ELTN_Parser_read_string(parser, data, datalen);
    lequal((int)datalen, (int)actual);
}

void assert_string_equal(ELTN_Parser* parser, const char* expected) {
    char* str = NULL;
    size_t len = 0;

    ELTN_Parser_string(parser, &str, &len);

    lok(str != NULL);
    lequal((int)strlen(expected), (int)len);
    if (str != NULL) {
        lsequal(expected, str);
        free(str);
    }
}

void assert_text_equal(ELTN_Parser* parser, const char* expected) {
    char* str = NULL;
    size_t len = 0;

    ELTN_Parser_text(parser, &str, &len);

    lok(str != NULL);
    lequal((int)strlen(expected), (int)len);
    if (str != NULL) {
        lsequal(expected, str);
        free(str);
    }
}

void empty_document() {
    ELTN_Parser* parser = ELTN_Parser_new();

    read_string(parser, "");

    lequal(ELTN_STREAM_START, ELTN_Parser_event(parser));
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_STREAM_END, ELTN_Parser_event(parser));
    lok(!ELTN_Parser_has_next(parser));

    ELTN_Parser_free(parser);
}

void empty_table() {
    ELTN_Parser* parser = ELTN_Parser_new();

    read_string(parser, "{}");

    lequal(ELTN_STREAM_START, ELTN_Parser_event(parser));
    lequal(0, (int)ELTN_Parser_depth(parser));
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_TABLE_START, ELTN_Parser_event(parser));
    lequal(1, (int)ELTN_Parser_depth(parser));
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_TABLE_END, ELTN_Parser_event(parser));
    lequal(0, (int)ELTN_Parser_depth(parser));
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_STREAM_END, ELTN_Parser_event(parser));
    lequal(0, (int)ELTN_Parser_depth(parser));
    lok(!ELTN_Parser_has_next(parser));

    ELTN_Parser_free(parser);
}

void single_definition() {
    ELTN_Parser* parser = ELTN_Parser_new();

    read_string(parser, "key = true");

    lequal(ELTN_STREAM_START, ELTN_Parser_event(parser));
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_DEF_NAME, ELTN_Parser_event(parser));
    assert_text_equal(parser, "key");
    assert_string_equal(parser, "key");
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_VALUE_TRUE, ELTN_Parser_event(parser));
    assert_text_equal(parser, "true");
    assert_string_equal(parser, "true");
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_STREAM_END, ELTN_Parser_event(parser));
    lok(!ELTN_Parser_has_next(parser));

    ELTN_Parser_free(parser);
}

void simple_table() {
    ELTN_Parser* parser = ELTN_Parser_new();

    read_string(parser, "{key=true}");

    lequal(ELTN_STREAM_START, ELTN_Parser_event(parser));
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_TABLE_START, ELTN_Parser_event(parser));
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_KEY_STRING, ELTN_Parser_event(parser));
    assert_string_equal(parser, "key");
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_VALUE_TRUE, ELTN_Parser_event(parser));
    assert_string_equal(parser, "true");
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_TABLE_END, ELTN_Parser_event(parser));
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_STREAM_END, ELTN_Parser_event(parser));
    lok(!ELTN_Parser_has_next(parser));

    ELTN_Parser_free(parser);
}

void complex_document() {
    const char* data =
        "key1 = { flag = true, number = 22, string = \"foo\" }\n"
        "key2 = { flag = false, number = 0x20, string = \'bar\' }\n"
        "key3 = { flag = nil, number = 3e8, string = \'quux\' }\n";

    ELTN_Parser* parser = ELTN_Parser_new();

    read_string(parser, data);

    lequal(ELTN_STREAM_START, ELTN_Parser_event(parser));
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_DEF_NAME, ELTN_Parser_event(parser));
    assert_text_equal(parser, "key1");
    assert_string_equal(parser, "key1");
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_TABLE_START, ELTN_Parser_event(parser));
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_KEY_STRING, ELTN_Parser_event(parser));
    assert_text_equal(parser, "flag");
    assert_string_equal(parser, "flag");
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_VALUE_TRUE, ELTN_Parser_event(parser));
    assert_text_equal(parser, "true");
    assert_string_equal(parser, "true");
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_KEY_STRING, ELTN_Parser_event(parser));
    assert_text_equal(parser, "number");
    assert_string_equal(parser, "number");
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_VALUE_NUMBER, ELTN_Parser_event(parser));
    assert_text_equal(parser, "22");
    assert_string_equal(parser, "22");
    lequal(22, (int)ELTN_Parser_integer(parser));
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_KEY_STRING, ELTN_Parser_event(parser));
    assert_text_equal(parser, "string");
    assert_string_equal(parser, "string");
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_VALUE_STRING, ELTN_Parser_event(parser));
    assert_text_equal(parser, "\"foo\"");
    assert_string_equal(parser, "foo");
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_TABLE_END, ELTN_Parser_event(parser));
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_DEF_NAME, ELTN_Parser_event(parser));
    assert_text_equal(parser, "key2");
    assert_string_equal(parser, "key2");
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_TABLE_START, ELTN_Parser_event(parser));
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_KEY_STRING, ELTN_Parser_event(parser));
    assert_text_equal(parser, "flag");
    assert_string_equal(parser, "flag");
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_VALUE_FALSE, ELTN_Parser_event(parser));
    assert_text_equal(parser, "false");
    assert_string_equal(parser, "false");
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_KEY_STRING, ELTN_Parser_event(parser));
    assert_text_equal(parser, "number");
    assert_string_equal(parser, "number");
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_VALUE_NUMBER, ELTN_Parser_event(parser));
    assert_text_equal(parser, "0x20");
    assert_string_equal(parser, "0x20");
    lequal(32, (int)ELTN_Parser_integer(parser));
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_KEY_STRING, ELTN_Parser_event(parser));
    assert_text_equal(parser, "string");
    assert_string_equal(parser, "string");
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_VALUE_STRING, ELTN_Parser_event(parser));
    assert_text_equal(parser, "'bar'");
    assert_string_equal(parser, "bar");
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_TABLE_END, ELTN_Parser_event(parser));
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_DEF_NAME, ELTN_Parser_event(parser));
    assert_text_equal(parser, "key3");
    assert_string_equal(parser, "key3");
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_TABLE_START, ELTN_Parser_event(parser));
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_KEY_STRING, ELTN_Parser_event(parser));
    assert_text_equal(parser, "flag");
    assert_string_equal(parser, "flag");
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_VALUE_NIL, ELTN_Parser_event(parser));
    assert_text_equal(parser, "nil");
    assert_string_equal(parser, "nil");
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_KEY_STRING, ELTN_Parser_event(parser));
    assert_text_equal(parser, "number");
    assert_string_equal(parser, "number");
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_VALUE_NUMBER, ELTN_Parser_event(parser));
    assert_text_equal(parser, "3e8");
    assert_string_equal(parser, "3e8");
    lfequal(300000000.0, ELTN_Parser_number(parser));
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_KEY_STRING, ELTN_Parser_event(parser));
    assert_text_equal(parser, "string");
    assert_string_equal(parser, "string");
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_VALUE_STRING, ELTN_Parser_event(parser));
    assert_text_equal(parser, "'quux'");
    assert_string_equal(parser, "quux");
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_TABLE_END, ELTN_Parser_event(parser));
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_STREAM_END, ELTN_Parser_event(parser));
    lok(!ELTN_Parser_has_next(parser));

    ELTN_Parser_free(parser);
}

int main(int argc, char* argv[]) {
    lrun("test_empty_document", empty_document);
    lrun("test_empty_table", empty_table);
    lrun("test_single_definition", single_definition);
    lrun("test_simple_table", simple_table);
    lrun("test_complex_document", complex_document);
    lresults();
    return lfails != 0;
}
