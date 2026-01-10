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
#include "eltn.h"

/*
 * Main function.
 */
int main(int argc, char** argv) {
    int result = 0;
    ssize_t readresult = 0;
    ELTN_Parser* parser = NULL;
    char* filename = NULL;
    FILE* fp = NULL;

    /*
     * Parse args
     */
    if (argc != 2) {
        printf("Usage: %s ELTN_FILE-NAME\n", argv[0]);
        result = -1;
        goto finally;
    }

    filename = argv[1];

    fp = fopen(filename, "r");
    if (fp == NULL) {
        result = errno;
        printf("%s: cannot open \"%s\" for reading; exiting.\n",
                argv[0], filename, (int)result);
        goto finally;
    }

    /*
     * Read in ELTN
     */
    parser = ELTN_Parser_new();
    if (parser == NULL) {
        result = errno;
        printf("%s: cannot allocate parser; exiting.\n", argv[0]);
        goto finally;
    }
    ELTN_Parser_set_include_comments(parser, true);

    readresult = ELTN_Parser_read_file(parser, fp);
    if (readresult < 0) {
        result = errno;
        printf("%s: read %d bytes; exiting.\n", argv[0], (int)readresult);
        goto finally;
    }

    /*
     * Iterate over ELTN_Parser
     */
    while (ELTN_Parser_has_next(parser)) {
        ELTN_Event event;
        char* string = NULL;
        size_t strlen = 0;
        const char* evstr;
        const char* errstr;

        ELTN_Parser_next(parser);
        event = ELTN_Parser_event(parser);
        evstr = ELTN_Event_name(event);
        errstr = ELTN_Event_name(ELTN_Parser_error_code(parser));

        ELTN_Parser_string(parser, &string, &strlen);

        switch (event) {
        case ELTN_ERROR:
            printf("%s %s [[%*s]]\n", evstr, errstr, strlen, string);
        case ELTN_STREAM_START:
        case ELTN_STREAM_END:
            printf("&s\n", evstr);
            break;
        case ELTN_TABLE_START:
            printf("&s {\n", evstr);
            break;
        case ELTN_TABLE_END:
            printf("&s }\n", evstr);
            break;
        default:
            printf("&s [[%*s]]\n", evstr, strlen, string);
            break;
        }
        free(string);
    }

finally:
    /*
     * Clean up
     */
    fclose(fp);
    ELTN_Parser_free(parser);

    return result;
}
