
  Core Provenance Library
===========================

Contents:
  0. Contact
  1. Introduction
  2. Compiling, installing, and setting up CPL

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
  test/standalone-test/*.cpp
  bindings/perl/CPL/test.pl

