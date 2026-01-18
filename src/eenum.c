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

#define  ELTN_CORE	1
#include "eltn.h"
#include "ealloc.h"

typedef struct Name_Record {
    int code;
    const char* name;
} Name_Record;

static Name_Record EVENT_NAMES[] = {
    {ELTN_ERROR, "ELTN_ERROR"},
    {ELTN_STREAM_START, "ELTN_STREAM_START"},
    {ELTN_COMMENT, "ELTN_COMMENT"},
    {ELTN_DEF_NAME, "ELTN_DEF_NAME"},
    {ELTN_KEY_STRING, "ELTN_KEY_STRING"},
    {ELTN_KEY_NUMBER, "ELTN_KEY_NUMBER"},
    {ELTN_KEY_INTEGER, "ELTN_KEY_INTEGER"},
    {ELTN_VALUE_STRING, "ELTN_VALUE_STRING"},
    {ELTN_VALUE_NUMBER, "ELTN_VALUE_NUMBER"},
    {ELTN_VALUE_INTEGER, "ELTN_VALUE_INTEGER"},
    {ELTN_VALUE_TRUE, "ELTN_VALUE_TRUE"},
    {ELTN_VALUE_FALSE, "ELTN_VALUE_FALSE"},
    {ELTN_VALUE_NIL, "ELTN_VALUE_NIL"},
    {ELTN_TABLE_START, "ELTN_TABLE_START"},
    {ELTN_TABLE_END, "ELTN_TABLE_END"},
    {ELTN_STREAM_END, "ELTN_STREAM_END"}
};

static Name_Record ERROR_NAMES[] = {
    {ELTN_ERR_UNKNOWN, "ELTN_ERR_UNKNOWN"},
    {ELTN_OK, "ELTN_OK"},
    {ELTN_ERR_OUT_OF_MEMORY, "ELTN_ERR_OUT_OF_MEMORY"},
    {ELTN_ERR_STREAM_END, "ELTN_ERR_STREAM_END"},
    {ELTN_ERR_UNEXPECTED_TOKEN, "ELTN_ERR_UNEXPECTED_TOKEN"},
    {ELTN_ERR_INVALID_TOKEN, "ELTN_ERR_INVALID_TOKEN"},
    {ELTN_ERR_DUPLICATE_KEY, "ELTN_ERR_DUPLICATE_KEY"}
};

static const size_t ERROR_NAME_COUNT =
    sizeof(ERROR_NAMES) / sizeof(Name_Record);

static void name_to_string(const char* name, char** strptr, size_t* sizeptr) {
    size_t len = strlen(name);

    ELTN_new_string(strptr, sizeptr, name, len);
}

ELTN_API const char* ELTN_Event_name(ELTN_Event e) {
    if (e < ELTN_ERROR || e > ELTN_STREAM_END) {
        return "";
    } else {
        return EVENT_NAMES[e - ELTN_ERROR].name;
    }
}

ELTN_API void ELTN_Event_string(ELTN_Event e, char** strptr, size_t* sizeptr) {
    name_to_string(ELTN_Event_name(e), strptr, sizeptr);
}

static int key_cmp(const void* key, const void* rec) {
    return ((Name_Record *) key)->code - ((Name_Record *) rec)->code;
}

ELTN_API const char* ELTN_Error_name(ELTN_Error e) {
    /*
     * Unlike the Event enum, this enum is still being written, and I'm not
     * sure how it will end up.  So we'll  take a slightly slower way ...
     */
    Name_Record key = { e, "" };
    Name_Record* result =
        bsearch(&key, ERROR_NAMES, ERROR_NAME_COUNT, sizeof(Name_Record),
                key_cmp);

    if (result == NULL) {
        return "";
    }
    return result->name;
}

ELTN_API void ELTN_Error_string(ELTN_Error e, char** strptr, size_t* sizeptr) {
    name_to_string(ELTN_Error_name(e), strptr, sizeptr);
}
