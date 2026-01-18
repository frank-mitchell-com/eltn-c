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

#ifndef __ELTN_PARSER
#define __ELTN_PARSER

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#ifndef ELTN_NO_STDIO
#include <stdio.h>
#endif
#include <sys/types.h>

/**
 * @file
 * @brief The main header file for the ELTN parser and emitter.
 */

/**
 * A marker for functions that form the ELTN-C API.
 */

#ifdef ELTN_BUILD_AS_DLL

#ifdef ELTN_CORE
#define ELTN_API __declspec(dllexport)
#else
#define ELTN_API __declspec(dllimport)
#endif

#else /* not ELTN_BUILD_AS_DLL */

#define ELTN_API extern

#endif /* ELTN_BUILD_AS_DLL */

/**
 * Parses an ELTN document as a series of {@link ELTN_Event} events,
 * from which the caller can reconstruct an ELTN document.  Its
 * "pull parser" design leaves the caller in control, and able to
 * pass the parser to various cub-sections and components.
 *
 * The parser is not designed for multi-threaded operation.
 * Multiple, simultaneous threads calling the parser can produce
 * inconsistent results or errors.
 */
typedef struct ELTN_Parser ELTN_Parser;

/**
 * A helper object to the {@link ELTN_Parser} which allows fine-grained
 * control of text fed into the parser.  Functions on it allow control
 * of the parser's buffer and feeding text directly into the buffer.
 *
 * The source is currently not designed for multi-threaded operation.
 * A future version *may* add support for one or more threads feeding
 * the source with ELTN_Source_write() calls while one thread runs the
 * parser.
 */
typedef struct ELTN_Source ELTN_Source;

/**
 * Stores events sent to it, and assembles an ELTN document based on those
 * events.  Each event is a function on ELTN_Emitter, and each returns a
 * boolean to indicate whether it is valid in that context or not.
 */
typedef struct ELTN_Emitter ELTN_Emitter;

/**
 * Encapsulates an {@link ELTN_Alloc} and its required state.
 * Callers can install instrumentation or a different memory manager to
 * control allocation of heap memory. Casual users should not need this.
 */
typedef struct ELTN_Pool ELTN_Pool;

/**
 * An allocation function, if normal `malloc()`/`free()` are insufficient.
 * If @p ptr is `NULL`, it should act like `malloc()` to allocate memory.
 * If @p ptr is not `NULL` and @p size is zero it should act like `free()`.
 * If @p ptr is not `NULL` and @p size is not zero, it should act like
 * `realloc()` to expand the memory passed into `ptr`.
 *
 * @param state a reference that assists in allocation, such as an allocator
 *              object
 * @param ptr pointer to memory to be reallocated or freed
 * @param size the (new) size of memory to be (re)allocated.
 *
 * @return pointer to the new memory area, or NULL if it freed memory.
 */
typedef void* (*ELTN_Alloc)(void* state, void* ptr, size_t size);

/**
 * A callback the {@link ELTN_Parser} uses to read an ELTN document.
 *
 * @param state (in) a reference used to read data, such as a file pointer
 * @param strptr (out) a pointer to the string read, as memory to be `free)_`d,
 *               or NULL if at end of the document.
 * @param sizeptr (out) a pointer to the size of the string read,
 *                or to 0 at the end of the document.
 *
 * @return 0 under normal circumstances;
 *         the value of `errno` at the time of a system error,
 *         or a negative value in case of an application error.
 */
typedef int (*ELTN_Reader)(void* state, char** strptr, size_t* sizeptr);

/**
 * A callback the {@link ELTN_Emitter} uses to write an ELTN document.
 * During normal operation, the function should return
 *
 * @param state a reference used to write data, such a file pointer.
 * @param text the next block of text to write
 * @param size the number of bytes in `text`
 * @param errptr on a system error, the function captures the value of
 *               `errno` at the time of error and returns it through
 *               this pointer.  If an error occurs but not a system
 *               error, it should contain a negative value.
 *
 * @return the number of bytes written, or a negative number if an error.
 */
typedef ssize_t(*ELTN_Writer) (void* state, const char* text, size_t size,
                               int* errptr);

/**
 * Events returned by {@link ELTN_Parser_event} as the caller advances
 * through an ELTN document.
 */
typedef enum ELTN_Event {
    ELTN_ERROR = -1,
    ELTN_STREAM_START = 0,
    ELTN_COMMENT,
    ELTN_DEF_NAME,
    ELTN_KEY_STRING,
    ELTN_KEY_NUMBER,
    ELTN_KEY_INTEGER,
    ELTN_VALUE_STRING,
    ELTN_VALUE_NUMBER,
    ELTN_VALUE_INTEGER,
    ELTN_VALUE_TRUE,
    ELTN_VALUE_FALSE,
    ELTN_VALUE_NIL,
    ELTN_TABLE_START,
    ELTN_TABLE_END,
    ELTN_STREAM_END
} ELTN_Event;

/**
 * Provides the name of every ELTN_Event instance.
 *
 * @param e the ELTN_Event code
 *
 * @returns the name of the event as a null-terminated string.
 */
ELTN_API const char* ELTN_Event_name(ELTN_Event e);

/**
 * Provides the symbolic name of every ELTN_Event instance.
 *
 * @param e the ELTN_Event code
 * @param strptr a pointer to a string variable that receives a newly allocated
 *               string; the caller must call `free()` on it when finished.
 * @param sizeptr a pointer to receive the length of @p strptr.
 */
ELTN_API void ELTN_Event_string(ELTN_Event e, char** strptr, size_t* sizeptr);


/**
 * Error codes returned by {@link ELTN_Parser_error_code} and
 * {@link ELTN_Emitter_error_code}
 * as the caller advances through an ELTN document.
 */
typedef enum ELTN_Error {
    ELTN_ERR_UNKNOWN = -1,
    ELTN_OK = 0,
    ELTN_ERR_OUT_OF_MEMORY,
    ELTN_ERR_STREAM_END,
    ELTN_ERR_UNEXPECTED_TOKEN,
    ELTN_ERR_INVALID_TOKEN,
    ELTN_ERR_DUPLICATE_KEY
} ELTN_Error;

/**
 * Provides the symbolic name of every ELTN_Error instance.
 *
 * @param e the ELTN_Error code
 *
 * @returns the name of the error as a null-terminated string.
 */
ELTN_API const char* ELTN_Error_name(ELTN_Error e);

/**
 * Provides the symbolic name of every ELTN_Error instance.
 *
 * @param e the ELTN_Error code
 * @param strptr a pointer to a string variable that receives a newly allocated
 *               string; the caller must call `free()` on it when finished.
 * @param sizeptr a pointer to receive the length of @p strptr.
 */
ELTN_API void ELTN_Error_string(ELTN_Error e, char** strptr, size_t* sizeptr);

/**
 * Create a new parser instance.
 *
 * @return the parser.
 */
ELTN_API ELTN_Parser* ELTN_Parser_new();

/**
 * Create a new parser instance with a custom allocator.
 *
 * @param pool a memory pool from which to allocate.
 *
 * @return the parser
 */
ELTN_API ELTN_Parser* ELTN_Parser_new_with_pool(ELTN_Pool * pool);

/**
 * Indicates whether the parser will issue `ELTN_COMMENT` events.
 * The default is `false`.
 *
 * @param p the parser
 *
 * @return 'true' if parser issues comment events, else false.
 */
ELTN_API bool ELTN_Parser_include_comments(ELTN_Parser * p);

/**
 * Sets whether the parser will issue `ELTN_COMMENT` events.
 *
 * @param p the parser
 * @param b new value of ELTN_Parser_include_comments().
 */
ELTN_API void ELTN_Parser_set_include_comments(ELTN_Parser * p, bool b);

/**
 * The instance that handles all the parser's text input.
 * The parser completely manages its source.
 *
 * @param p the parser
 *
 * @result the ELTN_Source instance
 */
ELTN_API ELTN_Source* ELTN_Parser_source(ELTN_Parser * p);

/**
 * Set a reader function and read the first chunk of the text to be parsed.
 *
 * @param p the parser
 * @param reader a function to read more bytes from a source
 * @param state  a state variable passed to `reader`, possibly its source.
 *
 * @return the initial number of bytes read, or -1 if error.
 */
ELTN_API ssize_t ELTN_Parser_read(ELTN_Parser * p, ELTN_Reader reader,
                                  void* state);

/**
 * Set a reader function and read the first chunk of the text to be parsed.
 * This function assumes the string contains an entire ELTN document.
 *
 * @param p the parser
 * @param str the string to be parsed.
 * @param len the length of the string.
 *
 * @return the total number of bytes in `str`, or -1 if error.
 */
ELTN_API ssize_t ELTN_Parser_read_string(ELTN_Parser * p, const char* str,
                                         size_t len);

#ifndef ELTN_NO_STDIO

/**
 * Set a reader function and read the first chunk of the text to be parsed.
 * This function assumes the string contains an entire ELTN document.
 *
 * @param p the parser
 * @param fp a pointer to the ELTN file to be read.
 *
 * @return the initial number of bytes read, or -1 if error.
 */
ELTN_API ssize_t ELTN_Parser_read_file(ELTN_Parser * p, FILE * fp);

#endif

/**
 * Whether the parser has other events to process.
 *
 * @param p the parser.
 *
 * @return true if more events.
 */
ELTN_API bool ELTN_Parser_has_next(ELTN_Parser * p);

/**
 * Advance to the next parsing event.
 *
 * @param p the parser.
 */
ELTN_API void ELTN_Parser_next(ELTN_Parser * p);

/**
 * The event encountered after calling `next()`.
 *
 * @param p the parser.
 *
 * @return the current event
 */
ELTN_API ELTN_Event ELTN_Parser_event(ELTN_Parser * p);

/**
 * The depth of nested tables in the document after the current event.
 * It increases by one with every `ELTN_TABLE_START` and decreases by one
 * with every `ELTN_TABLE_END`.  Depth 0 is the definition table level.
 *
 * @return the depth of nested tables.
 */
ELTN_API unsigned int ELTN_Parser_depth(ELTN_Parser * p);

/**
 * The name and type of the current key being processed.
 * Upon `ELTN_TABLE_START`
 * this is the string value of the key about to be set by the new Table.
 * Upon `ELTN_TABLE_END`
 * this is the string value of the key just set by the completed Table.
 * In all other cases, this value is the most recent key or definition
 * name encountered.
 */
ELTN_API void ELTN_Parser_current_key(ELTN_Parser * p, ELTN_Event * typeptr,
                                      char** strptr, size_t lenptr);


/**
 * Copies the text of the current event.
 * In all cases the text comes directly from the parsed document, save
 * for normalizing newlines to the single character "\n".
 * Upon `ELTN_ERROR` this is the text that provoked the error.
 * The copy may be freed by `free()`.
 *
 * @param p the parser.
 * @param strptr a pointer to receive a copy of the text.
 * @param lenptr a pointer to receive the length of the text.
 */
ELTN_API void ELTN_Parser_text(ELTN_Parser * p, char** strptr, size_t* lenptr);

/**
 * Copies the string value of the current event.
 * Upon `ELTN_DEF_NAME`, `ELTN_KEY_STRING` and `ELTN_VALUE_STRING`
 * this is the value of the key or value with quote marks removed and escaped
 * sequences resolved.
 * Upon `ELTN_STREAM_START`, `ELTN_STREAM_END`, `ELTN_TABLE_START` and
 * `ELTN_TABLE_END` this value may be blank or uninformative.
 * In all other cases it is the string value of the number, integer, boolean,
 * or nil of the current definition, key or value.
 * The copy may be freed by `free()`.
 *
 * @param p the parser.
 * @param strptr a pointer to receive a copy of the string.
 * @param lenptr a pointer to receive the length of the string.
 */
ELTN_API void ELTN_Parser_string(ELTN_Parser * p, char** strptr,
                                 size_t* lenptr);

/**
 * Returns the numeric value associated with the current event.
 * Results are undefined outside `ELTN_KEY_NUMBER`, `ELTN_KEY_INTEGER`,
 * `ELTN_VALUE_NUMBER` and `ELTN_VALUE_INTEGER`.
 *
 * @param p the parser
 * @return the numeric value
 */
ELTN_API double ELTN_Parser_number(ELTN_Parser * p);

/**
 * Returns the integer value associated with the current event.
 * Results are undefined outside `ELTN_KEY_INTEGER` and `ELTN_VALUE_INTEGER`.
 *
 * @param p the parser
 * @return the integer value
 */
ELTN_API long int ELTN_Parser_integer(ELTN_Parser * p);

/**
 * Returns the boolean value associated with the current event.
 * Outside `ELTN_VALUE_TRUE` or `ELTN_VALUE_FALSE`
 * any value not equal to `false` or `nil` should return `true`.
 *
 * @param p the parser
 * @return the integer value
 */
ELTN_API bool ELTN_Parser_boolean(ELTN_Parser * p);

/**
 * Provides the error code associated with an `ELTN_ERROR`.
 *
 * @param p the parser
 * @return the error code.
 */
ELTN_API ELTN_Error ELTN_Parser_error_code(ELTN_Parser * p);

/**
 * Provides the line on which an error occurred.
 * Undefined outside of `ELTN_ERROR`;
 *
 * @param p the parser
 * @return the line number.
 */
ELTN_API unsigned int ELTN_Parser_error_line(ELTN_Parser * p);

/**
 * Provides the approximate column in which an error occurred.
 * Undefined outside of `ELTN_ERROR`;
 *
 * @param p the parser
 * @return the line number.
 */
ELTN_API unsigned int ELTN_Parser_error_column(ELTN_Parser * p);

/**
 * Frees the parser's memory and all its state.
 * Call this after the parser has parsed a document to clean up.
 * Does not close any external open file objects or release memory provided
 * as strings.
 *
 * @param p the parser
 */
ELTN_API void ELTN_Parser_free(ELTN_Parser * p);

/* ------------------------ Source -------------------------------*/

/**
 * The upper bound of bytes this object can store at once.
 * The source will resize itself before it reaches this upper bound;
 * it needs at least one byte to act as a null terminator.
 *
 * @return number of bytes.
 */
ELTN_API size_t ELTN_Source_capacity(ELTN_Source * s);

/**
 * Set the maximum number of bytes this object can store at once.
 * If this number is lower than those needed to store current contents,
 * the buffer size is unchanged.
 *
 * @param s the source
 * @param newcap the new buffer size
 *
 * @return whether the size changed.
 */
ELTN_API bool ELTN_Source_set_capacity(ELTN_Source * s, size_t newcap);

/**
 * Whether the buffer has no more bytes to process.
 *
 * @param s the source
 *
 * @return whether the buffer has no more bytes.
 */
ELTN_API bool ELTN_Source_is_empty(ELTN_Source * s);

/**
 * Whether the buffer is closed for writing.
 *
 * @param s the source
 *
 * @return whether the buffer is closed for writing.
 */
ELTN_API bool ELTN_Source_is_closed(ELTN_Source * s);

/**
 * Write new text to the buffer.
 *
 * @param s the source
 * @param text string of ASCII or ASCII-like of text
 * @param len the number of *bytes* to read from `text`
 *
 * @return number of bytes written, or negative on error.
 */
ELTN_API ssize_t ELTN_Source_write(ELTN_Source * s, const char* text,
                                   size_t len);

/**
 * Close the buffer for writing.
 *
 * @param s the source
 */
ELTN_API void ELTN_Source_close(ELTN_Source * s);

/* ------------------------ Emitter -------------------------------*/

/**
 * Create a new instance of {@link ELTN_Emitter}.
 *
 * @return the new instance.
 */
ELTN_API ELTN_Emitter* ELTN_Emitter_new();

/**
 * Create a new instance of {@link ELTN_Emitter}.
 * This version uses a custom allocation function.
 *
 * @param pool a memory pool from which to allocate.
 *
 * @return the new instance.
 */
ELTN_API ELTN_Emitter* ELTN_Emitter_new_with_pool(ELTN_Pool * pool);

/**
 * Add a `DEF_NAME` event to the ELTN document.
 * This defines a definition name at the top level.
 * The following events should define value or table.
 *
 * @param e the emitter
 * @param n the null-terminated identifier of the definition name.
 *
 * @return `true` if no error, `false` if an error occurred.
 *          See ELTN_Emitter_error_code() for the error.
 */
ELTN_API bool ELTN_Emitter_def_name(ELTN_Emitter * e, const char* n);

/**
 * Add a `KEY_STRING` event to the ELTN document.
 * This defines a string key within a table.
 * The following events should define a value or table.
 *
 * @param e the emitter
 * @param s the string used as a key in the enclosing table,
 *          which may include nulls.
 * @param len the length of @p s in bytes.
 *
 * @return `true` if no error, `false` if an error occurred.
 *          See ELTN_Emitter_error_code() for the error.
 */
ELTN_API bool ELTN_Emitter_key_string(ELTN_Emitter * e, const char* s,
                                      size_t len);

/**
 * Add a `KEY_NUMBER` event to the ELTN document.
 * This defines a number key within a table.
 * The following events should define a value or table.
 *
 * @param e the emitter
 * @param n the number used as a key in the enclosing table
 * @param sigfigs the number of significant figures to include in @p n,
 *                in case of floating point rounding errors.
 *
 * @return `true` if no error, `false` if an error occurred.
 *          See ELTN_Emitter_error_code() for the error.
 */
ELTN_API bool ELTN_Emitter_key_number(ELTN_Emitter * e, double n,
                                      unsigned int sigfigs);

/**
 * Add a `KEY_INTEGER` event to the ELTN document.
 * This defines a integer key within a table.
 * The following events should define a value or table.
 *
 * @param e the emitter
 * @param i the integer used as a key in the enclosing table
 *
 * @return `true` if no error, `false` if an error occurred.
 *          See ELTN_Emitter_error_code() for the error.
 */
ELTN_API bool ELTN_Emitter_key_integer(ELTN_Emitter * e, int i);

/**
 * Add a `VALUE_STRING` event to the ELTN document.
 * This defines a string value within a table.
 * It usually precedes one of the `KEY_`... events; if not, it
 * will be assigned an integer key according to its order in the
 * table.
 * The next events should either define a new key, define a new
 * value or table, or close the existing table.
 *
 * @param e the emitter
 * @param s the string value in the enclosing table,
 *          which may include nulls.
 * @param len the length of @p s in bytes.
 *
 * @return `true` if no error, `false` if an error occurred.
 *          See ELTN_Emitter_error_code() for the error.
 */
ELTN_API bool ELTN_Emitter_value_string(ELTN_Emitter * e, const char* s,
                                        size_t len);

/**
 * Add a `VALUE_NUMBER` event to the ELTN document.
 * This defines a number value within a table.
 * It usually precedes one of the `KEY_`... events; if not, it
 * will be assigned an integer key according to its order in the
 * table.
 * The next events should either define a new key, define a new
 * value or table, or close the existing table.
 *
 * @param e the emitter
 * @param n the number value in the enclosing table
 * @param sigfigs the number of significant figures to include in @p n,
 *                in case of floating point rounding errors.
 *
 * @return `true` if no error, `false` if an error occurred.
 *          See ELTN_Emitter_error_code() for the error.
 */
ELTN_API bool ELTN_Emitter_value_number(ELTN_Emitter * e, double n,
                                        unsigned int sigfigs);

/**
 * Add a `VALUE_NUMBER` event to the ELTN document.
 * This defines an integer value within a table.
 * It usually precedes one of the `KEY_`... events; if not, it
 * will be assigned an integer key according to its order in the
 * table.
 * The next events should either define a new key, define a new
 * value or table, or close the existing table.
 *
 * @param e the emitter
 * @param i the integer value in the enclosing table
 *
 * @return `true` if no error, `false` if an error occurred.
 *          See ELTN_Emitter_error_code() for the error.
 */
ELTN_API bool ELTN_Emitter_value_integer(ELTN_Emitter * e, int i);

/**
 * Add a `VALUE_TRUE` or `VALUE_FALSE` event to the ELTN document.
 * This defines a string value within a table.
 * It usually precedes one of the `KEY_`... events; if not, it
 * will be assigned an integer key according to its order in the
 * table.
 * The next events should either define a new key, define a new
 * value or table, or close the existing table.
 *
 * @param e the emitter
 * @param b the boolean value in the enclosing table
 *
 * @return `true` if no error, `false` if an error occurred.
 *          See ELTN_Emitter_error_code() for the error.
 */
ELTN_API bool ELTN_Emitter_value_boolean(ELTN_Emitter * e, bool b);

/**
 * Add a `VALUE_NIL` event to the ELTN document.
 * This defines a nil value within a table.
 * It usually precedes one of the `KEY_`... events; if not, it
 * will be assigned an integer key according to its order in the
 * table.
 * The next events should either define a new key, define a new
 * value or table, or close the existing table.
 *
 * @param e the emitter
 *
 * @return `true` if no error, `false` if an error occurred.
 *          See ELTN_Emitter_error_code() for the error.
 */
ELTN_API bool ELTN_Emitter_value_nil(ELTN_Emitter * e);

/**
 * Add a `TABLE_START` event to the ELTN document.
 * This opens a new table within a table.
 * It usually precedes one of the `KEY_`... events; if not, it
 * will be assigned an integer key according to its order in the
 * table.
 * If issued at the start of the document, it replaces the
 * definition table with a free-standing table.
 * The next events should either define a new key, define a new
 * value or table, or close the existing table.
 *
 * @param e the emitter
 *
 * @return `true` if no error, `false` if an error occurred.
 *          See ELTN_Emitter_error_code() for the error.
 */
ELTN_API bool ELTN_Emitter_table_start(ELTN_Emitter * e);

/**
 * Add a `TABLE_END` event to the ELTN document.
 * This closes an existing table.
 * It *must* have a matching ELTN_Emitter_table_start(), or it is
 * an error.
 * If issued at the top level of the document, it effectively ends
 * the document.
 * Otherwise, the next events should either define a new key, define a new
 * value or table, or close the existing table.
 *
 * @param e the emitter
 *
 * @return `true` if no error, `false` if an error occurred.
 *          See ELTN_Emitter_error_code() for the error.
 */
ELTN_API bool ELTN_Emitter_table_end(ELTN_Emitter * e);

/**
 * Add a comment to the ELTN document.
 * It cannot be created immediately after a `KEY_`... event, for readability.
 * Its placement is at the discretion of the emitter.
 * If it is longer than 80 characters or contains newlines, the emitter
 * will create a long comment, otherwise it will create a short comment.
 *
 * @param e the emitter
 * @param comment comment the comment text to be written,
 *                which may contain nulls.
 * @param length the number of bytes in @p comment.
 *
 * @return `true` if no error, `false` if an error occurred.
 *          See ELTN_Emitter_error_code() for the error.
 */
ELTN_API bool ELTN_Emitter_comment(ELTN_Emitter * e, const char* comment,
                                   size_t length);

/**
 * Provides the current depth of keys in the document.
 * If the result is zero, the caller is either at the definition table or
 * the top-level table.
 *
 * @param e the emitter.
 *
 * @return the depth of keys.
 */
ELTN_API unsigned int ELTN_Emitter_current_depth(ELTN_Emitter * e);

/**
 * Provides the current path of keys in the document.
 * This will be a list of keys separated by "."; keys that are not identifiers
 * will be surrounded by `[` ... `]` marks.
 * String keys will contain raw characters, including newlines and
 * non-printable characters.  Numbers and integers will be printed as digits.
 * Boolean keys will be either "true" or "false".
 *
 * The resulting string will be allocated from the heap.  The caller must
 * `free()` it when done.
 *
 * @param e the emitter
 * @param strptr a pointer to the string to be created; may not be NULL.
 * @param lenptr a pointer to the length of the string to be created;
 *        may be NULL, but if so all nulls will be translated to "\0"
 *        sequences.
 */
ELTN_API void ELTN_Emitter_current_path(ELTN_Emitter * e, char** strptr,
                                        size_t* lenptr);

/**
 * The error code for the most recent error.
 *
 * @param e the emitter
 *
 * @returns an {@link ELTN_Error} code.
 */
ELTN_API ELTN_Error ELTN_Emitter_error_code(ELTN_Emitter * e);

/**
 * The path for the most recent error.
 * See ELTN_Emitter_current_path() for the format of a path string.
 *
 * The resulting string will be allocated from the heap.  The caller must
 * `free()` it when done.
 *
 * @param e the emitter
 * @param strptr a pointer to the string to be created; may not be NULL.
 * @param lenptr a pointer to the length of the string to be created;
 *        may be NULL, but if so all nulls will be translated to "\0"
 *        sequences.
 */
ELTN_API void ELTN_Emitter_error_path(ELTN_Emitter * e, char** strptr,
                                      size_t* lenptr);

/**
 * The current setting of the "pretty print" flag.
 * If set to `true` (the default), the document will contain newlines and
 * space characters to make it more readable.  Setting it to `false` creates
 * a smaller but less readable document.
 *
 * @param e the emitter
 * 
 * @return whether the "pretty print" flag is set.
 */
ELTN_API bool ELTN_Emitter_pretty_print(ELTN_Emitter * e);

/**
 * Sets the "pretty print" flag.
 * See ELTN_Emitter_pretty_print() for its significance.
 *
 * @param e the emitter
 * @param pretty whether to set the "pretty print" flag.
 */
ELTN_API void ELTN_Emitter_set_pretty_print(ELTN_Emitter * e, bool pretty);

/**
 * Provides the level of indentation.
 * Each table save the top-level one will be indented this number of spaces;
 * each of its elements will be indented one level deeper.
 * The default level is four.
 * This setting has no effect if ELTN_Emitter_pretty_print() is `false`.
 *
 * @param e the emitter
 *
 * @return the number of spaces of indentation.
 */
ELTN_API unsigned int ELTN_Emitter_indent(ELTN_Emitter * e);

/**
 * Sets the level of indentation.
 * See ELTN_Emitter_indent() for its significance and caveats.
 *
 * @param e the emitter.
 * @param indent the new level of indentation.
 */
ELTN_API void ELTN_Emitter_set_indent(ELTN_Emitter * e, unsigned int indent);

/**
 * Provides the length of the current document, if emitted now.
 *
 * @param e the emitter
 *
 * @return the length of the document in bytes.
 */
ELTN_API ssize_t ELTN_Emitter_length(ELTN_Emitter * e);

/**
 * Produces the current document text as a string.
 *
 * The string created will be allocated from the heap.  The caller must use
 * `free()` on it when they are done.
 *
 * @param e the emitter
 * @param strptr a pointer to the variable which will receive the string;
 *               may not be `NULL`.
 * @param lenptr a pointer to the variable which will receive the string's
 *               length; may not be `NULL`.
 */
ELTN_API void ELTN_Emitter_write_string(ELTN_Emitter * e, char** strptr,
                                        size_t* lenptr);

#ifndef ELTN_NO_STDIO

/**
 * Write the current document text to a file.
 *
 * @param e the emitter
 * @param fp a pointer to a file, which must be writable.
 *
 * @return the number of bytes written, or a negative number in case of
 *         a file or I/O error.
 */
ELTN_API ssize_t ELTN_Emitter_write_file(ELTN_Emitter * e, FILE * fp);

#endif

/**
 * Write the string's text using a custom method.
 * The caller must provide a function that writes the document to a file,
 * standard output, to a socket, or to any other output device.
 * If the function requires an external resource, the caller must manage
 * its life cycle themselves.
 *
 * @param e the emitter
 * @param writer a function capable of writing the text of the emitter's
 *               document; may not be `NULL`.
 * @param state an object used by @p writer, such as a file pointer;
 *              may be `NULL` if the function needs no state.
 *
 * @return the number of bytes written, or a negative number in case of error.
 */
ELTN_API ssize_t ELTN_Emitter_write(ELTN_Emitter * e, ELTN_Writer writer,
                                    void* state);

/**
 * Deletes all memory and state of the emitter.
 * Call this after the emitter has written its document for the last time,
 * to clean up.
 * Does not close any external open file objects or release memory provided
 * as strings.
 *
 * @param e the emitter
 */
ELTN_API void ELTN_Emitter_free(ELTN_Emitter * e);

/* ----------------------------- Memory Pool -------------------------------*/

/**
 * Define a new memory pool with a custom allocator.
 *
 * @param hptr pointer to the pointer that receives the new memory pool.
 * @param alloc the new allocator function.
 * @param state pointer to state required by the allocator; may be NULL.
 */
ELTN_API void ELTN_Pool_new_with_alloc(ELTN_Pool ** hptr, ELTN_Alloc alloc,
                                       void* state);

/**
 * Add a new reference to the memory pool.
 * The variable pointed to by @p hptr *may* change, but it will still point
 * to the same memory pool.
 *
 * @param hptr pointer to the pointer to the memory pool.
 */
ELTN_API void ELTN_Pool_acquire(ELTN_Pool ** hptr);

/**
 * Remove a reference to the memory pool.
 * The variable pointed to by @p hptr will be set to NULL.
 * When a memory pool has no more references, it will free its own memory.
 *
 * @param hptr pointer to the pointer to the memory pool.
 */
ELTN_API void ELTN_Pool_release(ELTN_Pool ** hptr);

/**
 * Convenience function to set @p dest with the value of @p src.
 *
 * @param dest the reference to modify with the value in @p src.
 * @param src the reference to copy into the value of @p dest.
 */
ELTN_API void ELTN_Pool_set(ELTN_Pool ** dest, ELTN_Pool ** src);

#endif /* __ELTN_PARSER */
