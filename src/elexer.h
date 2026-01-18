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
#ifndef __ELTN_LEXER
#define __ELTN_LEXER

#include "eltn.h"
#include "convert.h"

typedef int32_t(*ELTN_Char_Source) (void* state, bool consume);

typedef enum ELTN_Token {
    ELTN_TOKEN_ERROR = -1,
    ELTN_TOKEN_INVALID = 0,
    ELTN_TOKEN_CURLY_OPEN,
    ELTN_TOKEN_CURLY_CLOSE,
    ELTN_TOKEN_SQUARE_OPEN,
    ELTN_TOKEN_SQUARE_CLOSE,
    ELTN_TOKEN_SEMICOLON,
    ELTN_TOKEN_COMMA,
    ELTN_TOKEN_EQUALS,
    ELTN_TOKEN_NAME,
    ELTN_TOKEN_STRING,
    ELTN_TOKEN_LONG_STRING,
    ELTN_TOKEN_NUMBER,
    ELTN_TOKEN_INTEGER,
    ELTN_TOKEN_BOOLEAN_TRUE,
    ELTN_TOKEN_BOOLEAN_FALSE,
    ELTN_TOKEN_NIL,
    ELTN_TOKEN_COMMENT,
    ELTN_TOKEN_LONG_COMMENT,
    ELTN_TOKEN_EOF
} ELTN_Token;

typedef struct ELTN_Lexer ELTN_Lexer;

ELTN_Lexer* ELTN_Lexer_new_with_pool(ELTN_Pool * pool);

void ELTN_Lexer_set_char_source(ELTN_Lexer * self, ELTN_Char_Source fcn,
                                void* state);

ELTN_Token ELTN_Lexer_next_token(ELTN_Lexer * self, int* lineptr, int* colptr);

void ELTN_Lexer_token_string(ELTN_Lexer * self, char** strptr, size_t* lenptr);

void ELTN_Lexer_free(ELTN_Lexer * self);

#endif /* __ELTN_LEXER */
