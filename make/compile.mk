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
# Include the header
#

include $(ROOT)/make/header.mk


#
# Sources
#

ifndef SOURCES
	SOURCES := $(shell ls *.c *.cxx *.cc *.cpp *.C 2> /dev/null)
endif


#
# Linker dependencies
#

ifdef LINKER_SUBPROJECT_DEPENDENCIES
	LINKER_FLAGS := $(LINKER_FLAGS) \
					$(addprefix -L$(ROOT)/, \
						$(addsuffix /$(BUILD_DIR), \
							$(LINKER_SUBPROJECT_DEPENDENCIES)))
endif


#
# Commands
#

DEPEND_C    := $(CC)  -MM $(CFLAGS)   $(INCLUDE_FLAGS) $(DEBUG_FLAGS)
DEPEND_CXX  := $(CXX) -MM $(CXXFLAGS) $(INCLUDE_FLAGS) $(DEBUG_FLAGS)
DEPEND_C    := $(strip $(DEPEND_C))
DEPEND_CXX  := $(strip $(DEPEND_CXX))

COMPILE_C   := $(CC)  -c  $(CFLAGS)   $(INCLUDE_FLAGS) $(DEBUG_FLAGS)
COMPILE_CXX := $(CXX) -c  $(CXXFLAGS) $(INCLUDE_FLAGS) $(DEBUG_FLAGS)
COMPILE_C   := $(strip $(COMPILE_C))
COMPILE_CXX := $(strip $(COMPILE_CXX))

LINK        := $(CXX) $(LINKER_FLAGS) $(DEBUG_FLAGS)
LINK        := $(strip $(LINK))

LIBRARIES   := $(strip $(LIBRARIES))


#
# Preprocess the input data
#

SOURCES := $(sort $(SOURCES))
OBJECTS := $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCES)))
OBJECTS := $(patsubst %,$(BUILD_DIR)/%,$(sort $(OBJECTS)))

PROG_SOURCES := $(sort $(PROG_SOURCES))
PROG_OBJECTS := $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(PROG_SOURCES)))
PROG_OBJECTS := $(patsubst %,$(BUILD_DIR)/%,$(sort $(PROG_OBJECTS)))

ALL_SOURCES := $(sort $(PROG_SOURCES) $(SOURCES))
ALL_OBJECTS := $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(ALL_SOURCES)))
ALL_OBJECTS := $(patsubst %,$(BUILD_DIR)/%,$(sort $(ALL_OBJECTS)))


#
# Processing the compiler output
#

ifdef VISUAL_STUDIO
	PROCESS_GCC_OUTPUT := \
		| sed 's|\[|***BEGIN***|g' \
		| sed 's|\]|***END***|g' \
		| sed 's|^[^/][^ *:]*:|$(PWD_REL_SEP)&|' \
		| sed 's|:[0-9][0-9]*:|***LEFT***&***RIGHT*** :|' \
		| sed 's|\*\*\*LEFT\*\*\*:|(|' \
		| sed 's|:\*\*\*RIGHT\*\*\*|)|' \
		| sed 's|\*\*\*BEGIN\*\*\*|[|g' \
		| sed 's|\*\*\*END\*\*\*|]|g'
else
	PROCESS_GCC_OUTPUT := \
		| sed 's|\[|***BEGIN***|g' \
		| sed 's|\]|***END***|g' \
		| sed 's|^[^/][^ *:]*:|$(PWD_REL_SEP)&|' \
		| sed 's|\*\*\*BEGIN\*\*\*|[|g' \
		| sed 's|\*\*\*END\*\*\*|]|g'
endif


#
# Build-related phony and preliminary targets
#

.PHONY: all clean distclean messclean depend target targetclean relink release

all: $(BUILD_DIR)/$(TARGET)

target: $(BUILD_DIR)/$(TARGET)

targetclean:
	@rm -rf $(BUILD_DIR)/$(TARGET) || true

relink: targetclean
	@$(MAKE) target

clean:: messclean
	@rm -rf $(BUILD_DIR) || true

distclean:: messclean
	@rm -rf $(BUILD_DIR_ALL) || true

messclean::
	@find . -name '*~' -delete 2> /dev/null || true
	@rm -f core core.* vgcore vgcore.* 2> /dev/null || true
	@rm -f __db.* log.* 2> /dev/null || true

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)


#
# Shortcut targets
#

.PHONY: release Release debug Debug

debug: all

release:
	@$(MAKE) --no-print-directory RELEASE=yes all


# Aliases

Debug: debug

Release: release


#
# Other special-purpose targets
#

.PHONY: lines gprof todo list-subproject-lib-files \
		list-subproject-shared-lib-files list-subproject-libs

lines:
	@echo Total number of lines:
	@cat Makefile *.cpp *.c *.h | wc -l

gprof:
	@$(MAKE) GPROF=yes all

todo:
	@(grep -n 'TODO' $(ALL_SOURCES) $(shell ls *.h 2>/dev/null) Makefile ;	\
	  grep -n '###'  $(ALL_SOURCES) $(shell ls *.h 2>/dev/null) Makefile ;	\
	  grep -n 'XXX'  $(ALL_SOURCES) $(shell ls *.h 2>/dev/null) Makefile )	\
		| sed 's/\(^.*:.*:\).*TODO/\1\tTODO/'				\
		| sed 's/\(^.*:.*:\).*###/\1\t###/'				\
		| sed 's/\(^.*:.*:\).*XXX/\1\tXXX/'				\
		| sed 's|\(^.\)|$(PWD_REL_SEP)\1|' | sort

list-subproject-lib-files::

list-subproject-shared-lib-files::

list-subproject-libs::


#
# Compiler targets & dependencies
#

depend: $(BUILD_DIR)/project.mk

$(BUILD_DIR)/project.mk: $(ALL_SOURCES) $(shell ls *.h 2>/dev/null) \
                         $(DEPENDENCIES) Makefile
	@mkdir -p $(BUILD_DIR)
	@rm -f $@ 2> /dev/null || true
	@echo '.PHONY: all' >> $@
	@echo 'all: $(ALL_OBJECTS)' >> $@
	@echo >> $@
	@for S in $(ALL_SOURCES); do \
		if [[ `echo $$S | grep -c \.c$$` = "1" ]]; then \
			$(DEPEND_C) $$S -o $(BUILD_DIR)/$$S.dep 2>&1 $(PROCESS_GCC_OUTPUT); \
			if [[ $${PIPESTATUS[0]} -ne 0 ]]; then /bin/rm -f $@; exit 1; fi; \
			cat $(BUILD_DIR)/$$S.dep | sed 's|^[a-zA-Z0-9_\-]*\.o|$(BUILD_DIR)\/&|' >> $@; \
			if [[ $(OUTPUT_TYPE) = "kernel" ]]; then \
				echo '	@echo "  CC      $(PWD_REL_SEP)$$@"' >> $@; \
				echo '	@$(COMPILE_C) -o $$@ $$<' >> $@; \
			else \
				echo '	$(COMPILE_C) -o $$@ $$<' >> $@; \
			fi; \
			echo >> $@; \
		else \
			$(DEPEND_CXX) $$S -o $(BUILD_DIR)/$$S.dep 2>&1 $(PROCESS_GCC_OUTPUT); \
			if [[ $${PIPESTATUS[0]} -ne 0 ]]; then /bin/rm -f $@; exit 1; fi; \
			cat $(BUILD_DIR)/$$S.dep | sed 's|^[a-zA-Z0-9_\-]*\.o|$(BUILD_DIR)\/&|' >> $@; \
			if [[ $(OUTPUT_TYPE) = "kernel" ]]; then \
				echo '	@echo "  CXX     $(PWD_REL_SEP)$$@"' >> $@; \
				echo '	@$(COMPILE_CXX) -o $$@ $$<' >> $@; \
			else \
				echo '	$(COMPILE_CXX) -o $$@ $$<' >> $@; \
			fi; \
			echo >> $@; \
		fi \
	done

$(ALL_OBJECTS): $(BUILD_DIR)/project.mk $(ALL_SOURCES)
	@($(MAKE) --no-print-directory -f $(BUILD_DIR)/project.mk $@ 2>&1) \
		| grep -v 'is up to date' \
		| grep -v 'Nothing to be done for' \
		| grep -v 'Clock skew detected' \
		| grep -v 'has modification time' \
		$(PROCESS_GCC_OUTPUT) \
		; exit $${PIPESTATUS[0]}
