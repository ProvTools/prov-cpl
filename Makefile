#
# Core Provenance Library
#
# Copyright (c) Peter Macko
#

ROOT := .


#
# Subprojects
#

LIBRARIES := cpl-standalone backends private-lib
HEADERS := include
PROGRAMS := tools test


#
# Project configuration
#

RUN_PROGRAM := test


#
# Include the magic script
#

include $(ROOT)/make/project.mk

