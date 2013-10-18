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

include $(ROOT)/make/java-module-header.mk


#
# Compile Targets
#

WRAP_SOURCE := $(patsubst %.i,%_wrap.cxx,$(notdir $(INTERFACE)))
WRAP_OBJECT := $(patsubst %.cxx,%.o,$(WRAP_SOURCE))
WRAP_LIBRARY := lib$(PROJECT)-java.$(SOLIBRARY_EXT)
WRAP_CXX := $(CXX) $(CXXFLAGS)  -fno-strict-aliasing $(INCLUDE_FLAGS) \
			-c -O3 -fPIC $(JAVA_INCLUDE)
WRAP_LINK := $(CXX) $(LINKER_FLAGS) -L. $(SHARED_OPTION)

ifndef JAVA_PACKAGE
	JAVA_PACKAGE := swig.direct.$(PROJECT)
endif
JAVA_PACKAGE_DIR := $(subst .,/,$(JAVA_PACKAGE))

$(BUILD_DIR)/$(PROJECT).java $(BUILD_DIR)/$(WRAP_SOURCE): \
		$(INTERFACE) $(DEPENDENCIES)
	@mkdir -p $(BUILD_DIR)
ifeq ($(OUTPUT_TYPE),kernel)
	@echo '  SWIG    $(PWD_REL_SEP)$<'
	@swig -c++ -java -package $(JAVA_PACKAGE) -outdir $(BUILD_DIR) \
		-o $(BUILD_DIR)/$(WRAP_SOURCE) $<
else
	swig -c++ -java -package $(JAVA_PACKAGE) -outdir $(BUILD_DIR) \
		-o $(BUILD_DIR)/$(WRAP_SOURCE) $<
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
	@#$(WRAP_LINK) $(SONAME_OPTION)$(WRAP_LIBRARY) -o $@ $< $(LIBRARIES)
	cd $(BUILD_DIR) && $(WRAP_LINK) $(SONAME_OPTION)$(WRAP_LIBRARY) \
		-o $(WRAP_LIBRARY) $(WRAP_OBJECT) $(LIBRARIES)
endif

$(BUILD_DIR)/$(JAVA_PACKAGE_DIR)/$(PROJECT).java: $(BUILD_DIR)/$(PROJECT).java
	@mkdir -p $(BUILD_DIR)/$(JAVA_PACKAGE_DIR)
ifeq ($(OUTPUT_TYPE),kernel)
	@/bin/cp -f $(CP_UPDATE_FLAG) $(BUILD_DIR)/*.java \
		$(BUILD_DIR)/$(JAVA_PACKAGE_DIR)
else
	/bin/cp -f $(CP_UPDATE_FLAG) $(BUILD_DIR)/*.java \
		$(BUILD_DIR)/$(JAVA_PACKAGE_DIR)
endif

$(BUILD_DIR)/$(JAVA_PACKAGE_DIR)/$(PROJECT).class: \
		$(BUILD_DIR)/$(JAVA_PACKAGE_DIR)/$(PROJECT).java
ifeq ($(OUTPUT_TYPE),kernel)
	@echo "  JAVAC   $(PWD_REL_SEP)$(BUILD_DIR)/$(JAVA_PACKAGE_DIR)/*.java"
	@$(JAVAC) $(BUILD_DIR)/$(JAVA_PACKAGE_DIR)/*.java 
else
	$(JAVAC) $(BUILD_DIR)/$(JAVA_PACKAGE_DIR)/*.java 
endif

$(BUILD_DIR)/$(PROJECT).jar: $(BUILD_DIR)/$(JAVA_PACKAGE_DIR)/$(PROJECT).class \
		$(BUILD_DIR)/$(WRAP_LIBRARY)
ifeq ($(OUTPUT_TYPE),kernel)
	@echo "  JAR     $(PWD_REL_SEP)$(BUILD_DIR)/$(PROJECT).jar"
	@cd $(BUILD_DIR) && $(JAR) cf $(PROJECT).jar $(JAVA_PACKAGE_DIR)/*.class 
else
	cd $(BUILD_DIR) && $(JAR) cf $(PROJECT).jar $(JAVA_PACKAGE_DIR)/*.class 
endif


#
# Miscellaneous targets
#

list-subproject-shared-lib-files::
	@echo $(BUILD_DIR)/$(WRAP_LIBRARY)


#
# Install & unistall
#

ifdef INSTALL
SO_INSTALL_DIR := $(INSTALL_PREFIX)/$(LIB_DIR)
endif

install::
ifdef INSTALL
	@mkdir -p $(SO_INSTALL_DIR)
ifdef RELEASE
ifeq ($(OUTPUT_TYPE),kernel)
	@echo '  INSTALL $(PWD_REL_SEP)$(BUILD_DIR)/$(WRAP_LIBRARY)'
	@install -m 755 $(BUILD_DIR)/$(WRAP_LIBRARY) $(SO_INSTALL_DIR)
else
	install -m 755 $(BUILD_DIR)/$(WRAP_LIBRARY) $(SO_INSTALL_DIR)
endif
else
	@true
endif
else
	@true
endif

uninstall::
ifdef INSTALL
ifeq ($(OUTPUT_TYPE),kernel)
	@echo '  DELETE  $(SO_INSTALL_DIR)/$(WRAP_LIBRARY)'
	@/bin/rm -f $(SO_INSTALL_DIR)/$(WRAP_LIBRARY)
else
	/bin/rm -f $(SO_INSTALL_DIR)/$(WRAP_LIBRARY)
endif
else
	@true
endif

