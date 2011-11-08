
  ODBC Installation Notes
===========================

Contents:
  1. Installing ODBC in Ubuntu
  2. Installing ODBC data sources in Ubuntu
  3. Installing ODBC data sources in Windows
  4. Configuring PostgreSQL

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


  3. Installing ODBC data sources in Windows
----------------------------------------------

ODBC should be already installed if you are using a Server, a Professional, or
an Ultimate edition of Microsoft Windows. To add a data source, please use the
ODBC Data Source Administrator, located in Administrative Tools in the Control
Panel.


  4. Configuring PostgreSQL
-----------------------------

PostgreSQL might not work out of the box on Ubuntu. If you experience user
authentication issues for the user cpl, please add the following line to your
pg_hba.conf file:

local   cpl             cpl                                     md5

Make sure to add it just before the line:

local   all             all                                     peer


If you are able to connect to the database using psql (i.e. psql cpl -U cpl),
but you cannot open an ODBC connection, you might need to change the location
of the PostgreSQL socket to /tmp by setting the following property in your
postgresql.conf file and restarting the PostgreSQL service:

unix_socket_directory = '/tmp'
