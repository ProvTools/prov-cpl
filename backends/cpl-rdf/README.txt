
  RDF/SPARQL Driver Installation Notes
========================================

Contents:
  1. Required packages on Ubuntu
  2. Limitations

Copyright 2011 The President and Fellows of Harvard College.
Contributor(s): Peter Macko


  1. Required packages on Ubuntu
----------------------------------

Please make sure that the following packages are installed:
  libcurl4-openssl-dev
  libxml2-dev


  2. Limitations
------------------

The RDF driver has been tested on Linux (on Ubuntu) and OS X (Lion). It has not
yet been ported to Microsoft Windows.

Another limitation is that the RDF/SPARQL driver is very slow, because it
communicates with the server using the HTTP protocol.

