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

#define ELTN_CORE 1
#include "eltn.h"
#include "ebuffer.h"
#include "convert.h"
#include "ealloc.h"

#define INIT_BUF_SIZE 1024

/**************************** ELTN_Buffer ***********************************/

struct ELTN_Buffer {
    intptr_t _reserved;
    ELTN_Pool* pool;
    ELTN_Reader reader;
    void* reader_state;

    char8_t* buffer;
    int bufsize;
    char8_t* head;
    char8_t* tail;
    bool eof;
};

void ELTN_Buffer_free(ELTN_Buffer* self) {
    ELTN_Pool* h = self->pool;

    if (self->buffer != NULL) {
        ELTN_free(h, self->buffer);
    }
    ELTN_free(h, self);
    ELTN_Pool_release(&h);
}

ELTN_Buffer* ELTN_Buffer_new_with_pool(ELTN_Pool* pool) {
    ELTN_Buffer* result = (ELTN_Buffer *) ELTN_alloc(pool, sizeof(ELTN_Buffer));

    if (result == NULL) {
        return NULL;
    }
    memset(result, 0, sizeof(ELTN_Buffer));
    result->pool = pool;
    ELTN_Pool_acquire(&(result->pool));

    result->bufsize = INIT_BUF_SIZE;
    result->buffer = (char8_t *) ELTN_alloc(pool, result->bufsize);
    if (result->buffer == NULL) {
        ELTN_Buffer_free(result);
        return NULL;
    }
    memset(result->buffer, 0, INIT_BUF_SIZE);
    result->head = result->buffer;
    result->tail = result->buffer;
    result->eof = false;

    return result;
}

size_t ELTN_Buffer_length(ELTN_Buffer* self) {
    if (self->head <= self->tail) {
        return self->tail - self->head;
    } else {
        return self->bufsize - (self->head - self->tail);
    }
}

ELTN_API size_t ELTN_Buffer_capacity(ELTN_Buffer* self) {
    return self->bufsize;
}

ELTN_API bool ELTN_Buffer_set_capacity(ELTN_Buffer* self, size_t newcap) {
    const size_t length = ELTN_Buffer_length(self);

    if (newcap <= length) {
        return false;
    }

    if (length == 0) {
        char8_t* newbuf =
            (char8_t *) ELTN_realloc(self->pool, self->buffer, newcap);

        if (newbuf == NULL) {
            return false;
        }
        self->buffer = newbuf;
        self->head = newbuf;
        self->tail = newbuf;
        self->bufsize = newcap;
    } else {
        char8_t* newbuf = (char8_t *) ELTN_alloc(self->pool, newcap);

        if (newbuf == NULL) {
            return false;
        }
        if (self->head < self->tail) {
            memcpy(newbuf, self->head, length);
        } else {
            size_t taillen = self->tail - self->buffer;
            size_t headlen = length - taillen;

            memcpy(newbuf, self->head, headlen);
            memcpy(newbuf + headlen, self->buffer, taillen);
        }
        ELTN_free(self->pool, self->buffer);
        self->buffer = newbuf;
        self->head = newbuf;
        self->tail = newbuf + length;
        self->bufsize = newcap;
    }
    return true;
}

ELTN_API bool ELTN_Buffer_is_empty(ELTN_Buffer* self) {
    return self->head == self->tail;
}

ELTN_API bool ELTN_Buffer_is_closed(ELTN_Buffer* self) {
    return self->eof;
}

static ssize_t read_into_buffer(ELTN_Buffer* self, bool first) {
    ssize_t writeresult = 0;
    size_t readsize = 0;
    char* text = NULL;
    int errcode = self->reader(self->reader_state, &text, &readsize);

    if (text == NULL) {
        self->eof = true;
        return -1;
    }

    if (first) {
        /*
         * TODO: check for Byte Order Mark and non-byte data
         */
    }

    writeresult = ELTN_Buffer_write(self, text, readsize);
    free(text);

    if (errcode != 0) {
        self->eof = true;
        return -1;
    }

    if (writeresult < 0) {
        self->eof = true;
        return writeresult;
    }

    return readsize;
}

ssize_t ELTN_Buffer_read(ELTN_Buffer* self, ELTN_Reader reader,
                         void* reader_state) {
    if (!reader) {
        return -1;
    }

    self->reader = reader;
    self->reader_state = reader_state;

    return read_into_buffer(self, true);
}

ELTN_API ssize_t ELTN_Buffer_write(ELTN_Buffer* self, const char* text,
                                   size_t len) {
    const size_t currlen = ELTN_Buffer_length(self);

    if (self->eof) {
        return -1;
    }
    if (currlen + len >= ELTN_Buffer_capacity(self)) {
        bool status = ELTN_Buffer_set_capacity(self, (currlen + len) * 2);

        if (!status) {
            return -1;
        }
    }
    if (self->tail + len < self->buffer + self->bufsize) {
        memmove(self->tail, text, len);
        self->tail += len;
    } else {
        size_t headlen = self->buffer + self->bufsize - self->tail;
        size_t taillen = len - headlen;

        memmove(self->tail, text, headlen);
        memmove(self->buffer, text + headlen, taillen);
        self->tail = self->buffer + taillen;
    }
    return len;
}

ELTN_API void ELTN_Buffer_close(ELTN_Buffer* self) {
    self->eof = true;
}

static int32_t next_byte(ELTN_Buffer* self, bool consume) {
    if (self->head == self->tail) {
        return -1;
    }
    char8_t c = *(self->head);

    if (consume) {
        self->head++;
        if (self->head >= self->buffer + self->bufsize) {
            self->head = self->buffer;
        }
    }
    return c;
}

static bool ensure_more_bytes(ELTN_Buffer* self) {
    if (self->eof) {
        return false;
    }

    if (self->reader == NULL) {
        return false;
    }

    while (ELTN_Buffer_length(self) == 0) {
        if (read_into_buffer(self, false) < 0) {
            return false;
        }
    }
    return true;
}

int32_t ELTN_Buffer_next_char(void* state, bool consume) {
    ELTN_Buffer* self = (ELTN_Buffer *) state;

    if (self->head == self->tail) {
        if (!ensure_more_bytes(self)) {
            return -1;
        }
    }

    return next_byte(self, consume);
}
