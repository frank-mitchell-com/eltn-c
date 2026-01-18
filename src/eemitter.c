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

#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#define  ELTN_CORE	1
#include "eltn.h"
#include "ealloc.h"

#define INIT_BUF_SIZE   1024

#define PRETTY_PRINT_DEFAULT    true
#define INDENT_DEFAULT          4

/**************************** ELTN_Emitter ***********************************/

struct ELTN_Emitter {
    intptr_t _reserved;
    ELTN_Pool* pool;
    /*
     * configuration
     */
    bool pretty_print;
    unsigned int indent;
};

ELTN_API void ELTN_Emitter_free(ELTN_Emitter* self) {
    ELTN_Pool* h = self->pool;

    ELTN_free(h, self);
    ELTN_Pool_release(&h);
}

ELTN_API ELTN_Emitter* ELTN_Emitter_new() {
    return ELTN_Emitter_new_with_pool(NULL);
}

ELTN_API ELTN_Emitter* ELTN_Emitter_new_with_pool(ELTN_Pool* pool) {
    ELTN_Emitter* result =
        (ELTN_Emitter *) ELTN_alloc(pool, sizeof(ELTN_Emitter));

    if (result == NULL) {
        return NULL;
    }
    result->pool = pool;
    ELTN_Pool_acquire(&(result->pool));
    result->pretty_print = PRETTY_PRINT_DEFAULT;
    result->indent = INDENT_DEFAULT;
    /*
       other data structures go here 
     */
    return result;
}

ELTN_API bool ELTN_Emitter_def_name(ELTN_Emitter* e, const char* n) {
    return false;
}

ELTN_API bool ELTN_Emitter_key_string(ELTN_Emitter* e, const char* s,
                                      size_t len) {
    return false;
}

ELTN_API bool ELTN_Emitter_key_number(ELTN_Emitter* e, double n,
                                      unsigned int sigfigs) {
    return false;
}

ELTN_API bool ELTN_Emitter_key_integer(ELTN_Emitter* e, int i) {
    return false;
}

ELTN_API bool ELTN_Emitter_key_boolean(ELTN_Emitter* e, bool b) {
    return false;
}

ELTN_API bool ELTN_Emitter_value_string(ELTN_Emitter* e, const char* s,
                                        size_t len) {
    return false;
}

ELTN_API bool ELTN_Emitter_value_number(ELTN_Emitter* e, double n,
                                        unsigned int sigfigs) {
    return false;
}

ELTN_API bool ELTN_Emitter_value_integer(ELTN_Emitter* e, int i) {
    return false;
}

ELTN_API bool ELTN_Emitter_value_boolean(ELTN_Emitter* e, bool b) {
    return false;
}

ELTN_API bool ELTN_Emitter_value_nil(ELTN_Emitter* e) {
    return false;
}

ELTN_API bool ELTN_Emitter_table_start(ELTN_Emitter* e) {
    return false;
}

ELTN_API bool ELTN_Emitter_table_end(ELTN_Emitter* e) {
    return false;
}

ELTN_API bool ELTN_Emitter_comment(ELTN_Emitter* e, const char* s, size_t len) {
    return false;
}

ELTN_API ELTN_Error ELTN_Emitter_error_code(ELTN_Emitter* e) {
    return ELTN_ERR_UNKNOWN;
}

ELTN_API void ELTN_Emitter_error_path(ELTN_Emitter* e, char** strptr,
                                      size_t* lenptr) {
}

ELTN_API bool ELTN_Emitter_pretty_print(ELTN_Emitter* e) {
    return false;
}

ELTN_API void ELTN_Emitter_set_pretty_print(ELTN_Emitter* e, bool pretty) {
}

ELTN_API unsigned int ELTN_Emitter_indent(ELTN_Emitter* e) {
    return 0;
}

ELTN_API void ELTN_Emitter_set_indent(ELTN_Emitter* e, unsigned int indent) {
}

ELTN_API ssize_t ELTN_Emitter_length(ELTN_Emitter* self) {
    return 0;
}

ELTN_API void ELTN_Emitter_write_string(ELTN_Emitter* e, char** strptr,
                                        size_t* lenptr) {
}

ELTN_API ssize_t ELTN_Emitter_write(ELTN_Emitter* e, ELTN_Writer writer,
                                    void* state) {
    return 0;
}
