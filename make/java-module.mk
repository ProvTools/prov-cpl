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

include $(ROOT)/make/java-module-header.mk


#
# Java Setup
#

SOURCE_DIR := src
SOURCE_FILES := Manifest $(shell find $(SOURCE_DIR) -name '*.java')


#
# Ant Build System Setup and Delegation
# 

ANT_BUILD_XML_SED := sed 's|@PROJECT|$(PROJECT)|g'

$(BUILD_DIR)/build.xml: $(ROOT)/make/stubs/build.xml
	@mkdir -p $(BUILD_DIR)
ifeq ($(OUTPUT_TYPE),kernel)
	@echo "  STUB    $(PWD_REL_SEP)$@"
	@$(ANT_BUILD_XML_SED) < $< > $@
else
	$(ANT_BUILD_XML_SED) < $< > $@
endif

$(BUILD_DIR)/Manifest: Manifest
	@mkdir -p $(BUILD_DIR)
ifeq ($(OUTPUT_TYPE),kernel)
	@echo "  COPY    $(PWD_REL_SEP)$<"
	@/bin/cp -fu $< $@
else
	/bin/cp -fu $< $@
endif

# TODO Make the hard-coded "../../" to be dynamically computed

$(BUILD_DIR)/dist/$(TARGET): $(BUILD_DIR)/Manifest $(BUILD_DIR)/build.xml \
		$(SOURCE_FILES) $(EXTERNAL_JARS)
	@cd $(BUILD_DIR) && ln -fs ../../$(SOURCE_DIR) src
	@mkdir -p $(BUILD_DIR)/lib
ifneq ($(EXTERNAL_JARS),)
	@cd $(BUILD_DIR)/lib; \
		for i in $(EXTERNAL_JARS); do \
			/bin/rm -f `basename $$i` || true; \
			ln -s ../../../$$i .; \
		done
endif
ifeq ($(OUTPUT_TYPE),kernel)
	@echo "  ANT     $(PWD_REL)"
	@cd $(BUILD_DIR) && ant
else
	cd $(BUILD_DIR) && ant
endif



#
# Compile Targets
#

# TODO Make the hard-coded "../../" to be dynamically computed

$(BUILD_DIR)/$(PROJECT).jar: $(BUILD_DIR)/dist/$(TARGET)
ifneq ($(SUBPROJECT_SO_FILES),)
	@cd $(BUILD_DIR); \
		for i in $(SUBPROJECT_SO_FILES); do \
			/bin/rm -f `basename $$i` || true; \
			ln -s ../../$$i .; \
		done
endif
ifeq ($(OUTPUT_TYPE),kernel)
	@echo "  COPY    $(PWD_REL_SEP)$<"
	@/bin/cp -fu $< $@
else
	/bin/cp -fu $< $@
endif

