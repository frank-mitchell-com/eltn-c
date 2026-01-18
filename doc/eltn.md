---
title: "ELTN v1.0.0"
date: "2025-06-04T18:44:03-0500"
---

ELTN v1.0.0
===========

Extended Lua Table Notation

By Frank Mitchell


Table of Contents
-----------------

* [Objectives](#objectives)
* [Related Work](#related-work)
* [An Example of ELTN](#an-example-of-eltn)
* [Character Set](#character-set)
* [Lexical Elements](#lexical-elements)
  * [Comment](#comment)
  * [Long String](#long-string)
  * [Quoted String](#quoted-string)
    * [Escape Sequences](#escape-sequences)
  * [Identifier](#identifier)
  * [Number Literal](#number-literal)
  * [Fixed\-Length Tokens](#fixed-length-tokens)
    * [Reserved Words](#reserved-words)
* [Syntax and Semantics](#syntax-and-semantics)
  * [Definition](#definition)
  * [Table](#table)
  * [String](#string)
  * [Number](#number)
  * [Boolean](#boolean)
  * [Nil](#nil)
* [ELTN Files](#eltn-files)
  * [File Extension](#file-extension)
  * [Mime Type](#mime-type)
* [EBNF Grammar](#ebnf-grammar)
  * [Parser Rules](#parser-rules)
  * [Lexical Tokens](#lexical-tokens)
    * [Whitespace and Comments](#whitespace-and-comments)
    * [Strings](#strings)
    * [Names and Reserved Words](#names-and-reserved-words)
    * [Numbers](#numbers)
  * [Basic Definitions](#basic-definitions)
* [Appendix A: Characters Used in ELTN](#appendix-a-characters-used-in-eltn)
* [Appendix B: Self-Identification and Encoding Information](#appendix-b-self-identification-and-encoding-information)
* [Appendix C: Other Data Types](#appendix-c-other-data-types)
  * [Dates and Times](#dates-and-times)
  * [References](#references)
* [Appendix D: ELTN Path](#appendix-d-eltn-path)

Created by [gh-md-toc](https://github.com/ekalinin/github-markdown-toc.go)


[Lua]:      https://lua.org
[TOML]:     https://toml.io/
[YAML]:     https://yaml.org/
[JSON]:     http://json.org/
[XML]:      https://www.w3.org/XML/
[CORBA]:    https://www.corba.org/faq.htm

[JavaScript]:   https://developer.mozilla.org/en-US/docs/Web/JavaScript
[EcmaScript]:   https://tc39.es/ecma262/
[rockspec]:     https://github.com/luarocks/luarocks/blob/main/docs/creating_a_rock.md
[LuaRocks]:     https://luarocks.org


Objectives
----------

Extended Lua Table Notation (ELTN) provides a text representation of data
drawn from syntax of the [Lua programming language][Lua] in the same way
[JSON][] draws from [JavaScript][] / [ECMAScript][].
The author envisions ELTN as an alternative to [TOML][] for configuration and
manifest files, but ELTN could also play a role in data exchange similar to
JSON in exchanging data over HTTP or raw sockets.


Related Work
------------

As a strict subset of [Lua][],
ELTN owes a great deal to Lua's creators, Roberto Ierusalimschy,
Waldemar Celes, and Luiz Henrique de Figueiredo, as well as its many
contributors and maintainers over decades.

ELTN also draws inspiration from [LuaRocks][], Lua's semi-official package
manager, and its [rockspec][] format which is essentially Lua code.
The author hopes to provide a more general solution for Lua-like data formats.

[TOML][], [YAML][], and [JSON][] provide similar data representations to ELTN:
a directed graph of sequences and associative arrays with leaf nodes
representing text, numbers, boolean values, and other discrete data.
The author regards ELTN as simpler for both humans and machines to read than
YAML, but not quite as verbose as JSON.
Unlike TOML, ELTN represents data hierarchies transparently like YAML and JSON.

[XML][] first started the trend of simple text-based data exchange formats.
Over the years it acquired the same sort of standards bloat as binary protocols
like [CORBA][], which led to the current generation of "lightweight" data
exchange formats.


An Example of ELTN
------------------

Let's say a group of professional programmers keeps a distributed lending
library of the books on their shelves. Each member uploads an ELTN document
that lists of the books they're willing to lend out that might look like this:

```lua
memberid = 13

name = "Frank Mitchell"

contact = {
    email = "frank.mitchell@example.com",
    -- no other contact information
}

books = {
    { 
        author = "Donald E. Knuth", 
        title = "Literate Programming", 
        publisher = "CSLI", 
        year = 1992
    },
    { 
        author = "Jon Bentley", 
        title = "More Programming Pearls", 
        year = 1990, 
        publisher = "Addison-Wesley", 
    },
    --[[
    ... many more ...
    ]]
}
```

This data could be loaded into an SQL or NoSQL database. It could just
as easily remain in ELTN form if the schema is changing rapidly, or if the
backend is a set of scripts written in Lua or a language with an ELTN parser.
Not every program needs to be an enterprise-level Web application.


Character Set
-------------

[Character Set]:    #character-set

ELTN is a text format.  This specification assumes documents will use
ASCII or an encoding consistent with ASCII (1967 onward), such as Unicode,
an ISO-8859 standard, a Windows-125x encoding,
or [others](https://en.wikipedia.org/wiki/Extended_ASCII).
See [Appendix A][] for the specific characters ELTN uses.

Other, unrelated encodings like [EBCDIC](https://en.wikipedia.org/wiki/EBCDIC)
may have the required characters.  Porting ELTN parsers to IBM mainframes
lies outside this specification's scope.

Characters higher than hexadecimal 0x7F (DEL) may only appear in [strings][]
and [comments][].  When in a string or comment, an ELTN parser must pass
non-ASCII codes through without interpretation.
(But see [Appendix B][] for a possible exception to the rule.)


Lexical Elements
----------------

The syntax of ELTN ultimately comes from the [Lua][] language, as described
in the [Lua 5.4 manual](https://lua.org/manual/5.4/manual.html#3.1).
Any ambiguity in the author's descriptions are solely the fault of the author.
If any questions about implementing ELTN remain, use the Lua 5.4 interpreter
as a guide to legal syntax.

### Comment

[Comment]:  #comment
[Comments]: #comments

Comments are notes from the writer of an ELTN document not meant as data.
A comment takes two forms, short and long.
A "short comment" runs from a `--` to the end of the line.

```lua
name = "value"  -- this is a short comment that ends here.
                -- this is another short comment.
```

A long comment may run from the sequence `--[[` to a matching `]]`,
or `--[=[` to `]=]`, or `--[==[` to `]==]`, and so on.
Long comments use the same rules as [Long Strings][], below.

```lua
--[==[
This comment runs several lines, and may contain any characters.
It may even contain [[this]] or [===[this]===].
It ends only after the text includes these:
]==]
name = "value"
```

Parsers may skip comments as mere whitespace, or provide them to their
caller as added information.


### Long String

[Long String]:      #long-string
[Long Strings]:     #long-string

Like [quoted strings][] below, a "long string" captures text data.  Just like
open and close quotes enclose quoted strings, long brackets enclose long
strings.  A sequence like one of these -- `[[`, `[=[`, `[==[`, etc. --
is an <dfn>opening long bracket</dfn>.  The number of equals signs between
the first and second square bracket define the *level* of the bracket.
After the opening long bracket, and an optional newline immediately after
the long bracket, the string includes every character in the text, without
further interpretation, until the first <dfn>closing long bracket</dfn> of
the same level.  That is, if a long string begins with `[=[`, an ELTN
parser will interpret the first `]=]` it sees as the end of the string.
This means the parser will bypass closing brackets of a different level,
whether `]]` or `]==]`.

The following is an example of a Long String:

```lua
long_string = [=[this is a "Long String" but not a very long string.
It continues like this, until the closing bracket, which is not
"]]" or "]==]" but this:]=]
```

### Quoted String

[Quoted String]:    #quoted-string
[Quoted Strings]:   #quoted-string

Quoted strings represent text much like programming languages represent text.
Quoted strings begin and end with the same type of quote, either a single
quote (`'`) or double quote (`"`).  Most characters between the
quotes stand for themselves, with a few exceptions:

1. A string can include a quote mark of the same type that began the string
   *if* it is immediately preceded by a backslash (`\`).
1. Any newlines between the quotes must be <dfn>escaped</dfn> with a
   backslash.
1. A backslash also begins an **[escape sequence][]** that produces whitespace
   or non-printable characters, among other uses.
1. The sequence `\z` escapes not only a newline but all the whitespace
   preceding it until the next printable character, including another
   escape sequence.

#### Escape Sequences

[escape sequence]:  #escape-sequences
[escape sequences]: #escape-sequences

Within a quoted string the following escape sequence represent special
characters or, in the case of `\z`, removes characters from the final string.

| Escape | Byte(s) | Meaning
|:------:|:-------:|----------
| \a                | 0x07 | bell
| \b                | 0x08 | backspace
| \f                | 0x0c | form feed
| \n                | 0x0a | newline
| \r                | 0x0d | carriage return
| \t                | 0x09 | horizontal tab
| \v                | 0x0b | vertical tab
| \\\\              | `\`  | backslash
| \\"               | `"`  | quotation mark / double quote
| \\'               | `'`  | apostrophe / single quote
| \\&#x21a9;&#xfe0e;| 0x0d | escaped newline
| \z                |      | skip to the next non-whitespace character
| \\x**XX**   | 0x**XX**| byte value in hexadecimal digits
| \\**DDD**   | 0**DDD** | byte value in octal digits
| \\u{**XXXX**}|`utf8(`**XXXX**`)`| UTF-8 bytes for code point 0x**XXXX**.

Most of these escape sequences are familiar to users of C, C++, C#, and Java.
Most or all should be familiar to users of Lua.
The unfamiliar ones need some explaining.

***Escaped Newline***

The newline after the slash becomes part of the string.  So, for example:

```lua
s = "string begins, \
string continues"
```

is equivalent to:

```lua
s = "string begins, \nstring continues"
```

***The "z" Escape***

All the whitespace disappears, along with the `\z`.  So, for example:

```lua
s = "string begins, \z
                     string continues"
```

is equivalent to:

```lua
s = "string begins, string continues"
```
To quote Ierusalimschy *et al.* verbatim from the
[Lua 5.4 manual](https://lua.org/manual/5.4/manual.html#3.1),
<q> [...] it is particularly useful to break and indent a long
literal string into multiple lines without adding the newlines and spaces
into the string contents.</q>

***Octal Digits***

A sequence of octal digits may have only one or two octal digits unless
the following character(s) are also digits, to avoid ambiguity.

In the Lua interpreter, the string "\0" produces a string of length one
containing a zero byte.  Since C (and other languages) use a zero byte
to terminate their strings, implementers of ELTN parsers should always
provide not only the bytes of an interpreted string but its length.

***Unicode Sequence***

This sequence should encode a Unicode code point into a one to three byte
UTF-8 character.

The enclosing brackets are mandatory, and in many ways a really good idea.
The code can specify any number of hexadecimal digits, from one to six
(or more).


### Identifier

[Identifier]:   #identifier
[Identifiers]:  #identifiers

In ELTN an <dfn>identifier</dfn> is the most common key in a [Table][],
and is required in a [Definition][].

All identifiers must start with an underscore or ASCII letter
and contain zero or more ASCII letters, ASCII numbers, or underscores.
No valid identifier may match match the words `nil`, `true`, `false`, or any
[reserved words][].

The following are valid identifiers:

```lua
foo
testing123
snake_case
Capital_Case
camelCase
ALL_CAPS
is_local        -- "local" is a reserved word, but "is_local" is not.
___             -- confusing and hard to read, but valid
```

The following are invalid identifiers:

```lua
123skidoo   -- starts with a number
beta-test   -- includes a '-' in the middle
+larry      -- doesn't start with a letter or '_' character.
local       -- reserved word
```

### Number Literal

[Number Literal]:   #number-literal
[Number Literals]:  #number-literal

Numbers in ELTN resemble those in other languages: an integer part, a decimal
part, and an exponent.  However, like Lua, both integers and decimal numbers
may use hexadecimal notation by prefixing the constant with `0x` or `0X`.

The following are valid integers:

```lua
1234
0
-74
10294928
0x3e8       -- the hexadecimal value 0x3E8 or 1000
037         -- produces 37, not the octal value.
```

The following are valid decimal (floating-point) numbers:

```lua
1000.000    -- 1000 to three decimal places
3e8         -- 3 * 10^8, approximately the speed of light in m/s
0x3e8p8     -- 0x3e8 * 2^8, or 256000
```

The following are invalid numbers:

```lua
100,000     -- the comma is invalid ELTN syntax
1_000       -- so is the underscore (this isn't Python or Eiffel)
23d7        -- lacks an "0x" prefix, if this was hexadecimal
```

If your application requires octal numbers, prettier decimal numbers, or
anything else outside ELTN syntax, you can use a [quoted string](#quoted-string)
and do the conversion in your application itself.


### Fixed-Length Tokens

The language requires the following fixed-length tokens.

| Token | Meaning
|:-----:|---------
| `,`   | separates table entries
| `;`   | separates table entries or top-level definitions
| `=`   | associates a definition or table key with a value
| `false` | Boolean false
| `nil`   | a reference to nothing
| `true`  | Boolean true
| `[`   | precedes a non-identifier key
| `]`   | follows a non-identifier key
| `{`   | creates a new table
| `}`   | marks the end of a table

#### Reserved Words

[reserved word]:    #reserved-words
[reserved words]:   #reserved-words

The words `nil`, `true`, and `false` may not be used as identifiers, since
they denote a [Nil][] value or [Boolean][] values, respectively.

For compatibility with Lua, ELTN reserves the following additional tokens,
which are not available as identifier names:

    and       break     do        else      elseif
    end       for       function  goto      if
    in        local     not       or        repeat
    return    then      until     while


Syntax and Semantics
--------------------

### Definition

[Definition]:   #definition
[Definitions]:  #definition

This is a valid ELTN document:

```lua
{
    markup = {
        tableOfContents = { startLevel = 2, endLevel = 5 };
        highlight = {
            style = "github";
            tabWidth = 4;
        };
        goldmark = { renderer = { unsafe = true }};
    },
    taxonomies = { tag = "tags" }
}
```

So is this:

```lua
markup = {
  tableOfContents = { startLevel = 2, endLevel = 5 };
  highlight = {
    style = "github";
    tabWidth = 4;
  };
  goldmark = { renderer = { unsafe = true }};
}
taxonomies = { tag = "tags" }
```

Unlike [JSON][], but like [TOML][], ELTN has a top level name-space called a
<dfn>definition table</dfn>. Think of the definition table as a JavaScript
Object, Python dictionary, or Ruby Hash mapping identifiers to values,
which may be nested [Tables][].

An identifier may be defined at most once; multiple definitions of the same
identifier are invalid.

The definition table have a few restrictions relative to [Tables][]:

1. The only keys permitted are identifiers, *not* strings or numbers.
1. The only separators allowed are semicolons.  On the other hand, separators
   are not required at all.

The corresponding [Lua][] construct is the "global table" where all global
variables reside.


### Table

[Table]:    #table
[Tables]:   #table

As in [Lua][], tables are the fundamental building block of ELTN.  Everything
that is not a scalar ([String][], [Number][], [Boolean][], or [Nil][]) is
a Table.  The <dfn>key</dfn> or <dfn>index</dfn> of a table is
a [String][] or [Number][] used to refer to a contained <dfn>value</dfn>,
which may a [String][], a [Number][], a [Boolean][] a [Nil][], or another
[Table][].

Keys that are [Identifiers][] may be set just like [Definitions][]:

```lua
{
    summary = "A parser and emitter for the ELTN data language in C",
    homepage = "https://github.com/frank-mitchell-com/eltnc/",
    license = "MIT"
}
```

Keys that are arbitrary strings or numbers must be enclosed in square
brackets (`[` ... `]`):

```lua
{
    ["this key has spaces"] = true,
    [1] = "first item"
}
```

This syntax maintains compatibility with [Lua][].

Many languages have a distinction between an Array, List, or Sequence, which
indexes its contents by [Number][] (almost always integer values starting at
0 or 1) and a Dictionary, Hash, or Map, which indexes its contents using a
[String][] or sometimes another datatype.  ELTN mixes the two.  This is a
valid Table in ELTN:

```lua
{ "one", "two", "three", [4] = "four", count = 4, ["creepy laugh"] = "ah ah ah"}
```

The elements not preceded by key assignments are implicitly assigned keys based
on the order of their occurrence in the Table.  The above is equivalent to
the following:

```lua
{
    [1] = "one",
    [2] = "two",
    [3] = "three",
    [4] = "four",
    count = 4,
    ["creepy laugh"] = "ah ah ah",
}
```

Implementers should use their native Hash, Map, Object, etc. structures to
represent tables, converting numbers or booleans to strings if necessary.
*If* a table looks sufficiently array-like, i.e. only integer keys starting
at 1 and forming a continuous range, they *might* use a List or Array.
Parsers are not obliged to assess the "sequence-like" or "mapping-like"
tendencies of a parsed Table, however.

Any key that isn't an identifier must be enclosed in square brackets: '['...']'.
Each String or Number key must be unique within that table; even if they
look different, two numbers or strings that evaluate to the same value,
after processing escape characters or converting to double precision floating
point representation, are the same.

Also note that an identifier evaluates to its String, so this is invalid:

```lua
{
    some_name = 3,
    ["some_name"] = 3   -- INVALID: same name, different syntax
}
```

Unlike [Lua][] tables, an ELTN table allows only [Strings][] and [Numbers][]
as keys.  This is for the sanity of both parser writers and implementers
in specific languages.  Also, unlike Lua tables, ELTN tables have
no identity, only a value determined by their contents.  One can *give* tables
an identity by referring to them with [Definitions][], but that lies in the
application domain, beyond the scope of an ELTN specification.


### String

[String]:   #string
[Strings]:  #string

To a certain extent Strings in ELTN resemble strings in nearly every
programming language: they consist of a length and a sequence of characters.
[Quoted Strings][] and [Long Strings][] resolve to the same String type;
parsers should make no distinction in their API.

For preference ELTN treats strings as bytes which *may* correspond to ASCII,
Latin-1, or UTF-8 character encodings, but it is up to the language
and application to decide on the character encoding.  A Java parser may, for
example, use UTF-16 characters, while a parser ported to an old IBM mainframe
may translate all characters to EBCDIC.


### Number

[Number]:   #number
[Numbers]:  #number

Numbers, expressed as a [Number Literal][], notionally include all real numbers.

Due to the limitations of the floating point representation in most computers,
they have only a finite number of digits of precision, represented as binary
bits.  If the language allows, parsers should represent numbers as standard
integers, infinite-precision integers, or infinite precision decimal numbers
if the [Number Literal][] expresses an integer or decimal value.
Otherwise double-precision floating point is sufficient.


### Boolean

[Boolean]:  #boolean
[Booleans]: #boolean

Booleans have two values, `true` and `false`.  They have counterparts, sometimes
with slightly different names, in JavaScript, Python, Ruby, Scheme, Lisp, and
even C/C++ after 2011.


### Nil

[Nil]:      #nil

`nil` corresponds to `NULL` in C, `null` in JavaScript or Java,
`None` in Python, and `nil` in Ruby.  It's the value provided when
there's no value to give.

JavaScript has a distinction between an "undefined" value, i.e. a missing
[Definition][] or [Table][] key, and a `null` value, which was defined as
something other than an Object or primitive value.
[Lua][] and many other languages lack this distinction.
A parser *must* convey that a `nil` is present.
Other APIs may or may not use a special "ELTN Nil Object" when serializing
and deserializing native structures to and from ELTN.


ELTN Files
----------

As described under [Character Set][], an ELTN file is a text file encoded as
ASCII or a compatible character encoding.

### File Extension

The suggested file extension for ELTN files is `.eltn`.

### Mime Type

The suggested mime type is "application/eltn".


EBNF Grammar
------------

The grammar's notation is as follows:

`lower_case`
: A reference to a grammar rule, defined with the symbol `=`.

`SOME WORDS`
: A description of a rule in plain English.

`"`*x*`"`
: A literal character or sequence of characters.

`"\`*x*`"`
: An escape sequence denoting a non-printable or confusing character.
  A literal `"` is written as `"\"`; a literal \\ is written as `"\\"`.
  See [**Escape Sequences**](#escape-sequences), above.

*x* `,` *y*
: A sequence containing both *x* and *y*.

*x* `|` *y*
: Either *x* or *y*. 

`[` *x* `]`
: Zero or one of *x*.

`{` *x* `}`
: Zero or more of *x*.

`(` *x* `)`
: A group of items that are treated as a unit.

For example `"y", [ "a" | "b" ] , "z"` means
a sequence of zero or more "a"s *or* "b"s, starting with a "y" and ending
with a "z": "yz" "yaz", "ybz", "yaaz", "ybaz", etc.


### Parser Rules

```ebnf
document        = ( deflist | ( ws , table ) ) , ws ;

deflist         = { ws , definition } ;

definition      = defname , ws , "=", ws , value , [ ws, ";" ] ;

defname         = name ;

value           = nil | boolean | number | string | table ;

table           = "{" , [ ws , entrylist ] , ws , "}" ;

entrylist       = entry , { ws , entrysep , ws , entry }, [ ws , entrysep ] ;

entry           = ( key , ws , "=" , ws , value ) | value  ;

key             = name | ( "[" , ws , keyvalue , ws , "]" ) ;

keyvalue        = number | string ;

entrysep        = "," | ";" ;
```

### Lexical Tokens

#### Whitespace and Comments

```ebnf
ws              = { whitespace | comment } ;

whitespace      = " " | "\f" | "\n" | "\r" | "\t" | "\v" ;

comment         = ( "--" , NOT A NEWLINE , newline )
                    | ( "--" , long_string ) ;

newline         = "\n" | "\r\n" ;
```

#### Strings

```ebnf
string          = quoted_string | long_string ;

quoted_string   = dquo , { qstr_char_dquo | escape_sequence  } , dquo
                    | squo , { qstr_char_squo | escape_sequence } , squo ;

dquo            = """" ;    (* should be "\"" but for syntax highlighter *)

squo            = "'" ;

qstr_char_dquo  = NOT A dquo OR A NEWLINE;

qstr_char_squo  = NOT A squo OR A NEWLINE;

escape_sequence = "\\" , dquo
                    | "\\" , squo
                    | "\\" , newline
                    | "\\" , "z", { whitespace }
                    | "\\" , ("a"|"b"|"f"|"n"|"r"|"t"|"v")
                    | "\\" , "x" , hexdigit , hexdigit
                    | "\\" , octdigit [ octdigit [ octdigit ] ]
                    | "\\" , "u{" , hexdigit , { hexdigit } , "}" ;

long_string     = long_open_N , long_sequence_N , long_close_N ;

long_open_N     = ( "[" , { "=" } , "[" ) FOR EXACTLY N "=" ;

long_sequence_N = NOT A long_close_N ;

long_close_N    = ( "]" , { "=" } , "]" ) FOR EXACTLY N "=" ;
```

#### Names and Reserved Words

```ebnf
boolean         = "true" | "false" ;

nil             = "nil" ;

reserved_word   = "and" | "break" | "do" | "else" | "elseif" | "end" | "for"
                    | "function" | "goto" | "if" | "in" | "local" | "not"
                    | "or" | "repeat" | "return" | "then" | "until" | "while" ;

name            = (namestart { namepart })
                    AND NOT nil, A boolean, OR A reserved_word ;

namestart       = letter | "_" ;

namepart        = letter | digit | "_" ;
```

#### Numbers

```ebnf
number          = dec_number | hex_number ;

dec_number      = dec_integer , [ "." , { digit } ] , [ dec_exponent ]
                    | sign , "." , digit , { digit } , [ dec_exponent ] ;

dec_integer     = sign , digit , { digit } ;

dec_exponent    = ("e" | "E") , [ "+" | "-" ] , digit , { digit } ;

hex_number      = hex_integer , [ "." , { hexdigit } ] , [hex_exponent]
                    | sign , hex , "." , hexdigit , { hexdigit }, [ hex_exponent ] ;

hex             = "0", ("X"|"x") ;

hex_integer     = sign , hex , hexdigit , { hexdigit } ;

hex_exponent    = ("p" | "P") , [ "+" | "-" ] , hexdigit , { hexdigit } ;

sign            = [ "-" ] ;
```

### Basic Definitions

```ebnf
letter          = "A" | "B" | "C" | "D" | "E" | "F" | "G" | "H" | "I" 
                    | "J" | "K" | "L" | "M" |"N" | "O" | "P" | "Q" | "R"
                    | "S" | "T" | "U" | "V" | "W" | "X" | "Y" | "Z"
                    | "a" | "b" | "c" | "d" | "e" | "f" | "g" | "h" | "i" 
                    | "j" | "k" | "l" | "m" | "n" | "o" | "p" | "q" | "r"
                    | "s" | "t" | "u" | "v" | "w" | "x" | "y" | "z" ;

digit           = "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9";

hexdigit        = "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9"
                    | "A" | "B" | "C" | "D" | "E" | "F"
                    | "a" | "b" | "c" | "d" | "e" | "f" ;

octdigit        = "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7";
```

## Appendix A: Characters Used in ELTN

[Appendix A]:   #appendix-a-characters-used-in-eltn

Certain 7-bit characters play a crucial role in defining the ELTN format:

| ASCII Values (hexadecimal) | Characters | Significance
|:--------------------------:|:----------:|-------------------------------
| 0x09  | *horizontal tab*  | whitespace
| 0x0A  | *line feed*       | whitespace, end of a short [Comment][]
| 0x0B  | *vertical tab*    | whitespace
| 0x0C  | *form feed*       | whitespace
| 0x0D  | *carriage return* | whitespace, end of a short [Comment][]
| 0x20  | *space*           | whitespace
| 0x22  | `"`               | start/end of a [Quoted String][]
| 0x27  | `'`               | start/end of a [Quoted String][]
| 0x2B  | `+`               | part of a [Number Literal][]
| 0x2C  | `,`               | separates entries in a [Table][]
| 0x2D  | `-`               | part of a [Number Literal][] or [Comment][]
| 0x30 ... 0x39 | `0` ... `9`| part of a [Number Literal][] or [Identifier][]
| 0x3B  | `;`               | separates entries in a [Table][] or [Definitions][]
| 0x3D  | `=`               | [Definition][] or [Table][] key assignment
| 0x41 ... 0x5A | `A` ... `Z`| part of an [Identifier][]
| 0x5B  | `[`               | start of a [Table][] key, [Long String][], or long [Comment][]
| 0x5C  | `\`               | escapes special characters in a [Quoted string][]
| 0x5D  | `]`               | end of a [Table][] key, [Long String][], or long [Comment][]
| 0x5F  | `_`               | part of an [Identifier][]
| 0x61 ... 0x7A | `a` ... `z`| part of an [Identifier][] or [reserved word][]
| 0x7B  | `{`               | start of a [Table][]
| 0x7D  | `}`               | end of a [Table][]


## Appendix B: Self-Identification and Encoding Information

[Appendix B]:   #appendix-b-self-identification-and-encoding-information

ELTN parsers should leave interpreting any bytes in strings and comments to
the calling program.  Some, however, may want to label their documents with
a version of the ELTN specification and possibly
a character encoding, or may encode their documents in unusual formats like
UCS-2 (UTF-16), UCS-4 (UTF-32), or EBCDIC.  A parser may therefore have to
transcode its input stream into something it can recognize.

In an effort to establish some standard for encoding ELTN documents,
the author submits this additional syntax.
A document is not *required* to provide encoding information, and parsers may
simply treat this proposed convention like any other ELTN comment.

The very first bytes in an ELTN document stream may be these:

```ebnf
identification  = [byteordermark] ,
                    "--" ,
                    space ,
                    "ELTN" , space , "=" space, dquo, "1.0", dquo
                    space ,
                    [ "charset" , space , "=" , space , dquo, charset, dquo ],
                    space ,
                    newline ;

byteordermark   = "\u{FEFF}" ;

space           = { " " | "\t" } ;

charset         = cschar { cschar } ;

cschar          = letter | number | "-" | "_" | "." | ":" | "/" ;
```

Unicode uses the Byte Order Mark to determine the byte encoding of a document.
The order in which the two bytes occur tells a parser whether a document is
UTF-8, UTF-16, or UTF-32 and in what order the rest of the document's bytes|
will appear.  The following was adapted from an appendix to the
[XML Specification](https://www.w3.org/TR/2006/REC-xml11-20060816/#sec-guessing).

| First Four Bytes  | Encoding  | Comments
|:-----------------:|-----------|------------------
| `00 00 FE FF`     | UTF-32BE  | four bytes, highest first (big endian)
| `FF FE 00 00`     | UTF-32LE  | four bytes, lowest first (little endian)
| `00 00 FF FE`     | ???       | four bytes, unusual ordering
| `FE FF 00 00`     | ???       | four bytes, unusual ordering
| `FE FF 00 00`     | UTF-16BE  | two bytes, higher first (big endian)
| `FF FE ?? ??`     | UTF-16LE  | two bytes, lower first (little endian)
| `EF BB BF ??`     | UTF-8     | the Byte Order Mark in UTF-8 encoding.
| `00 00 00 2D`     | UTF-32BE  | no BOM, but a `-` in UTF-32.
| `2D 00 00 00`     | UTF-32LE  | no BOM, but a `-` in UTF-32.
| `00 2D 00 2D`     | UTF-16BE  | no BOM, but a `--` in UTF-16.
| `2D 00 2D 00`     | UTF-16LE  | no BOM, but a `--` in UTF-16.
| `2D 2D 20 45`     | ASCII?    | no BOM, but a `-- E` in ASCII or UTF-8.
| `60 60 40 C5`     | EBCDIC?   | `-- E` in EBCDIC; parser may need to decode.

(Bytes noted as `??` are irrelevant as long as they are not `00`.)

Following that is a standard ELTN / Lua comment that identifies the 
document as ELTN version 1.0.  What follows (perhaps) is a single directive
"charset = X", where X is a known encoding name used by `iconv`,
Java"s character encoding system, and other common systems.

```lua
-- ELTN = "1.0" charset = "UTF-8"
```

This information may benefit a parser's caller, so parsers may want to relay
comment contents to that level.


## Appendix C: Other Data Types

Other data representation formats, notably [TOML][] and [YAML][], support
other data types, including dates, times, date-times, and object references.
To represent other data types, ELTN offers [Strings][] and [Tables][].

This appendix offers non-normative suggestions about how applications
can interpret these as other common data types.

### Dates and Times

Applications can represent dates and times using
[ISO-8601](https://en.wikipedia.org/wiki/ISO_8601) date and time strings.
For example, May 23, 2025, at 7:30:19 AM in the North American Central Time
Zone, translates to `"2025-05-23T07:30:19-0500"`.  (The -0500 part reflects
the offset from Greenwich Mean Time for Central Daylight Time.)

ISO-8601 also has a standard to capture intervals of time, but something
informal like "3M" for three months or "30m" for 30 minutes should suffice
for many purposes.

Many languages offer libraries that translate ISO-8601 to their own native
date-time representations and back.

### References

ELTN represents data as an acyclic directed graph of table keys toward their
values.  The specification encourages writers of ELTN emitters and serializers
to check for possible cycles in the graph of objects being submitted to be
serialized as ELTN.  How, then, can a serializer or emitter handle two
references to the same Table (Object, Array, List, Dictionary, etc.) in the
graph of objects?

One possible method relies on [Definitions][].  The top level may contain
a number of entries of the form `__ref__<ID>`, where `<ID>` is some randomly
or sequentially generated numbers and letters.  Where the table for that
definition should appear, the application has instead "`__ref__<ID>`" for
that ID.  If the output is supposed to have only one table, it could be marked
"`__top`".

References to external data could use URLs or URIs ([Strings][]) using
[REST](https://restfulapi.net/), [GraphQL](https://graphql.org/), or other
modern Web principles and standards.


## Appendix D: ELTN Path

An ELTN path indicates a specific ELTN value through a chain of keys.
It has the following syntax:

```ebnf
path    =  ( name | ("[" , keyvalue , "]" ) ) ,
                 { ( "." , name ) | ( "[" , keyvalue  , "]" ) } ;
```

All paths are rooted in the Definition Table, or in the highest-level Table
if no Definition Table is present.

For example, in the [sample document above](#an-example-of-eltn), the path
"`books[1].author`" would indicate the value `"Donald E. Knuth"`.
If a table entry has no explicit key value, you may recall, it is assigned
one from the cardinal numbers, starting at 1.
