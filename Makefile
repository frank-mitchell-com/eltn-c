# Makefile for Frank Mitchell's C library

CC=gcc
# CC=clang
# CC=tcc

LIBVERSION := $(file <VERSION.txt)
DLLVERSION := $(subst .,,$(LIBVERSION))

LIBNAME=eltnc
HEADERNAME=eltn.h

SRCDIR=.
OBJDIR=./obj
LIBDIR=./lib
DLLDIR=./obj-dll
TESTDIR=./test

INSTALL_DIR=/usr/local
INSTALL_INC=$(INSTALL_DIR)/include/$(LIBNAME)-$(LIBVERSION)
INSTALL_LIB=$(INSTALL_DIR)/lib

DIST_NAME=$(LIBNAME)-$(LIBVERSION)

LIB=$(LIBDIR)/lib$(LIBNAME)-$(LIBVERSION).a
SHLIB=$(LIBDIR)/lib$(LIBNAME).so.$(LIBVERSION)
SHLIB_ALIAS=$(LIBDIR)/lib$(LIBNAME).so
SONAME=lib$(LIBNAME).so.$(LIBVERSION)
DLL=$(LIBDIR)/$(LIBNAME)$(DLLVERSION).dll

CFLAGS=-g -Wall -fPIC
IFLAGS= -I $(SRCDIR) -I $(TESTDIR)
LFLAGS=-L$(LIBDIR) -l$(LIBNAME)-$(LIBVERSION) -lm

HEADERS=$(wildcard $(SRCDIR)/*.h)
SOURCES=$(wildcard $(SRCDIR)/*.c)
TESTSRCS=$(wildcard $(TESTDIR)/*.c)
TESTINCS=$(wildcard $(TESTDIR)/*.h)
OBJECTS=$(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SOURCES))
DLLOBJS=$(patsubst $(SRCDIR)/%.c,$(DLLDIR)/%.o,$(SOURCES))
TESTS=$(patsubst $(TESTDIR)/%.c,$(TESTDIR)/test-%,$(TESTSRCS))

.PHONY: core posix mingw clean test install dist

core: $(LIB) test

posix: core $(SHLIB)

mingw: core $(DLL)

test: $(TESTS)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(LIBDIR):
	mkdir -p $(LIBDIR)

$(DLLDIR):
	mkdir -p $(DLLDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(HEADERS) | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(DLLDIR)/%.o: $(SRCDIR)/%.c $(HEADERS) | $(DLLDIR)
	$(CC) $(CFLAGS) -DELTN_BUILD_AS_DLL -c $< -o $@

$(LIB): $(OBJECTS) | $(LIBDIR)
	ar rcs $(LIB) $^ 

$(TESTDIR)/test-%: $(TESTDIR)/%.c $(LIB) $(HEADERS) $(TESTINCS)
	$(CC) -static -g -O0 $(IFLAGS) -o $@ $< $(LFLAGS)
	./$@

$(SHLIB): $(OBJECTS)
	$(CC) -shared -Wl,-soname,$(SONAME) -o $(SHLIB) $^
	ln -s -r $(SHLIB) $(SHLIB_ALIAS)

$(DLL): $(DLLOBJS)
	$(CC) -shared -o $(DLL) $^

install: $(LIB) $(SHLIB)
	install -p -t $(INSTALL_LIB) $(LIB) $(SHLIB)
	install -p -t $(INSTALL_INC) $(HEADERNAME)

dist: $(LIB) $(DLL)
	mkdir -p $(DIST_NAME)/include
	mkdir -p $(DIST_NAME)/lib
	install -p -t $(DIST_NAME)/lib $(LIB) $(DLL)
	install -p -t $(DIST_NAME)/include $(HEADERNAME)
	zip -r $(DIST_NAME).zip $(DIST_NAME)
	rm -rf $(DIST_NAME)

clean:
	rm -f $(OBJECTS) $(DLLOBJS) $(LIB) $(SHLIB) $(SHLIB_ALIAS) $(DLL) $(TESTS)
	rmdir $(OBJDIR) $(LIBDIR) $(DLLDIR)

