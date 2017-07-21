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
# Configure the project
#

ifndef PROJECT
	PROJECT := $(strip $(notdir $(shell "pwd")))
endif

ifndef TARGET
ifdef SHARED
	TARGET := lib$(PROJECT).$(SOLIBRARY_EXT)
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
	TARGET_WITH_MAJ_VER := lib$(PROJECT).$(SO_MAJOR_VERSION).$(SOLIBRARY_EXT)
	TARGET_WITH_VER := lib$(PROJECT).$(SO_MAJOR_VERSION).$(SO_MINOR_VERSION).$(SOLIBRARY_EXT)
endif

ifdef INSTALL
#INSTALL_DEPENDENCIES := $(INSTALL_DEPENDENCIES) $(BUILD_DIR)/$(TARGET)
	INSTALL_DIR := $(INSTALL_PREFIX)/$(LIB_DIR)
else
	INSTALL_DEPENDENCIES :=
endif


#
# Include the compile script
#

include $(ROOT)/make/compile.mk


#
# Linker targets
#

# NOTE The shared library target for Darwin currently depends on the ability
# to write to the source code directory, because otherwise the logical library
# names would include the name of the build directory (which can be made to
# work, but it is ugly). Alternatively, we can cd into the build directory
# and run the build there - if we can figrue out how to correctly modify all
# paths used by the linker command - or if we figure out a better solution
# that would not depend on the paths being rewritten.

$(BUILD_DIR)/$(TARGET): $(BUILD_DIR)/project.mk $(OBJECTS)
	@mkdir -p $(BUILD_DIR)
	@rm -f $@ 2> /dev/null || true
#
# ---------- Shared Library ----------
#
ifdef SHARED
	@rm -f $(BUILD_DIR)/$(TARGET) 2> /dev/null || true
	@rm -f $(BUILD_DIR)/$(TARGET_WITH_MAJ_VER) 2> /dev/null || true
	@rm -f $(BUILD_DIR)/$(TARGET_WITH_VER) 2> /dev/null || true
#
# ----- Shared Library: Kernel Output Type -----
#
ifeq ($(OUTPUT_TYPE),kernel)
	@echo '  LD      $(PWD_REL_SEP)$@'
ifeq ($(OSTYPE),darwin)
	@$(LINK) $(SHARED_OPTION) $(SONAME_OPTION)$(INSTALL_DIR)/$(TARGET_WITH_MAJ_VER) \
		-o $(TARGET_WITH_VER) $(OBJECTS) $(LIBRARIES)
	@/bin/mv -f $(TARGET_WITH_VER) $(BUILD_DIR)/$(TARGET_WITH_VER) \
		|| (/bin/rm -f $(TARGET_WITH_VER) ; false)
else
	@$(LINK) $(SHARED_OPTION) $(SONAME_OPTION)$(TARGET_WITH_MAJ_VER) \
		-o $(BUILD_DIR)/$(TARGET_WITH_VER) $(OBJECTS) $(LIBRARIES)
endif
	@cd $(BUILD_DIR) && ln -s $(TARGET_WITH_VER) $(TARGET_WITH_MAJ_VER)
	@cd $(BUILD_DIR) && ln -s $(TARGET_WITH_VER) $(TARGET)
#
# ----- Shared Library: Normal Output Type -----
#
else
ifeq ($(OSTYPE),darwin)
	$(LINK) $(SHARED_OPTION) $(SONAME_OPTION)$(INSTALL_DIR)/$(TARGET_WITH_MAJ_VER) \
		-o $(TARGET_WITH_VER) $(OBJECTS) $(LIBRARIES)
	/bin/mv -f $(TARGET_WITH_VER) $(BUILD_DIR)/$(TARGET_WITH_VER) \
		|| (/bin/rm -f $(TARGET_WITH_VER) ; false)
else
	#TODO possibly add $(INSTALL_DIR)
	$(LINK) $(SHARED_OPTION) $(SONAME_OPTION)$(TARGET_WITH_MAJ_VER) \
		-o $(BUILD_DIR)/$(TARGET_WITH_VER) $(OBJECTS) $(LIBRARIES)
endif
	cd $(BUILD_DIR) && ln -s $(TARGET_WITH_VER) $(TARGET_WITH_MAJ_VER)
	cd $(BUILD_DIR) && ln -s $(TARGET_WITH_VER) $(TARGET)
endif
#
# ---------- Static Library ----------
#
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
	@echo $(BUILD_DIR)/$(TARGET_WITH_MAJ_VER)
	@echo $(BUILD_DIR)/$(TARGET)
else
	@true
endif

list-subproject-lib-build-dirs::
	@echo $(BUILD_DIR)


#
# Install & unistall
#

.PHONY: install uninstall

ifdef SHARED
INSTALL_PERM := 755
else
INSTALL_PERM := 644
endif

install:: release $(INSTALL_DEPENDENCIES)
ifdef INSTALL
	@mkdir -p $(INSTALL_DIR)
ifdef RELEASE
ifdef SHARED
	@rm -f $(INSTALL_DIR)/$(TARGET_WITH_VER) 2> /dev/null || true
ifeq ($(OUTPUT_TYPE),kernel)
	@echo '  INSTALL $(PWD_REL_SEP)$(BUILD_DIR)/$(TARGET_WITH_VER)'
	@install -m $(INSTALL_PERM) $(BUILD_DIR)/$(TARGET_WITH_VER) $(INSTALL_DIR)
else
	install -m $(INSTALL_PERM) $(BUILD_DIR)/$(TARGET_WITH_VER) $(INSTALL_DIR)
endif
	@rm -f $(INSTALL_DIR)/$(TARGET) 2> /dev/null || true
	@rm -f $(INSTALL_DIR)/$(TARGET_WITH_MAJ_VER) 2> /dev/null || true
ifeq ($(OUTPUT_TYPE),kernel)
	@echo '  INSTALL $(PWD_REL_SEP)$(BUILD_DIR)/$(TARGET_WITH_MAJ_VER)'
	@echo '  INSTALL $(PWD_REL_SEP)$(BUILD_DIR)/$(TARGET)'
	@cd $(INSTALL_DIR) && ln -s $(TARGET_WITH_VER) $(TARGET_WITH_MAJ_VER)
	@cd $(INSTALL_DIR) && ln -s $(TARGET_WITH_VER) $(TARGET)
else
	cd $(INSTALL_DIR) && ln -s $(TARGET_WITH_VER) $(TARGET_WITH_MAJ_VER)
	cd $(INSTALL_DIR) && ln -s $(TARGET_WITH_VER) $(TARGET)
endif
else
ifeq ($(OUTPUT_TYPE),kernel)
	@echo '  INSTALL $(PWD_REL_SEP)$(BUILD_DIR)/$(TARGET)'
	@install -m $(INSTALL_PERM) $(BUILD_DIR)/$(TARGET) $(INSTALL_DIR)
else
	install -m $(INSTALL_PERM) $(BUILD_DIR)/$(TARGET) $(INSTALL_DIR)
endif
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
ifdef SHARED
ifeq ($(OUTPUT_TYPE),kernel)
	@echo '  DELETE  $(INSTALL_DIR)/$(TARGET_WITH_MAJ_VER)'
	@/bin/rm -f $(INSTALL_DIR)/$(TARGET_WITH_MAJ_VER)
	@echo '  DELETE  $(INSTALL_DIR)/$(TARGET_WITH_VER)'
	@/bin/rm -f $(INSTALL_DIR)/$(TARGET_WITH_VER)
else
	/bin/rm -f $(INSTALL_DIR)/$(TARGET_WITH_VER)
endif
endif
else
	@true
endif

