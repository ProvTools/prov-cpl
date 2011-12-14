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

include $(ROOT)/make/perl-module-header.mk


#
# Compile Targets
#

WRAP_SOURCE := $(patsubst %.i,%_wrap.cxx,$(notdir $(INTERFACE)))

$(BUILD_DIR)/$(PROJECT).pm $(BUILD_DIR)/$(WRAP_SOURCE): \
		$(INTERFACE) $(DEPENDENCIES)
	@mkdir -p $(BUILD_DIR)
ifeq ($(OUTPUT_TYPE),kernel)
	@echo '  SWIG    $(PWD_REL_SEP)$<'
	@swig -c++ -perl5 -outdir $(BUILD_DIR) -o $(BUILD_DIR)/$(WRAP_SOURCE) $<
else
	swig -c++ -perl5 -outdir $(BUILD_DIR) -o $(BUILD_DIR)/$(WRAP_SOURCE) $<
endif

$(BUILD_DIR)/Makefile.PL: makemake.pl
	@mkdir -p $(BUILD_DIR)
ifeq ($(OUTPUT_TYPE),kernel)
	@/bin/cp -f $< $@
else
	/bin/cp -f $< $@
endif

# TODO Make the hard-coded "../../" to be dynamically computed

$(BUILD_DIR)/Makefile: $(BUILD_DIR)/Makefile.PL
ifneq ($(SUBPROJECT_SO_FILES),)
	@cd $(BUILD_DIR); \
		for i in $(SUBPROJECT_SO_FILES); do \
			/bin/rm -f `basename $$i` || true; \
			ln -s ../../$$i .; \
		done
endif
ifeq ($(OUTPUT_TYPE),kernel)
	@echo '  PERL    $(PWD_REL_SEP)$<'
	@cd "$(BUILD_DIR)" && perl Makefile.PL
else
	cd "$(BUILD_DIR)" && perl Makefile.PL
endif

$(BUILD_DIR)/blib/lib/$(TARGET): $(BUILD_DIR)/$(WRAP_SOURCE) \
								 $(BUILD_DIR)/Makefile
ifeq ($(OUTPUT_TYPE),kernel)
	@echo '  MAKE    $(PWD_REL_SEP)$(BUILD_DIR)'
	@make --no-print-directory -C "$(BUILD_DIR)"
else
	make -C "$(BUILD_DIR)"
endif
	@touch $@


