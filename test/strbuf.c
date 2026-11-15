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

#include "minctest.h"
#include "estrbuf.h"

void string_buffer_core() {
    char* actual_str;
    size_t actual_len;
    String_Buffer* strbuf = String_Buffer_new_with_pool(NULL);

    lok(strbuf != NULL);
    if (strbuf == NULL) {
        return;
    }
        
    String_Buffer_append_string(strbuf, "This ", 5);
    String_Buffer_append_string(strbuf, "is ", 3);
    String_Buffer_append_char(strbuf, 'a');
    String_Buffer_append_string(strbuf, " test", 5);

    String_Buffer_to_string(strbuf, &actual_str, &actual_len);
    lequal(14, (int)actual_len);
    lsequal("This is a test", actual_str);

    String_Buffer_free(strbuf);
}

int main() {
    lrun("test_string_buffer_core", string_buffer_core);
    lresults();
    return lfails != 0;
}
