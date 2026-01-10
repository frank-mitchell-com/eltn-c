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
#include "ekeyset.h"

void happy_path() {
    Key_Set* ks = Key_Set_new_with_pool(NULL);

    lok(Key_Set_add_key(ks, KEY_SET_STRING, "foo", 3));
    lok(Key_Set_add_key(ks, KEY_SET_NUMBER, "123", 3));
    lok(Key_Set_add_key(ks, KEY_SET_STRING, "bar", 4));
    lequal(3, (int)Key_Set_size(ks));

    lok(Key_Set_has_key(ks, KEY_SET_NUMBER, "123", 3));
    lok(!Key_Set_has_key(ks, KEY_SET_STRING, "quux", 4));

    Key_Set_free(ks);
}

void iterator() {
    Key_Set* ks = Key_Set_new_with_pool(NULL);

    lok(Key_Set_add_key(ks, KEY_SET_STRING, "foo", 3));
    lok(Key_Set_add_key(ks, KEY_SET_NUMBER, "123", 3));
    lok(Key_Set_add_key(ks, KEY_SET_NUMBER, "45.6", 4));
    lequal(3, (int)Key_Set_size(ks));

    Key_Set_Iterator* ksi = Key_Set_iterator(ks);
    int ksi_count = 0;
    Key_Type kt = KEY_SET_EMPTY;

    while ((kt = Key_Set_Iterator_next(ksi)) != KEY_SET_EMPTY) {
        char* str = NULL;
        size_t len = 0;

        Key_Set_Iterator_string(ksi, &str, &len);
        switch (kt) {
        case KEY_SET_STRING:
            lsequal("foo", str);
            break;
        case KEY_SET_NUMBER:
            if (strcmp("123", str) == 0) {
                lsequal("123", str);
            } else {
                lsequal("45.6", str);
            }
            break;
        default:
            lok(false);
            break;
        }
        ksi_count++;
    }
    lequal(3, ksi_count);

    Key_Set_Iterator_free(ksi);

    Key_Set_free(ks);
}

void resize() {
    Key_Set* ks = Key_Set_new_with_pool(NULL);

    int oldcap = (int)Key_Set_capacity(ks);

    lok(oldcap > 0);

    lok(Key_Set_add_key(ks, KEY_SET_STRING, "foo", 3));
    lok(Key_Set_add_key(ks, KEY_SET_STRING, "bar", 3));
    lok(Key_Set_add_key(ks, KEY_SET_STRING, "baz", 3));
    lok(Key_Set_add_key(ks, KEY_SET_STRING, "quux", 4));
    lok(Key_Set_add_key(ks, KEY_SET_STRING, "one fish", 8));
    lok(Key_Set_add_key(ks, KEY_SET_STRING, "two fish", 8));
    lok(Key_Set_add_key(ks, KEY_SET_STRING, "red fish", 3));
    lok(Key_Set_add_key(ks, KEY_SET_STRING, "blue fish", 9));

    lequal(8, (int)Key_Set_size(ks));

    lok(Key_Set_capacity(ks) > oldcap);

    lok(Key_Set_has_key(ks, KEY_SET_STRING, "foo", 3));
    lok(Key_Set_has_key(ks, KEY_SET_STRING, "bar", 3));
    lok(Key_Set_has_key(ks, KEY_SET_STRING, "baz", 3));
    lok(Key_Set_has_key(ks, KEY_SET_STRING, "quux", 4));
    lok(Key_Set_has_key(ks, KEY_SET_STRING, "one fish", 8));
    lok(Key_Set_has_key(ks, KEY_SET_STRING, "two fish", 8));
    lok(Key_Set_has_key(ks, KEY_SET_STRING, "red fish", 3));
    lok(Key_Set_has_key(ks, KEY_SET_STRING, "blue fish", 9));

    Key_Set_free(ks);
}

int main(int argc, char* argv[]) {
    lrun("test_keyset_happy_path", happy_path);
    lrun("test_keyset_iterator", iterator);
    lrun("test_keyset_resize", resize);
    lresults();
    return lfails != 0;
}
