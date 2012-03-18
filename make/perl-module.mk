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
# Module source directory
#

ifndef MODULE_SOURCE_DIR
	MODULE_SOURCE_DIR := module
endif


#
# Build dir
#

BUILD_DIR_SUFFIX := /$(strip $(notdir $(MODULE_SOURCE_DIR)))


#
# Include the header script
#

include $(ROOT)/make/perl-module-header.mk


#
# Module files
#

BUILD_DIR_PURE := $(BUILD_DIR_WITHOUT_SUFFIX)
MODULE_FILES := $(filter-out %.swp %~, \
							 $(shell find $(MODULE_SOURCE_DIR) -type f))
MODULE_FILES_IN_BUILD_DIR := $(addprefix $(BUILD_DIR_PURE)/,$(MODULE_FILES))


#
# Compile Targets
#

$(BUILD_DIR)/%: $(MODULE_SOURCE_DIR)/%
	@mkdir -p $(dir $@)
ifeq ($(OUTPUT_TYPE),kernel)
	@echo '  COPY    $(PWD_REL_SEP)$<'
	@/bin/cp -fp $(CP_UPDATE_FLAG) $< $@
else
	/bin/cp -fp $(CP_UPDATE_FLAG) $< $@
endif

# TODO Make the hard-coded "../../" to be dynamically computed

$(BUILD_DIR)/Makefile: $(BUILD_DIR)/Makefile.PL \
					   $(MODULE_FILES_IN_BUILD_DIR)
ifneq ($(SUBPROJECT_SO_FILES),)
	@cd $(BUILD_DIR); \
		for i in $(SUBPROJECT_SO_FILES); do \
			/bin/rm -f `basename $$i` || true; \
			ln -s ../../../$$i .; \
		done
endif
ifeq ($(OUTPUT_TYPE),kernel)
	@echo '  PERL    $(PWD_REL_SEP)$<'
	@cd "$(BUILD_DIR)" && $(PERL) Makefile.PL
else
	cd "$(BUILD_DIR)" && $(PERL) Makefile.PL
endif

$(BUILD_DIR)/blib/lib/$(TARGET): $(BUILD_DIR)/Makefile \
								 $(MODULE_FILES_IN_BUILD_DIR)
ifeq ($(OUTPUT_TYPE),kernel)
	@echo '  MAKE    $(PWD_REL_SEP)$(BUILD_DIR)'
	@make --no-print-directory -C "$(BUILD_DIR)"
else
	make -C "$(BUILD_DIR)"
endif
	@touch $@

