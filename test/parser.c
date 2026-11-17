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

void parse_string(ELTN_Parser* parser, const char* data) {
    size_t datalen = strlen(data);
    ssize_t actual;

    actual = ELTN_Parser_parse(parser, data, datalen);
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

void assert_current_key_equal(ELTN_Parser* parser, ELTN_Event expectedtype,
                              const char* expected) {
    ELTN_Event type = ELTN_ERROR;
    char* str = NULL;
    size_t len = 0;

    ELTN_Parser_current_key(parser, &type, &str, &len);

    lequal(expectedtype, type);
    lok(str != NULL);
    lequal((int)strlen(expected), (int)len);
    if (str != NULL) {
        lsequal(expected, str);
        free(str);
    }
}

void empty_document() {
    ELTN_Parser* parser = ELTN_Parser_new();

    parse_string(parser, "");

    lequal(ELTN_STREAM_START, ELTN_Parser_event(parser));
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_STREAM_END, ELTN_Parser_event(parser));
    lok(!ELTN_Parser_has_next(parser));

    ELTN_Parser_free(parser);
}

void empty_table() {
    ELTN_Parser* parser = ELTN_Parser_new();

    parse_string(parser, "{}");

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

void triple_table() {
    ELTN_Parser* parser = ELTN_Parser_new();

    parse_string(parser, "{}{}{}");

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
    lequal(ELTN_ERROR, ELTN_Parser_event(parser));
    lequal(ELTN_ERR_UNEXPECTED_TOKEN, ELTN_Parser_error_code(parser));
    lequal(0, (int)ELTN_Parser_depth(parser));
    lok(!ELTN_Parser_has_next(parser));

    ELTN_Parser_free(parser);
}

/*
 * TODO: test that top-level definitions and tables don't mix.
 */

void single_definition() {
    ELTN_Parser* parser = ELTN_Parser_new();

    parse_string(parser, "key = true");

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

void non_identifier_keys() {
    ELTN_Parser* parser = ELTN_Parser_new();

    parse_string(parser, "{['this key works']=true,[1]=true}");

    lequal(ELTN_STREAM_START, ELTN_Parser_event(parser));
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_TABLE_START, ELTN_Parser_event(parser));
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_KEY_STRING, ELTN_Parser_event(parser));
    assert_string_equal(parser, "this key works");
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_VALUE_TRUE, ELTN_Parser_event(parser));
    assert_string_equal(parser, "true");
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_KEY_NUMBER, ELTN_Parser_event(parser));
    assert_string_equal(parser, "1");
    lfequal(1.0, ELTN_Parser_number(parser));
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

void simple_table() {
    ELTN_Parser* parser = ELTN_Parser_new();

    parse_string(parser, "{key=true}");

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

    parse_string(parser, data);

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

void document_with_levels() {
    const char* data =
        "toplevel = { array = { 4, 6, 8 } , key = \"value\" }";

    ELTN_Parser* parser = ELTN_Parser_new();

    parse_string(parser, data);

    lequal(ELTN_STREAM_START, ELTN_Parser_event(parser));
    lequal(0, (int)ELTN_Parser_depth(parser));
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_DEF_NAME, ELTN_Parser_event(parser));
    assert_text_equal(parser, "toplevel");
    assert_string_equal(parser, "toplevel");
    lequal(0, (int)ELTN_Parser_depth(parser));
    assert_current_key_equal(parser, ELTN_DEF_NAME, "toplevel");
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_TABLE_START, ELTN_Parser_event(parser));
    lequal(1, (int)ELTN_Parser_depth(parser));
    assert_current_key_equal(parser, ELTN_DEF_NAME, "toplevel");
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_KEY_STRING, ELTN_Parser_event(parser));
    assert_text_equal(parser, "array");
    assert_string_equal(parser, "array");
    lequal(1, (int)ELTN_Parser_depth(parser));
    assert_current_key_equal(parser, ELTN_KEY_STRING, "array");
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_TABLE_START, ELTN_Parser_event(parser));
    lequal(2, (int)ELTN_Parser_depth(parser));
    assert_current_key_equal(parser, ELTN_KEY_STRING, "array");
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_VALUE_NUMBER, ELTN_Parser_event(parser));
    assert_text_equal(parser, "4");
    assert_string_equal(parser, "4");
    lequal(4, (int)ELTN_Parser_integer(parser));
    lequal(2, (int)ELTN_Parser_depth(parser));
    assert_current_key_equal(parser, ELTN_KEY_INTEGER, "1");
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_VALUE_NUMBER, ELTN_Parser_event(parser));
    assert_text_equal(parser, "6");
    assert_string_equal(parser, "6");
    lequal(6, (int)ELTN_Parser_integer(parser));
    lequal(2, (int)ELTN_Parser_depth(parser));
    assert_current_key_equal(parser, ELTN_KEY_INTEGER, "2");
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_VALUE_NUMBER, ELTN_Parser_event(parser));
    assert_text_equal(parser, "8");
    assert_string_equal(parser, "8");
    lequal(8, (int)ELTN_Parser_integer(parser));
    lequal(2, (int)ELTN_Parser_depth(parser));
    assert_current_key_equal(parser, ELTN_KEY_INTEGER, "3");
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_TABLE_END, ELTN_Parser_event(parser));
    lequal(1, (int)ELTN_Parser_depth(parser));
    assert_current_key_equal(parser, ELTN_KEY_STRING, "array");
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_KEY_STRING, ELTN_Parser_event(parser));
    assert_text_equal(parser, "key");
    assert_string_equal(parser, "key");
    lequal(1, (int)ELTN_Parser_depth(parser));
    assert_current_key_equal(parser, ELTN_KEY_STRING, "key");
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_VALUE_STRING, ELTN_Parser_event(parser));
    assert_text_equal(parser, "\"value\"");
    assert_string_equal(parser, "value");
    lequal(1, (int)ELTN_Parser_depth(parser));
    assert_current_key_equal(parser, ELTN_KEY_STRING, "key");
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_TABLE_END, ELTN_Parser_event(parser));
    lequal(0, (int)ELTN_Parser_depth(parser));
    assert_current_key_equal(parser, ELTN_DEF_NAME, "toplevel");
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_STREAM_END, ELTN_Parser_event(parser));
    lequal(0, (int)ELTN_Parser_depth(parser));
    lok(!ELTN_Parser_has_next(parser));

    ELTN_Parser_free(parser);
}

void long_string_error() {
    const char* data =
        "s = \"long string\"\nt = [[error string";

    ELTN_Parser* parser = ELTN_Parser_new();

    parse_string(parser, data);

    lequal(ELTN_STREAM_START, ELTN_Parser_event(parser));
    lequal(0, (int)ELTN_Parser_depth(parser));
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_DEF_NAME, ELTN_Parser_event(parser));
    assert_string_equal(parser, "s");
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_VALUE_STRING, ELTN_Parser_event(parser));
    assert_string_equal(parser, "long string");
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_DEF_NAME, ELTN_Parser_event(parser));
    assert_string_equal(parser, "t");
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_ERROR, ELTN_Parser_event(parser));
    lequal(ELTN_ERR_INVALID_TOKEN, ELTN_Parser_error_code(parser));
    lequal(2, ELTN_Parser_error_line(parser));
    lequal(5, ELTN_Parser_error_column(parser));
    assert_string_equal(parser, "[[error string");
    lok(!ELTN_Parser_has_next(parser));

    ELTN_Parser_free(parser);
}

void unexpected_token_error() {
    const char* data = "s = 1 false";

    ELTN_Parser* parser = ELTN_Parser_new();

    parse_string(parser, data);

    lequal(ELTN_STREAM_START, ELTN_Parser_event(parser));
    lequal(0, (int)ELTN_Parser_depth(parser));
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_DEF_NAME, ELTN_Parser_event(parser));
    assert_string_equal(parser, "s");
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_VALUE_NUMBER, ELTN_Parser_event(parser));
    assert_string_equal(parser, "1");
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_ERROR, ELTN_Parser_event(parser));
    lequal(ELTN_ERR_UNEXPECTED_TOKEN, ELTN_Parser_error_code(parser));
    lequal(1, ELTN_Parser_error_line(parser));
    lequal(7, ELTN_Parser_error_column(parser));
    assert_string_equal(parser, "false");
    lok(!ELTN_Parser_has_next(parser));

    ELTN_Parser_free(parser);
}

void duplicate_key() {
    ELTN_Parser* parser = ELTN_Parser_new();

    parse_string(parser, "{n=true, n=false}");

    lequal(ELTN_STREAM_START, ELTN_Parser_event(parser));
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_TABLE_START, ELTN_Parser_event(parser));
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_KEY_STRING, ELTN_Parser_event(parser));
    assert_string_equal(parser, "n");
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_VALUE_TRUE, ELTN_Parser_event(parser));
    assert_string_equal(parser, "true");
    lok(ELTN_Parser_has_next(parser));

    ELTN_Parser_next(parser);
    lequal(ELTN_ERROR, ELTN_Parser_event(parser));
    lequal(ELTN_ERR_DUPLICATE_KEY, ELTN_Parser_error_code(parser));
    assert_string_equal(parser, "n");
    lequal(1, ELTN_Parser_error_line(parser));
    lequal(10, ELTN_Parser_error_column(parser));
    lok(!ELTN_Parser_has_next(parser));

    ELTN_Parser_free(parser);
}

/*
 * TODO: verify long strings
 * TODO: turn on comment events, verify events arrive
 * TODO: verify comments
 * TODO: verify long comments
 */

int main() {
    lrun("test_empty_document", empty_document);
    lrun("test_empty_table", empty_table);
    lrun("test_triple_table", triple_table);
    lrun("test_single_definition", single_definition);
    lrun("test_simple_table", simple_table);
    lrun("test_non_identifier_keys", non_identifier_keys);
    lrun("test_complex_document", complex_document);
    lrun("test_document_with_levels", document_with_levels);
    lrun("test_long_string_error", long_string_error);
    lrun("test_unexpected_token_error", unexpected_token_error);
    lrun("test_duplicate_key", duplicate_key);
    lresults();
    return lfails != 0;
}
