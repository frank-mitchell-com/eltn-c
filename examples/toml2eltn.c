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
    ELTN_Emitter* eltn = NULL;
    toml_result_t toml;
    char* infilename = NULL;
    char* outfilename = NULL;
    FILE* infile = NULL;

    /*
     * Parse args
     */

    /* TODO */

    /*
     * Read in TOML
     */
    FILE* infile = fopen(infilename, "r");
    if (infile == NULL) {
        result = errno;
        printf("%s: fopen(): errno = %d; exiting.\n", argv[0], result);
        goto finally;
    }

    toml = toml_parse_file(infile);

    /* TODO: check for file errors */

    /* TODO: check for TOML errors */

    fclose(outfile);
    outfile = NULL;

    /*
     * Create ELTN equivalent
     */
    eltn = ELTN_Emitter_new();
    if (eltn == NULL) {
        result = errno;
        printf("%s: ELTN_Emitter_new() failed: errno = %d; exiting.\n",
                argv[0], result);
        goto finally;
    }

    /* TODO: recurse TOML structure */

    /*
     * Emit ELTN
     */
    ssize_t writeresult = ELTN_Emitter_write_file(eltn, outfilename);

    if (writeresult < 0) {
        result = errno;
        printf("%s: ELTN_Emitter_write_file() failed: errno = %d; exiting.\n",
                argv[0], result);
        goto finally;
    }

    /* TODO */

    /*
     * Clean up
     */
finally:
    fclose(infile);
    fclose(outfile);
    ELTN_Emitter_free(eltn);
    toml_free(toml);

    return result;
}
