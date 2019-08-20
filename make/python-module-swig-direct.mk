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
# The interface file
#

ifndef INTERFACE
    INTERFACE := $(shell ls -1 *.i | head -n 1)
endif


#
# Include the header script
#

include $(ROOT)/make/python-module-header.mk


#
# Compile Targets
#

WRAP_SOURCE := $(patsubst %.i,%_wrap.cxx,$(notdir $(INTERFACE)))
WRAP_OBJECT := $(patsubst %.cxx,%.o,$(WRAP_SOURCE))
WRAP_LIBRARY := _$(PROJECT).$(SOLIBRARY_EXT)
WRAP_CXX := $(CXX) $(CXXFLAGS) -fno-strict-aliasing $(INCLUDE_FLAGS) \
			-c -O3 -fPIC -I$(PYTHON_INCLUDE)
WRAP_LINK := $(CXX) $(LINKER_FLAGS) -L. -lpthread $(SHARED_OPTION)

PYTHON_INCLUDE="/usr/include/$(shell ls -1 /usr/include | grep python \
			   | grep -v '_d' | tail -n 1)"

$(BUILD_DIR)/$(PROJECT).py $(BUILD_DIR)/$(WRAP_SOURCE): \
		$(INTERFACE) $(DEPENDENCIES)
	@mkdir -p $(BUILD_DIR)
ifeq ($(OUTPUT_TYPE),kernel)
	@echo '  SWIG    $(PWD_REL_SEP)$<'
	@swig -c++ -python -outdir $(BUILD_DIR) -o $(BUILD_DIR)/$(WRAP_SOURCE) $<
else
	swig -c++ -python -outdir $(BUILD_DIR) -o $(BUILD_DIR)/$(WRAP_SOURCE) $<
endif

$(BUILD_DIR)/$(WRAP_OBJECT): $(BUILD_DIR)/$(WRAP_SOURCE)
ifeq ($(OUTPUT_TYPE),kernel)
	@echo "  CXX     $(PWD_REL_SEP)$@"
	@$(WRAP_CXX) -o $@ $<
else
	$(WRAP_CXX) -o $@ $<
endif

# TODO Make the hard-coded "../../" to be dynamically computed

$(BUILD_DIR)/$(WRAP_LIBRARY): $(BUILD_DIR)/$(WRAP_OBJECT)
ifneq ($(SUBPROJECT_SO_FILES),)
	@cd $(BUILD_DIR); \
		for i in $(SUBPROJECT_SO_FILES); do \
			/bin/rm -f `basename $$i` || true; \
			ln -s ../../$$i .; \
		done
endif
ifeq ($(OUTPUT_TYPE),kernel)
	@echo "  LD      $(PWD_REL_SEP)$@"
	@cd $(BUILD_DIR) && $(WRAP_LINK) $(SONAME_OPTION)$(WRAP_LIBRARY) \
		-o $(WRAP_LIBRARY) $(WRAP_OBJECT) $(LIBRARIES)
else
	cd $(BUILD_DIR) && $(WRAP_LINK) $(SONAME_OPTION)$(WRAP_LIBRARY) \
		-o $(WRAP_LIBRARY) $(WRAP_OBJECT) $(LIBRARIES)
endif

$(BUILD_DIR)/$(TARGET): $(BUILD_DIR)/$(WRAP_LIBRARY) \
								 $(BUILD_DIR)/$(PROJECT).py
ifeq ($(OUTPUT_TYPE),kernel)
	@echo '  COMPILE $(PWD_REL_SEP)$@'
	@cd $(BUILD_DIR) && python -m py_compile $(PROJECT).py
else
	cd $(BUILD_DIR) && python -m py_compile $(PROJECT).py
endif

