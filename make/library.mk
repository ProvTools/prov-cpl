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
ifdef SHARED
	TARGET := lib$(PROJECT).so
else
	TARGET := lib$(PROJECT).a
endif
endif


#
# Defaults for the shared libraries
#

ifndef SO_MAJOR_VERSION
	SO_MAJOR_VERSION := 1
endif

ifndef SO_MINOR_VERSION
	SO_MINOR_VERSION := 0
endif


#
# Configure the compile
#

ifdef SHARED
	SO_COMPILER_FLAGS := -fPIC
	CFLAGS := $(CFLAGS) $(SO_COMPILER_FLAGS)
	CXXFLAGS := $(CXXFLAGS) $(SO_COMPILER_FLAGS)
	BUILD_DIR_SUFFIX := $(BUILD_DIR_SUFFIX).PIC
	TARGET_WITH_VER := $(TARGET).$(SO_MAJOR_VERSION).$(SO_MINOR_VERSION)
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
ifdef SHARED
	@rm -f $(BUILD_DIR)/$(TARGET).$(SO_MAJOR_VERSION) 2> /dev/null || true
	@rm -f $(BUILD_DIR)/$(TARGET_WITH_VER) 2> /dev/null || true
ifeq ($(OUTPUT_TYPE),kernel)
	@echo '  LD      $(PWD_REL_SEP)$@'
	@gcc -shared -Wl,-soname,$(TARGET).$(SO_MAJOR_VERSION) \
		-o $(BUILD_DIR)/$(TARGET_WITH_VER) $(OBJECTS)
	@cd $(BUILD_DIR) && ln -s $(TARGET_WITH_VER) $(TARGET).$(SO_MAJOR_VERSION)
	@cd $(BUILD_DIR) && ln -s $(TARGET_WITH_VER) $(TARGET)
else
	gcc -shared -Wl,-soname,$(TARGET).$(SO_MAJOR_VERSION) \
		-o $(BUILD_DIR)/$(TARGET_WITH_VER) $(OBJECTS)
	cd $(BUILD_DIR) && ln -s $(TARGET_WITH_VER) $(TARGET).$(SO_MAJOR_VERSION)
	cd $(BUILD_DIR) && ln -s $(TARGET_WITH_VER) $(TARGET)
endif
else
ifeq ($(OUTPUT_TYPE),kernel)
	@echo '  AR      $(PWD_REL_SEP)$@'
	@ar cq $@ $(OBJECTS)
	@ranlib $@
else
	ar cq $@ $(OBJECTS)
	ranlib $@
endif
endif


#
# Special targets
#

list-subproject-libs::
	@echo $(PROJECT)

list-subproject-lib-files::
	@echo $(BUILD_DIR)/$(TARGET)

list-subproject-shared-lib-files::
ifdef SHARED
	@echo $(BUILD_DIR)/$(TARGET_WITH_VER)
	@echo $(BUILD_DIR)/$(TARGET).$(SO_MAJOR_VERSION)
	@echo $(BUILD_DIR)/$(TARGET)
else
	@true
endif

list-subproject-lib-build-dirs::
	@echo $(BUILD_DIR)

