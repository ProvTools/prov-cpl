# prov-CPL

Provenance is metadata that describes the history of a digital object: where it came from, how it came to be in its 
present state, who or what acted upon it, etc. It is especially important in computational science, where it enables 
the researches to precisely track how each document came into existence, provides a means to experimental reproducibility, 
and aids them in debugging what went wrong during a computation.

The adoption of provenance among computational scientists is low, because most existing systems require the users
to adopt a particular tool set in order to benefit from their functionality, such as the requirement to use a 
particular programming language, operating system, or a workflow engine. Core Provenance Library (CPL) takes the 
opposite approach by enabling the scientists to easily integrate provenance collection to their existing tools.

The Core Provenance Library is designed to run on a variety of platforms, work with multiple programming languages,
and be able to use a several different database backends. An application would use the library's API to disclose 
its provenance by creating provenance objects and disclosing data and control flow between the objects. The 
library would take care of persistently storing the provenance, detecting cycles, and providing 
an interface to query the collected provenance.

## Disclosing Provenance

Quick summary:
- Attach/detach from the database backend: `cpl_attach/cpl_detach`
- Create/lookup provenance: `cpl_create_[bundle/object/relation]/cpl_lookup_[...]`
- Add custom properties: `cpl_add_[...]_property`

Check test files for details.

## Accessing Provenance

Use the built-in programmatic API. Check test files for details.

## JSON Handling

Quick summary:
- Validate JSON document: `validate_json`
- Import JSON document: `import_document_json`
- Export bundle as JSON: `export_bundle_json`

Check test files for details.

## Supported Languages

* C/C++
* Java
* Python
* R

## Installation

Required:
* C/C++ Standalone
* ODBC Backend

Optional:
* Language specific bindings

### C/C++ Standalone

Required Packages:
- nlohmann_json
- boost

To compile and install CPL, please type the following commands:
```
make release
[sudo] (for linux) make install
```

The default installation directory is /usr/local.

To clean the compile, please use the `clean` or `distclean` make targets.
To uninstall, please use the `uninstall` target - separately for the main
CPL build and for the language-specific bindings.
### ODBC Backend

Required Packages:
- unixodbc
- unixodbc-dev
- odbcinst
- odbc-postgresql

#### Installing ODBC data sources on Ubuntu

The easiest way to add a data source is to use ODBCConfig available from
the `unixodbc-bin` package. Alternatively, you can edit the `/etc/odbc.ini` file
manually. For example, if you use the PostgreSQL ODBC driver, the database is
running locally, and the database name is `cpl`, the appropriate configuration
entry would be:

      [CPL]
      Description     = PostgreSQL Core Provenance Library
      Driver          = PostgreSQL
      Server          = localhost
      Database        = cpl
      Port            = 
      Socket          = 
      Option          = 
      Stmt            = 
      User            = cpl
      Password        = cplcplcpl

You can verify that the data source is installed by running: `odbcinst -q -s`

#### Installing ODBC on Mac OS X

Please download and install the ODBC Administrator Tool for Mac OS X from:

    http://support.apple.com/kb/dl895

And install the appropriate ODBC drivers:

    For PostgreSQL: https://odbc.postgresql.org/

Alternatively, you can use purchase and use commercial ODBC drivers, such as:

    http://www.actualtech.com/product_opensourcedatabases.php

Then create either `User DSN` or `System DSN` (depending if you are installing
CPL just for yourself or also for other users) for the database that you plan
to use, and enter the following properties:

    Name    : CPL
    Server  : localhost
    User    : cpl
    Password: cplcplcpl
    Database: cpl

#### Configuring PostgreSQL

PostgreSQL might not work out of the box on Ubuntu. If you experience user
authentication issues for the user cpl, please add the following line to your
`pg_hba.conf` file:

    local   cpl             cpl                                     md5

Make sure to add it just before the line:

    local   all             all                                     peer


If you are able to connect to the database using psql (i.e. `psql cpl -U cpl`),
but you cannot open an ODBC connection, you might need to change the location
of the PostgreSQL socket to `/tmp` by setting the following property in your
postgresql.conf file and restarting the PostgreSQL service:

     unix_socket_directory = '/tmp'

Finally, run the configuration script `../../scripts/postgresql-setup.sql` 
as the user postgres. (Note: if you've used homebrew to install postgres, then 
your default username may instead be root.) On Linux and Unix systems, this can
be easily achieved by cd-ing into CPL's main project directory and running from 
the command line:

     sudo psql -U postgres postgres < scripts/postgresql-setup.sql

This will create user `cpl` with password `cplcplcpl`, database `cpl`, and its
corresponding schema.

### Java Bindings

Required Packages:
* default-jdk
* swig

If you plan to use the CPL bindings via Maven, please do the following:
```
cd bindings/java  (if necessary)
make release
sudo make -C CPLDirect install
make -C CPL maven-install
```
The Maven group ID is `edu.harvard.pass`, and the artifact ID is `cpl`. You can
find the most recent version of the project by examining `CPL/Makefile`.

To use the bindings by manually including the .jar or to use with Ant:
```
cd bindings/java (if necessary)
make release
sudo make install
```
This will (among other things) create `/usr/local/java/CPL.jar`, which you can
then include in your project. You do not need to include `CPLDirect.jar`, since
it is statically included in CPL.jar.

In both cases, you will also need to move `libCPLDirect-java.dylib` into your `java.library.path` 
or change your `java.library.path` to include `/usr/local/lib`.

Note: If `make release` fails with the "Permission denied" error, please run
`sudo make distclean` first. If `make release` fails mysteriously, perhaps
`sudo updatedb` will help.

### Python Bindings

Required Packages:
* python-dev
* swig

To compile and install the CPL python bindings, please do the following:  
```
cd bindings/python
make release
[sudo] make install
```
  
### R Bindings

Required Packages:
* Rcpp

To compile and install the CPL R bindings, please do the following: 
```
cd bindings/r/CPL  
(From inside R:)
  > library(Rcpp)
  > compileAttributes()
cd ..  
R CMD build CPL 
R CMD install CPL_3.0.tar.gz
```
The bindings currently only implement basic JSON document handling.

## To-Dos

* Expand R bindings to full library support
* Add MySQL and RDF database backends
* Improve querying with CypherToSQL tool


