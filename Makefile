#
# Core Provenance Library
#
# Copyright (c) Peter Macko
#

ROOT := .

#TODO add test
#
# Subprojects
#

LIBRARIES := cpl-standalone backends
HEADERS := include
PROGRAMS :=

#TODO add test back in when that exists
#
# Project configuration
#

RUN_PROGRAM :=


#
# Include the magic script
#

include $(ROOT)/make/project.mk
