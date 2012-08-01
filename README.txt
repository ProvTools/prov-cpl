
  Core Provenance Library
===========================

Contents:
  0. Contact
  1. Introduction
  2. Compiling, installing, and setting up CPL
  3. Directory hierarchy
  4. Libraries and header files
  5. Using CPL in your C/C++ programs

Copyright 2011 The President and Fellows of Harvard College.
Contributor(s): Peter Macko


  0. Contact
--------------

Project Website:           http://code.google.com/p/core-provenance-library/
Research Group Website:    http://www.eecs.harvard.edu/syrah/pass/
Email:                     pass@eecs.harvard.edu


  1. Introduction
-------------------

Provenance is metadata that describes the history of a digital object: where
it came from, how it came to be in its present state, who or what acted upon
it, etc. It is especially important in computational science, where it enables
the researches to precisely track how each document came into existence,
provides a means to experimental reproducibility, and aids them in debugging
what went wrong during a computation.

The adoption of provenance among computational scientists is low, because most
existing systems require the users to adopt a particular tool set in order to
benefit from their functionality, such as the requirement to use a particular
programming language, operating system, or a workflow engine. Core Provenance
Library (CPL) takes the opposite approach by enabling the scientists to easily
integrate provenance collection to their existing tools.

Core Provenance Library is designed to run on a variety of platforms, work
with multiple programming languages, and be able to use a several different
database backends. An application would use the library's API to disclose its
provenance by creating provenance objects and disclosing data and control flow
between the objects. The library would take care of persistently storing the
provenance, detecting and breaking the cycles, and providing an interface to
query and visualize the collected provenance. 


  2. Compiling, installing, and setting up CPL
------------------------------------------------

The following two commands should build and install CPL on most Unix-based
platforms:
  make release
  sudo make install

To clean the compile, please use the "clean" or "distclean" make targets.
To uninstall, please use the "uninstall" target - separately for the main
CPL build and for the language-specific bindings.

For more platform-specific instructions, please refer to the INSTALL.* file
for your spefic platforms. Also, README.txt files in the various
subdirectories of the project contain information that is useful for the
specific CPL components.

In particular, please refer to the following files for help with setting up
backends for CPL:
  scripts/*.sql
  scripts/*.sh
  backends/cpl-odbc/README.txt
  backends/cpl-rdf/README.txt

To learn how to use CPL in your own programs, please refer to the design
document doc/Design.docx and the simple tests:
  test/standalone-test/test-simple.cpp
  bindings/perl/CPL/test.pl
  bindings/java/CPL/test.java


  3. Directory hierarchy
--------------------------

This source distribution of CPL is organized as follows:

backends                     - Database backends
backends/cpl-odbc            - The ODBC backend for CPL (libcpl-odbc)
backends/cpl-rdf             - The RDF/SPARQL backend for CPL (libcpl-rdf)
bindings                     - Language bindings
bindings/java                - Java bindings (package edu.harvard.pass.cpl)
bindings/perl                - Perl bindings (module CPL)
cpl-standalone               - The main CPL library (libcpl)
doc                          - Additional documentation
include                      - C/C++ headers
include/backends             - Public C/C++ headers for the database backends
include/private              - Private headers used by libcpl, tools, and tests
make                         - The build system
private-lib                  - Private libraries used by tools and tests
private-lib/platform-compat  - Platform compatibility library for tools & tests
scripts                      - Database setup/cleanup scripts
test                         - Tests
test/standalone-test         - A collection of tests for cpl-standalone
tools                        - Command-line tools
tools/cpl-tool               - The main "cpl" command-line tool


  4. Libraries and header files
---------------------------------

Libraries:
  libcpl            - The main CPL library (source: cpl-standalone)
  libcpl-odbc       - ODBC database backend (source: backends/cpl-odbc)
  libcpl-rdf        - RDF/SPARQL database backend (source: backends/cpl-rdf)

Header files:
  cpl.h             - The main C/C++ header file for CPL
  cplxx.h           - C++ extensions
  cpl-db-backend.h  - The interface that a database backend must implement
  cpl-exception.h   - A convenience C++ exception class
  cpl-file.h        - Functions for integrating provenance with the file system


  5. Using CPL in your C/C++ programs
---------------------------------------

Setting up your program to use CPL:
  1. Include cpl.h in your C program, or cplxx.h in your C++ program
  2. Include cpl-odbc.h and/or cpl-rdf.h depending on which database backend
     you plan to use
  3. Make sure your program links with -lcpl, and then depending on the kind of
     database backend you use, -lcpl-odbc and/or -lcpl-rdf
  4. When the program starts, create a datavase backend and attach CPL to it;
     for example, the following would connect to a database using ODBC's
     predefined data source name (DSN) "CPL":

           cpl_db_backend_t* backend = NULL;
         cpl_return_t ret;

         ret = cpl_create_odbc_backend_dsn("CPL", CPL_ODBC_GENERIC, &backend);
         if (!CPL_IS_OK(ret)) {
             fprintf(stderr, "Error creating an ODBC backend: %s",
                     cpl_error_string(ret));
             return 1;
         }

         ret = cpl_attach(backend);
         if (!CPL_IS_OK(ret)) {
             fprintf(stderr, "Error attaching CPL: %s", cpl_error_string(ret));
             return 1;
         }

  5. Detach CPL when the program exits:

         cpl_detach();

     If you are using C++, you can instead put in your main() function the
     following code right after you initialize CPL (requires cplxx.h):

         CPLInitializationHelper __cpl(NULL); (void) __cpl;
    
     This automatically detaches CPL when the variable __cpl falls out of
     scope.


For how to use CPL, please refer to the documentation in doc/, to Doxygen
comments in the include files, and to the examples of how the individual CPL
functions are used in test/standalone-test/test-simple.cpp

