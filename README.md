This project implements the ELTN spec located at
<https://frank-mitchell.com/projects/eltn/>
according to the (probably out of date) spec at
<https://frank-mitchell.com/projects/eltn-c/>

## Building ELTN-C

The Makefile requires no configuration step (as yet), but I have only tested
it on Linux and under UCRT64/MSYS2 on Windows.  It requires no other libaries
besides `libc` and maybe the math library.

If you have GNU Make, GCC, and the usual suite of Posix tools, simply type:

```sh
$ make
```

This makes the static library and runs all unit tests.

### Linux

To make a shared library and install in the default (Linux) directories of
`/usr/local`, type:

```sh
$ sudo make install
```

If you just want to create the shared library, use the `posix` target.


### Windows (UCRT64/MSYS2)

To make a DLL and bundle it and the header file in a zipped directory, type

```sh
$ make dist
```

You should have a file named `eltnc-$(VERSION).zip` in the project's main
directory.

To simply make the DLL, use the `mingw` target.


### Other Build Systems

I have not tried other build systems, particularly the Microsoft compiler.
I tried to stick to the C '11, standard, but there must be many special cases
I have not considered.  Patches would be welcome.


## Using ELTN-C

### `ELTN_Parser`

ELTN-C uses a pull parser; you point it at some text using 
`ELTN_Parser_read()`, `ELTN_Parser_read_file()`, or `ELTN_Parser_read_string()`
then turn the crank, so to speak, with
`ELTN_Parser_has_next()`, `ELTN_Parser_next()`, and `ELTN_Parser_event()` 
until the document is done.

[A sample parser program](examples/eventlog.c) reads a document
and prints out all the events it finds.

### `ELTN_Emitter`

The emitter is the parser in reverse: the caller issues events to the emitter,
which then builds an internal representation of a document, assuming the
caller has not violated any constraints.  They can then write the document
as a string (`ELTN_Emitter_write_string()`), file (`ELTN_Emitter_write_file()`),
or any other method somebody can code (`ELTN_Emitter_write()`).

- A [sample program](examples/eltn2eltn.c) translates ELTN to ELTN for
  demonstration purposes.

- [A more useful program](examples/json2eltn.c) parses a JSON document
  using an included [JSON parser](https://github.com/DaveGamble/cJSON)
  and emits the equivalent in ELTN.

- [Another emitter program](examples/toml2eltn.c) translates a TOML document
  using a [TOML parser](https://github.com/cktan/tomlc17).


## Version Roadmap

This lays out the roadmap for future development.

### 0.1.0

The parser is feature-complete, but likely has significant bugs that emerge
parsing real-world texts.  Certain features, like detection of Unicode
documents as described in Appendix B of the specification, may not work
correctly in all cases.

The emitter is not in a usable state.

Source compiles using gcc, GNUMake, and the usual suite of tools.
Testing with `clang` and `tcc` continues.  Modern `clang` seems to have
no issues but `tcc` cannot parse strings prefixed by "U" or "u".
Since these are only in test code, I will have to rewrite the tests to
specify Unicode bytes directly.

(`tcc` also core-dumps compiling the other tests. When `gcc` compiles tests
with `tcc`-compiled libraries, half the tests dump core.  `gdb` refuses to
run them, or anything linked with the libraries created by `tcc`.
I'm still not sure how to solve that ...)


### 0.2.0

The parser and emitter are feature-complete.  A suite of system tests for
the parser and emitter exist, which they all pass (almost always ...).
Nevertheless, ELTN-C lacks real-world experience.

Issues with `clang` and (maybe) `tcc` have been resolved.

### 0.4.0

At this point I would also like to have a [LuaRock](https://luarocks.org)
for ELTN-C with Lua bindings, possibly provided by [SWIG](https://swig.org).
The binding will be fully documented as a Lua module.

The libary compiles under other environments to be decided, but certainly
Microsoft Visual C and the LuaRocks "builtin" compile system.

### 0.8.0

By this release any major issues with the parser, emitter, and Lua bindings
to both have been resolved.  Remaining issues will be either "heisenbugs"
or gaps in the original specification, which I will attempt to address.

Python and Ruby bindings will follow the same patterns as the Lua binding.
They may not be idiomatic at this stage, but they will be usable.

### 1.0.0

The parser and emitter have been used by others in a real-world application.
This will require a sponsor of sorts who is willing to try out something that
may be buggy.

All the major, obvious bugs have been flushed out, By version 1.0.0, the
ELTN-C source has been compiled on more platforms than I have accessible
to me, and any issues have been resolved.  It's possible that the library
uses some sort of `configure` script at this point, perhaps to compile using
C99 (which lacks `bool` and some other types) or some odd C compiler of which
I'm not aware.

### 1.1.0 and beyond

Unless I (or others?) change the specification, the ELTN-C API will not change.
Improvements will hopefully make it faster and/or less memory intensive.

Another set of improvements would make `ELTN_Buffer` functions thread-safe.
This would allow asyncronous I/O to feed text to a parser
while the (single) parser thread waits for input,
in the classic producer/consumer pattern.

As the current API stands, the parser and emitter cannot be used meaninfully
in multithreaded programs.  Maybe in 2.0 ...

