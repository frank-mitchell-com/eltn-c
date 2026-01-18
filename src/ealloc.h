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

#ifndef __ELTN_ALLOCATOR
#define __ELTN_ALLOCATOR

#include <stdint.h>
#include "eltn.h"

/**
 * Allocate a new chunk of memory from the pool, and zero it out.
 */
void* ELTN_alloc(ELTN_Pool * h, size_t size);

/**
 * Extend or reduce a chunk of memory to the new size.
 */
void* ELTN_realloc(ELTN_Pool * h, void* ptr, size_t size);

/**
 * Free a chunk of memory.
 */
void ELTN_free(ELTN_Pool * h, void* ptr);

/**
 * Create a copy of a string using the default allocator.
 */
void ELTN_new_string(char** strptr, size_t* lenptr, const char* srcstr,
                     size_t srclen);

/**
 * Free a string using the default allocator.
 */
void ELTN_free_string(char* str);

#endif /* __ELTN_ALLOCATOR */
