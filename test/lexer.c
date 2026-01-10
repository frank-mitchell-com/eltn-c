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
#include "elexer.h"
#include "ealloc.h"

#define BUFFER_SIZE     128
#define BUFFER_INCR     3

static bool __debug = false;

typedef struct Mock_Source {
    const char8_t* buf;
    const char8_t* ptr;
    size_t len;
} Mock_Source;

static void set_debug(bool debug) {
    __debug = debug;
}

static int32_t Mock_Source_next_char(void* state, bool consume) {
    Mock_Source* self = (Mock_Source *) state;
    char8_t result;

    if (self->ptr >= self->buf + self->len) {
        return -1;
    }
    result = *(self->ptr);
    if (consume) {
        self->ptr++;
    }
    return result;
}

static ELTN_Lexer* set_up(Mock_Source* self, const char* data) {
    self->buf = (const char8_t *)data;
    self->ptr = (const char8_t *)data;
    self->len = strlen(data);

    ELTN_Lexer* lexer = ELTN_Lexer_new_with_pool(NULL);

    ELTN_Lexer_set_char_source(lexer, Mock_Source_next_char, self);
    return lexer;
}

static void assert_token(ELTN_Lexer* lexer,
                         ELTN_Token exptoken,
                         const char* exptokstr, int expline, int expcol) {
    ELTN_Token token;
    char* tokstr;
    size_t toklen;
    int line;
    int col;

    token = ELTN_Lexer_next_token(lexer, &line, &col);
    ELTN_Lexer_token_string(lexer, &tokstr, &toklen);
    if (__debug) {
        printf("DEBUG: assert_token(%d, [=[%s]=], %d, %d) => [=[%s]=] (%x)\n",
               (int)exptoken, exptokstr, expline, expcol, tokstr, tokstr[0]);
    }
    lequal(exptoken, token);
    lsequal(exptokstr, tokstr);
    lequal(expline, line);
    lequal(expcol, col);
    ELTN_free_string(tokstr);
}

void lexer_semicolon() {
    ELTN_Lexer* lexer;
    Mock_Source source;

    lexer = set_up(&source, "  ;  ");

    lok(lexer != NULL);

    assert_token(lexer, ELTN_TOKEN_SEMICOLON, ";", 1, 3);
    assert_token(lexer, ELTN_TOKEN_EOF, "", 1, 6);

    ELTN_Lexer_free(lexer);
}

void lexer_equals() {
    ELTN_Lexer* lexer;
    Mock_Source source;

    lexer = set_up(&source, "  =  ");

    lok(lexer != NULL);

    assert_token(lexer, ELTN_TOKEN_EQUALS, "=", 1, 3);
    assert_token(lexer, ELTN_TOKEN_EOF, "", 1, 6);

    ELTN_Lexer_free(lexer);
}

void lexer_comma() {
    ELTN_Lexer* lexer;
    Mock_Source source;

    lexer = set_up(&source, "  ,  ");

    lok(lexer != NULL);

    assert_token(lexer, ELTN_TOKEN_COMMA, ",", 1, 3);
    assert_token(lexer, ELTN_TOKEN_EOF, "", 1, 6);

    ELTN_Lexer_free(lexer);
}

void lexer_curly_open() {
    ELTN_Lexer* lexer;
    Mock_Source source;

    lexer = set_up(&source, "  {  ");

    lok(lexer != NULL);

    assert_token(lexer, ELTN_TOKEN_CURLY_OPEN, "{", 1, 3);
    assert_token(lexer, ELTN_TOKEN_EOF, "", 1, 6);

    ELTN_Lexer_free(lexer);
}

void lexer_curly_close() {
    ELTN_Lexer* lexer;
    Mock_Source source;

    lexer = set_up(&source, "  }  ");

    lok(lexer != NULL);

    assert_token(lexer, ELTN_TOKEN_CURLY_CLOSE, "}", 1, 3);
    assert_token(lexer, ELTN_TOKEN_EOF, "", 1, 6);

    ELTN_Lexer_free(lexer);
}

void lexer_square_open() {
    ELTN_Lexer* lexer;
    Mock_Source source;

    lexer = set_up(&source, "  [  ");

    lok(lexer != NULL);

    assert_token(lexer, ELTN_TOKEN_SQUARE_OPEN, "[", 1, 3);
    assert_token(lexer, ELTN_TOKEN_EOF, "", 1, 6);

    ELTN_Lexer_free(lexer);
}

void lexer_square_close() {
    ELTN_Lexer* lexer;
    Mock_Source source;

    lexer = set_up(&source, "  ]  ");

    lok(lexer != NULL);

    assert_token(lexer, ELTN_TOKEN_SQUARE_CLOSE, "]", 1, 3);
    assert_token(lexer, ELTN_TOKEN_EOF, "", 1, 6);

    ELTN_Lexer_free(lexer);
}

void lexer_nil() {
    ELTN_Lexer* lexer;
    Mock_Source source;

    lexer = set_up(&source, "  nil  ");

    lok(lexer != NULL);

    assert_token(lexer, ELTN_TOKEN_NIL, "nil", 1, 3);
    assert_token(lexer, ELTN_TOKEN_EOF, "", 1, 8);

    ELTN_Lexer_free(lexer);
}

void lexer_boolean_true() {
    ELTN_Lexer* lexer;
    Mock_Source source;

    lexer = set_up(&source, "  true  ");

    lok(lexer != NULL);

    assert_token(lexer, ELTN_TOKEN_BOOLEAN_TRUE, "true", 1, 3);
    assert_token(lexer, ELTN_TOKEN_EOF, "", 1, 9);

    ELTN_Lexer_free(lexer);
}

void lexer_boolean_false() {
    ELTN_Lexer* lexer;
    Mock_Source source;

    lexer = set_up(&source, "  false  ");

    lok(lexer != NULL);

    assert_token(lexer, ELTN_TOKEN_BOOLEAN_FALSE, "false", 1, 3);
    assert_token(lexer, ELTN_TOKEN_EOF, "", 1, 10);

    ELTN_Lexer_free(lexer);
}

void lexer_boolean_false_positive() {
    ELTN_Lexer* lexer;
    Mock_Source source;

    lexer = set_up(&source, "  falsetto  ");

    lok(lexer != NULL);

    assert_token(lexer, ELTN_TOKEN_NAME, "falsetto", 1, 3);
    assert_token(lexer, ELTN_TOKEN_EOF, "", 1, 13);

    ELTN_Lexer_free(lexer);
}

void lexer_names() {
    ELTN_Lexer* lexer;
    Mock_Source source;
    const char* names = "simple camelCase snake_case\nnum123 ___";

    lexer = set_up(&source, names);

    lok(lexer != NULL);

    assert_token(lexer, ELTN_TOKEN_NAME, "simple", 1, 1);
    assert_token(lexer, ELTN_TOKEN_NAME, "camelCase", 1, 8);
    assert_token(lexer, ELTN_TOKEN_NAME, "snake_case", 1, 18);
    assert_token(lexer, ELTN_TOKEN_NAME, "num123", 2, 1);
    assert_token(lexer, ELTN_TOKEN_NAME, "___", 2, 8);
    assert_token(lexer, ELTN_TOKEN_EOF, "", 2, 11);

    ELTN_Lexer_free(lexer);
}

void lexer_strings() {
    ELTN_Lexer* lexer;
    Mock_Source source;
    const char* data = "'foo' \"bar\" \"string with \\\"escapes\\\"\"";

    lexer = set_up(&source, data);

    lok(lexer != NULL);

    assert_token(lexer, ELTN_TOKEN_STRING, "'foo'", 1, 1);
    assert_token(lexer, ELTN_TOKEN_STRING, "\"bar\"", 1, 7);
    assert_token(lexer, ELTN_TOKEN_STRING, "\"string with \\\"escapes\\\"\"", 1,
                 13);
    assert_token(lexer, ELTN_TOKEN_EOF, "", 1, strlen(data) + 1);

    ELTN_Lexer_free(lexer);
}

void lexer_strings_2() {
    ELTN_Lexer* lexer;
    Mock_Source source;
    const char* data = "\"string with \\z\n       whitespace \\\n\\\"\"";

    lexer = set_up(&source, data);

    lok(lexer != NULL);

    assert_token(lexer, ELTN_TOKEN_STRING, data, 1, 1);
    assert_token(lexer, ELTN_TOKEN_EOF, "", 3, 4);

    ELTN_Lexer_free(lexer);
}

void lexer_incomplete_string() {
    ELTN_Lexer* lexer;
    Mock_Source source;
    const char* data = "\"string with";

    lexer = set_up(&source, data);

    lok(lexer != NULL);

    assert_token(lexer, ELTN_TOKEN_INVALID, data, 1, 1);
    assert_token(lexer, ELTN_TOKEN_EOF, "", 1, 13);

    ELTN_Lexer_free(lexer);
}

void lexer_invalid_characters() {
    ELTN_Lexer* lexer;
    Mock_Source source;
    const char* data = "~`!@#$%^*()|\\:<>?/\x80\u00DF";

    lexer = set_up(&source, data);

    lok(lexer != NULL);

    assert_token(lexer, ELTN_TOKEN_INVALID, "~", 1, 1);
    assert_token(lexer, ELTN_TOKEN_INVALID, "`", 1, 2);
    assert_token(lexer, ELTN_TOKEN_INVALID, "!", 1, 3);
    assert_token(lexer, ELTN_TOKEN_INVALID, "@", 1, 4);
    assert_token(lexer, ELTN_TOKEN_INVALID, "#", 1, 5);
    assert_token(lexer, ELTN_TOKEN_INVALID, "$", 1, 6);
    assert_token(lexer, ELTN_TOKEN_INVALID, "%", 1, 7);
    assert_token(lexer, ELTN_TOKEN_INVALID, "^", 1, 8);
    assert_token(lexer, ELTN_TOKEN_INVALID, "*", 1, 9);
    assert_token(lexer, ELTN_TOKEN_INVALID, "(", 1, 10);
    assert_token(lexer, ELTN_TOKEN_INVALID, ")", 1, 11);
    assert_token(lexer, ELTN_TOKEN_INVALID, "|", 1, 12);
    assert_token(lexer, ELTN_TOKEN_INVALID, "\\", 1, 13);
    assert_token(lexer, ELTN_TOKEN_INVALID, ":", 1, 14);
    assert_token(lexer, ELTN_TOKEN_INVALID, "<", 1, 15);
    assert_token(lexer, ELTN_TOKEN_INVALID, ">", 1, 16);
    assert_token(lexer, ELTN_TOKEN_INVALID, "?", 1, 17);
    assert_token(lexer, ELTN_TOKEN_INVALID, "/", 1, 18);
    assert_token(lexer, ELTN_TOKEN_INVALID, "\x80", 1, 19);
    assert_token(lexer, ELTN_TOKEN_INVALID, "\xC3", 1, 20);
    assert_token(lexer, ELTN_TOKEN_INVALID, "\x9F", 1, 21);
    assert_token(lexer, ELTN_TOKEN_EOF, "", 1, 22);

    ELTN_Lexer_free(lexer);
}

void lexer_invalid_keywords() {
    ELTN_Lexer* lexer;
    Mock_Source source;
    const char* data =
        "and break do else elseif end for function global goto if in "
        "local not or repeat return then until while";

    lexer = set_up(&source, data);

    lok(lexer != NULL);

    assert_token(lexer, ELTN_TOKEN_INVALID, "and", 1, 1);
    assert_token(lexer, ELTN_TOKEN_INVALID, "break", 1, 5);
    assert_token(lexer, ELTN_TOKEN_INVALID, "do", 1, 11);
    assert_token(lexer, ELTN_TOKEN_INVALID, "else", 1, 14);
    assert_token(lexer, ELTN_TOKEN_INVALID, "elseif", 1, 19);
    assert_token(lexer, ELTN_TOKEN_INVALID, "end", 1, 26);
    assert_token(lexer, ELTN_TOKEN_INVALID, "for", 1, 30);
    assert_token(lexer, ELTN_TOKEN_INVALID, "function", 1, 34);
    assert_token(lexer, ELTN_TOKEN_NAME, "global", 1, 43);
    assert_token(lexer, ELTN_TOKEN_INVALID, "goto", 1, 50);
    assert_token(lexer, ELTN_TOKEN_INVALID, "if", 1, 55);
    assert_token(lexer, ELTN_TOKEN_INVALID, "in", 1, 58);
    assert_token(lexer, ELTN_TOKEN_INVALID, "local", 1, 61);
    assert_token(lexer, ELTN_TOKEN_INVALID, "not", 1, 67);
    assert_token(lexer, ELTN_TOKEN_INVALID, "or", 1, 71);
    assert_token(lexer, ELTN_TOKEN_INVALID, "repeat", 1, 74);
    assert_token(lexer, ELTN_TOKEN_INVALID, "return", 1, 81);
    assert_token(lexer, ELTN_TOKEN_INVALID, "then", 1, 88);
    assert_token(lexer, ELTN_TOKEN_INVALID, "until", 1, 93);
    assert_token(lexer, ELTN_TOKEN_INVALID, "while", 1, 99);
    assert_token(lexer, ELTN_TOKEN_EOF, "", 1, strlen(data) + 1);

    ELTN_Lexer_free(lexer);
}

void lexer_comment() {
    ELTN_Lexer* lexer;
    Mock_Source source;
    const char* data =
        "  -- this is a short comment\n"
        "  -- this is also a comment\r\n" "\"this isn't\"";

    lexer = set_up(&source, data);

    lok(lexer != NULL);

    assert_token(lexer, ELTN_TOKEN_COMMENT, "-- this is a short comment\n", 1,
                 3);
    assert_token(lexer, ELTN_TOKEN_COMMENT, "-- this is also a comment\n", 2,
                 3);
    assert_token(lexer, ELTN_TOKEN_STRING, "\"this isn't\"", 3, 1);
    assert_token(lexer, ELTN_TOKEN_EOF, "", 3, 13);

    ELTN_Lexer_free(lexer);
}

void lexer_long_comment() {
    ELTN_Lexer* lexer;
    Mock_Source source;
    const char* data =
        "--[==[ this is a long comment\n"
        "it ends when I feel like it.\n"
        "[[Not yet.]]\n" "OK, how about now ]==]";

    lexer = set_up(&source, data);

    lok(lexer != NULL);

    assert_token(lexer, ELTN_TOKEN_LONG_COMMENT, data, 1, 1);
    assert_token(lexer, ELTN_TOKEN_EOF, "", 4, 23);

    ELTN_Lexer_free(lexer);
}

void lexer_long_comment_2() {
    ELTN_Lexer* lexer;
    Mock_Source source;
    const char* data =
        "--[[test]]\n" "--[=[test]=]\n" "--[==[\n" "test]==]\n" "\"the end\"\n";

    lexer = set_up(&source, data);

    lok(lexer != NULL);

    assert_token(lexer, ELTN_TOKEN_LONG_COMMENT, "--[[test]]", 1, 1);
    assert_token(lexer, ELTN_TOKEN_LONG_COMMENT, "--[=[test]=]", 2, 1);
    assert_token(lexer, ELTN_TOKEN_LONG_COMMENT, "--[==[\ntest]==]", 3, 1);
    assert_token(lexer, ELTN_TOKEN_STRING, "\"the end\"", 5, 1);
    assert_token(lexer, ELTN_TOKEN_EOF, "", 6, 1);

    ELTN_Lexer_free(lexer);
}

void lexer_long_comment_not() {
    ELTN_Lexer* lexer;
    Mock_Source source;
    const char* data =
        "--[i][j] this is not a long comment\n"
        "--[-[ it's just a series of comments\n"
        "--[===x[meant to fool the lexer\n" "--[OK, I'll stop now]\n\n";

    lexer = set_up(&source, data);

    lok(lexer != NULL);

    assert_token(lexer, ELTN_TOKEN_COMMENT,
                 "--[i][j] this is not a long comment\n", 1, 1);
    assert_token(lexer, ELTN_TOKEN_COMMENT,
                 "--[-[ it's just a series of comments\n", 2, 1);
    assert_token(lexer, ELTN_TOKEN_COMMENT, "--[===x[meant to fool the lexer\n",
                 3, 1);
    assert_token(lexer, ELTN_TOKEN_COMMENT, "--[OK, I'll stop now]\n", 4, 1);
    assert_token(lexer, ELTN_TOKEN_EOF, "", 6, 1);

    ELTN_Lexer_free(lexer);
}

void lexer_long_string() {
    ELTN_Lexer* lexer;
    Mock_Source source;
    const char* data =
        "[==[ this is a long string\n"
        "it ends when I feel like it.\n"
        "[[Not yet.]]\n" "OK, how about now ]==]";

    lexer = set_up(&source, data);

    lok(lexer != NULL);

    assert_token(lexer, ELTN_TOKEN_LONG_STRING, data, 1, 1);
    assert_token(lexer, ELTN_TOKEN_EOF, "", 4, 23);

    ELTN_Lexer_free(lexer);
}


void lexer_long_string_not() {
    ELTN_Lexer* lexer;
    Mock_Source source;
    const char* data = "[[test]]\n" "[==*[\n" "[=======\n" "[===[\n" "[[]\n\n";

    lexer = set_up(&source, data);

    lok(lexer != NULL);

    assert_token(lexer, ELTN_TOKEN_LONG_STRING, "[[test]]", 1, 1);
    assert_token(lexer, ELTN_TOKEN_INVALID, "[==*", 2, 1);
    assert_token(lexer, ELTN_TOKEN_SQUARE_OPEN, "[", 2, 5);
    assert_token(lexer, ELTN_TOKEN_INVALID, "[=======\n", 3, 1);
    assert_token(lexer, ELTN_TOKEN_INVALID, "[===[\n[[]\n\n", 4, 1);
    assert_token(lexer, ELTN_TOKEN_EOF, "", 7, 1);

    ELTN_Lexer_free(lexer);
}

void lexer_numbers_good() {
    ELTN_Lexer* lexer;
    Mock_Source source;
    const char* data =
        "0 -0 1 -3 3e8 0x3e8 007 0x3e8p+8 1000 -.5 3.14159 "
        "0x0.1E  0xA23p-4   0X1.921FB54442D18P+1";

    lexer = set_up(&source, data);

    lok(lexer != NULL);

    assert_token(lexer, ELTN_TOKEN_NUMBER, "0", 1, 1);
    assert_token(lexer, ELTN_TOKEN_NUMBER, "-0", 1, 3);
    assert_token(lexer, ELTN_TOKEN_NUMBER, "1", 1, 6);
    assert_token(lexer, ELTN_TOKEN_NUMBER, "-3", 1, 8);
    assert_token(lexer, ELTN_TOKEN_NUMBER, "3e8", 1, 11);
    assert_token(lexer, ELTN_TOKEN_NUMBER, "0x3e8", 1, 15);
    assert_token(lexer, ELTN_TOKEN_NUMBER, "007", 1, 21);
    assert_token(lexer, ELTN_TOKEN_NUMBER, "0x3e8p+8", 1, 25);
    assert_token(lexer, ELTN_TOKEN_NUMBER, "1000", 1, 34);
    assert_token(lexer, ELTN_TOKEN_NUMBER, "-.5", 1, 39);
    assert_token(lexer, ELTN_TOKEN_NUMBER, "3.14159", 1, 43);
    assert_token(lexer, ELTN_TOKEN_NUMBER, "0x0.1E", 1, 51);
    assert_token(lexer, ELTN_TOKEN_NUMBER, "0xA23p-4", 1, 59);
    assert_token(lexer, ELTN_TOKEN_NUMBER, "0X1.921FB54442D18P+1", 1, 70);
    assert_token(lexer, ELTN_TOKEN_EOF, "", 1, 90);

    ELTN_Lexer_free(lexer);
}

void lexer_numbers_bad() {
    ELTN_Lexer* lexer;
    Mock_Source source;
    const char* data = "- -. 23skidoo 3df 100,000 +3 twelve";

    lexer = set_up(&source, data);

    lok(lexer != NULL);

    assert_token(lexer, ELTN_TOKEN_INVALID, "-", 1, 1);
    assert_token(lexer, ELTN_TOKEN_INVALID, "-.", 1, 3);
    assert_token(lexer, ELTN_TOKEN_NUMBER, "23", 1, 6);
    assert_token(lexer, ELTN_TOKEN_NAME, "skidoo", 1, 8);
    assert_token(lexer, ELTN_TOKEN_INVALID, "3df", 1, 15);
    assert_token(lexer, ELTN_TOKEN_NUMBER, "100", 1, 19);
    assert_token(lexer, ELTN_TOKEN_COMMA, ",", 1, 22);
    assert_token(lexer, ELTN_TOKEN_NUMBER, "000", 1, 23);
    assert_token(lexer, ELTN_TOKEN_INVALID, "+", 1, 27);
    assert_token(lexer, ELTN_TOKEN_NUMBER, "3", 1, 28);
    assert_token(lexer, ELTN_TOKEN_NAME, "twelve", 1, 30);
    assert_token(lexer, ELTN_TOKEN_EOF, "", 1, 36);

    ELTN_Lexer_free(lexer);
}


/*
 * Main function: runs all the tests
 */
int main(int argc, char* argv[]) {
    lrun("test_lexer_semicolon", lexer_semicolon);
    lrun("test_lexer_equals", lexer_equals);
    lrun("test_lexer_comma", lexer_comma);
    lrun("test_lexer_curly_open", lexer_curly_open);
    lrun("test_lexer_curly_close", lexer_curly_close);
    lrun("test_lexer_square_open", lexer_square_open);
    lrun("test_lexer_square_close", lexer_square_close);
    lrun("test_lexer_names", lexer_names);
    lrun("test_lexer_nil", lexer_nil);
    lrun("test_lexer_boolean_true", lexer_boolean_true);
    lrun("test_lexer_boolean_false", lexer_boolean_false);
    lrun("test_lexer_boolean_false_positive", lexer_boolean_false_positive);
    lrun("test_lexer_strings", lexer_strings);
    lrun("test_lexer_strings_2", lexer_strings_2);
    lrun("test_lexer_incomplete_string", lexer_incomplete_string);
    lrun("test_lexer_invalid_characters", lexer_invalid_characters);
    lrun("test_lexer_invalid_keywords", lexer_invalid_keywords);
    lrun("test_lexer_comment", lexer_comment);
    lrun("test_lexer_long_comment", lexer_long_comment);
    lrun("test_lexer_long_comment_2", lexer_long_comment_2);
    lrun("test_lexer_long_comment_not", lexer_long_comment_not);
    lrun("test_lexer_long_string", lexer_long_string);
    lrun("test_lexer_long_string_not", lexer_long_string_not);
    lrun("test_lexer_numbers_good", lexer_numbers_good);
    lrun("test_lexer_numbers_bad", lexer_numbers_bad);
    lresults();
    return lfails != 0;
}
