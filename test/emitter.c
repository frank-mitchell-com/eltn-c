/*
 * Copyright 2026 Frank Mitchell
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

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "minctest.h"
#include "eltn.h"

void debug_string(const char* string, const size_t len) {
    printf("DEBUG: <<");

    for (size_t i = 0; i < len; i++) {
        char c = string[i];
        if (c == ' ' || isprint(c)) {
            printf("%c", c);
        } else {
            printf("\\x%02x", (int)c);
        }
    }
    printf(">>\n");
}

void emitter_smoke() {
    ELTN_Emitter* emitter = ELTN_Emitter_new();
    char* result = NULL;
    size_t reslen = 0;

    lequal(0, (int)ELTN_Emitter_length(emitter));
    lequal(0, (int)ELTN_Emitter_current_depth(emitter));
    lequal(ELTN_OK, ELTN_Emitter_error_code(emitter));

    ELTN_Emitter_write_string(emitter, &result, &reslen);

    lok(result != NULL);
    if (result != NULL) {
        lsequal("", result);
        lequal(0, (int)reslen);
        free(result);
    }

    ELTN_Emitter_free(emitter);
}

void emitter_def_table() {
    ELTN_Emitter* emitter = ELTN_Emitter_new();
    char* result = NULL;
    size_t reslen = 0;

    const char* expected = "test = 1;\nanother = 2;\nthird = 3;\n";
    const int explen = strlen(expected);

    // ELTN_Emitter_set_pretty_print(emitter, false);

    lok(ELTN_Emitter_def_name(emitter, "test"));
    lok(ELTN_Emitter_value_number(emitter, 1.0));
    lok(ELTN_Emitter_def_name(emitter, "another"));
    lok(ELTN_Emitter_value_number(emitter, 2.0));
    lok(ELTN_Emitter_def_name(emitter, "third"));
    lok(ELTN_Emitter_value_number(emitter, 3.0));

    lequal(ELTN_OK, ELTN_Emitter_error_code(emitter));

    ELTN_Emitter_write_string(emitter, &result, &reslen);

    lok(result != NULL);
    if (result != NULL) {
        lsequal(expected, result);
        lequal(explen, (int)reslen);
        debug_string(result, reslen);
        free(result);
    }

    ELTN_Emitter_free(emitter);
}

int main(void) {
    lrun("test_emitter_smoke", emitter_smoke);
    lrun("test_emitter_def_table", emitter_def_table);
    lresults();
    return lfails != 0;
}
