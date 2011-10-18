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
# Include the header file
#

include $(ROOT)/make/header.mk


#
# Configure the project
#

ifndef PROJECT
	PROJECT := $(strip $(notdir $(PWD)))
endif

ifndef TARGET
	TARGET := $(PROJECT)
endif

TARGET := $(TARGET)$(EXECUTABLE_SUFFIX)


#
# Handle subproject libraries
#

ifdef SUBPROJECTS
	SUBPROJECT_FILES := $(foreach proj, \
                                  $(SUBPROJECTS), \
                                  $(ROOT)/$(proj)/$(BUILD_DIR)/lib$(notdir $(proj)).a)
else
	ifdef __PROGRAM__NO_SUBPROJECT_FILES
		SUBPROJECT_FILES :=
	else
		SUBPROJECT_FILES := $(shell unset MAKEFLAGS MFLAGS && $(MAKE) \
						--no-print-directory --quiet \
	                                        -C $(ROOT) list-subproject-lib-files)
		SUBPROJECT_FILES := $(foreach proj, $(SUBPROJECT_FILES), $(ROOT)/$(proj))
	endif
endif

DEPENDENCIES := $(DEPENDENCIES) $(SUBPROJECT_FILES)
LIBRARIES    := $(SUBPROJECT_FILES) $(LIBRARIES)


#
# Include the compile script
#

include $(ROOT)/make/compile.mk


#
# The executable
#

EXECUTABLE := "$(realpath $(BUILD_DIR)/$(TARGET))"


#
# Phony targets
#

.PHONY: pre-run run run-dev time time-dev

pre-run::

run: all pre-run
	@cd "$(RUN_DIR)" && \
	$(EXECUTABLE)

run-dev: all pre-run
	@cd "$(RUN_DIR)" && \
	$(EXECUTABLE) $(RUN_DEV_ARGS)

time: all pre-run
	@cd "$(RUN_DIR)" && \
	/usr/bin/time -v $(EXECUTABLE)

time-dev: all pre-run
	@cd "$(RUN_DIR)" && \
	/usr/bin/time -v $(EXECUTABLE) $(RUN_DEV_ARGS)


#
# Shortcut targets
#

.PHONY: release release-run Release debug Debug

debug: all

release:
	@$(MAKE) --no-print-directory RELEASE=yes all

release-run:
	@$(MAKE) --no-print-directory RELEASE=yes run


# Aliases

Debug: debug

Release: release


#
# Debugging
#

.PHONY: gdb valgrind vg vg-dev vg-all vg-dev-all

gdb: all pre-run
	@cd "$(RUN_DIR)" && \
	gdb $(EXECUTABLE)

vg valgrind: all pre-run
	@cd "$(RUN_DIR)" && \
	valgrind --tool=memcheck --leak-check=yes --num-callers=24 \
	          $(EXECUTABLE)

vg-dev: all pre-run
	@cd "$(RUN_DIR)" && \
	valgrind --tool=memcheck --leak-check=yes --num-callers=24 \
	          $(EXECUTABLE) $(RUN_DEV_ARGS)

vg-all: all pre-run
	@cd "$(RUN_DIR)" && \
	valgrind --tool=memcheck --leak-check=yes --num-callers=24 \
	          --show-reachable=yes --track-origins=yes \
	          $(EXECUTABLE) $(RUN_VG_ARGS)

vg-dev-all: all pre-run
	@cd "$(RUN_DIR)" && \
	valgrind --tool=memcheck --leak-check=yes --num-callers=24 \
	          --show-reachable=yes --track-origins=yes \
	          $(EXECUTABLE) $(RUN_VG_ARGS) $(RUN_DEV_ARGS)


#
# Profiling
#

.PHONY: gprof_finish

gprof_finish:
	@cd "$(RUN_DIR)" && \
		gprof $(EXECUTABLE) > $(ROOT)/gprof.out && \
		rm -f gmon.out 2> /dev/null


#
# Linker targets
#

$(BUILD_DIR)/$(TARGET): $(BUILD_DIR)/project.mk $(OBJECTS) $(PROG_OBJECTS)
	@mkdir -p $(BUILD_DIR)
ifeq ($(OUTPUT_TYPE),kernel)
	@echo '  LD      $(PWD_REL_SEP)$@'
	@$(LINK) -o $@ $(OBJECTS) $(PROG_OBJECTS) $(LIBRARIES)
else
	$(LINK) -o $@ $(OBJECTS) $(PROG_OBJECTS) $(LIBRARIES)
endif


#
# Subproject targets
#

$(SUBPROJECT_FILES):
	make -C $(ROOT)/$(word 1,$(subst /$(BUILD_DIR)/lib, ,$(patsubst $(ROOT)/%,%,$@)))
