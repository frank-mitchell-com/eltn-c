/*****************************************************************************
 *
 * Copyright 2026 Frank Mitchell
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

#ifndef __ELTN_STRING_BUFFER
#define __ELTN_STRING_BUFFER

#include <stddef.h>
#include "eltn.h"

typedef struct String_Buffer String_Buffer;

String_Buffer* String_Buffer_new_with_pool(ELTN_Pool * pool);

bool String_Buffer_memory_error(String_Buffer * self);

size_t String_Buffer_length(String_Buffer * self);

void String_Buffer_append_char(String_Buffer * self, char c);

void String_Buffer_append_string(String_Buffer * self, const char* s,
                                 size_t len);

void String_Buffer_to_string(String_Buffer * self, char** outstr,
                             size_t* outlen);

void String_Buffer_clear(String_Buffer * self);

void String_Buffer_free(String_Buffer * self);

#endif /* __ELTN_STRING_BUFFER */
