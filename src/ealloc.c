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

#include <stdlib.h>
#include <string.h>

#define ELTN_CORE 1
#include "eltn.h"
#include "ealloc.h"


struct ELTN_Pool {
    intptr_t _reserved;
    unsigned int refcnt;
    ELTN_Alloc alloc;
    void* alloc_state;
};


ELTN_API void ELTN_Pool_new_with_alloc(ELTN_Pool** hptr, ELTN_Alloc alloc,
                                       void* state) {
    if (hptr == NULL) {
        return;
    }

    (*hptr) = NULL;

    if (alloc == NULL) {
        return;
    }

    ELTN_Pool* self = (ELTN_Pool *) alloc(state, NULL, sizeof(ELTN_Pool));

    if (self == NULL) {
        return;
    }
    self->refcnt = 1;
    self->alloc = alloc;
    self->alloc_state = state;

    (*hptr) = self;
}

ELTN_API void ELTN_Pool_acquire(ELTN_Pool** hptr) {
    if (hptr != NULL && (*hptr) != NULL) {
        (*hptr)->refcnt++;
    }
}

ELTN_API void ELTN_Pool_release(ELTN_Pool** hptr) {
    if (hptr != NULL && (*hptr) != NULL) {
        ELTN_Pool* h = (*hptr);

        h->refcnt--;
        if (h->refcnt == 0) {
            h->alloc(h->alloc_state, h, 0);
        }
        (*hptr) = NULL;
    }
}

ELTN_API void ELTN_Pool_set(ELTN_Pool** destptr, ELTN_Pool** srcptr) {
    if (srcptr == NULL) {
        ELTN_Pool_release(destptr);
    } else if (destptr != NULL && (*srcptr) != (*destptr)) {
        ELTN_Pool_release(destptr);
        (*destptr) = (*srcptr);
        ELTN_Pool_acquire(destptr);
    }
}

void* ELTN_alloc(ELTN_Pool* h, size_t size) {
    void* result;
    if (h == NULL) {
        result = malloc(size);
    } else {
        result = h->alloc(h->alloc_state, NULL, size);
    }
    if (result != NULL) {
        memset(result, 0, size);
    }
    return result;
}

void* ELTN_realloc(ELTN_Pool* h, void* ptr, size_t size) {
    if (h == NULL) {
        return realloc(ptr, size);
    } else {
        return h->alloc(h->alloc_state, ptr, size);
    }
}

void ELTN_free(ELTN_Pool* h, void* ptr) {
    if (h == NULL) {
        free(ptr);
    } else {
        h->alloc(h->alloc_state, ptr, 0);
    }
}

void ELTN_new_string(char** strptr, size_t* lenptr,
                     const char* srcstr, size_t srclen) {
    char* dest = (char *)malloc(srclen + 1);

    memset(dest, 0, srclen + 1);
    memcpy(dest, srcstr, srclen);
    (*strptr) = dest;
    (*lenptr) = srclen;
}

void ELTN_free_string(char* str) {
    free(str);
}
