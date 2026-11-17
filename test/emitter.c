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
#include <ctype.h>
#include "minctest.h"
#include "eltn.h"

static void debug_string(const char* string, const size_t len) {
    printf("DEBUG: <<");

    for (size_t i = 0; i < len; i++) {
        char c = string[i];
        if (c == ' ' || isprint(c)) {
            printf("%c", c);
        } else {
            printf("\\x%02x", (int)c);
        }
    }
    printf(">>\n");
}

static void assert_current_path(ELTN_Emitter* emitter, const char* path) {
    char* str = NULL;
    size_t len = 0;

    ELTN_Emitter_current_path(emitter, &str, &len);
    lok(str != NULL);
    if (str != NULL) {
        lsequal(path, str);
    }
    free(str);
}

void emitter_smoke() {
    ELTN_Emitter* emitter = ELTN_Emitter_new();
    char* result = NULL;
    size_t reslen = 0;

    lequal(0, (int)ELTN_Emitter_length(emitter));
    lequal(0, (int)ELTN_Emitter_current_depth(emitter));
    lequal(ELTN_OK, ELTN_Emitter_error_code(emitter));

    ELTN_Emitter_write_string(emitter, &result, &reslen);

    lok(result != NULL);
    if (result != NULL) {
        lsequal("", result);
        lequal(0, (int)reslen);
        free(result);
    }

    ELTN_Emitter_free(emitter);
}

void emitter_def_table() {
    ELTN_Emitter* emitter = ELTN_Emitter_new();
    char* result = NULL;
    size_t reslen = 0;
    bool ok = true;

    const char* expected = "test = 1;\nanother = 2;\nthird = 3;\n";
    const int explen = strlen(expected);

    ok = (ELTN_Emitter_def_name(emitter, "test", 4));
    lequal(ELTN_OK, ELTN_Emitter_error_code(emitter));
    lok(ok);
    ok = (ELTN_Emitter_value_number(emitter, 1.0));
    lequal(ELTN_OK, ELTN_Emitter_error_code(emitter));
    lok(ok);
    ok = (ELTN_Emitter_def_name(emitter, "another", 7));
    lequal(ELTN_OK, ELTN_Emitter_error_code(emitter));
    lok(ok);
    ok = (ELTN_Emitter_value_number(emitter, 2.0));
    lequal(ELTN_OK, ELTN_Emitter_error_code(emitter));
    lok(ok);
    ok = (ELTN_Emitter_def_name(emitter, "third", 5));
    lequal(ELTN_OK, ELTN_Emitter_error_code(emitter));
    lok(ok);
    ok = (ELTN_Emitter_value_number(emitter, 3.0));
    lequal(ELTN_OK, ELTN_Emitter_error_code(emitter));
    lok(ok);

    lequal(ELTN_OK, ELTN_Emitter_error_code(emitter));

    ELTN_Emitter_write_string(emitter, &result, &reslen);

    ok = (result != NULL);
    if (result != NULL) {
        lsequal(expected, result);
        lequal(explen, (int)reslen);
        free(result);
    }

    ELTN_Emitter_free(emitter);
}



void emitter_table() {
    const char* expected =
        "table = {\n    test = 1,\n    another = 2,\n    third = 3,\n}\n";
    const int explen = strlen(expected);

    ELTN_Emitter* emitter = ELTN_Emitter_new();
    char* result = NULL;
    size_t reslen = 0;
    bool ok = true;

    ok = (ELTN_Emitter_def_name(emitter, "table", 5));
    lequal(ELTN_OK, ELTN_Emitter_error_code(emitter));
    lok(ok);
    ok = (ELTN_Emitter_table_start(emitter));
    lequal(ELTN_OK, ELTN_Emitter_error_code(emitter));
    lok(ok);
    ok = (ELTN_Emitter_key_string(emitter, "test", 4));
    lequal(ELTN_OK, ELTN_Emitter_error_code(emitter));
    lok(ok);
    ok = (ELTN_Emitter_value_number(emitter, 1.0));
    lequal(ELTN_OK, ELTN_Emitter_error_code(emitter));
    lok(ok);
    ok = (ELTN_Emitter_key_string(emitter, "another", 7));
    lequal(ELTN_OK, ELTN_Emitter_error_code(emitter));
    lok(ok);
    ok = (ELTN_Emitter_value_number(emitter, 2.0));
    lequal(ELTN_OK, ELTN_Emitter_error_code(emitter));
    lok(ok);
    ok = (ELTN_Emitter_key_string(emitter, "third", 5));
    lequal(ELTN_OK, ELTN_Emitter_error_code(emitter));
    lok(ok);
    ok = (ELTN_Emitter_value_number(emitter, 3.0));
    lequal(ELTN_OK, ELTN_Emitter_error_code(emitter));
    lok(ok);
    ok = (ELTN_Emitter_table_end(emitter));
    lequal(ELTN_OK, ELTN_Emitter_error_code(emitter));
    lok(ok);

    lequal(ELTN_OK, ELTN_Emitter_error_code(emitter));

    ELTN_Emitter_write_string(emitter, &result, &reslen);

    ok = (result != NULL);
    if (result != NULL) {
        lsequal(expected, result);
        lequal(explen, (int)reslen);
        free(result);
    }

    ELTN_Emitter_free(emitter);
}

void emitter_current_path() {
    ELTN_Emitter* emitter = ELTN_Emitter_new();
    char* result = NULL;
    size_t reslen = 0;
    bool ok = true;

    const char* expected =
        "table = {\n"
        "    count = 3,\n"
        "    list = {\n"
        "        1,\n"
        "        2,\n"
        "        3,\n"
        "    }\n"
        "    total = 6,\n"
        "}\n";
    const int explen = strlen(expected);

    ok = (ELTN_Emitter_def_name(emitter, "table", 5));
    lequal(ELTN_OK, ELTN_Emitter_error_code(emitter));
    lok(ok);
    lequal(0, (int)ELTN_Emitter_current_depth(emitter));
    assert_current_path(emitter, "table");

    ok = (ELTN_Emitter_table_start(emitter));
    lequal(ELTN_OK, ELTN_Emitter_error_code(emitter));
    lok(ok);
    lequal(1, (int)ELTN_Emitter_current_depth(emitter));
    assert_current_path(emitter, "table");

    ok = (ELTN_Emitter_key_string(emitter, "count", 5));
    lequal(ELTN_OK, ELTN_Emitter_error_code(emitter));
    lok(ok);
    lequal(1, (int)ELTN_Emitter_current_depth(emitter));
    assert_current_path(emitter, "table.count");

    ok = (ELTN_Emitter_value_number(emitter, 3.0));
    lequal(ELTN_OK, ELTN_Emitter_error_code(emitter));
    lok(ok);
    lequal(1, (int)ELTN_Emitter_current_depth(emitter));
    assert_current_path(emitter, "table.count");

    ok = (ELTN_Emitter_key_string(emitter, "list", 4));
    lequal(ELTN_OK, ELTN_Emitter_error_code(emitter));
    lok(ok);
    lequal(1, (int)ELTN_Emitter_current_depth(emitter));
    assert_current_path(emitter, "table.list");

    ok = (ELTN_Emitter_table_start(emitter));
    lequal(ELTN_OK, ELTN_Emitter_error_code(emitter));
    lok(ok);
    lequal(2, (int)ELTN_Emitter_current_depth(emitter));
    assert_current_path(emitter, "table.list");

    ok = (ELTN_Emitter_value_number(emitter, 1.0));
    lequal(ELTN_OK, ELTN_Emitter_error_code(emitter));
    lok(ok);
    lequal(2, (int)ELTN_Emitter_current_depth(emitter));
    assert_current_path(emitter, "table.list[1]");

    ok = (ELTN_Emitter_value_number(emitter, 2.0));
    lequal(ELTN_OK, ELTN_Emitter_error_code(emitter));
    lok(ok);
    lequal(2, (int)ELTN_Emitter_current_depth(emitter));
    assert_current_path(emitter, "table.list[2]");

    ok = (ELTN_Emitter_value_number(emitter, 3.0));
    lequal(ELTN_OK, ELTN_Emitter_error_code(emitter));
    lok(ok);
    lequal(2, (int)ELTN_Emitter_current_depth(emitter));
    assert_current_path(emitter, "table.list[3]");

    ok = (ELTN_Emitter_table_end(emitter));
    lequal(ELTN_OK, ELTN_Emitter_error_code(emitter));
    lok(ok);
    lequal(1, (int)ELTN_Emitter_current_depth(emitter));
    assert_current_path(emitter, "table.list");

    ok = (ELTN_Emitter_key_string(emitter, "total", 5));
    lequal(ELTN_OK, ELTN_Emitter_error_code(emitter));
    lok(ok);
    lequal(1, (int)ELTN_Emitter_current_depth(emitter));
    assert_current_path(emitter, "table.total");

    ok = (ELTN_Emitter_value_number(emitter, 6.0));
    lequal(ELTN_OK, ELTN_Emitter_error_code(emitter));
    lok(ok);
    lequal(1, (int)ELTN_Emitter_current_depth(emitter));
    assert_current_path(emitter, "table.total");

    ok = (ELTN_Emitter_table_end(emitter));
    lequal(ELTN_OK, ELTN_Emitter_error_code(emitter));
    lok(ok);
    lequal(0, (int)ELTN_Emitter_current_depth(emitter));
    assert_current_path(emitter, "table");

    lequal(ELTN_OK, ELTN_Emitter_error_code(emitter));

    ELTN_Emitter_write_string(emitter, &result, &reslen);

    ok = (result != NULL);
    if (result != NULL) {
        lsequal(expected, result);
        lequal(explen, (int)reslen);
        free(result);
    }

    ELTN_Emitter_free(emitter);
}

void emitter_all_key_types() {
    ELTN_Emitter* emitter = ELTN_Emitter_new();
    char* result = NULL;
    size_t reslen = 0;
    bool ok = true;

    const char* expected =
        "{\n"
        "    string = 1,\n"
        "    [ \"quoted string\" ] = 2,\n"
        "    [11] = 3,\n"
        "    [5.5] = 4,\n"
        "}\n";
    const int explen = strlen(expected);

    ok = (ELTN_Emitter_table_start(emitter));
    lequal(ELTN_OK, ELTN_Emitter_error_code(emitter));
    lok(ok);

    ok = (ELTN_Emitter_key_string(emitter, "string", 6));
    lequal(ELTN_OK, ELTN_Emitter_error_code(emitter));
    lok(ok);

    ok = (ELTN_Emitter_value_number(emitter, 1.0));
    lequal(ELTN_OK, ELTN_Emitter_error_code(emitter));
    lok(ok);

    ok = (ELTN_Emitter_key_string(emitter, "quoted string", 13));
    lequal(ELTN_OK, ELTN_Emitter_error_code(emitter));
    lok(ok);

    ok = (ELTN_Emitter_value_number(emitter, 2.0));
    lequal(ELTN_OK, ELTN_Emitter_error_code(emitter));
    lok(ok);

    ok = (ELTN_Emitter_key_integer(emitter, 11));
    lequal(ELTN_OK, ELTN_Emitter_error_code(emitter));
    lok(ok);

    ok = (ELTN_Emitter_value_number(emitter, 3.0));
    lequal(ELTN_OK, ELTN_Emitter_error_code(emitter));
    lok(ok);

    ok = (ELTN_Emitter_key_number(emitter, 5.5));
    lequal(ELTN_OK, ELTN_Emitter_error_code(emitter));
    lok(ok);

    ok = (ELTN_Emitter_value_number(emitter, 4.0));
    lequal(ELTN_OK, ELTN_Emitter_error_code(emitter));
    lok(ok);

    ok = (ELTN_Emitter_table_end(emitter));
    lequal(ELTN_OK, ELTN_Emitter_error_code(emitter));
    lok(ok);

    ELTN_Emitter_write_string(emitter, &result, &reslen);

    ok = (result != NULL);
    if (result != NULL) {
        lsequal(expected, result);
        lequal(explen, (int)reslen);
        free(result);
    }

    ELTN_Emitter_free(emitter);
}

void emitter_all_value_types() {
    ELTN_Emitter* emitter = ELTN_Emitter_new();
    char* result = NULL;
    size_t reslen = 0;
    bool ok = true;

    const char* expected =
        "{\n"
        "    \"string\",\n"
        "    5.5,\n"
        "    3,\n"
        "    true,\n"
        "    false,\n"
        "    nil,\n"
        "}\n";
    const int explen = strlen(expected);

    ok = (ELTN_Emitter_table_start(emitter));
    lequal(ELTN_OK, ELTN_Emitter_error_code(emitter));
    lok(ok);
    assert_current_path(emitter, "");

    ok = (ELTN_Emitter_value_string(emitter, "string", 6));
    lequal(ELTN_OK, ELTN_Emitter_error_code(emitter));
    lok(ok);
    assert_current_path(emitter, "[1]");

    ok = (ELTN_Emitter_value_number(emitter, 5.5));
    lequal(ELTN_OK, ELTN_Emitter_error_code(emitter));
    lok(ok);
    assert_current_path(emitter, "[2]");

    ok = (ELTN_Emitter_value_integer(emitter, 3.0));
    lequal(ELTN_OK, ELTN_Emitter_error_code(emitter));
    lok(ok);
    assert_current_path(emitter, "[3]");

    ok = (ELTN_Emitter_value_boolean(emitter, true));
    lequal(ELTN_OK, ELTN_Emitter_error_code(emitter));
    lok(ok);
    assert_current_path(emitter, "[4]");

    ok = (ELTN_Emitter_value_boolean(emitter, false));
    lequal(ELTN_OK, ELTN_Emitter_error_code(emitter));
    lok(ok);
    assert_current_path(emitter, "[5]");

    ok = (ELTN_Emitter_value_nil(emitter));
    lequal(ELTN_OK, ELTN_Emitter_error_code(emitter));
    lok(ok);
    assert_current_path(emitter, "[6]");

    ok = (ELTN_Emitter_table_end(emitter));
    lequal(ELTN_OK, ELTN_Emitter_error_code(emitter));
    lok(ok);
    assert_current_path(emitter, "");

    ELTN_Emitter_write_string(emitter, &result, &reslen);

    ok = (result != NULL);
    if (result != NULL) {
        lsequal(expected, result);
        lequal(explen, (int)reslen);
        free(result);
    }

    ELTN_Emitter_free(emitter);
}

void emitter_comments() {
    /*
     * TODO: test short comments
     * TODO: test long comments
     * TODO: test different indent
     */
}

void emitter_no_pretty() {
    /*
     * TODO: test without pretty printing
     */
}

void emitter_error() {
    /*
     * TODO: test error path
     * TODO: on error, other functions disabled (like the parser)
     */
}

int main(void) {
    lrun("test_emitter_smoke", emitter_smoke);
    lrun("test_emitter_def_table", emitter_def_table);
    lrun("test_emitter_table", emitter_table);
    lrun("test_emitter_current_path", emitter_current_path);
    lrun("test_emitter_all_key_types", emitter_all_key_types);
    lrun("test_emitter_all_value_types", emitter_all_value_types);
    lrun("test_emitter_comments", emitter_comments);
    lrun("test_emitter_no_pretty", emitter_no_pretty);
    lrun("test_emitter_error", emitter_error);
    lresults();
    return lfails != 0;
}
