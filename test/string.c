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

#include <string.h>
#include "minctest.h"
#include "estring.h"

void string_quotes() {
    char* str;
    size_t len;
    const char* data = "\"this is a quoted string\"";
    const char* expect = "this is a quoted string";

    ELTN_unescape_quoted_string(NULL, data, strlen(data), &str, &len);
    lequal((int)strlen(expect), (int)len);
    lsequal(expect, str);
}

void string_simple_escapes() {
    char* str;
    size_t len;
    const char* data = "'\\'\\a\\b\\f\\n\\r\\t\\v\\\\\\\"'";
    const char* expect = "'\a\b\f\n\r\t\v\\\"";

    ELTN_unescape_quoted_string(NULL, data, strlen(data), &str, &len);
    lequal((int)strlen(expect), (int)len);
    lsequal(expect, str);
}

void string_escaped_newline() {
    char* str;
    size_t len;
    const char* data = "'this text has \\\nan escaped newline'";
    const char* expect = "this text has \nan escaped newline";

    ELTN_unescape_quoted_string(NULL, data, strlen(data), &str, &len);
    lequal((int)strlen(expect), (int)len);
    lsequal(expect, str);
}

void string_backslash_z() {
    char* str;
    size_t len;
    const char* data = "'this text has \\z \\\n\t     no newline'";
    const char* expect = "this text has no newline";

    ELTN_unescape_quoted_string(NULL, data, strlen(data), &str, &len);
    lequal((int)strlen(expect), (int)len);
    lsequal(expect, str);
}

void string_hex_escapes() {
    char* str;
    size_t len;
    const char* data = "'this text had \\x68\\x65\\x78 escapes\\x2e\\x2e\\x2E'";
    const char* expect = "this text had hex escapes...";

    ELTN_unescape_quoted_string(NULL, data, strlen(data), &str, &len);
    lequal((int)strlen(expect), (int)len);
    lsequal(expect, str);
}

void string_octal_escapes() {
    char* str;
    size_t len;
    const char* data = "'this text had octal escapes\\56\\056\\056\\0'";
    const char* expect = "this text had octal escapes...\0";

    ELTN_unescape_quoted_string(NULL, data, strlen(data), &str, &len);
    lequal((int)strlen(expect) + 1, (int)len);
    lsequal(expect, str);
}

void string_unicode_escapes() {
    char* str;
    size_t len;
    const char* data =
        "'this text has unicode:\\u{a}\\u{A9}\\u{1e9e}\\u{1047F}`'";
    const char* expect = "this text has unicode:\n\u00A9\u1e9e\U0001047F";

    ELTN_unescape_quoted_string(NULL, data, strlen(data), &str, &len);
    lequal((int)strlen(expect), (int)len);
    lsequal(expect, str);
}

/*
 * TODO: test bad escape sequences and missing quotes.
 */

int main(int argc, char* argv[]) {
    lrun("test_string_quotes", string_quotes);
    lrun("test_string_simple_escapes", string_simple_escapes);
    lrun("test_string_escaped_newline", string_escaped_newline);
    lrun("test_string_backslash_z", string_backslash_z);
    lrun("test_string_hex_escapes", string_hex_escapes);
    lrun("test_string_octal_escapes", string_octal_escapes);
    lrun("test_string_unicode_escapes", string_unicode_escapes);
    lresults();
    return lfails != 0;
}
