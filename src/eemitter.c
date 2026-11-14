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
#include <wchar.h>
#include <errno.h>

#define  ELTN_CORE	1
#include "eltn.h"
#include "ealloc.h"
#include "ekeyset.h"
#include "etokutil.h"
#include "estrbuf.h"

#define INIT_BUFFER_SIZE   1024

#define PRETTY_PRINT_DEFAULT    true
#define INDENT_DEFAULT          4

/**************************** ELTN_Emitter ***********************************/

typedef struct Node Node;
typedef struct Key_Stack_Frame Key_Stack_Frame;

struct Node {
    Node* next;
    Node* prev;
    ELTN_Event event;
    char* string;
    size_t strlen;
    unsigned int depth;
};

struct Key_Stack_Frame {
    Key_Stack_Frame* next;
    char8_t* last_key;
    size_t last_key_size;
    Key_Set* previous_keys;
    unsigned int depth;
    unsigned int last_implicit_key;
    bool is_def_table;
};

struct ELTN_Emitter {
    intptr_t _reserved;
    ELTN_Pool* pool;
    /*
     * configuration
     */
    bool pretty_print;
    unsigned int indent;
    /*
     * state
     */
    Node* first;
    Node* last;
    Key_Stack_Frame *top;
    unsigned int depth;
    ELTN_Error error_code;
    char* error_path;
    size_t error_path_len;
};

ELTN_API void ELTN_Emitter_free(ELTN_Emitter* self) {
    ELTN_Pool* h = self->pool;

    self->last = NULL;
    while (self->first != NULL) {
        Node* oldnode = self->first;

        self->first = oldnode->next;

        ELTN_free(h, oldnode->string);
        ELTN_free(h, oldnode);
    }
    while (self->top != NULL) {
        Key_Stack_Frame* oldframe = self->top;

        self->top = self->top->next;

        Key_Set_free(oldframe->previous_keys);
        if (oldframe->last_key != NULL) {
            ELTN_free(h, oldframe->last_key);
        }
        ELTN_free(h, oldframe);
    }
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
    /*
     * Properties
     */
    result->pretty_print = PRETTY_PRINT_DEFAULT;
    result->indent = INDENT_DEFAULT;
    /*
     * Current state
     */
    result->first = NULL;
    result->last = NULL;
    result->top = NULL;
    result->depth = 0;
    /*
     * Error reporting
     */
    result->error_code = ELTN_OK;
    result->error_path = NULL;
    result->error_path_len = 0;
    return result;
}

static void add_node(ELTN_Emitter* self, ELTN_Event event,
                     const char* string, size_t strsiz) {
    // create node
    Node* newnode = ELTN_alloc(self->pool, sizeof(Node));

    newnode->event = event;
    newnode->depth = self->depth;
    newnode->strlen = strsiz;
    newnode->string = ELTN_alloc(self->pool, strsiz+1);
    strncpy(newnode->string, string, strsiz);

    // add to last node of linked list
    if (self->first == NULL) {
        newnode->prev = NULL;
        newnode->next = NULL;
        self->first = newnode;
        self->last = newnode;
    } else {
        newnode->prev = self->last; 
        newnode->next = NULL;
        self->last->next = newnode;
        self->last = newnode;
    }
}

static void push_stack(ELTN_Emitter* self) {
    // push new frame onto top of stack
    Key_Stack_Frame* newframe = ELTN_alloc(self->pool, sizeof(Key_Stack_Frame));

    newframe->depth = self->depth;
    newframe->last_key = NULL;
    newframe->last_key_size = 0;
    newframe->previous_keys = Key_Set_new_with_pool(self->pool);
    newframe->last_implicit_key = 0;
    newframe->is_def_table = false;

    newframe->next = self->top;
    self->top = newframe;
}

static void pop_stack(ELTN_Emitter* self) {
    Key_Stack_Frame* oldframe = self->top;

    if (oldframe == NULL) {
        return;
    }
    // pop frame onto top of stack
    self->top = oldframe->next;

    // free space of oldframe
    Key_Set_free(oldframe->previous_keys);
    if (oldframe->last_key != NULL) {
        ELTN_free(self->pool, oldframe->last_key);
    }
    ELTN_free(self->pool, oldframe);
}

static void set_last_key(ELTN_Emitter* self, const char* key, size_t keylen) {
    char8_t* copystr;
    size_t copysiz;

    if (self->top != NULL) {
        ELTN_copy_string(self->pool, &copystr, &copysiz,
                         (const char8_t*) key, keylen);
        if (self->top->last_key != NULL) {
            ELTN_free(self->pool, self->top->last_key);
        }
        self->top->last_key = copystr;
        self->top->last_key_size = copysiz;
    }
}

static void raise_error(ELTN_Emitter* self, ELTN_Error errcode) {
    // TODO: Should this be an error list?
    if (self->error_code == ELTN_OK) {
        self->error_code = errcode;
        // compute error_path from stack's current keys
    }
}

ELTN_API bool ELTN_Emitter_def_name(ELTN_Emitter* self, const char* n) {
    size_t length = strlen(n); // TODO: Unsafe!
    char buffer[length + 4];

    if (self->top == NULL) {
        push_stack(self);
        self->top->is_def_table = true;
    }

    // check that name is an identifier (unqualified error)
    if (!ELTN_is_name(n)) {
        raise_error(self, ELTN_ERR_INVALID_TOKEN);
        return false;
    }

    // check that string is not a keyword (unqualified error)
    if (ELTN_is_keyword(n)) {
        raise_error(self, ELTN_ERR_INVALID_TOKEN);
        return false;
    }

    // check that name isn't a duplicate key (unqualified error)
    if (Key_Set_has_key(self->top->previous_keys, KEY_SET_STRING,
                        (const char8_t*)n, length)) {
        raise_error(self, ELTN_ERR_DUPLICATE_KEY);
        return false;
    }

    // check that we are at the top level (BAD form but let's say not an error).
    if (!self->top->is_def_table) {
        return ELTN_Emitter_key_string(self, n, length);
    }

    snprintf(buffer, sizeof(buffer), "%s = ", n);
    add_node(self, ELTN_DEF_NAME, buffer, strlen(buffer));
    set_last_key(self, n, length);
    Key_Set_add_key(self->top->previous_keys, KEY_SET_STRING,
                    (const char8_t*)n, length);
    return true;
}

ELTN_API bool ELTN_Emitter_key_string(ELTN_Emitter* self, const char* s,
                                      size_t len) {
    // TODO: check that the preceding Node is a VALUE_* or TABLE_START.
    // TODO: check that key is not a keyword (must be quoted)
    // TODO: check that key is an identifier (otherwise must be quoted)
    // TODO: check that key is not a duplicate (unqualified error)
    return false;
}

ELTN_API bool ELTN_Emitter_key_number(ELTN_Emitter* self, double n) {
    // TODO: check that key is not a duplicate (unqualified error)
    // TODO: format with as few decimals as possinle
    return false;
}

ELTN_API bool ELTN_Emitter_key_integer(ELTN_Emitter* self, long int i) {
    // TODO: check that key is not a duplicate (unqualified error)
    return false;
}

ELTN_API bool ELTN_Emitter_value_string(ELTN_Emitter* self, const char* s,
                                        size_t len) {
    // TODO: use long string if string is long.
    return false;
}

ELTN_API bool ELTN_Emitter_value_number(ELTN_Emitter* self, double n) {
    char buffer[INIT_BUFFER_SIZE];
    char sep = (self->depth == 0) ? ';' : ',';

    memset(buffer, 0, sizeof(buffer));
    snprintf(buffer, sizeof(buffer)-1, "%g%c", n, sep);
    add_node(self, ELTN_VALUE_NUMBER, buffer, strlen(buffer));
    return true;
}

ELTN_API bool ELTN_Emitter_value_integer(ELTN_Emitter* self, long int i) {
    return false;
}

ELTN_API bool ELTN_Emitter_value_boolean(ELTN_Emitter* self, bool b) {
    return false;
}

ELTN_API bool ELTN_Emitter_value_nil(ELTN_Emitter* self) {
    return false;
}

ELTN_API bool ELTN_Emitter_table_start(ELTN_Emitter* self) {
    add_node(self, ELTN_TABLE_START, "{", 1);
    self->depth++;
    push_stack(self);
    return true;
}

ELTN_API bool ELTN_Emitter_table_end(ELTN_Emitter* self) {
    add_node(self, ELTN_TABLE_END, "}", 1);
    if (self->depth > 0) {
        self->depth--;
    }
    pop_stack(self);
    return true;
}

ELTN_API bool ELTN_Emitter_comment(ELTN_Emitter* self, const char* s, size_t len) {
    // TODO: make long comment if comment is long or has newlines.
    return false;
}

/*----------------------------- Queries ----------------------------------*/

ELTN_API unsigned int ELTN_Emitter_current_depth(ELTN_Emitter * self) {
    return self->depth;
}

ELTN_API void ELTN_Emitter_current_path(ELTN_Emitter * self, char** strptr,
                                        size_t* lenptr) {
    // TODO: derive path from Key_Stack_Frames
    ELTN_new_string(strptr, lenptr, "", 0);
}

ELTN_API ELTN_Error ELTN_Emitter_error_code(ELTN_Emitter* self) {
    return self->error_code;
}

ELTN_API void ELTN_Emitter_error_path(ELTN_Emitter * self, char** strptr,
                                      size_t* lenptr) {
    // TODO save path from last error.
    if (self->error_path == NULL) {
        ELTN_new_string(strptr, lenptr, "", 0);
    } else {
        ELTN_new_string(strptr, lenptr, self->error_path, self->error_path_len);
    }
}

/*----------------------------- Properties -----------------------------*/

ELTN_API bool ELTN_Emitter_pretty_print(ELTN_Emitter* self) {
    return self->pretty_print; 
}

ELTN_API void ELTN_Emitter_set_pretty_print(ELTN_Emitter* self, bool pretty) {
    self->pretty_print = pretty;
}

ELTN_API unsigned int ELTN_Emitter_indent(ELTN_Emitter* self) {
    return self->indent;
}

ELTN_API void ELTN_Emitter_set_indent(ELTN_Emitter* self, unsigned int indent) {
    self->indent = indent;
}

/*------------------------------ Writing ----------------------------------*/

typedef struct Length_Buffer {
    size_t length;
} Length_Buffer;

static int Length_Writer(void* state, const char* text, size_t size,
                         size_t* nwriteptr) {
    Length_Buffer* buffer = (Length_Buffer*)state;
    buffer->length += size;
    (*nwriteptr) = size;
    return 0;
}

ELTN_API ssize_t ELTN_Emitter_length(ELTN_Emitter* self) {
    Length_Buffer buffer = { 0L };
    
    return ELTN_Emitter_write(self, Length_Writer, &buffer);
}

static int String_Buffer_Writer(void *state, const char* text, size_t size,
                                size_t* nwriteptr) {
    String_Buffer* strbuf = (String_Buffer*)state;

    String_Buffer_append_string(strbuf, text, size);
    (*nwriteptr) = size;
    return 0;
}

ELTN_API void ELTN_Emitter_write_string(ELTN_Emitter* self, char** strptr,
                                        size_t* lenptr) {
    ssize_t nwritten = 0;
    String_Buffer* buffer = String_Buffer_new_with_pool(NULL);
    
    nwritten = ELTN_Emitter_write(self, String_Buffer_Writer, buffer);
    if (nwritten < 0 && self->error_code == ELTN_OK) {
        self->error_code = ELTN_ERR_UNKNOWN;
    }

    //String_Buffer_append_char(buffer, '\0');
    String_Buffer_to_string(buffer, strptr, lenptr);
    String_Buffer_free(buffer);
}

static const char* repeat(char c, unsigned int reps) {
    static char* buf = NULL;
    static size_t bufsiz = 0;

    if (reps == 0 && buf != NULL) {
        free(buf);
        buf = NULL;
        bufsiz = 0;
        return "";
    }

    if (buf == NULL || bufsiz < reps+1) {
        buf = realloc(buf, reps+1);
        if (buf == NULL) {
            bufsiz = 0;
            return "";
        }
        bufsiz = reps+1;
    }
    for (unsigned int i = 0; i < reps; i++) {
        buf[i] = c;
    }
    buf[reps] = '\0';
    return buf;
}

static bool end_of_line(ELTN_Event event) {
    switch (event) {
        case ELTN_DEF_NAME:
        case ELTN_KEY_STRING:
        case ELTN_KEY_NUMBER:
        case ELTN_KEY_INTEGER:
            return false;
        default:
            return true;
    }
}

ELTN_API ssize_t ELTN_Emitter_write(ELTN_Emitter* self, ELTN_Writer writer,
                                    void* state) {
    ssize_t total_bytes = 0;
    size_t nwrite = 0;
    int error = 0;

    // Assemble contents from stored events and flags
    for (Node* node = self->first; node != NULL; node = node->next) {

        if (ELTN_Emitter_pretty_print(self)) {
            unsigned int spaces =
                node->depth * ELTN_Emitter_indent(self);
            error = writer(state, repeat(' ', spaces), spaces, &nwrite);
            if (error) {
                break;
            }
            total_bytes += nwrite;
        }      
        error = writer(state, (char*)node->string, node->strlen, &nwrite);
        if (error) {
            break;
        }
        total_bytes += nwrite;
        if (ELTN_Emitter_pretty_print(self) && end_of_line(node->event)) {
            error = writer(state, "\n", 1, &nwrite);
            if (error) {
                break;
            }
            total_bytes += nwrite;
        }      
    }
    if (error == 0) {
        error = writer(state, "", 0, &nwrite);
    }
    if (error != 0) {
        // TODO: distinguish errno codes
        raise_error(self, ELTN_ERR_UNKNOWN);
    }
    repeat('\0', 0);
    return error == 0 ? total_bytes : -1;
}
