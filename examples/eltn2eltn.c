/*
 * Copyright 2025 Frank Mitchell
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

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "tomlc17.h"
#include "eltn.h"

/*
 * Main function.
 */
int main(int argc, char** argv) {
    int result = 0;
    ELTN_Parser* parse = NULL;
    ELTN_Emitter* emit = NULL;
    char* infilename = NULL;
    char* outfilename = NULL;
    FILE* ifp = NULL;
    FILE* ofp = NULL;

    /*
     * Parse args
     */

    /* TODO */

    /*
     * Read in ELTN
     */
    ifp = fopen(infilename, "r");
    if (ifp == NULL) {
        result = errno;
        printf("%s: fopen(\"%s\", \"r\") failed: errno = %d; exiting.\n",
                argv[0], infilename, result);
        goto finally;
    }

    parse = ELTN_Parser_new();
    if (parse == NULL) {
        result = errno;
        printf("%s: ELTN_Parser_new() failed: errno = %d; exiting.\n",
                argv[0], result);
        goto finally;
    }

    ssize_t readresult = ELTN_Parser_read_file(parse, ifp);

    if (readresult < 0) {
        result = errno;
        printf("%s: ELTN_Parser_read_file() failed: errno = %d; exiting.\n",
                argv[0], result);
        goto finally;
    }

    /*
     * Create ELTN equivalent
     */
    emit = ELTN_Emitter_new();
    if (emit == NULL) {
        result = errno;
        printf("%s: ELTN_Emitter_new() failed: errno = %d; exiting.\n",
                argv[0], result);
        goto finally;
    }

    /* TODO: echo ELTN from parser to emitter */

    /*
     * Emit ELTN
     */
    ofp = fopen(outfilename, "w");
    if (ofp == NULL) {
        result = errno;
        printf("%s: fopen(\"%s\", \"w\") failed: errno = %d; exiting.\n",
                argv[0], outfilename, result);
        goto finally;
    }

    ssize_t writeresult = ELTN_Emitter_write_file(emit, ofp);

    if (writeresult < 0) {
        result = errno;
        printf("%s: ELTN_Emitter_write_file() failed: errno = %d; exiting.\n",
                argv[0], result);
        goto finally;
    }

finally:
    /*
     * Clean up
     */
    fclose(ifp);
    fclose(ofp);
    ELTN_Emitter_free(parse);
    ELTN_Emitter_free(emit);

    return result;
}
