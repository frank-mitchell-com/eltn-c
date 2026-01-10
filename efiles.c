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

#ifndef ELTN_NO_STDIO

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define  ELTN_CORE	1
#include "eltn.h"

#define BUFSIZE   256


static int File_Reader(void* state, char** strptr, size_t* sizeptr) {
    FILE* fp;
    unsigned char* rbuf;
    size_t nread;

    if (state == NULL || strptr == NULL || sizeptr == NULL) {
        return -1;
    }

    fp = (FILE *) state;

    (*strptr) = NULL;
    (*sizeptr) = 0;

    if (feof(fp)) {
        return 0;
    }

    rbuf = (unsigned char *)malloc(BUFSIZE * sizeof(char));

    if (rbuf == NULL) {
        return errno;
    }

    if (ferror(fp)) {
        clearerr(fp);
    }

    nread = fread(rbuf, sizeof(char), BUFSIZE, fp);

    if (nread == 0) {
        int error = errno;

        free(rbuf);
        return error;
    }
    (*strptr) = (char *)rbuf;
    (*sizeptr) = (size_t)nread;
    return 0;
}

ELTN_API ssize_t ELTN_Parser_read_file(ELTN_Parser* self, FILE* fp) {
    if (fp == NULL) {
        return -1;
    }

    return ELTN_Parser_read(self, File_Reader, fp);
}

static ssize_t File_Writer(void* state, const char* text, size_t size,
                           int* errptr) {
    /*
       TODO: implement 
     */
    return -1;
}

ELTN_API ssize_t ELTN_Emitter_write_file(ELTN_Emitter* self, FILE* fp) {
    if (fp == NULL) {
        return -1;
    }

    return ELTN_Emitter_write(self, File_Writer, fp);
}

#endif
