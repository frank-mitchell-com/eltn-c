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

#ifndef __ELTN_KEY_SET
#define __ELTN_KEY_SET

#include <stdint.h>
#include "eltn.h"

typedef struct Key_Set Key_Set;

typedef struct Key_Set_Iterator Key_Set_Iterator;

typedef enum Key_Type {
    KEY_SET_EMPTY = 0,
    KEY_SET_STRING,
    KEY_SET_NUMBER
} Key_Type;

Key_Set* Key_Set_new_with_pool(ELTN_Pool * pool);

size_t Key_Set_size(Key_Set * s);

size_t Key_Set_capacity(Key_Set * s);

bool Key_Set_has_key(Key_Set * s, Key_Type t, const char* str, size_t len);

bool Key_Set_add_key(Key_Set * s, Key_Type t, const char* str, size_t len);

Key_Set_Iterator* Key_Set_iterator(Key_Set * s);

Key_Type Key_Set_Iterator_next(Key_Set_Iterator * i);

void Key_Set_Iterator_string(Key_Set_Iterator * i, char** strptr,
                             size_t* lenptr);

void Key_Set_Iterator_free(Key_Set_Iterator * i);

void Key_Set_free(Key_Set * s);

#endif /* __ELTN_KEY_SET */
