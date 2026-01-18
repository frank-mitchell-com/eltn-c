/*****************************************************************************
 *
 * Copyright 2025 Frank Mitchell
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 ****************************************************************************/

#include <math.h>
#include <stdlib.h>
#include <string.h>

#define ELTN_CORE   1
#include "eltn.h"
#include "elexer.h"
#include "esource.h"
#include "ealloc.h"
#include "estring.h"

#define INIT_BUF_SIZE 1024

#define KEYWORDS_SIZE 22

const char* KEYWORDS[] = {
    "and", "break", "do", "else", "elseif", "end",
    "false", "for", "function", "goto", "if", "in",
    "local", "nil", "not", "or", "repeat", "return",
    "then", "true", "until", "while"
};

struct ELTN_Lexer {
    intptr_t _reserved;
    ELTN_Pool* pool;
    ELTN_Char_Source get_next_char;
    void* source;

    char8_t current_char;
    int count;
    int line;
    int column;
    char8_t* token_buffer;
    char8_t* token_buffer_tail;
    size_t token_buffer_size;
    bool pushback;
    bool eos;
};

ELTN_Lexer* ELTN_Lexer_new_with_pool(ELTN_Pool* pool) {
    ELTN_Lexer* self = (ELTN_Lexer *) ELTN_alloc(pool, sizeof(ELTN_Lexer));

    if (self == NULL) {
        return NULL;
    }
    self->pool = pool;
    ELTN_Pool_acquire(&(self->pool));
    self->token_buffer_size = INIT_BUF_SIZE;
    self->token_buffer = ELTN_alloc(pool, self->token_buffer_size);
    if (self->token_buffer == NULL) {
        ELTN_Lexer_free(self);
        return false;
    }
    self->token_buffer_tail = self->token_buffer;
    return self;
}

void ELTN_Lexer_free(ELTN_Lexer* self) {
    ELTN_Pool* h = self->pool;

    ELTN_free(h, self->token_buffer);
    ELTN_free(h, self);
    ELTN_Pool_release(&h);
}

void ELTN_Lexer_set_char_source(ELTN_Lexer* self, ELTN_Char_Source fcn,
                                void* state) {
    self->get_next_char = fcn;
    self->source = state;
}

void ELTN_Lexer_token_string(ELTN_Lexer* self, char** strptr, size_t* lenptr) {
    if (strptr && lenptr) {
        size_t toklen = self->token_buffer_tail - self->token_buffer;

        ELTN_new_string(strptr, lenptr, (const char *)self->token_buffer,
                        toklen);
    }
}

static int32_t get_next_char(ELTN_Lexer* self) {
    const char8_t last = self->current_char;

    if (self->pushback) {
        self->pushback = false;
        return last;
    }

    if (self->eos) {
        return -1;
    }

    int32_t result = self->get_next_char(self->source, true);

    self->count++;
    if (self->count == 1) {
        self->line = 1;
        self->column = 1;
    } else if (last == '\n') {
        self->line++;
        self->column = 1;
    } else {
        self->column++;
    }

    if (result < 0) {
        self->eos = true;
        return -1;
    }

    self->current_char = 0xFF & result;

    return self->current_char;
}

static bool token_buffer_clear(ELTN_Lexer* self) {
    self->token_buffer_tail = self->token_buffer;
    memset(self->token_buffer, 0, self->token_buffer_size);
    return true;
}

static bool token_buffer_append(ELTN_Lexer* self, int32_t cp) {
    const size_t toklen = self->token_buffer_tail - self->token_buffer;
    const size_t tokmax = self->token_buffer_size;

    if (cp < 0) {
        return false;
    }

    if (toklen + 1 >= tokmax) {
        char8_t* tmp = ELTN_realloc(self->pool, self->token_buffer,
                                    tokmax + INIT_BUF_SIZE);

        if (tmp == NULL) {
            return false;
        }
        self->token_buffer = tmp;
        self->token_buffer_tail = tmp + toklen;
    }
    *(self->token_buffer_tail) = (char8_t) cp;
    self->token_buffer_tail++;
    (*self->token_buffer_tail) = '\0';
    return true;
}

static size_t token_buffer_length(ELTN_Lexer* self) {
    return self->token_buffer_tail - self->token_buffer;
}

static bool token_buffer_equals(ELTN_Lexer* self, const char* str) {
    const size_t toklen = self->token_buffer_tail - self->token_buffer;

    return strncmp((const char *)(self->token_buffer), str, toklen) == 0;
}

static bool token_buffer_starts_with(ELTN_Lexer* self, const char* str) {
    const size_t cmplen = strlen(str);

    return strncmp((const char *)(self->token_buffer), str, cmplen) == 0;
}

static bool token_buffer_ends_with(ELTN_Lexer* self, const char* str) {
    const size_t cmplen = strlen(str);
    const char* token_end = (const char *)(self->token_buffer_tail - cmplen);

    return strncmp(token_end, str, cmplen) == 0;
}

static int strptrcmp(const void* a, const void* b) {
    const char** astrptr = (const char **)a;
    const char** bstrptr = (const char **)b;

    return strcmp(*astrptr, *bstrptr);
}

static bool token_buffer_is_keyword(ELTN_Lexer* self) {
    const char* token = (const char *)self->token_buffer;

    void* result =
        bsearch(&token, KEYWORDS, KEYWORDS_SIZE, sizeof(char8_t *), strptrcmp);

    return result != NULL;
}

static ELTN_Token consume_until_matching_quote(ELTN_Lexer* self, char8_t quote) {
    int32_t prev = quote;
    int32_t curr = get_next_char(self);
    bool quote_found = false;

    while (!self->eos) {
        /*
         * No linebreaks in a string unless escaped with "\\" or "\\z".
         */
        if (curr == '\r') {
            curr = get_next_char(self);
            continue;
        }
        if (curr == '\n') {
            if (prev != '\\' && !token_buffer_ends_with(self, "\\z")) {
                break;
            }
        }
        token_buffer_append(self, curr);
        /*
         * Now check if we have a matching quote
         */
        if (curr == quote && prev != '\\') {
            quote_found = true;
            break;
        }
        prev = curr;
        curr = get_next_char(self);
    }
    return quote_found ? ELTN_TOKEN_STRING : ELTN_TOKEN_INVALID;
}

static bool in_long_bracket(ELTN_Lexer* self, const char* prefix,
                            ssize_t* depthptr) {
    size_t prefix_length = strlen(prefix);
    ssize_t depth = 0;

    if (depthptr) {
        depth = (*depthptr);
    }
    if (depth >= 0
        && token_buffer_length(self) > prefix_length
        && token_buffer_starts_with(self, prefix)
        && token_buffer_ends_with(self, "[")) {
        if (token_buffer_ends_with(self, "[[")) {
            if (depthptr) {
                *depthptr = depth;
            }
            return true;
        }
        for (const char8_t * ptr = self->token_buffer + prefix_length;
             ptr <= self->token_buffer_tail; ptr++) {
            if ((*ptr) == '=') {
                depth++;
            } else if ((*ptr) == '[') {
                if (depthptr) {
                    *depthptr = depth;
                }
                return true;
            } else {
                if (depthptr) {
                    *depthptr = -1;
                }
                return false;
            }
        }
    }
    return false;
}

static bool token_buffer_ends_with_long_bracket(ELTN_Lexer* self, size_t depth) {
    size_t bufidx = 0;
    size_t buflen = depth + 4;
    char bracket[buflen];

    if (token_buffer_ends_with(self, "]")) {
        bracket[0] = ']';
        bufidx++;
        for (int i = 0; i < depth; i++) {
            bracket[bufidx] = '=';
            bufidx++;
        }
        bracket[bufidx] = ']';
        bufidx++;
        bracket[bufidx] = '\0';
        if (token_buffer_ends_with(self, bracket)) {
            return true;
        }
    }
    return false;
}

static ELTN_Token consume_until_end_of_comment(ELTN_Lexer* self) {
    int32_t curr = get_next_char(self);
    bool is_long_start = false;
    bool is_long_end = false;
    ssize_t depth = 0;

    while (!self->eos) {
        if (curr != '\r') {
            token_buffer_append(self, curr);
        }
        is_long_start = is_long_start || in_long_bracket(self, "--[", &depth);
        if (!is_long_start && curr == '\n') {
            break;
        }
        if (is_long_start && token_buffer_ends_with_long_bracket(self, depth)) {
            is_long_end = true;
            break;
        }
        curr = get_next_char(self);
    }
    if (is_long_start) {
        return is_long_end ? ELTN_TOKEN_LONG_COMMENT : ELTN_TOKEN_INVALID;
    } else {
        return ELTN_TOKEN_COMMENT;
    }
}

static ELTN_Token parse_long_string(ELTN_Lexer* self) {
    bool past_open = token_buffer_starts_with(self, "[[");
    bool past_close = token_buffer_ends_with(self, "]]");
    int32_t curr = get_next_char(self);
    ssize_t depth = 0;

    while (!self->eos && !past_close) {
        if (curr != '\r') {
            token_buffer_append(self, curr);
        }
        past_open = past_open || in_long_bracket(self, "[", &depth);
        if (!past_open && (curr != '=' || depth < 0)) {
            return ELTN_TOKEN_INVALID;
        }
        if (past_open && token_buffer_ends_with_long_bracket(self, depth)) {
            past_close = true;
            break;
        }
        curr = get_next_char(self);
    }
    return past_close ? ELTN_TOKEN_LONG_STRING : ELTN_TOKEN_INVALID;
}

static ELTN_Token parse_number(ELTN_Lexer* self, char32_t curr) {
    /*
     * This is a very cheap way to parse a number, but I'm short on time.
     */
    char* ptr = NULL;
    char32_t tmp = get_next_char(self);
    double test;

    while (ELTN_is_number_part(tmp)) {
        token_buffer_append(self, tmp);
        tmp = get_next_char(self);
    }
    self->pushback = true;

    test = strtod((const char *)self->token_buffer, &ptr);
    if (!isnan(test) && !isinf(test)
        && ptr == (char *)self->token_buffer_tail) {
        return ELTN_TOKEN_NUMBER;
    }

    return ELTN_TOKEN_INVALID;
}

ELTN_Token ELTN_Lexer_next_token(ELTN_Lexer* self, int* lineptr, int* colptr) {
    int32_t curr = get_next_char(self);

    token_buffer_clear(self);

    if (curr < 0 || self->eos) {
        if (lineptr) {
            *lineptr = self->line;
        }
        if (colptr) {
            *colptr = self->column;
        }
        return ELTN_TOKEN_EOF;
    }

    while (ELTN_is_space(curr)) {
        curr = get_next_char(self);
    }

    token_buffer_append(self, curr);

    if (lineptr) {
        *lineptr = self->line;
    }
    if (colptr) {
        *colptr = self->column;
    }

    switch (curr) {
    case '[':
        /*
           "[" or /[=*[/" 
         */
        curr = get_next_char(self);
        if (curr == '[' || curr == '=') {
            token_buffer_append(self, curr);
            return parse_long_string(self);
        }
        self->pushback = true;
        return ELTN_TOKEN_SQUARE_OPEN;
    case ']':
        return ELTN_TOKEN_SQUARE_CLOSE;
    case '{':
        return ELTN_TOKEN_CURLY_OPEN;
    case '}':
        return ELTN_TOKEN_CURLY_CLOSE;
    case '=':
        return ELTN_TOKEN_EQUALS;
    case ',':
        return ELTN_TOKEN_COMMA;
    case ';':
        return ELTN_TOKEN_SEMICOLON;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case '.':
        /*
           start of number 
         */
        return parse_number(self, curr);
    case '-':
        /*
           start of number "-" or start of comment "--" 
         */
        curr = get_next_char(self);
        if (curr == '-') {
            token_buffer_append(self, curr);
            return consume_until_end_of_comment(self);
        } else if (ELTN_is_digit(curr) || curr == '.') {
            token_buffer_append(self, curr);
            return parse_number(self, curr);
        } else {
            self->pushback = true;
            return ELTN_TOKEN_INVALID;
        }
    case '\'':
    case '\"':
        /*
           start of (short) string 
         */
        return consume_until_matching_quote(self, curr);
    default:
        /*
           identifier, "true", "false", "nil", or illegal keyword 
         */
        if (ELTN_is_name_start(curr)) {
            curr = get_next_char(self);
            while (!self->eos && ELTN_is_name_part(curr)) {
                token_buffer_append(self, curr);
                curr = get_next_char(self);
            }

            /*
               `curr` is beginning of next token; save for next time 
             */
            self->pushback = true;

            if (token_buffer_equals(self, "true")) {
                return ELTN_TOKEN_BOOLEAN_TRUE;
            } else if (token_buffer_equals(self, "false")) {
                return ELTN_TOKEN_BOOLEAN_FALSE;
            } else if (token_buffer_equals(self, "nil")) {
                return ELTN_TOKEN_NIL;
            } else if (token_buffer_is_keyword(self)) {
                return ELTN_TOKEN_INVALID;
            } else {
                return ELTN_TOKEN_NAME;
            }
        }
        return (curr < 0) ? ELTN_TOKEN_EOF : ELTN_TOKEN_INVALID;
    }
}
