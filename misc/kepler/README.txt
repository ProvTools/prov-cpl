  CPL Kepler Integration
==========================

Contents:
  1. Introduction
  2. Installing Kepler from source
  3. Adding CPL support to Kepler
  4. Hacking the CPL recording module

Copyright 2012 The President and Fellows of Harvard College.
Contributor(s): Peter Macko


  1. Introduction
-------------------

Kepler can be easily modified to record the provenance using the CPL by
patching its provenance module as described in the rest of this document.

Please note that this work is in a very early stage. In particular, the CPL
recording module does not yet fully integrate with the file system, because
it needs to have special handling for each file I/O actor.


  2. Installing Kepler from source
------------------------------------

If you have already installed Kepler and its provenance module from source,
please skip to the next section.

First, create a directory for Kepler:

  mkdir kepler
  cd kepler

Then check out the and compile Kepler:

  mkdir kepler.modules
  cd kepler.modules
  svn co https://code.kepler-project.org/code/kepler/trunk/modules/build-area
  cd build-area
  ant change-to -Dsuite=kepler

Note that the installation of Ptolemy might fail if you have not previously
authorized its keys in SVN; if this happens, just do "cd .." to go back to the
directory of Kepler modules and run the subversion checkout command that was
printed as a part of the error message. Then go back to the build area using
"cd build-area" and re-execute the last command.

Finally, check that everything is working by running:

  and run

This will start Kepler, but the provenance module will be most likely not
installed. To install it, please do the following:

  ant change-to -Dsuite=provenance

And then make sure everything works by typing:

  ant run

The "Provenance" actors should appear in the list of actors on the left side
of the application window.

You can find further documentation at:

  http://kepler-project.org/developers/teams/build/systems/build-system
  http://kepler-project.org/developers/interest-groups/provenance-interest-group


  3. Adding CPL support to Kepler
-----------------------------------

Please follow the following steps:
  1. Build and install the CPL (see README.txt in the top-level directory)
  2. Build and install the Java language bindings (in bindings/java)
  3. Copy the installed CPL.jar from bindings/java/CPL/build/release/ (the
     exact path might be slightly different on your system) or /usr/java/
     to your Kepler's kepler.modules/provenance/lib/jar/, such as by running:
	
       cd kepler.modules/provenance
       cp /usr/java/CPL.jar lib/jar/.

  4. Patch the provenance module (note that you would need to modify the paths
     in the command below):

       cd kepler.modules/provenance
       patch -p0 < kepler-provenance-cpl.patch

  5. Copy the relevant CPL shared libraries to kepler.modules/common/lib64
     or kepler.modules/common/lib, depending on your platform. You probably
     want to copy the following files:

       libcpl-odbc.so
       libcpl.so
       libcpl-rdf.so
       libCPLDirect-java.so

     An easy way to do this is to do the following:

       cd kepler.modules/common
       cp /usr/lib/libcpl*.so lib64/.
       cp /usr/lib/libCPL*.so lib64/.

     If you are using OS X, please note that the file extension for shared
     libraries is .dylib instead of .so. And if you are using a 32-bit system,
     make sure to copy the libraries to lib instead of lib64.

  6. Test that everything works by running Kepler by executing "ant run" from
     the build-area and then creating and running a workflow with a provenance
     recording actor configured to use CPL. Please pay close attention to the
     console/terminal output to catch any CPL error or diagnostic information
     in the case that something does not work.


  4. Hacking the CPL recording module
---------------------------------------

Before you make any changes, please backup your provenance module directory,
such as by running:

  cd kepler.modules
  cp -r provenance provenance.original

And after you make changes to the CPL module, you can recreate the patch by
running:

  cd kepler.modules/provenance
  diff -Nru -x .svn ../provenance.original/ . > kepler-provenance-cpl.patch

