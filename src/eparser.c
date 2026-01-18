/*****************************************************************************
 *
 * Copyright 2025 Frank Mitchell
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the “Software”),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software->
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT-> IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE->
 *
 ****************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <wctype.h>
#include <errno.h>

#define  ELTN_CORE    1
#include "eltn.h"
#include "elexer.h"
#include "esource.h"
#include "convert.h"
#include "ealloc.h"
#include "estring.h"
#include "ekeyset.h"

#define INIT_BUF_SIZE   512

typedef struct Stack_Frame Stack_Frame;

struct Stack_Frame {
    unsigned int depth;         /* should match the current depth */

    ELTN_Event key_type;
    const char* key;
    unsigned int last_ikey;     /* for values without explicit keys */
    Key_Set* keys;
    Stack_Frame* next;
};

struct ELTN_Parser {
    intptr_t _reserved;
    ELTN_Pool* pool;
    ELTN_Source* source;
    ELTN_Lexer* lexer;

    /*
     * configuration
     */
    bool include_comments;

    /*
     * event state
     */
    ELTN_Event last_event;
    ELTN_Event event;
    char* text;
    size_t text_len;
    char8_t* string;
    size_t string_len;
    size_t string_max;
    /*
     * Table stack
     */
    unsigned int depth;
    bool no_defs;
    Stack_Frame* top;
    /*
     * Error handling and reporting
     */
    ELTN_Error errcode;
    int errline;
    int errcolumn;
};

ELTN_API ELTN_Parser* ELTN_Parser_new() {
    return ELTN_Parser_new_with_pool(NULL);
}

ELTN_API ELTN_Parser* ELTN_Parser_new_with_pool(ELTN_Pool* pool) {
    ELTN_Parser* self = (ELTN_Parser *) ELTN_alloc(pool, sizeof(ELTN_Parser));

    if (self == NULL) {
        return NULL;
    }
    self->pool = pool;
    ELTN_Pool_acquire(&(self->pool));

    self->source = ELTN_Source_new_with_pool(pool);
    if (self->source == NULL) {
        ELTN_Parser_free(self);
        return NULL;
    }
    self->lexer = ELTN_Lexer_new_with_pool(pool);
    if (self->lexer == NULL) {
        ELTN_Parser_free(self);
        return NULL;
    }
    ELTN_Lexer_set_char_source(self->lexer, ELTN_Source_next_char,
                               self->source);
    return self;
}

ELTN_API void ELTN_Parser_free(ELTN_Parser* self) {
    if (self == NULL) {
        return;
    }
    ELTN_Pool* h = self->pool;

    ELTN_Source_free(self->source);
    ELTN_Lexer_free(self->lexer);
    ELTN_free(h, self->string);
    ELTN_free(h, self);
    ELTN_Pool_release(&h);
}

ELTN_API ELTN_Source* ELTN_Parser_source(ELTN_Parser* self) {
    return self->source;
}

ELTN_API bool ELTN_Parser_include_comments(ELTN_Parser* self) {
    return self->include_comments;
}

ELTN_API void ELTN_Parser_set_include_comments(ELTN_Parser* self, bool b) {
    self->include_comments = b;
}

ELTN_API ssize_t ELTN_Parser_read(ELTN_Parser* self, ELTN_Reader reader,
                                  void* state) {
    return ELTN_Source_read(self->source, reader, state);
}

ELTN_API ssize_t ELTN_Parser_read_string(ELTN_Parser* self, const char* text,
                                         size_t len) {
    ELTN_Source* src = ELTN_Parser_source(self);
    ssize_t result = ELTN_Source_write(src, text, len);

    ELTN_Source_close(src);
    return result;
}

ELTN_API bool ELTN_Parser_has_next(ELTN_Parser* self) {
    ELTN_Event ev = ELTN_Parser_event(self);

    return ev != ELTN_STREAM_END && ev != ELTN_ERROR;
}

ELTN_API ELTN_Event ELTN_Parser_event(ELTN_Parser* self) {
    return self->event;
}

ELTN_API unsigned int ELTN_Parser_depth(ELTN_Parser* self) {
    /*
     * TODO: take from depth of context stack
     */
    return self->depth;
}

ELTN_API void ELTN_Parser_current_key(ELTN_Parser* self, ELTN_Event* typeptr,
                                      char** strptr, size_t lenptr) {
    /*
     * TODO: take from context stack
     */
}

ELTN_API void ELTN_Parser_text(ELTN_Parser* self, char** strptr,
                               size_t* sizeptr) {
    if (strptr == NULL || sizeptr == NULL) {
        return;
    }

    if (self->text == NULL) {
        ELTN_new_string(strptr, sizeptr, "", 0);
        return;
    }

    ELTN_new_string(strptr, sizeptr, (const char *)self->text, self->text_len);
}

ELTN_API void ELTN_Parser_string(ELTN_Parser* self, char** strptr,
                                 size_t* sizeptr) {
    if (strptr == NULL || sizeptr == NULL) {
        return;
    }

    if (self->string == NULL) {
        ELTN_new_string(strptr, sizeptr, "", 0);
        return;
    }

    ELTN_new_string(strptr, sizeptr, (const char *)self->string,
                    self->string_len);
}

ELTN_API double ELTN_Parser_number(ELTN_Parser* self) {
    ELTN_Event ev = ELTN_Parser_event(self);

    switch (ev) {
    case ELTN_KEY_NUMBER:
    case ELTN_KEY_INTEGER:
    case ELTN_VALUE_NUMBER:
    case ELTN_VALUE_INTEGER:
        return strtod((const char *)self->string, NULL);
    default:
        return 0.0;
    }
}

ELTN_API long int ELTN_Parser_integer(ELTN_Parser* self) {
    ELTN_Event ev = ELTN_Parser_event(self);
    int base = 10;

    switch (ev) {
    case ELTN_KEY_NUMBER:
    case ELTN_KEY_INTEGER:
    case ELTN_VALUE_NUMBER:
    case ELTN_VALUE_INTEGER:
        if (strncasecmp((const char *)self->string, "0x", 2) == 0) {
            base = 16;
        }
        return strtol((const char *)self->string, NULL, base);
    default:
        return 0.0;
    }
}

ELTN_API bool ELTN_Parser_boolean(ELTN_Parser* self) {
    ELTN_Event ev = ELTN_Parser_event(self);

    switch (ev) {
    case ELTN_VALUE_NIL:
    case ELTN_VALUE_FALSE:
        return false;
    default:
        return true;
    }
}

ELTN_API ELTN_Error ELTN_Parser_error_code(ELTN_Parser* self) {
    return self->errcode;
}

ELTN_API unsigned int ELTN_Parser_error_line(ELTN_Parser* self) {
    return self->errline;
}

ELTN_API unsigned int ELTN_Parser_error_column(ELTN_Parser* self) {
    return self->errcolumn;
}

/* ------------------------- ACTUAL PARSING CODE ------------------------ */

static void signal_out_of_memory(ELTN_Parser* self) {
    self->event = ELTN_ERROR;
    self->errcode = ELTN_ERR_OUT_OF_MEMORY;
}

static void set_string_ref(ELTN_Parser* self, char* str, size_t len) {
    if (str != NULL) {
        ELTN_free(self->pool, self->string);
        self->string = (char8_t *) str;
        self->string_len = len;
        self->string_max = len;
        return;
    }
}

static bool set_string_copy(ELTN_Parser* self, char* str, size_t len) {
    if (self->string == NULL || self->string_max <= len) {
        self->string = ELTN_realloc(self->pool, self->string, len + 1);
        if (self->string == NULL) {
            signal_out_of_memory(self);
            return false;
        }
        self->string_len = len;
        self->string_max = len + 1;
    }
    memset(self->string, 0, self->string_max);
    if (str == NULL) {
        self->string_len = 0;
    } else {
        memcpy(self->string, str, len);
        self->string_len = len;
    }
    return true;
}

static void capture_token_buffer(ELTN_Parser* self) {
    char* tokstr = NULL;
    size_t toklen = 0;

    ELTN_Lexer_token_string(self->lexer, &tokstr, &toklen);
    ELTN_free_string(self->text);
    self->text = tokstr;
    self->text_len = toklen;
}

static void set_event(ELTN_Parser* self, ELTN_Token token, ELTN_Event event) {
    char* str = NULL;
    size_t len = 0;

    self->event = event;
    capture_token_buffer(self);
    switch (token) {
    case ELTN_TOKEN_STRING:
        ELTN_unescape_quoted_string(self->pool, self->text, self->text_len,
                                    &str, &len);
        if (str == NULL) {
            signal_out_of_memory(self);
            return;
        }
        set_string_ref(self, str, len);
        break;
    case ELTN_TOKEN_LONG_STRING:
        ELTN_unquote_long_string(self->pool, self->text, self->text_len, &str,
                                 &len);
        if (str == NULL) {
            signal_out_of_memory(self);
            return;
        }
        set_string_ref(self, str, len);
        break;
    case ELTN_TOKEN_COMMENT:
    case ELTN_TOKEN_LONG_COMMENT:
        ELTN_trim_comment(self->pool, self->text, self->text_len, &str, &len);
        if (str == NULL) {
            signal_out_of_memory(self);
            return;
        }
        set_string_ref(self, str, len);
        break;
    default:
        if (!set_string_copy(self, self->text, self->text_len)) {
            signal_out_of_memory(self);
            return;
        }
        break;
    }
}

static void signal_error(ELTN_Parser* self, ELTN_Token token,
                         int line, int column) {
    self->event = ELTN_ERROR;
    capture_token_buffer(self);
    set_string_copy(self, self->text, self->text_len);
    self->errline = line;
    self->errcolumn = column;
    if (token == ELTN_TOKEN_INVALID) {
        self->errcode = ELTN_ERR_INVALID_TOKEN;
    } else if (token == ELTN_TOKEN_EOF) {
        self->errcode = ELTN_ERR_STREAM_END;
    } else if (token == ELTN_TOKEN_ERROR) {
        /*
         * TODO: Be more specific
         */
        self->errcode = ELTN_ERR_UNKNOWN;
    } else {
        self->errcode = ELTN_ERR_UNEXPECTED_TOKEN;
    }
}

static ELTN_Token next_token(ELTN_Parser* self, int* lineptr, int* columnptr) {
    ELTN_Token nextToken =
        ELTN_Lexer_next_token(self->lexer, lineptr, columnptr);
    while (nextToken == ELTN_TOKEN_COMMENT ||
           nextToken == ELTN_TOKEN_LONG_COMMENT) {
        /*
         * TODO: Something like this:
         *
         * if (ELTN_Parser_include_comments(self)) {
         *     enqueue_event(self, ELTN_COMMENT, token, lineptr, columnptr);
         *     continue;
         * }
         */
        nextToken = ELTN_Lexer_next_token(self->lexer, lineptr, columnptr);
    }
    return nextToken;
}

static bool expect_value(ELTN_Parser* self, ELTN_Token token) {
    switch (token) {
    case ELTN_TOKEN_STRING:
        set_event(self, token, ELTN_VALUE_STRING);
        return true;
    case ELTN_TOKEN_LONG_STRING:
        set_event(self, token, ELTN_VALUE_STRING);
        return true;
    case ELTN_TOKEN_NUMBER:
        set_event(self, token, ELTN_VALUE_NUMBER);
        return true;
    case ELTN_TOKEN_INTEGER:
        set_event(self, token, ELTN_VALUE_INTEGER);
        return true;
    case ELTN_TOKEN_BOOLEAN_TRUE:
        set_event(self, token, ELTN_VALUE_TRUE);
        return true;
    case ELTN_TOKEN_BOOLEAN_FALSE:
        set_event(self, token, ELTN_VALUE_FALSE);
        return true;
    case ELTN_TOKEN_NIL:
        set_event(self, token, ELTN_VALUE_NIL);
        return true;
    case ELTN_TOKEN_CURLY_OPEN:
        set_event(self, token, ELTN_TABLE_START);
        // TODO: Push a new frame on the stack
        self->depth++;
        return true;
    default:
        return false;
    }
    return false;
}

static bool expect_new_entry(ELTN_Parser* self, ELTN_Token token, int* lineptr,
                             int* colptr) {
    if (token == ELTN_TOKEN_NAME) {
        set_event(self, token, ELTN_KEY_STRING);
        return true;
    }
    if (token == ELTN_TOKEN_SQUARE_OPEN) {
        // a non-identifier key
        token = next_token(self, lineptr, colptr);
        switch (token) {
        case ELTN_TOKEN_STRING:
            set_event(self, token, ELTN_KEY_STRING);
            break;
        case ELTN_TOKEN_NUMBER:
            set_event(self, token, ELTN_KEY_NUMBER);
            break;
        case ELTN_TOKEN_INTEGER:
            set_event(self, token, ELTN_KEY_INTEGER);
            break;
        case ELTN_TOKEN_LONG_STRING:
            set_event(self, token, ELTN_KEY_STRING);
            break;
        default:
            return false;
        }
        /*
         * TODO: update current_key on the current stack frame.
         */

        token = next_token(self, lineptr, colptr);
        if (token != ELTN_TOKEN_SQUARE_CLOSE) {
            signal_error(self, token, *lineptr, *colptr);
        }
        return true;
    }
    // Maybe values with implicit indexes?
    bool result = expect_value(self, token);

    if (result) {
        /*
         * TODO: Update stack frame with implicit index.
         */
    }
    return result;
}

static bool expect_table_start(ELTN_Parser* self, ELTN_Token token) {
    if (token == ELTN_TOKEN_CURLY_OPEN) {
        set_event(self, token, ELTN_TABLE_START);
        // TODO: Push a new frame on the stack
        self->depth++;
        return true;
    }
    return false;
}

static bool expect_table_end(ELTN_Parser* self, ELTN_Token token) {
    if (token == ELTN_TOKEN_CURLY_CLOSE && self->depth > 0) {
        set_event(self, token, ELTN_TABLE_END);
        // TODO: Pop a frame off the stack
        self->depth--;
        return true;
    }
    return false;
}

static bool expect_new_definition(ELTN_Parser* self, ELTN_Token token,
                                  int* lineptr, int* colptr) {
    while (token == ELTN_TOKEN_SEMICOLON) {
        token = next_token(self, lineptr, colptr);
    }
    if (token == ELTN_TOKEN_NAME) {
        set_event(self, token, ELTN_DEF_NAME);
        /*
         * TODO: if no definition level frame on the stack, push one on.
         * TODO: set the current_key (sic) on the zero-depth frame.
         */
        return true;
    }
    return false;
}

static bool expect_stream_end(ELTN_Parser* self, ELTN_Token token) {
    if (token == ELTN_TOKEN_EOF) {
        set_event(self, token, ELTN_STREAM_END);
        return true;
    }
    return false;
}

ELTN_API void ELTN_Parser_next(ELTN_Parser* self) {
    ELTN_Token token;
    int line, column;

    if (self->event != ELTN_COMMENT) {
        self->last_event = self->event;
    }

    token = next_token(self, &line, &column);

    switch (self->last_event) {
    case ELTN_STREAM_START:
        // expect either a definition name or a new table (or a comment)
        if (expect_table_start(self, token)) {
            self->no_defs = true;
            return;
        } else if (expect_new_definition(self, token, &line, &column)) {
            return;
        } else if (expect_stream_end(self, token)) {
            return;
        } else {
            signal_error(self, token, line, column);
        }
        break;
    case ELTN_COMMENT:
        // check what the previous (non-comment) event was ...
        ELTN_Parser_next(self);
        break;
    case ELTN_DEF_NAME:
    case ELTN_KEY_STRING:
    case ELTN_KEY_NUMBER:
    case ELTN_KEY_INTEGER:
        // expect a value for this key or a table start
        if (token != ELTN_TOKEN_EQUALS) {
            signal_error(self, token, line, column);
            return;
        }
        token = next_token(self, &line, &column);
        if (expect_value(self, token)) {
            return;
        } else {
            signal_error(self, token, line, column);
        }
        break;
    case ELTN_TABLE_END:
    case ELTN_VALUE_STRING:
    case ELTN_VALUE_NUMBER:
    case ELTN_VALUE_INTEGER:
    case ELTN_VALUE_TRUE:
    case ELTN_VALUE_FALSE:
    case ELTN_VALUE_NIL:
        if (self->depth == 0) {
            if (!self->no_defs
                && expect_new_definition(self, token, &line, &column)) {
                return;
            } else if (expect_stream_end(self, token)) {
                return;
            } else {
                signal_error(self, token, line, column);
            }
        } else if (token == ELTN_TOKEN_COMMA || token == ELTN_TOKEN_SEMICOLON) {
            // bypass the (required) separator, unless this is the top level
            // TODO: only semicolons or nothing at the definition level
            token = next_token(self, &line, &column);
            if (expect_new_entry(self, token, &line, &column)) {
                return;
            } else if (expect_table_end(self, token)) {
                return;
            } else {
                signal_error(self, token, line, column);
            }
        } else {
            if (expect_table_end(self, token)) {
                return;
            } else {
                signal_error(self, token, line, column);
            }
        }
        break;
    case ELTN_TABLE_START:
        // expect new keys or values, or a table end
        if (expect_new_entry(self, token, &line, &column)) {
            return;
        } else if (expect_table_end(self, token)) {
            return;
        } else {
            signal_error(self, token, line, column);
        }
        break;
    case ELTN_STREAM_END:
    case ELTN_ERROR:
        // expect nothing: we're at an end state
        break;
    }
}
