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
#include "etokutil.h"
#include "econv.h"

void string_quotes() {
    char8_t* str;
    size_t len;
    const char* data = "\"this is a quoted string\"";
    const char* expect = "this is a quoted string";

    ELTN_unescape_quoted_string(NULL, (const char8_t*)data, strlen(data), &str, &len);
    lequal((int)strlen(expect), (int)len);
    lsequal(expect, (const char*)str);
}

void string_simple_escapes() {
    char8_t* str;
    size_t len;
    const char* data = "'\\'\\a\\b\\f\\n\\r\\t\\v\\\\\\\"'";
    const char* expect = "'\a\b\f\n\r\t\v\\\"";

    ELTN_unescape_quoted_string(NULL, (const char8_t*)data, strlen(data), &str, &len);
    lequal((int)strlen(expect), (int)len);
    lsequal(expect, (const char*)str);
}

void string_escaped_newline() {
    char8_t* str;
    size_t len;
    const char* data = "'this text has \\\nan escaped newline'";
    const char* expect = "this text has \nan escaped newline";

    ELTN_unescape_quoted_string(NULL, (const char8_t *)data, strlen(data), &str, &len);
    lequal((int)strlen(expect), (int)len);
    lsequal(expect, (const char*)str);
}

void string_backslash_z() {
    char8_t* str;
    size_t len;
    const char* data = "'this text has \\z \n\t     no newline'";
    const char* expect = "this text has no newline";

    ELTN_unescape_quoted_string(NULL, (const char8_t *)data, strlen(data), &str, &len);
    lequal((int)strlen(expect), (int)len);
    lsequal(expect, (const char*)str);
}

void string_hex_escapes() {
    char8_t* str;
    size_t len;
    const char* data = "'this text had \\x68\\x65\\x78 escapes\\x2e\\x2e\\x2E'";
    const char* expect = "this text had hex escapes...";

    ELTN_unescape_quoted_string(NULL, (const char8_t *)data, strlen(data), &str, &len);
    lequal((int)strlen(expect), (int)len);
    lsequal(expect, (const char*)str);
}

void string_octal_escapes() {
    char8_t* str;
    size_t len;
    const char* data = "'this text had octal escapes\\56\\056\\056\\0'";
    const char* expect = "this text had octal escapes...\0";

    ELTN_unescape_quoted_string(NULL, (const char8_t *)data, strlen(data), &str, &len);
    lequal((int)strlen(expect) + 1, (int)len);
    lsequal(expect, (const char*)str);
}

void string_unicode_escapes() {
    char8_t* str;
    size_t len;
    const char* data =
        "'this text has unicode:\\u{a}\\u{A9}\\u{1e9e}\\u{1047F}'";
    const char* expect = "this text has unicode:\n\u00A9\u1e9e\U0001047F";

    ELTN_unescape_quoted_string(NULL, (const char8_t *)data, strlen(data), &str, &len);
    lequal((int)strlen(expect), (int)len);
    lsequal(expect, (const char*)str);
}

void long_string_quotes() {
    char8_t* str;
    size_t len;
    const char* data =
        "[==[\nthis is a\nshort long string\n]==]";
    const char* expect = "this is a\nshort long string\n";

    ELTN_unquote_long_string(NULL, (const char8_t *)data, strlen(data),
                             &str, &len);
    lequal((int)strlen(expect), (int)len);
    lsequal(expect, (const char*)str);
}

void long_string_short() {
    char8_t* str;
    size_t len;
    const char* data = "[[o]]";
    const char* expect = "o";

    ELTN_unquote_long_string(NULL, (const char8_t *)data, strlen(data),
                             &str, &len);
    lequal((int)strlen(expect), (int)len);
    lsequal(expect, (const char*)str);
}

void comment_content() {
    char8_t* str;
    size_t len;
    const char* data =
        "-- this is a simple comment\n";
    const char* expect = " this is a simple comment";

    ELTN_trim_comment(NULL, (const char8_t *)data, strlen(data), &str, &len);
    lequal((int)strlen(expect), (int)len);
    lsequal(expect, (const char*)str);
}

void long_comment_content() {
    char8_t* str;
    size_t len;
    const char* data =
        "--[[\nthis is a long\ncomment\n]]";
    const char* expect = "this is a long\ncomment\n";

    ELTN_trim_comment(NULL, (const char8_t *)data, strlen(data), &str, &len);
    lequal((int)strlen(expect), (int)len);
    lsequal(expect, (const char*)str);
}

void long_comment_not() {
    char8_t* str;
    size_t len;
    const char* data =
        "--[oh] this is not a long comment\n";
    const char* expect = "[oh] this is not a long comment";

    ELTN_trim_comment(NULL, (const char8_t *)data, strlen(data), &str, &len);
    lequal((int)strlen(expect), (int)len);
    lsequal(expect, (const char*)str);
}

void quoted_string() {
    const char* input = "This is a\trandom\nstring with \" quotes";
    const char* expect = "\"This is a\\trandom\\nstring with \\\" quotes\"";
    int expectlen = (int)strlen(expect);
    char* actual = NULL;
    size_t actual_len = 0;

    ELTN_quote_string(NULL, (const char8_t *)input, strlen(input),
                      (char8_t **)&actual, &actual_len);
    lequal(expectlen, (int)actual_len);
    lok(actual != NULL);
    if (actual != NULL) {
        lsequal(expect, actual);
    }
    free(actual);
}

void quoted_string_with_utf8() {
    const char* input = "This string has unicode: \u20AC See?";
    const char* expect = "\"This string has unicode: \u20AC See?\"";
    int expectlen = (int)strlen(expect);
    char* actual = NULL;
    size_t actual_len = 0;

    ELTN_quote_string(NULL, (const char8_t *)input, strlen(input),
                      (char8_t **)&actual, &actual_len);
    lequal(expectlen, (int)actual_len);
    lok(actual != NULL);
    if (actual != NULL) {
        lsequal(expect, actual);
    }
    free(actual);
}

void quoted_string_with_latin1() {
    const char* input = "This string has Latin-1: \xC7 See?";
    const char* expect = "\"This string has Latin-1: \xC7 See?\"";
    int expectlen = (int)strlen(expect);
    char* actual = NULL;
    size_t actual_len = 0;

    ELTN_quote_string(NULL, (const char8_t*)input, strlen(input),
                      (char8_t**)&actual, &actual_len);
    lequal(expectlen, (int)actual_len);
    lok(actual != NULL);
    if (actual != NULL) {
        lsequal(expect, (char *)actual);
    }
    free(actual);
}

void quoted_long_string() {
    const char* input = 
        "This is a long string both syntactically\n"
        "and in fact, since it contains [[confusing]] long quotes"
        "[=[inside it]=]";
    const char* expect = 
        "[==[" 
        "This is a long string both syntactically\n"
        "and in fact, since it contains [[confusing]] long quotes"
        "[=[inside it]=]"
        "]==]";
    int expectlen = (int)strlen(expect);
    char* actual = NULL;
    size_t actual_len = 0;

    ELTN_quote_string(NULL, (const char8_t*)input, strlen(input),
                      (char8_t**)&actual, &actual_len);
    lequal(expectlen, (int)actual_len);
    lok(actual != NULL);
    if (actual != NULL) {
        lsequal(expect, (char *)actual);
    }
    free(actual);
}

void formatted_comment() {
    const char* input = 
        "This is a short comment with no newlines";
    const char* expect = 
        "-- This is a short comment with no newlines\n";
    int expectlen = (int)strlen(expect);
    char* actual = NULL;
    size_t actual_len = 0;

    ELTN_format_comment(NULL, (const char8_t*)input, strlen(input),
                        (char8_t**)&actual, &actual_len);
    lequal(expectlen, (int)actual_len);
    lok(actual != NULL);
    if (actual != NULL) {
        lsequal(expect, (char *)actual);
    }
    free(actual);
}

void formatted_long_comment() {
    const char* input = 
        "This is a long string both syntactically\n"
        "and in fact, since it contains [[confusing]] long quotes"
        "[=[inside it]=]";
    const char* expect = 
        "--[==[" 
        "This is a long string both syntactically\n"
        "and in fact, since it contains [[confusing]] long quotes"
        "[=[inside it]=]"
        "]==]";
    int expectlen = (int)strlen(expect);
    char* actual = NULL;
    size_t actual_len = 0;

    ELTN_format_comment(NULL, (const char8_t*)input, strlen(input),
                        (char8_t**)&actual, &actual_len);
    lequal(expectlen, (int)actual_len);
    lok(actual != NULL);
    if (actual != NULL) {
        lsequal(expect, (char *)actual);
    }
    free(actual);
}

/*
 * TODO: test quoting long strings
 * TODO: test formatting comments (long and short)
 * TODO: test bad escape sequences and missing quotes.
 */

int main() {
    lrun("test_string_quotes", string_quotes);
    lrun("test_string_simple_escapes", string_simple_escapes);
    lrun("test_string_escaped_newline", string_escaped_newline);
    lrun("test_string_backslash_z", string_backslash_z);
    lrun("test_string_hex_escapes", string_hex_escapes);
    lrun("test_string_octal_escapes", string_octal_escapes);
    lrun("test_string_unicode_escapes", string_unicode_escapes);
    lrun("test_long_string_quotes", long_string_quotes);
    lrun("test_long_string_short", long_string_short);
    lrun("test_comment_content", comment_content);
    lrun("test_long_comment_content", long_comment_content);
    lrun("test_long_comment_not", long_comment_content);
    lrun("test_quoted_string", quoted_string);
    lrun("test_quoted_string_with_utf8", quoted_string_with_utf8);
    lrun("test_quoted_string_with_latin1", quoted_string_with_latin1);
    lrun("test_quoted_long_string", quoted_long_string);
    lrun("test_formatted_comment", formatted_comment);
    lrun("test_formatted_long_comment", formatted_long_comment);

    lresults();
    return lfails != 0;
}
