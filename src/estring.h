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
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 ****************************************************************************/

#ifndef __ELTN_STRING_UTILITIES
#define __ELTN_STRING_UTILITIES

#include <stdbool.h>
#include <stdint.h>
#include "eltn.h"

void ELTN_unescape_quoted_string(ELTN_Pool * h,
                                 const char* instr, const size_t inlen,
                                 char** outstrptr, size_t* lenptr);

void ELTN_unquote_long_string(ELTN_Pool * h,
                              const char* instr, const size_t inlen,
                              char** outstrptr, size_t* lenptr);

void ELTN_trim_comment(ELTN_Pool * h, const char* instr, const size_t inlen,
                       char** outstrptr, size_t* lenptr);

bool ELTN_is_space(uint32_t c);

bool ELTN_is_letter(uint32_t c);

bool ELTN_is_digit(uint32_t c);

bool ELTN_is_hexdigit(uint32_t c);

bool ELTN_is_octdigit(uint32_t c);

bool ELTN_is_name_start(uint32_t c);

bool ELTN_is_name_part(uint32_t c);

bool ELTN_is_number_part(uint32_t c);

#endif /* __ELTN_STRING_UTILITIES */
