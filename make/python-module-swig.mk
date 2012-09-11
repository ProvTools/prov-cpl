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

# Reference:
#   http://www.swig.org/Doc1.3/Python.html#Python_nn6

WRAP_SOURCE := $(patsubst %.i,%_wrap.cxx,$(notdir $(INTERFACE)))
WRAP_LIBRARY := _$(PROJECT).$(SOLIBRARY_EXT)

$(BUILD_DIR)/$(PROJECT).py $(BUILD_DIR)/$(WRAP_SOURCE): \
		$(INTERFACE) $(DEPENDENCIES)
	@mkdir -p $(BUILD_DIR)
ifeq ($(OUTPUT_TYPE),kernel)
	@echo '  SWIG    $(PWD_REL_SEP)$<'
	@swig -c++ -python -outdir $(BUILD_DIR) -o $(BUILD_DIR)/$(WRAP_SOURCE) $<
else
	swig -c++ -python -outdir $(BUILD_DIR) -o $(BUILD_DIR)/$(WRAP_SOURCE) $<
endif

# TODO Make the hard-coded "../../" to be dynamically computed

$(BUILD_DIR)/$(WRAP_LIBRARY): \
		$(BUILD_DIR)/$(PROJECT).py $(BUILD_DIR)/$(WRAP_SOURCE) setup.py
	@mkdir -p $(BUILD_DIR)
ifneq ($(SUBPROJECT_SO_FILES),)
	@cd $(BUILD_DIR); \
		for i in $(SUBPROJECT_SO_FILES); do \
			/bin/rm -f `basename $$i` || true; \
			ln -s ../../$$i .; \
		done
endif
ifeq ($(OUTPUT_TYPE),kernel)
	@echo '  PYTHON  $(PWD_REL_SEP)setup.py'
	@cd $(BUILD_DIR) && python $(abspath setup.py) build
	@cd $(BUILD_DIR) \
		&& /bin/cp -f $(CP_UPDATE_FLAG) "`find build -name $(WRAP_LIBRARY)`" .
else
	cd $(BUILD_DIR) && python $(abspath setup.py) build
	cd $(BUILD_DIR) \
		&& /bin/cp -f $(CP_UPDATE_FLAG) "`find build -name $(WRAP_LIBRARY)`" .
endif

$(BUILD_DIR)/$(TARGET): $(BUILD_DIR)/$(WRAP_LIBRARY) \
								 $(BUILD_DIR)/$(PROJECT).py
ifeq ($(OUTPUT_TYPE),kernel)
	@echo '  COMPILE $(PWD_REL_SEP)$@'
	@cd $(BUILD_DIR) && python -m py_compile $(PROJECT).py
else
	cd $(BUILD_DIR) && python -m py_compile $(PROJECT).py
endif

