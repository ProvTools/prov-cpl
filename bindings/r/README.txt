  R Bindings Installation Notes
=================================

Contents:
  1. Required packages
  2. Compiling and installing the Java bindings
  3. Limitations

Copyright 2017 The President and Fellows of Harvard College.
Contributor(s): Jackson Okuhn, Margo Seltzer, Peter Macko


  1. Required packages on Ubuntu
----------------------------------

Please make sure that the following libraries are installed:
  Rcpp

  2. Compiling and installing the R bindings
----------------------------------------------

Navigate into bindings/r/CPL. From inside R:
  > library(Rcpp)
  > compileAttributes()

Then:
  cd ..
  R CMD build CPL
  R CMD install CPL_3.0.tar.gz 

  3. Limitations
------------------

The bindings currently only implement basic JSON document handling

