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
# Include the header script
#

include $(ROOT)/make/header.mk


#
# Perl
#

ifndef PERL
	PERL := perl
endif


#
# Target
#

ifndef TARGET
	TARGET := $(PROJECT).jar
endif


#
# Handle subproject libraries
#

ifndef PROJECT_DIRS
	PROJECT_DIRS := $(ROOT) $(ADDITIONAL_PROJECT_DIRS) \
					$(PERL_MODULE_DEPENDECIES)
endif

ifdef __PROGRAM__NO_SUBPROJECT_FILES
	SUBPROJECT_SO_FILES :=
else
	SUBPROJECT_SO_FILES := \
		$(foreach pdir, \
			$(PROJECT_DIRS), \
			$(foreach proj, \
				$(shell unset MAKEFLAGS MFLAGS && $(MAKE) --no-print-directory \
					--quiet -C $(pdir) list-subproject-shared-lib-files), \
				$(pdir)/$(proj)))
endif

ifdef PERL_MODULE_DEPENDECIES
	PERL := $(PERL) \
		$(foreach pdir, \
			$(PERL_MODULE_DEPENDECIES), \
			$(foreach incdir, \
				$(shell unset MAKEFLAGS MFLAGS && \
					$(MAKE) --no-print-directory --quiet -C $(pdir) \
					list-perl-inc-dirs), \
				-I $(realpath $(pdir)/$(incdir))))
endif


#
# Java settings
#

ifndef JAVA_HOME
	JAVA_INCLUDE := $(shell dirname `locate include/jni.h | head -n 1`)
	JAVA_HOME := $(shell dirname "$(JAVA_INCLUDE)")
else
	JAVA_INCLUDE := $(JAVA_HOME)/include
endif


#
# Java executables
#

ifndef JAVA
	JAVA := java
endif

ifndef JAVAC
	JAVAC := javac
endif

ifndef JAR
	JAR := jar
endif


#
# Build-related phony and preliminary targets
#

.PHONY: all clean distclean messclean

all: $(BUILD_DIR)/$(TARGET)

clean:: messclean
	@rm -rf $(BUILD_DIR) || true

distclean:: messclean
	@rm -rf $(BUILD_DIR_ALL) || true

messclean::
	@find . -name '*~' -delete 2> /dev/null || true

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)


#
# Shortcut targets
#

.PHONY: release Release debug Debug

debug: all

release:
	@$(MAKE) --no-print-directory RELEASE=yes all


# Aliases

Debug: debug

Release: release


#
# Special-purpose targets
#

.PHONY: lines todo list-subproject-lib-files \
		list-subproject-shared-lib-files list-subproject-libs

lines:
	@echo Total number of lines:
	@cat Makefile *.i *.cxx *.pl *.PL *.cpp *.c *.h *.hpp | wc -l

todo:
	@(grep -n 'TODO' $(ALL_SOURCES) $(shell ls *.h 2>/dev/null) Makefile ;	\
	  grep -n '###'  $(ALL_SOURCES) $(shell ls *.h 2>/dev/null) Makefile ;	\
	  grep -n 'XXX'  $(ALL_SOURCES) $(shell ls *.h 2>/dev/null) Makefile )	\
		| sed 's/\(^.*:.*:\).*TODO/\1\tTODO/'				\
		| sed 's/\(^.*:.*:\).*###/\1\t###/'				\
		| sed 's/\(^.*:.*:\).*XXX/\1\tXXX/'				\
		| sed 's|\(^.\)|$(PWD_REL_SEP)\1|' | sort

list-subproject-lib-files::

list-subproject-shared-lib-files::
	@echo $(BUILD_DIR)/blib/arch/auto/$(PROJECT)/$(PROJECT).so

list-subproject-libs::


#
# The library test target
#

.PHONY: test

ifndef RUN_DIR
RUN_DIR=.
endif

ABS_BUILD_DIR := $(abspath $(BUILD_DIR))
EXTRA_LD_PATHS := $(ABS_BUILD_DIR)

ifndef TEST
	TEST := test.java
endif
TEST_CLASS_FILE := $(patsubst %.java,%.class,$(TEST))
TEST_CLASS_NAME := $(patsubst %.java,%,$(TEST))
TEST_CLASSPATH  := $(ABS_BUILD_DIR)/$(PROJECT).jar

$(BUILD_DIR)/$(TEST_CLASS_FILE): $(TEST)
ifeq ($(OUTPUT_TYPE),kernel)
	@/bin/cp -fu $< $(BUILD_DIR)/
else
	/bin/cp -fu $< $(BUILD_DIR)/
endif
ifeq ($(OUTPUT_TYPE),kernel)
	@echo "  JAVAC   $(PWD_REL_SEP)$(BUILD_DIR)/$(JAVA_PACKAGE_DIR)/*.java"
	@cd $(BUILD_DIR) && $(JAVAC) -classpath $(TEST_CLASSPATH) `basename $<`
else
	@cd $(BUILD_DIR) && $(JAVAC) -classpath $(TEST_CLASSPATH) `basename $<`
endif

test: all $(BUILD_DIR)/$(TEST_CLASS_FILE)
	@cd "$(BUILD_DIR)" && $(LD_PATH_VAR)=$$$(LD_PATH_VAR):$(EXTRA_LD_PATHS) \
		$(JAVA) -classpath $(TEST_CLASSPATH):. $(TEST_CLASS_NAME)


#
# Install & unistall
#

.PHONY: install uninstall

ifdef INSTALL
INSTALL_DIR := $(INSTALL_PREFIX)/java
else
INSTALL_DEPENDENCIES :=
endif

install:: release $(INSTALL_DEPENDENCIES)
ifdef INSTALL
	@mkdir -p $(INSTALL_DIR)
ifdef RELEASE
ifeq ($(OUTPUT_TYPE),kernel)
	@echo '  INSTALL $(PWD_REL_SEP)$(BUILD_DIR)/$(TARGET)'
	@install -D -m 644 -t $(INSTALL_DIR) $(BUILD_DIR)/$(TARGET)
else
	install -D -m 644 -t $(INSTALL_DIR) $(BUILD_DIR)/$(TARGET)
endif
else
	@$(MAKE) --no-print-directory RELEASE=yes install
endif
else
	@true
endif

uninstall::
ifdef INSTALL
ifeq ($(OUTPUT_TYPE),kernel)
	@echo '  DELETE  $(INSTALL_DIR)/$(TARGET)'
	@/bin/rm -f $(INSTALL_DIR)/$(TARGET)
else
	/bin/rm -f $(INSTALL_DIR)/$(TARGET)
endif
else
	@true
endif

