# TODO

## Coding

### Lexer

- Implement Lexer
  - pass on i/o error
  - pass on out-of-memory error

- Implement String Routines
  - unquote long strings
  - trim comments and long comments
  - plug leak under error conditions
  - signal error conditions to caller

### Parser

- Implement Parser
  - handle illegal sequences of tokens
  - handle illegal tokens
  - handle i/o errors from source
  - track keys already used in each table

- Potential added API for KEY_STRING, VALUE_STRING, DEF_NAME, *et al.*:
  - `ELTN_Parser_string_copy(p, char *str, int len)`: copies string to
     buffer at `str`, with the maximum length `len`.
  - `ELTN_Parser_string_ref(p)` provides temporary buffer containing a
     null-terminated string.
  - `ELTN_Parser_string_len(p)` provides length of the string.
  - `ELTN_String_alloc(pool, str, len) -> ref` alllocates strings from
    `ELTN_Parser_string` and `ELTN_Parser_text`, to be freed by
    `ELTN_String_free(ref)`.  Implies string tagged with pool, if any.
  - Change `ELTN_Parser_string(p)` to return a pseudo-class `ELTN_String`,
    which kicks the can down the road a bit *but* could mesh well with
    SWIG when we get to that point.

- Potential added/revised API for `ELTN_Parser_text` along the lines above.


### Emitter

- Implement Emitter
  - each event method
  - error detection
  - create final string
  - write to file

### Other Concerns

- List of error codes
  - every error needs a distinct code
  - pass messages up from Source (?) and Lexer

- Downgrade to C99?  What version am I coding? C11?

## Testing

### Parser

- Test `ELTN_Buffer`
  - reader returns an error condition
  - forcing a read without a reader function
  - attempting to write to a closed Source
  - forcing a read of an empty Source
  - allocator failure?
  - test for byte order mark with swapped bytes
  - incremental reading of a string (or other in-memory construct)

- Test `ELTN_Lexer`
  - long comments (more)
  - long strings (more)
  - force token buffer to resize
  - EOF conditions

- Test `Key_Set`
  - performance when fully loaded
  - abnormal input, e.g. long strings

- Test `ELTN_Parser`
  - unquoted quoted strings
  - unquoted long strings
  - depth and current-key
  - implicit indexes

- Test `ELTN_Parser_read_file`
  - normal usage
  - error in file read

### Emitter

- Test each event method
  - `ELTN_Emitter_def_name`
  - `ELTN_Emitter_key_string`
  - `ELTN_Emitter_key_number`
  - `ELTN_Emitter_key_integer`
  - `ELTN_Emitter_value_string`
  - `ELTN_Emitter_value_number`
  - `ELTN_Emitter_value_integer`
  - `ELTN_Emitter_value_boolean`
  - `ELTN_Emitter_value_nil`
  - `ELTN_Emitter_table_start`
  - `ELTN_Emitter_table_end`
  - `ELTN_Emitter_comment`

- Test queries
  - `ELTN_Emitter_current_depth`
  - `ELTN_Emitter_current_path`

- Test error detection (`ELTN_Emitter_error_code`)
  - duplicate key in table
  - unclosed table
  - key / def-name without value

- Test `ELTN_Emitter_error_path`
  - normal usage
  - buffer too short

- Test `ELTN_Emitter_write`
  - pretty-printed
  - indented
  - matches reported length

- Test `ELTN_Emitter_write_string`
  - normal usage
  - buffer too short

- Test `ELTN_Emitter_write_file`
  - normal usage
  - error in file write

### Other Concerns

- Test `ELTN_Pool` and "ELTN alloc"
  - reference counting
  - allocation of memory
  - strings!

- Revise tests so that even `tcc` can compile them.
  - Replace U"..." and u"..." with strings of bytes.

- System tests in `eltn-test`
  - parse event file, compare values
  - replicate simple unit tests
  - progressively harder docs

## Documentation

- Emerging ELTN spec in `./doc`
- Maybe some fancy diagrams for the perplexed.

## Packaging

- Verify that SHLIB *works*
- Verify that DLL *works*
  - Replicate how Lua, cJSON, and tomlc17 do it.
- Write and *test* sample code for emitter and parser, as described in README.
  - eventlog.c
  - eltn2eltn.c
  - json2eltn.c + cJSON code (compile with Makefile?)
  - toml2eltn.c + toml17 code (compile with Makefile?)

## Launch

- Announce on Lua-L list
- Monitor and answer any questions
- Respond to Pull Requests

## Post-Launch

### Functionality

- Deduce character byte width and endianness by assuming the first lines
  will be the comment "-- ELTN", etc., as suggested in Appendix B of the
  specification.

- Other design or implementation defects as needed.

### Performance

- Profile with `gcov` for testing and `gprof` for performance.
  - Repeatable gcov coverage
    - Apparently it wants all its files in a single directory.
      Use a distinct Makefile or shell script to compile, e.g.
      ```
      gcc -Wall -fprofile-arcs -ftest-coverage -o test-parser *.c test/parser.c
      ```
  - gprof may be similar, e.g. `-pg` flag.  See
    <https://ftp.gnu.org/old-gnu/Manuals/gprof-2.9.1/html_mono/gprof.html>

- Memory usage
  - Use Valgrind to profile heap memory.
  - Simulate if malloc() just stopped working.
  - Performance tuning ideas:
    - Reuse token strings from `ELTN_Lexer_token_string()` in parser
    - Derive said strings from old token buffers.
    - Refuse any string that exceeds a maximum capacity.

- Improve memory usage.

- Improve performance.

### Portability

- Compile and test using native Windows compiler.

- Maybe smush whole thing into one file like the other guys?

- Multithreding library: pthreads? Microsoft? [OpenMP](https://www.openmp.org/)?
  One of the "portable" libraries?
  - mutex
  - condition variable (is buffer empty? is buffer full?)
  - read-write locks?

### Robustness

- Improve error handling.

### Multithreading

- Multithreaded Mode
  - Instead of expanding capacity in `ELTN_Buffer`,
    block until parser catches up.
    - May need to carve incoming text into smaller chunks.
  - Non-blocking `ELTN_Buffer_write_nb` that simply gives up or times out
    if the source is at maximum capacity.

### Lua Module

- Package entire C API as a Lua module using SWIG.
