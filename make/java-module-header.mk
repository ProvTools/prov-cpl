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
					$(JAVA_MODULE_DEPENDECIES)
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

ifdef JAVA_MODULE_DEPENDECIES
	EXTERNAL_JARS := $(EXTERNAL_JARS) \
		$(foreach pdir, \
			$(JAVA_MODULE_DEPENDECIES), \
			$(foreach jar, \
				$(shell unset MAKEFLAGS MFLAGS && \
					$(MAKE) --no-print-directory --quiet -C $(pdir) \
					list-project-jars), \
				$(pdir)/$(jar)))
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
		list-subproject-shared-lib-files list-subproject-libs \
		list-project-jars

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

list-subproject-libs::

list-project-jars::
	@echo $(BUILD_DIR)/$(TARGET)


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
	@echo "  JAVAC   $(PWD_REL_SEP)$(TEST_CLASS_FILE)"
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

.PHONY: install uninstall maven-install maven-uninstall

ifdef INSTALL
INSTALL_DIR := $(INSTALL_PREFIX)/java
else
ifndef MAVEN_INSTALL
INSTALL_DEPENDENCIES :=
endif
endif

ifdef MAVEN_INSTALL
ifndef MAVEN_GROUP_ID
	MAVEN_GROUP_ID := $(shell whoami | tr A-Z a-z)
endif
ifndef MAVEN_ARTIFACT_ID
	MAVEN_ARTIFACT_ID := $(shell echo $(PROJECT) | tr A-Z a-z)
endif
ifndef MAVEN_VERSION
ifndef PROJECT_VERSION
	MAVEN_VERSION := 1.00
else
	MAVEN_VERSION := $(PROJECT_VERSION)
endif
endif
ifndef MAVEN_PACKAGING
	MAVEN_PACKAGING := jar
endif
endif

install:: release $(INSTALL_DEPENDENCIES)
ifdef INSTALL
	@mkdir -p $(INSTALL_DIR)
ifdef RELEASE
ifeq ($(OUTPUT_TYPE),kernel)
	@echo '  INSTALL $(PWD_REL_SEP)$(BUILD_DIR)/$(TARGET)'
	@install -m 644 $(BUILD_DIR)/$(TARGET) $(INSTALL_DIR)
else
	install -m 644 $(BUILD_DIR)/$(TARGET) $(INSTALL_DIR)
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

maven-install:: release $(INSTALL_DEPENDENCIES)
ifdef MAVEN_INSTALL
ifeq ($(OUTPUT_TYPE),kernel)
	@echo '  MVNINST $(PWD_REL_SEP)$(BUILD_DIR)/$(TARGET)'
	@mvn install:install-file -Dfile=$(BUILD_DIR)/$(TARGET) \
		-DgroupId=$(MAVEN_GROUP_ID) \
		-DartifactId=$(MAVEN_ARTIFACT_ID) \
		-Dversion=$(MAVEN_VERSION) \
		-Dpackaging=$(MAVEN_PACKAGING)
else
	mvn install:install-file -Dfile=$(BUILD_DIR)/$(TARGET) \
		-DgroupId=$(MAVEN_GROUP_ID) \
		-DartifactId=$(MAVEN_ARTIFACT_ID) \
		-Dversion=$(MAVEN_VERSION) \
		-Dpackaging=$(MAVEN_PACKAGING)
endif
else
	@true
endif

maven-uninstall::
ifdef MAVEN_INSTALL
ifeq ($(OUTPUT_TYPE),kernel)
	@echo '  MVNUNIN $(MAVEN_GROUP_ID).$(MAVEN_ARTIFACT_ID) ver. $(MAVEN_VERSION)'
	@mvn uninstall:artifact -DgroupId=$(MAVEN_GROUP_ID) \
		-DartifactId=$(MAVEN_ARTIFACT_ID) \
		-Dversion=$(MAVEN_VERSION)
else
	@mvn uninstall:artifact -DgroupId=$(MAVEN_GROUP_ID) \
		-DartifactId=$(MAVEN_ARTIFACT_ID) \
		-Dversion=$(MAVEN_VERSION)
endif
else
	@true
endif

