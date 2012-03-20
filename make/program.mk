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
		SUBPROJECT_SO_FILES :=
	else
		SUBPROJECT_FILES := $(shell unset MAKEFLAGS MFLAGS && $(MAKE) \
						--no-print-directory --quiet \
	                                        -C $(ROOT) list-subproject-lib-files)
		SUBPROJECT_FILES := $(foreach proj, $(SUBPROJECT_FILES), $(ROOT)/$(proj))

		SUBPROJECT_SO_FILES := $(shell unset MAKEFLAGS MFLAGS && $(MAKE) \
						--no-print-directory --quiet \
	                        -C $(ROOT) list-subproject-shared-lib-files)
		SUBPROJECT_SO_FILES := $(foreach proj, $(SUBPROJECT_SO_FILES), $(ROOT)/$(proj))
	endif
endif

DEPENDENCIES := $(DEPENDENCIES) $(SUBPROJECT_FILES)
LIBRARIES    := $(SUBPROJECT_FILES) $(LIBRARIES)


#
# Include the compile script
#

include $(ROOT)/make/compile.mk


#
# The real paths
#

EXECUTABLE := "$(realpath $(BUILD_DIR)/$(TARGET))"
REAL_BUILD_DIR := "$(realpath $(BUILD_DIR))"


#
# Phony targets
#

.PHONY: pre-run run run-dev time time-dev

pre-run::

run: all pre-run
	@cd "$(RUN_DIR)" && $(LD_PATH_VAR)=$$$(LD_PATH_VAR):$(REAL_BUILD_DIR) \
	$(EXECUTABLE)

run-dev: all pre-run
	@cd "$(RUN_DIR)" && $(LD_PATH_VAR)=$$$(LD_PATH_VAR):$(REAL_BUILD_DIR) \
	$(EXECUTABLE) $(RUN_DEV_ARGS)

time: all pre-run
	@cd "$(RUN_DIR)" && $(LD_PATH_VAR)=$$$(LD_PATH_VAR):$(REAL_BUILD_DIR) \
	/usr/bin/time -v $(EXECUTABLE)

time-dev: all pre-run
	@cd "$(RUN_DIR)" && $(LD_PATH_VAR)=$$$(LD_PATH_VAR):$(REAL_BUILD_DIR) \
	/usr/bin/time -v $(EXECUTABLE) $(RUN_DEV_ARGS)


#
# Shortcut targets
#

.PHONY: release-run

release-run:
	@$(MAKE) --no-print-directory RELEASE=yes run


#
# Debugging
#

.PHONY: gdb valgrind vg vg-dev vg-all vg-dev-all

gdb: all pre-run
	@cd "$(RUN_DIR)" && $(LD_PATH_VAR)=$$$(LD_PATH_VAR):$(REAL_BUILD_DIR) \
	gdb $(EXECUTABLE)

vg valgrind: all pre-run
	@cd "$(RUN_DIR)" && $(LD_PATH_VAR)=$$$(LD_PATH_VAR):$(REAL_BUILD_DIR) \
	valgrind --tool=memcheck --leak-check=yes --num-callers=24 \
	          $(EXECUTABLE)

vg-dev: all pre-run
	@cd "$(RUN_DIR)" && $(LD_PATH_VAR)=$$$(LD_PATH_VAR):$(REAL_BUILD_DIR) \
	valgrind --tool=memcheck --leak-check=yes --num-callers=24 \
	          $(EXECUTABLE) $(RUN_DEV_ARGS)

vg-all: all pre-run
	@cd "$(RUN_DIR)" && $(LD_PATH_VAR)=$$$(LD_PATH_VAR):$(REAL_BUILD_DIR) \
	valgrind --tool=memcheck --leak-check=yes --num-callers=24 \
	          --show-reachable=yes --track-origins=yes \
	          $(EXECUTABLE) $(RUN_VG_ARGS)

vg-dev-all: all pre-run
	@cd "$(RUN_DIR)" && $(LD_PATH_VAR)=$$$(LD_PATH_VAR):$(REAL_BUILD_DIR) \
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

# TODO Make the hard-coded "../../" to be dynamically computed

$(BUILD_DIR)/$(TARGET): $(BUILD_DIR)/project.mk $(OBJECTS) $(PROG_OBJECTS)
	@mkdir -p $(BUILD_DIR)
ifneq ($(SUBPROJECT_SO_FILES),)
	@cd $(BUILD_DIR); \
		for i in $(SUBPROJECT_SO_FILES); do \
			/bin/rm -f `basename $$i` || true; \
			ln -s ../../$$i .; \
		done
endif
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


#
# Install & unistall
#

.PHONY: install uninstall

ifdef INSTALL
#INSTALL_DEPENDENCIES := $(INSTALL_DEPENDENCIES) $(BUILD_DIR)/$(TARGET)
INSTALL_DIR := $(INSTALL_PREFIX)/bin
else
INSTALL_DEPENDENCIES :=
endif

install:: release $(INSTALL_DEPENDENCIES)
ifdef INSTALL
ifdef RELEASE
	@mkdir -p $(INSTALL_DIR)
ifeq ($(OUTPUT_TYPE),kernel)
	@echo '  INSTALL $(PWD_REL_SEP)$(BUILD_DIR)/$(TARGET)'
	@install -m 755 $(BUILD_DIR)/$(TARGET) $(INSTALL_DIR)
else
	install -m 755 $(BUILD_DIR)/$(TARGET) $(INSTALL_DIR)
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

