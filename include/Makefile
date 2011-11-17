#
# Core Provenance Library
#
# Copyright (c) Peter Macko
#

ROOT :=..


#
# Customize the configuration
#

INSTALL := yes


#
# Get the appropriate header files
#

HEADER_FILES := $(shell find . \( -name '*.h' -or -name '*.hpp' \) -print \
							| fgrep -v '/private/' \
							| sed 's/^\.\///')


#
# Include the magic script
#

include ../make/include.mk

