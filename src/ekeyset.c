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

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "ekeyset.h"
#include "ealloc.h"

#define TABLE_MINSIZ    5
#define TABLE_LOAD      0.65

typedef struct Key {
    Key_Type type;
    size_t len;
    char* str;
    double num;
} Key;

struct Key_Set {
    intptr_t _reserved;
    ELTN_Pool* pool;

    size_t nitems;
    size_t arraysize;
    Key* array;
};

struct Key_Set_Iterator {
    intptr_t _reserved;
    ELTN_Pool* pool;

    ssize_t index;
    ssize_t length;
    Key* keys;
};

Key_Set* Key_Set_new_with_pool(ELTN_Pool* pool) {
    Key_Set* self = ELTN_alloc(pool, sizeof(Key_Set));

    if (self == NULL) {
        return NULL;
    }
    self->pool = pool;
    ELTN_Pool_acquire(&(self->pool));

    self->arraysize = TABLE_MINSIZ;
    self->array = ELTN_alloc(pool, sizeof(Key) * self->arraysize);
    if (self == NULL) {
        ELTN_free(pool, self);
        return NULL;
    }
    return self;
}

void Key_Set_free(Key_Set* self) {
    ELTN_Pool* h = self->pool;

    for (size_t i = 0; i < self->arraysize; i++) {
        ELTN_free(h, self->array[i].str);
    }
    ELTN_free(h, self->array);
    ELTN_free(h, self);
    ELTN_Pool_release(&h);
}

static bool init_key(Key_Set* self, Key* value, Key_Type t,
                     const char* str, size_t len) {
    char* cstr = ELTN_alloc(self->pool, len);

    if (cstr == NULL) {
        return false;
    }

    strncpy(cstr, str, len);

    value->type = t;
    value->len = len;
    value->str = cstr;
    if (t == KEY_SET_NUMBER) {
        value->num = strtod(cstr, NULL);
    } else {
        value->num = 0.0;
    }
    return true;
}

static void clear_key(Key_Set* self, Key* value) {
    ELTN_free(self->pool, value->str);
    memset(value, 0, sizeof(Key));
}

static bool is_equal(Key* a, Key* b) {
    if (a->type != b->type) {
        return false;
    }

    switch (a->type) {
    case KEY_SET_NUMBER:
        return a->num == b->num;
    case KEY_SET_STRING:
        return a->len == b->len && strncmp(a->str, b->str, a->len) == 0;
    default:
        return false;
    }
}

static uint_fast64_t hash_string(const char* str, size_t len) {
    /*
     * Vibe-coded from
     * <https://cp-algorithms.com/string/string-hashing.html>
     */
    if (str == NULL || len == 0) {
        return 0;
    }
    const int p = 31;
    const int m = 1e9 + 9;
    int_fast64_t hash_value = 0;
    int_fast64_t p_pow = 1;

    for (int i = 0; i < len; i++) {
        char c = str[i];

        hash_value = (hash_value + (c - 'a' + 1) * p_pow) % m;
        p_pow = (p_pow * p) % m;
    }
    return 0x7FFFFFFF & hash_value;
}
static uint_fast64_t hash_double(double val) {
    return hash_string((const char *)&val, sizeof(double) / sizeof(char));
}

static uint_fast64_t hashvalue(const Key* value) {
    switch (value->type) {
    case KEY_SET_NUMBER:
        return hash_double(value->num);
    default:
        return hash_string(value->str, value->len);
    }
}

size_t Key_Set_size(Key_Set* self) {
    return self->nitems;
}

size_t Key_Set_capacity(Key_Set* self) {
    return self->arraysize;
}

bool Key_Set_has_key(Key_Set* self, Key_Type t, const char* str, size_t len) {
    bool result;
    Key value;

    if (!init_key(self, &value, t, str, len)) {
        return false;
    }

    const size_t original = hashvalue(&value) % self->arraysize;
    Key* op = &(self->array[original]);

    if (op->type == KEY_SET_EMPTY) {
        result = false;
    } else if (is_equal(&value, op)) {
        result = true;
    } else {
        size_t index = original;
        Key* p;

        do {
            index = (index + 1) % self->arraysize;
            p = &(self->array[index]);
            if (is_equal(&value, p)) {
                result = true;
                break;
            }
        } while (index != original);
        if (index == original) {
            result = false;
        }
    }
    clear_key(self, &value);
    return result;
}

static bool add_key(Key_Set* self, const Key* p) {
    const size_t original = hashvalue(p) % self->arraysize;
    size_t index = original;

    if (self->array[index].type != KEY_SET_EMPTY) {
        do {
            index = (index + 1) % self->arraysize;
        } while (index != original && self->array[index].type != KEY_SET_EMPTY);
        if (index == original) {
            return false;
        }
    }
    memcpy(&(self->array[index]), p, sizeof(Key));
    self->nitems++;
    return true;
}

static void resize(Key_Set* self) {
    size_t oldlen = self->arraysize;
    size_t newlen = oldlen * 2 + 1;
    Key* oldarray = self->array;
    Key* newarray = ELTN_alloc(self->pool, newlen * sizeof(Key));

    if (newarray == NULL) {
        return;
    }

    memset(newarray, 0, newlen * sizeof(Key));

    self->array = newarray;
    self->arraysize = newlen;
    self->nitems = 0;

    for (size_t i = 0; i < oldlen; i++) {
        const Key* p = &(oldarray[i]);

        if (p->type == KEY_SET_EMPTY) {
            continue;
        }

        add_key(self, p);
    }
    ELTN_free(self->pool, oldarray);
}

bool Key_Set_add_key(Key_Set* self, Key_Type t, const char* str, size_t len) {
    Key value;

    if (t == KEY_SET_EMPTY || Key_Set_has_key(self, t, str, len)) {
        return false;
    }

    if ((self->nitems + 1) >= TABLE_LOAD * self->arraysize) {
        resize(self);
    }

    if (!init_key(self, &value, t, str, len)) {
        return false;
    }

    return add_key(self, &value);
}

Key_Set_Iterator* Key_Set_iterator(Key_Set* self) {
    Key_Set_Iterator* iter =
        (Key_Set_Iterator *) ELTN_alloc(self->pool, sizeof(Key_Set_Iterator));
    if (iter == NULL) {
        return NULL;
    }
    memset(iter, 0, sizeof(Key_Set_iterator));

    iter->pool = self->pool;
    ELTN_Pool_acquire(&(self->pool));

    iter->index = -1;
    iter->length = self->nitems;
    iter->keys = ELTN_alloc(self->pool, sizeof(Key) * self->nitems);
    if (iter->keys == NULL) {
        ELTN_free(self->pool, iter);
        return NULL;
    }
    size_t j = 0;

    for (size_t i = 0; i < self->arraysize && j < iter->length; i++) {
        Key* p = &(self->array[i]);

        if (p->type != KEY_SET_EMPTY) {
            if (!(init_key(self, &(iter->keys[j]), p->type, p->str, p->len))) {
                Key_Set_Iterator_free(iter);
                return NULL;
            }
            j++;
        }
    }
    return iter;
}

void Key_Set_Iterator_free(Key_Set_Iterator* self) {
    ELTN_Pool* h = self->pool;

    for (size_t i = 0; i < self->length; i++) {
        ELTN_free(h, self->keys[i].str);
    }
    ELTN_free(h, self->keys);
    ELTN_free(h, self);
    ELTN_Pool_release(&h);
}

Key_Type Key_Set_Iterator_next(Key_Set_Iterator* self) {
    self->index++;
    if (self->index >= self->length) {
        return KEY_SET_EMPTY;
    }
    return self->keys[self->index].type;
}

void Key_Set_Iterator_string(Key_Set_Iterator* self, char** strptr,
                             size_t* lenptr) {
    if (self->index <= self->length && strptr != NULL && lenptr != NULL) {
        Key* p = &(self->keys[self->index]);

        ELTN_new_string(strptr, lenptr, p->str, p->len);
    }
}
