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

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "convert.h"

#define OCTET(x) ((uint8_t)(0xff & (x)))

/* ---------------------- String Type Helpers --------------------------- */

#define MAX(a, b)   (((a) > (b)) ? (a) : (b))
#define MIN(a, b)   (((a) > (b)) ? (b) : (a))

bool C_Conv_is_ascii(size_t sz, const char* buf) {
    for (int i = 0; i < sz; i++) {
        if (buf[i] < 0 || buf[i] > 127) {
            return false;
        }
    }
    return true;
}

FMC_Byte_Order C_Conv_byte_order(size_t insz, char8_t* inbuf, size_t* skipptr) {

    /*
     * Look for Byte Order Mark.
     */
    if (insz >= 3 && inbuf[0] == 0xEF && inbuf[1] == 0xBB && inbuf[2] == 0xBF) {
        /*
         * UTF-8 byte order mark.
         */
        if (skipptr) {
            *skipptr = 3;
        }
        return FMC_BYTE;
    }
    if (insz >= 4) {
        /*
         * We only care if the byte ordering differs from the local
         * machine.  Taking a word at a time compensates for the
         * big-endinanness or little-endianness of the host platform.
         */
        uint32_t first_word = ((uint32_t *) inbuf)[0];

        if (first_word == 0x0000FEFF) {
            if (skipptr) {
                *skipptr = 4;
            }
            return FMC_UCS_4;
        } else if (first_word == 0xFFFE0000) {
            if (skipptr) {
                *skipptr = 4;
            }
            return FMC_UCS_4_SWAP_4321;
        } else if (first_word == 0xFEFF0000) {
            if (skipptr) {
                *skipptr = 4;
            }
            return FMC_UCS_4_SWAP_3412;
        } else if (first_word == 0x0000FFEF) {
            if (skipptr) {
                *skipptr = 4;
            }
            return FMC_UCS_4_SWAP_2143;
        }
        uint16_t first_half_word = ((uint16_t *) inbuf)[0];

        if (first_half_word == 0xFEFF) {
            if (skipptr) {
                *skipptr = 2;
            }
            return FMC_UCS_2;
        } else if (first_half_word == 0xFFEF) {
            if (skipptr) {
                *skipptr = 2;
            }
            return FMC_UCS_2_SWAP;
        }
    }
    if (skipptr) {
        *skipptr = 0;
    }
    return FMC_UNKNOWN;
}

/* -------------------------- UTF-x Conversions -------------------------- */

inline static void swap_bytes(char8_t* a, char8_t* b) {
    char8_t tmp = *a;

    *a = *b;
    *b = tmp;
}

static void flip_bytes_4(size_t length, char8_t* buf) {
    for (int i = 0; i < length; i += 4) {
        swap_bytes(buf + i + 1, buf + i + 4);
        swap_bytes(buf + i + 2, buf + i + 3);
    }
}

static void flip_bytes_2(size_t length, char8_t* buf) {
    for (int i = 0; i < length; i += 2) {
        swap_bytes(buf + i + 1, buf + i + 2);
    }
}

static int write_utf8(char32_t cp, size_t outsz, char8_t* outbuf, size_t j) {
    if (cp <= 0x7f) {
        outbuf[j] = cp;
        return 1;
    } else if (cp <= 0x7ff && j + 2 < outsz) {
        outbuf[j + 0] = (uint8_t) (0xC0 | (0x1F & (cp >> 6)));
        outbuf[j + 1] = (uint8_t) (0x80 | (0x3F & cp));
        return 2;
    } else if (cp <= 0xffff && j + 3 < outsz) {
        outbuf[j + 0] = (uint8_t) (0xE0 | (0x0F & (cp >> 12)));
        outbuf[j + 1] = (uint8_t) (0x80 | (0x3F & (cp >> 6)));
        outbuf[j + 2] = (uint8_t) (0x80 | (0x3F & cp));
        return 3;
    } else if (cp <= 0x1fffff && j + 4 < outsz) {
        // Anything above 0x10ffff not defined by the Unicode Standard 
        // ... yet.
        outbuf[j + 0] = (uint8_t) (0xF0 | (0x07 & (cp >> 18)));
        outbuf[j + 1] = (uint8_t) (0x80 | (0x3F & (cp >> 12)));
        outbuf[j + 2] = (uint8_t) (0x80 | (0x3F & (cp >> 6)));
        outbuf[j + 3] = (uint8_t) (0x80 | (0x3F & cp));
        return 4;
    } else if (cp <= 0x3fffff && j + 5 < outsz) {
        outbuf[j + 0] = (uint8_t) (0xF8 | (0x03 & (cp >> 24)));
        outbuf[j + 1] = (uint8_t) (0x80 | (0x3F & (cp >> 18)));
        outbuf[j + 2] = (uint8_t) (0x80 | (0x3F & (cp >> 12)));
        outbuf[j + 3] = (uint8_t) (0x80 | (0x3F & (cp >> 6)));
        outbuf[j + 4] = (uint8_t) (0x80 | (0x3F & cp));
        return 5;
    } else if (cp <= 0x7fffff && j + 6 < outsz) {
        outbuf[j + 0] = (uint8_t) (0xFC | (0x01 & (cp >> 24)));
        outbuf[j + 1] = (uint8_t) (0x80 | (0x3F & (cp >> 18)));
        outbuf[j + 2] = (uint8_t) (0x80 | (0x3F & (cp >> 12)));
        outbuf[j + 3] = (uint8_t) (0x80 | (0x3F & (cp >> 6)));
        outbuf[j + 4] = (uint8_t) (0x80 | (0x3F & (cp >> 6)));
        outbuf[j + 5] = (uint8_t) (0x80 | (0x3F & cp));
        return 6;
    }
    // TODO: Set errno?
    return 0;
}

static inline bool is_high_surrogate(char16_t v) {
    return v >= 0xD800 && v <= 0xDBFF;
}

static inline bool is_low_surrogate(char16_t v) {
    return v >= 0xDC00 && v <= 0xDFFF;
}

static inline bool is_surrogate(char16_t v) {
    return v >= 0xD800 && v <= 0xDFFF;
}

static char32_t surrogate_pair(char16_t high, char16_t low) {
    return (char32_t) 0x10000 + (((high - 0xD800) << 10))
        + ((low - 0xDC00));
}

static int read_utf16(char32_t* cpp, size_t insz, const char16_t* inbuf,
                      size_t i) {
    char32_t cp = inbuf[i];

    if (!is_surrogate(cp)) {
        (*cpp) = cp;
        return 1;
    } else {
        char32_t cp2 = inbuf[i + 1];

        if (is_high_surrogate(cp) && is_low_surrogate(cp2)) {
            (*cpp) = surrogate_pair(cp, cp2);
            return 2;
        } else if (is_high_surrogate(cp2) && is_low_surrogate(cp)) {
            (*cpp) = surrogate_pair(cp2, cp);
            return 2;
        } else {
            // TODO: Set errno?
            return 0;
        }
    }
}

size_t C_Conv_char16_to_8(size_t insz, const char16_t* inbuf, size_t outsz,
                          char8_t* outbuf) {
    int i = 0;
    int j = 0;

    while (i < insz && j < outsz) {
        char32_t cp;
        int inci, incj;

        inci = read_utf16(&cp, insz, inbuf, i);
        if (inci <= 0) {
            break;
        }
        i += inci;

        incj = write_utf8(cp, outsz, outbuf, j);
        if (incj <= 0) {
            break;
        }
        j += incj;
    }
    return j;
}

size_t C_Conv_char32_to_8(size_t insz, const char32_t* inbuf, size_t outsz,
                          char8_t* outbuf) {
    size_t i;
    size_t j = 0;

    for (i = 0; i < insz && j < outsz; i++) {
        int incj = write_utf8(inbuf[i], outsz, outbuf, j);

        if (incj <= 0) {
            break;
        }
        j += incj;
    }
    return j;
}

void C_Conv_to_char8(FMC_Byte_Order order, size_t insz, const char8_t* inbuf,
                     size_t outsz, char8_t* outbuf, size_t* nreadptr,
                     size_t* nwriteptr) {
    FMC_Byte_Order base_order = order;
    size_t nread = 0;
    size_t nwrite = 0;
    char8_t tmpbuf[insz + 1];

    memcpy(tmpbuf, inbuf, insz);
    tmpbuf[insz] = '\0';

    switch (order) {
    case FMC_UCS_4_SWAP_4321:
        flip_bytes_4(insz, tmpbuf);
        base_order = FMC_UCS_4;
        break;
    case FMC_UCS_4_SWAP_3412:
        flip_bytes_4(insz, tmpbuf);
        flip_bytes_2(insz, tmpbuf);
        base_order = FMC_UCS_4;
        break;
    case FMC_UCS_4_SWAP_2143:
        flip_bytes_2(insz, tmpbuf);
        base_order = FMC_UCS_4;
        break;
    case FMC_UCS_2_SWAP:
        flip_bytes_2(insz, tmpbuf);
        base_order = FMC_UCS_2;
        break;
    default:
        base_order = order;
        break;
    }
    if (base_order == FMC_UCS_4) {
        const size_t insz32 = insz / sizeof(char32_t);

        nread = insz32 * sizeof(char32_t);
        nwrite =
            C_Conv_char32_to_8(insz32, (const char32_t *)tmpbuf, outsz, outbuf);
    } else if (base_order == FMC_UCS_2) {
        const size_t insz16 = insz / sizeof(char16_t);

        nread = insz16 * sizeof(char16_t);
        nwrite =
            C_Conv_char16_to_8(insz16, (const char16_t *)tmpbuf, outsz, outbuf);
    } else {
        const size_t size = MIN(insz, outsz);

        nread = size;
        nwrite = size;
        memmove(outbuf, tmpbuf, size);
    }

    if (nreadptr) {
        (*nreadptr) = nread;
    }
    if (nwriteptr) {
        (*nwriteptr) = nwrite;
    }
}

size_t C_Conv_codepoint_to_char8(char32_t cp, size_t outsz, char8_t* outbuf) {
    return write_utf8(cp, outsz, outbuf, 0);
}
