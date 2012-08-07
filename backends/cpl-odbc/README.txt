
  ODBC Installation Notes
===========================

Contents:
  1. Installing ODBC on Ubuntu
  2. Installing ODBC data sources on Ubuntu
  3. Installing ODBC data sources on Windows
  4. Installing ODBC on Mac OS X
  5. Configuring MySQL
  6. Configuring PostgreSQL

Copyright 2012 The President and Fellows of Harvard College.
Contributor(s): Peter Macko


  1. Installing ODBC on Ubuntu
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


  2. Installing ODBC data sources on Ubuntu
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
User            = cpl
Password        = cplcplcpl

You can verify that the data source is installed by running: odbcinst -q -s


  3. Installing ODBC data sources on Windows
----------------------------------------------

ODBC should be already installed if you are using a Server, a Professional, or
an Ultimate edition of Microsoft Windows. To add a data source, please use the
ODBC Data Source Administrator, located in Administrative Tools in the Control
Panel.

Then, create either "User DSN" or "System DSN" (depending if you are installing
CPL just for yourself or also for other users) for the database that you plan
to use, and enter the following properties:

    Name    : CPL
    Server  : localhost
    User    : cpl
    Password: cplcplcpl
    Database: cpl


  4. Installing ODBC on Mac OS X
----------------------------------

Please download and install the ODBC Administrator Tool for Mac OS X from:

    http://support.apple.com/kb/dl895

Then, create either "User DSN" or "System DSN" (depending if you are installing
CPL just for yourself or also for other users) for the database that you plan
to use, and enter the following properties:

    Name    : CPL
    Server  : localhost
    User    : cpl
    Password: cplcplcpl
    Database: cpl


  5. Configuring MySQL
------------------------

Please run the MySQL configuration script ../../scripts/mysql-setup.sql
as the MySQL root. On Linux and Unix systems, this can be easily achieved
by cd-ing into CPL's main project directory and running from the command line:

    mysql -u root -p < scripts/mysql-setup.sql

This will create user cpl with password "cplcplcpl", database cpl, and its
corresponding schema.


  6. Configuring PostgreSQL
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


Finally, run the configuration script ../../scripts/postgresql-setup.sql
as the user postgres. On Linux and Unix systems, this can be easily achieved
by cd-ing into CPL's main project directory and running from the command line:

    sudo -u postgres psql postgres < scripts/postgresql-setup.sql

This will create user cpl with password "cplcplcpl", database cpl, and its
corresponding schema.
