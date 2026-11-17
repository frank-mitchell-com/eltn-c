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

#include <stdint.h>
#include <string.h>
#include <strings.h>
#include "estrbuf.h"
#include "ealloc.h"

#define INIT_BUFFER_SIZE   256

#define MAX(a, b)   (((a) > (b)) ? (a) : (b))

struct String_Buffer {
    intptr_t __reserved;
    ELTN_Pool* pool;
    char* bytes;
    size_t size;
    size_t used;
    bool memory_error;
};

String_Buffer* String_Buffer_new_with_pool(ELTN_Pool* pool) {
    String_Buffer* result = ELTN_alloc(pool, sizeof(String_Buffer));
    result->pool = pool;
    result->bytes = NULL;
    result->size = 0;
    result->used = 0;
    result->memory_error = false;
    return result;
}

bool String_Buffer_memory_error(String_Buffer* self) {
    return self->memory_error;
}

size_t String_Buffer_length(String_Buffer* self) {
    return self->used;
}

static bool make_room(String_Buffer* self, size_t incr) {
    if (self->bytes == NULL || self->used + incr >= self->size) {
        size_t newsize = self->size + MAX(incr, INIT_BUFFER_SIZE);
        char* result =
            (char *)ELTN_realloc(self->pool, self->bytes, newsize);
        if (result == NULL) {
            self->memory_error = true;
            return false;
        }
        self->bytes = result;
        self->size = newsize;
    }
    return true;
}

void String_Buffer_append_char(String_Buffer* self, char c) {
    if (!make_room(self, 1)) {
        return;
    }
    self->bytes[self->used] = c;
    self->used++;
}

void String_Buffer_append_string(String_Buffer* self, const char* text,
                                 size_t size) {
    if (!make_room(self, size)) {
        return;
    }
    memcpy(self->bytes + self->used, text, size); 
    self->used += size;
}

void String_Buffer_to_string(String_Buffer* self, char** outstr, size_t* outlen) {
    String_Buffer_append_char(self, '\0');
    (*outstr) = self->bytes;
    (*outlen) = self->used - 1;
    ELTN_realloc(self->pool, self->bytes, self->used);
    self->bytes = NULL;
    self->used = 0;
    self->size = 0;
}

void String_Buffer_clear(String_Buffer* self) {
    if (self->bytes != NULL) {
        ELTN_free(self->pool, self->bytes);
        self->bytes = NULL;
        self->used = 0;
        self->size = 0;
    }
}

void String_Buffer_free(String_Buffer* self) {
    if (self->bytes != NULL) {
        ELTN_free(self->pool, self->bytes);
    }
    ELTN_free(self->pool, self);
}
