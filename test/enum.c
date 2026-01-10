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

#include "minctest.h"
#include "eltn.h"

void event_name() {
    lsequal("ELTN_ERROR", ELTN_Event_name(ELTN_ERROR));
    lsequal("ELTN_KEY_INTEGER", ELTN_Event_name(ELTN_KEY_INTEGER));
    lsequal("ELTN_STREAM_END", ELTN_Event_name(ELTN_STREAM_END));
}

void error_name() {
    lsequal("ELTN_ERR_UNKNOWN", ELTN_Error_name(ELTN_ERR_UNKNOWN));
    lsequal("ELTN_OK", ELTN_Error_name(ELTN_OK));
    lsequal("ELTN_ERR_DUPLICATE_KEY", ELTN_Error_name(ELTN_ERR_DUPLICATE_KEY));
}

int main(int argc, char* argv[]) {
    lrun("test_event_name", event_name);
    lrun("test_error_name", error_name);
    lresults();
    return lfails != 0;
}
