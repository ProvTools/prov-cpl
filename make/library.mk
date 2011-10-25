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
# Configure the project
#

ifndef PROJECT
	PROJECT := $(strip $(notdir $(shell "pwd")))
endif

ifndef TARGET
	TARGET := lib$(PROJECT).a
endif


#
# Include the compile script
#

include $(ROOT)/make/compile.mk


#
# Linker targets
#

$(BUILD_DIR)/$(TARGET): $(BUILD_DIR)/project.mk $(OBJECTS)
	@mkdir -p $(BUILD_DIR)
	@rm -f $@ 2> /dev/null || true
ifeq ($(OUTPUT_TYPE),kernel)
	@echo '  AR      $(PWD_REL_SEP)$@'
	@ar cq $@ $(OBJECTS)
	@ranlib $@
else
	ar cq $@ $(OBJECTS)
	ranlib $@
endif


#
# Special targets
#

list-subproject-libs::
	@echo $(PROJECT)

list-subproject-lib-files::
	@echo $(BUILD_DIR)/$(TARGET)
