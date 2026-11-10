TODO
====

Coding
------

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
  - enable comments as events, verify they come through

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


Testing
-------

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
  - unquoted unescaped quoted strings
  - unquoted long strings
  - no comments if flag is not set
  - unquoted comments if flag is set
  - unquoted long comments if flag is set

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

- Variants of basic data types:
  - right quotes on string data
  - escape codes in quoted strings
  - long strings become Long Strings
  - DEF_NAMEs screened for non-identifier chars

- Test error detection (`ELTN_Emitter_error_code`)
  - duplicate key in table
  - unclosed table (error or auto-fix?)
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

- System tests in `eltn-test`
  - parse event file, compare values
  - replicate simple unit tests
  - progressively harder docs


Documentation
-------------

- Emerging ELTN spec in `./doc`
- Better in-code documentation (i.e. comments)


Packaging
---------

- Verify that SHLIB *works*
- Verify that DLL *works*
  - Replicate how Lua, cJSON, and tomlc17 do it.
- Write and *test* sample code for emitter and parser, as described in README.
  - eventlog.c
  - eltn2eltn.c
  - json2eltn.c + cJSON code (compile with Makefile?)
  - toml2eltn.c + toml17 code (compile with Makefile?)


Launch
------

- Announce on Lua-L list
- Monitor and answer any questions
- Respond to Pull Requests


Post-Launch
-----------

### Documentation

- User documentation, specifically:
  - using a pull parser vs. a push parser (e.g. SAX)
  - the emitter and the Builder Pattern
  - string handling conventions
- Specification and design documents for:
  - the parser
  - the emitter
  - the string class, if any
- Maybe some fancy diagrams for the perplexed.

### Functionality

- Deduce character byte width and endianness by assuming the first lines
  will be the comment "-- ELTN", etc., as suggested in Appendix B of the
  specification.

- Add API to specify the input and/or output encoding of strings.
  In particular, users of ELTN_Parser may want to see strings in Latin-1,
  and users of ELTN_Emitter may wish to submit string data as Latin-1.
  - Depends on availability/portability of `iconv`, in the general case.

- Add API to emit numbers in hexadecimal, decimal, or octal format.

- Add API to force ELTN_Emitter to emit specific strings as long strings.

- Add API to force ELTN_Emitter to emit or not to emit long comments.

- Revise decision on returning strings -- provide a new chunk of memory
  that must be freed -- to allow:
  1. Copying the string into client-side memory.
  1. Copying the string directly from library-side memory.
  1. Querying the length of a string separately.
  1. Allocate all/some strings from a custom memory pool.
  1. An ELTN_String object that does all of the above.

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

- Compile and test using native Windows compiler (*not* MINGW).

- Adapt source and tests for a pre-2011 compiler, e.g. `tcc` or GCC '99.
  - Biggest obstacle is the lack of unicode strings for testing purposes.
    Maybe do conversions by hand in code?

- Maybe smush whole thing into one file like the other guys?

- Multithreding library: pthreads? Microsoft? [OpenMP](https://www.openmp.org/)?
  One of the "portable" libraries?
  Note that current design presumes one (or many writers?), one reader.
  - mutex (one for buffer, one for parser/lexer, one per emitter)
  - condition variable (is buffer empty? is buffer full?)

### Robustness

- Improve error handling.

- Verify / enforce strict Pool boundaries:
  - All internal code allocates objects (e.g. strings) from their assigned pool,
    and frees them the same way.
  - All C strings released to the caller are assumed to be cleaned up by
    the caller using `free()`.
  - If using string objects, define a "common" pool for all ELTN_Strings and
    clean them up using a common API, so that developers don't have to remember
    which ELTN_Strings came from which pool.

### Multithreading

- Multithreaded Mode
  - Instead of expanding capacity in `ELTN_Buffer`,
    block until parser catches up.
    - May need to carve incoming text into smaller chunks.
  - Non-blocking `ELTN_Buffer_write_nb` that simply gives up or times out
    if the source is at maximum capacity.

### Scriptability

- Package entire C API as a Lua module using SWIG.

- Package entire C API as a Python module using SWIG.

- Package entire C API as a Ruby module using SWIG.
