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

include $(ROOT)/make/python-module-header.mk


#
# Compile Targets
#

$(BUILD_DIR)/$(PROJECT).py: $(PROJECT).py
	@mkdir -p $(BUILD_DIR)
ifneq ($(SUBPROJECT_SO_FILES),)
	@cd $(BUILD_DIR); \
		for i in $(SUBPROJECT_SO_FILES); do \
			/bin/rm -f `basename $$i` || true; \
			ln -s ../../$$i .; \
		done
endif
ifeq ($(OUTPUT_TYPE),kernel)
	@echo "  COPY    $(PWD_REL_SEP)$<"
	@/bin/cp -f $(CP_UPDATE_FLAG) $< $@
else
	/bin/cp -f $(CP_UPDATE_FLAG) $< $@
endif
ifeq ($(OUTPUT_TYPE),kernel)
	@echo '  PYTHON  $(PWD_REL_SEP)setup.py'
	@cd $(BUILD_DIR) && python $(abspath setup.py) build
else
	cd $(BUILD_DIR) && python $(abspath setup.py) build
endif

$(BUILD_DIR)/$(TARGET): $(BUILD_DIR)/$(PROJECT).py
ifeq ($(OUTPUT_TYPE),kernel)
	@echo '  COMPILE $(PWD_REL_SEP)$@'
	@cd $(BUILD_DIR) && python -m py_compile $(PROJECT).py
else
	cd $(BUILD_DIR) && python -m py_compile $(PROJECT).py
endif

