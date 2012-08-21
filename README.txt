
  Core Provenance Library
===========================

Contents:
  0. Contact
  1. Introduction
  2. Compiling, installing, and setting up the CPL
  3. Directory hierarchy
  4. Libraries and header files
  5. Using the CPL in your C/C++ programs
  6. Querying Provenance

Copyright 2012 The President and Fellows of Harvard College.
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


  2. Compiling, installing, and setting up the CPL
----------------------------------------------------

Installing the CPL using a script:

The easiest way to install CPL on Ubuntu is using the installation script
located in the project's main directory:
  ./install.sh

But please note that the script is still in an early stage of development, so
USE IT AT YOUR OWN RISK. The script requires root permissions to run, but you
should not run it as root; rather enter your "sudo" password when prompted by
the script.

The script works to some degree also on other distributions of Linux and other
Unix-based systems, but it is largely untested and unsupported. Instead, it
is recommended that you install the CPL manually on such systems by following
the steps below.


Installing the CPL manually:

The following two commands should build and install the CPL on most Unix-based
platforms:
  make release
  sudo make install

Please note that typing "make" instead of "make release" creates a debug
build of the CPL. To clean the compile, please use the "clean" or "distclean"
make targets. To uninstall, please use the "uninstall" target - separately for
the main CPL build and for the language-specific bindings.

For more platform-specific instructions, please refer to the INSTALL.* file
for your specific platforms. Also, README.txt files in the various
subdirectories of the project contain information that is useful for the
specific CPL components.

In particular, please refer to the following files for help with setting up
backends for the CPL:
  scripts/*.sql
  scripts/*.sh
  backends/cpl-odbc/README.txt
  backends/cpl-rdf/README.txt

To learn how to use the CPL in your own programs, please refer to the design
document doc/Design.docx, the rest of this README, and the following simple
tests:
  test/standalone-test/test-simple.cpp
  bindings/perl/CPL/test.pl
  bindings/java/CPL/test.java


Running tests:

The easiest way to make sure everything is working properly is to run the tests
that accompany the source distribution of the CPL. First, compile the CPL with
all debug symbols by typing:
  make

Then run the tests by typing:
  make test

This will run the tests in the test/ subdirectory in the silent mode by default,
but if any test fails, it will print all details verbosely.

To run the tests manually using the installed version of the CPL instead of the
current build, cd into the test/standalone-test directory and run (the path to
the executable might be slightly different on your system):
  build/debug/standalone-test

This will run the tests using ODBC's DSN "CPL". To use a different CPL backend,
different test options, and/or to select only a particular test to run, please
refer to the list of options that you can obtain by running:
  build/debug/standalone-test --help

For example, you can run the following:
  build/debug/standalone-test --db-type MySQL -v --odbc CPL Simple


  3. Directory hierarchy
--------------------------

This source distribution of the CPL is organized as follows:

backends                     - Database backends
backends/cpl-odbc            - The ODBC backend for the CPL (libcpl-odbc)
backends/cpl-rdf             - The RDF/SPARQL backend for the CPL (libcpl-rdf)
bindings                     - Language bindings
bindings/java                - Java bindings (package edu.harvard.pass.cpl)
bindings/perl                - Perl bindings (module CPL)
cpl-standalone               - The main CPL library (libcpl)
doc                          - Additional documentation
include                      - C/C++ headers
include/backends             - Public C/C++ headers for the database backends
include/private              - Private headers used by libcpl, tools, and tests
make                         - The build system
misc                         - Miscellaneous
misc/kepler                  - Kepler integration patch and instructions
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
  cpl.h             - The main C/C++ header file for the CPL
  cplxx.h           - C++ extensions
  cpl-db-backend.h  - The interface that a database backend must implement
  cpl-exception.h   - A convenience C++ exception class
  cpl-file.h        - Functions for integrating provenance with the file system


  5. Using the CPL in your C/C++ programs
-------------------------------------------

Setting up your program to use the CPL:
  1. Include cpl.h in your C program, or cplxx.h in your C++ program
  2. Include cpl-odbc.h and/or cpl-rdf.h depending on which database backend
     you plan to use
  3. Make sure your program links with -lcpl, and then depending on the kind of
     database backend you use, -lcpl-odbc and/or -lcpl-rdf
  4. When the program starts, create a database backend and attach the CPL to
     it; for example, the following would connect to a database using ODBC's
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


For how to use CPL, please refer to the documentation in doc/, to the Doxygen
comments in the include files, and to the examples of how the individual CPL
functions are used in test/standalone-test/test-simple.cpp


  6. Querying Provenance
--------------------------

You can query provenance of files using the cpl command-line tool, but please
note that this tool is still under development, and thus not very capable. To
get help about how to use this tool, just run the following:
  cpl

Alternatively, you can use CPL-enabled applications that are not included in
this distribution, such as Provenance Map Orbiter.

To query the provenance manually using the CPL's provenance access API, please
refer to the documentation in doc/, to the Doxygen comments in the include
files, and to the examples of how such CPL are used in the simple tests, such
as test/standalone-test/test-simple.cpp or bindings/java/CPL/test.java (for
Java bindings).
