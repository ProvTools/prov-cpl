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
# Subprojects
#

SUBPROJECTS := $(LIBRARIES) $(PROGRAMS) $(HEADERS)
SUBPROJECTS_SORTED := $(sort $(LIBRARIES)) $(sort $(HEADERS)) $(sort $(PROGRAMS))

ifdef SORT_SUBPROJECTS
	SUBPROJECTS := $(SUBPROJECTS_SORTED)
endif


#
# Program to execute
#

ifndef RUN_PROGRAM
	RUN_PROGRAM := $(word 1,$(PROGRAMS))
endif


#
# Recursive targets
#

TARGETS_BUILD := all relink
TARGETS_CLEAN := clean distclean messclean
TARGETS_INSTALL := install uninstall

TARGETS := $(TARGETS_BUILD) $(TARGETS_CLEAN) $(TARGETS_INSTALL)

.PHONY: $(TARGETS) $(SUBPROJECTS) $(OPTIONAL)

$(TARGETS_BUILD) $(TARGETS_INSTALL)::
ifeq ($(OUTPUT_TYPE),kernel)
	@for S in $(SUBPROJECTS); do \
		($(NMAKE) --no-print-directory -C "$$S" $@ 2>&1) \
			| grep --line-buffered -v 'is up to date' \
			| grep --line-buffered -v 'Nothing to be done for' \
			| grep --line-buffered -v 'Clock skew detected' \
			| grep --line-buffered -v 'has modification time' \
			; \
		if [[ $${PIPESTATUS[0]} -ne 0 ]]; then exit 1; fi; \
	done
else
	@for S in $(SUBPROJECTS); do \
		echo -e $(COLOR_MAKE)make[$(MAKELEVEL)]: \
			Entering $(COLOR_MAKE_PATH)$(PWD_REL_SEP)$$S \
			$(COLOR_NORMAL); \
		($(COLORMAKE) OUTPUT_TYPE=$(OUTPUT_TYPE) --no-print-directory -C "$$S" $@ 2>&1) \
			| grep --line-buffered -v 'Clock skew detected' \
			| grep --line-buffered -v 'has modification time' \
			; \
		if [[ $${PIPESTATUS[0]} -ne 0 ]]; then exit 1; fi; \
	done
endif

$(TARGETS_CLEAN)::
ifeq ($(OUTPUT_TYPE),kernel)
	@for S in $(SUBPROJECTS) $(OPTIONAL); do \
		echo "  CLEAN   $(PWD_REL_SEP)$$S"; \
		($(NMAKE) --no-print-directory -C "$$S" $@ 2>&1) \
			| grep --line-buffered -v 'Clock skew detected' \
			| grep --line-buffered -v 'has modification time' \
			; \
		if [[ $${PIPESTATUS[0]} -ne 0 ]]; then exit 1; fi; \
	done
else
	@for S in $(SUBPROJECTS) $(OPTIONAL); do \
		echo -e $(COLOR_MAKE)make[$(MAKELEVEL)]: \
			Entering $(COLOR_MAKE_PATH)$(PWD_REL_SEP)$$S \
			$(COLOR_NORMAL); \
		($(COLORMAKE) --no-print-directory -C "$$S" $@ 2>&1) \
			| grep --line-buffered -v 'Clock skew detected' \
			| grep --line-buffered -v 'has modification time' \
			; \
		if [[ $${PIPESTATUS[0]} -ne 0 ]]; then exit 1; fi; \
	done
endif
	@rm -rf core core.* vgcore vgcore.* 2> /dev/null || true
	@find . \( -name "*~" -or -name "*.bak" -or -name "__db.*" \) -delete 2> /dev/null || true

$(SUBPROJECTS) $(OPTIONAL):
	@$(COLORMAKE) --no-print-directory -C $@


#
# Program targets
#

PROGRAM_TARGETS := run run-dev time time-dev gdb vg valgrind vg-all vg-dev vg-dev-all

$(PROGRAM_TARGETS): all
	@$(MAKE) --no-print-directory -C $(RUN_PROGRAM) $@

.PHONY: gprof

gprof:
	@$(MAKE) --no-print-directory GPROF=yes run || exit 1
	@$(MAKE) --no-print-directory GPROF=yes -C $(RUN_PROGRAM) gprof_finish


#
# Shortcut targets
#

.PHONY: release release-run Release debug Debug

debug: all

release:
	@$(MAKE) --no-print-directory RELEASE=yes all

release-run:
	@$(MAKE) --no-print-directory RELEASE=yes run


# Aliases

Debug: debug

Release: release


#
# Special targets
#

.PHONY: lines todo list-subproject-libs list-subproject-shared-lib-files \
		list-subproject-lib-files

lines:
	@find . \( -name "*.cpp" -or -name "*.c" -or -name "*.h" -or -name "*.mk" \
		-or -name "*.py" -or -name "*.pl" -or -name "*.java" \
		-or -name "Makefile" \) -exec cat \{} \; | wc -l

list-subproject-libs list-subproject-shared-lib-files \
		list-subproject-lib-files:
	@for S in $(SUBPROJECTS); do \
		$(NMAKE) --no-print-directory --quiet \
		         __PROGRAM__NO_SUBPROJECT_FILES=yes \
				-C $$S $@ \
			| sed s:^:$$S/: \
			; if [ $${PIPESTATUS[0]} == 0 ]; \
				then echo OK > /dev/null; \
				else exit $${PIPESTATUS[0]}; \
			  fi; \
	done

todo:
	@for S in $(SUBPROJECTS) $(OPTIONAL); do \
		(cd "$$S" && $(COLORMAKE) --no-print-directory $@) || exit 1; \
	done


#
# Fix targets
#

.PHONY: fix fix-permissions fix-whitespace

fix:: fix-whitespace fix-permissions

fix-whitespace:
	@find . \( -name "*.cpp" -or -name "*.c" -or -name "*.h" -or -name "*.mk" \
		-or -name "*.py" -or -name "*.pl" -or -name "Makefile" \) \
		-exec $(ROOT)/make/tools/remove_trailing_whitespace.sh \{} \;

fix-permissions::
	@find . -executable -and -type f -and \( -not -path "*/build/*" \) \
	        -and -not \( -name "*.sh" -or -name "*.pl" -or -name "*.py" \
			                  -or -name "*.so" -or -name "*.so.*" \
			          \) \
		    -exec chmod -x \{} \;
	@find . -not -executable -and -type f -and \( -not -path "*/build/*" \) \
	        -and \( -name "*.sh" -or -name "*.pl" -or -name "*.py" \
			             -or -name "*.so" -or -name "*.so.*" \
			          \) \
		    -exec chmod ug+x \{} \;
