#
# Core Provenance Library
#
# Copyright (c) Peter Macko
#

ROOT := .


#
# Subprojects
#

LIBRARIES := cpl-standalone backends
PROGRAMS := test


#
# Include the magic script
#

include $(ROOT)/make/project.mk

