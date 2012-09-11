#
# Alpha Make
#
# Copyright (c) Peter Macko
#


#
# Set the project root
#

ifndef ROOT
ROOT := .
endif


#
# Do this only once
#

ifndef __MAKE__INCLUDE__HEADER__
__MAKE__INCLUDE__HEADER__ := yes


#
# Import machine-specific settings
#

ifndef __MAKE__INCLUDE__DEFS__
	__MAKE__INCLUDE__DEFS__ := yes
	include $(ROOT)/make/defs.mk
endif


#
# Make-related flags
#

SHELL := /bin/bash


#
# Useful constants
#

UNAME := $(shell uname)
UNAME_ALL := $(shell uname -a)
 
ifndef OSTYPE
ifeq ($(UNAME),Darwin)
	OSTYPE := darwin
else
ifeq ($(UNAME),Linux)
	OSTYPE := linux
endif
endif
endif

EMPTY :=
SPACE := $(EMPTY) $(EMPTY)
 

#
# Machine configuration
#

ifeq ($(findstring x86_64, $(UNAME_ALL)),x86_64)
	BITS := 64
else
	BITS := 32
endif

ifeq ($(OSTYPE),darwin)
	ifeq ($(shell sw_vers | fgrep -c 10.6.),1)
		BITS := 64
	endif
endif

ifeq ($(BITS),64)
	CFLAGS      := $(CFLAGS)   -m64
	CXXFLAGS    := $(CXXFLAGS) -m64
endif


#
# Build output
#

ifndef BUILD_DIR_ALL
	BUILD_DIR_ALL := build
endif

ifndef BUILD_DIR
	BUILD_DIR := $(BUILD_DIR_ALL)
endif


#
# Important variables & defaults
#

ifndef EXECUTABLE_SUFFIX
	EXECUTABLE_SUFFIX :=
endif

ifndef OUTPUT_TYPE
	OUTPUT_TYPE := kernel
endif


#
# Current directory
#

PWD := $(shell pwd)


#
# Current directory relative to the root
#

PWD_REL := $(subst ++++$(realpath $(ROOT))/,,++++$(realpath .))

ifneq ($(subst ++++,,$(PWD_REL)),$(PWD_REL))
	PWD_REL := $(subst ++++$(realpath $(ROOT)),,++++$(realpath .))
endif

PWD_REL := $(subst ++++,,$(PWD_REL))

ifeq (++$(strip $(PWD_REL))++,++++)
	PWD_REL_SEP := $(PWD_REL)
else
	PWD_REL_SEP := $(PWD_REL)/
endif


#
# The project name
#

ifndef PROJECT
	PROJECT := $(strip $(notdir $(PWD)))
endif


#
# Run directory
#

ifndef RUN_DIR
	RUN_DIR := .
endif

MAKE := $(MAKE) RUN_DIR="$(strip $(realpath $(RUN_DIR)))"


#
# Release
#

ifdef RELEASE
	MAKE := $(MAKE) RELEASE=$(RELEASE)
	BUILD_DIR := $(BUILD_DIR)/release
else
	DEBUG := yes
	BUILD_DIR := $(BUILD_DIR)/debug
endif


#
# C and C++ flags
#

ifdef DEBUG
	DEBUG_FLAGS := $(DEBUG_FLAGS) -ggdb -D_DEBUG
	CFLAGS      := $(CFLAGS)
	CXXFLAGS    := $(CXXFLAGS)
else
	DEBUG_FLAGS := $(DEBUG_FLAGS) -g -DNDEBUG
	CFLAGS      := $(CFLAGS)   -O3
	CXXFLAGS    := $(CXXFLAGS) -O3
endif

CFLAGS   := -Wall $(CFLAGS)
CXXFLAGS := -Wall -Wno-deprecated $(CXXFLAGS)


#
# Platform-specific settings for shared libraries
#

ifeq ($(OSTYPE),darwin)
	SONAME_OPTION := -WL,-install_name,
	SHARED_OPTION := -dynamiclib
	SOLIBRARY_EXT := dylib
else
	SONAME_OPTION := -Wl,-soname,
	SHARED_OPTION := -shared
	SOLIBRARY_EXT := so
endif


#
# Profiling
#

ifdef GPROF
	DEBUG_FLAGS := $(DEBUG_FLAGS) -pg
	MAKE := $(MAKE) GPROF=yes
	BUILD_DIR := $(BUILD_DIR).gprof
endif


#
# Cygwin & Microsoft Visual Studio (using Cygwin build system)
#

ifeq ($(shell echo $$TERM),cygwin)
	BUILD_DIR := $(BUILD_DIR).cygwin
	EXECUTABLE_SUFFIX :=.exe

	#GCC_VERSION :=$(shell gcc -v 2>&1 | grep "gcc version" | awk '{ print $$3 }')
	#INCLUDE_FLAGS := $(INCLUDE_FLAGS) -I/lib/gcc/$(shell echo $$MACHTYPE)/$(GCC_VERSION)/include/c++/ext
	#INCLUDE_FLAGS := $(INCLUDE_FLAGS) -I/usr/lib/gcc/$(shell echo $$MACHTYPE)/$(GCC_VERSION)/include/c++/ext
endif

ifneq ($(shell echo ++$$VS_UNICODE_OUTPUT),++)
	VISUAL_STUDIO := yes
	NO_COLOR := yes
endif


#
# 32 Bits
#

ifeq ($(BITS),32)
	BUILD_DIR := $(BUILD_DIR).i386
endif


#
# Color
#

ifeq ($(OSTYPE),darwin)
	NO_COLOR := yes
endif

ifndef NO_COLOR
	COLOR_UNDERLINE :='\E[4m'
	COLOR_NORMAL :='\E[0;0m'

	COLOR_MAKE := '\E[0;36m'
	COLOR_MAKE_PATH := '\E[1;36m'
endif

NMAKE := make $(wordlist 2, $(words $(MAKE)), $(MAKE))

ifndef NO_COLOR
	ifeq ($(shell which colormake 2>/dev/null | grep -c colormake),1)
		COLORMAKE := colormake $(wordlist 2, $(words $(MAKE)), $(MAKE))
	else
		COLORMAKE := $(MAKE)
	endif
else
	COLORMAKE := $(MAKE)
endif


#
# Install
#

ifndef INSTALL_PREFIX
	INSTALL_PREFIX := /usr
endif


#
# The shell variable that contains paths to the shared libraries
#

ifeq ($(OSTYPE),darwin)
	LD_PATH_VAR := DYLD_LIBRARY_PATH
else
	LD_PATH_VAR := LD_LIBRARY_PATH
endif


#
# Copying
#

ifeq ($(OSTYPE),darwin)
	CP_UPDATE_FLAG := 
else
	CP_UPDATE_FLAG := -u
endif


#
# Add the build dir suffix
#

BUILD_DIR_WITHOUT_SUFFIX := $(BUILD_DIR)
ifdef BUILD_DIR_SUFFIX
	BUILD_DIR := $(BUILD_DIR)$(BUILD_DIR_SUFFIX)
endif


#
# Finalize
#

endif
