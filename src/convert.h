/*
 * Copyright 2023 Frank Mitchell
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

#ifndef FMC_CONVERT_INCLUDED
#define FMC_CONVERT_INCLUDED

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "eltn.h"

#ifndef char8_t
typedef uint8_t char8_t;
#endif

#ifndef char16_t
typedef uint16_t char16_t;
#endif

#ifndef char32_t
typedef uint32_t char32_t;
#endif


typedef enum FMC_Byte_Order {
    FMC_BYTE,
    FMC_UCS_2,
    FMC_UCS_2_SWAP,
    FMC_UCS_4,
    FMC_UCS_4_SWAP_4321,
    FMC_UCS_4_SWAP_3412,
    FMC_UCS_4_SWAP_2143,
    FMC_UNKNOWN
} FMC_Byte_Order;


/**
 * Determine whether a string is pure ASCII.
 * `false` implies UTF-8 or a Latin encoding.
 */
bool C_Conv_is_ascii(size_t sz, const char* buf);

/**
 * Derive a byte ordering from the first four bytes.
 * Ideally the first bytes would contain the Byte Order Mark, but if not
 * assume the text translates to ASCII characters.
 */
FMC_Byte_Order C_Conv_byte_order(size_t insz, char8_t * inbuf, size_t* skipptr);

size_t C_Conv_char16_to_8(size_t insz, const char16_t * inbuf, size_t outsz,
                          char8_t * outbuf);


size_t C_Conv_char32_to_8(size_t insz, const char32_t * inbuf, size_t outsz,
                          char8_t * outbuf);

/**
 * Given a byte ordering, convert multibyte text to single bytes of UTF-8.
 */
void C_Conv_to_char8(FMC_Byte_Order order, size_t insz, const char8_t * inbuf,
                     size_t outsz, char8_t * outbuf, size_t* nreadptr,
                     size_t* nwrittenptr);

/**
 * Convert a single UCS-4 codepoint to UTF-8.
 */
size_t C_Conv_codepoint_to_char8(char32_t cp, size_t outsz, char8_t * outbuf);

#endif // FMC_CONVERT_INCLUDED
