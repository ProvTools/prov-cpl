
  ODBC Installation Notes
===========================

Contents:
  1. Installing ODBC in Ubuntu
  2. Installing ODBC data sources in Ubuntu

Copyright 2011 The President and Fellows of Harvard College.
Contributor(s): Peter Macko


  1. Installing ODBC in Ubuntu
--------------------------------

Please make sure that the following packages are installed:
  unixodbc
  unixodbc-dev
  odbcinst
  
Optional packages:
  unixodbc-bin

Packages for ODBC drivers:
  libmyodbc (for MySQL)
  odbc-postgresql (for PostgreSQL)


After you install the MySQL driver, add the following to /etc/odbcinst.ini:

[MySQL]                                                                                                                                                                                                        
Description     = MySQL driver                                                                                                                                                                                 
Driver          = /usr/lib/odbc/libmyodbc.so                                                                                                                                                                   
Setup           = /usr/lib/odbc/libodbcmyS.so

If you install the PostgreSQL driver, the installer performs the corresponding
update for you.

To verify that the drivers are installed, please execute: odbcinst -q -d


  2. Installing ODBC data sources in Ubuntu
---------------------------------------------

The easiest way to add a data source is to use ODBCConfig available from
the unixodbc-bin package. Alternatively, you can edit the /etc/odbc.ini file
manually. For example, if you use the MySQL ODBC driver, the database is
running locally, and the database name is cpl, the appropriate configuration
entry would be:

[CPL]
Description     = MySQL Core Provenance Library
Driver          = MySQL
Server          = localhost
Database        = cpl
Port            = 
Socket          = 
Option          = 
Stmt            = 

You can verify that the data source is installed by running: odbcinst -q -s
