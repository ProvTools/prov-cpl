#!/bin/bash

echo "Welcome to the CPL installer."

# navigate to CPL directory
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $DIR

#check for dependencies
PSQL=$(find /usr -name psql -perm +111 -print -quit)
if ! command -v $PSQL >/dev/null 2>&1; then
	echo "psql executable not found. Please install postgres and try again."
	exit 1
fi

ODBCINST=$(find /usr -name odbcinst -perm +111 -print -quit)
if ! command -v $ODBCINST >/dev/null 2>&1; then
	echo "odbcinst executable not found. Please install unixodbc and try again."
	exit 1
fi

if ! find /usr -name psqlodbcw.so | egrep '.*'; then
	echo "psqlodbcw.so not found. Please install psqlodbc and try again."
	exit 1
fi

if ! find /usr -name boost | egrep '.*'; then
	echo "Boost libraries not found. Please install and try again."
	exit 1
fi

if ! find /usr -name nlohmann_json | egrep '.*'; then
	echo "nlohmann_json not found. Please install and try again."
	exit 1
fi

if ! find /usr -name swig | egrep '.*'; then
	echo "swig not found. Please install SWIG and try again."
	exit 1
fi



# get postgres DB connection info

echo "We'll need some information before we get started."

read -p "Postgres ADMIN username (default 'postgres'): " POSTGRES_ADMIN
POSTGRES_ADMIN=${POSTGRES_ADMIN:-postgres}
read -p -s "Postgres ADMIN password (default ''): " POSTGRES_ADMIN_PASSWORD
POSTGRES_ADMIN_PASSWORD=${POSTGRES_ADMIN_PASSWORD:-}
read -p "Postgres server address (default 'localhost'): " POSTGRES_SERVER
POSTGRES_SERVER=${POSTGRES_SERVER:-localhost}
read -p "Postgres server port (default '5432'): " POSTGRES_PORT
POSTGRES_PORT=${POSTGRES_PORT:-5432}


LOCAL_CONNECTION=0
if POSTGRES_SERVER -eq 'localhost' || POSTGRES_SERVER -eq '127.0.0.1'; then
	$LOCAL_CONNECTION = 1
fi

# check if connection is possible
export PGPASSWORD = $POSTGRES_ADMIN_PASSWORD
CHECK_CONNECTIONS
if ! $($PSQL -U $POSTGRES_ADMIN -h POSTGRES_SERVER -p POSTGRES_PORT &>/dev/null); then
	if [$LOCAL_CONNECTION]; then
		echo "We haven't been able to connect to the local instance of PostgresQL as the admin user.\n"
		echo "Is postgresql running?"
		echo "Also, please make sure that the daemon is listening to network connections!"
		echo "   - at least on the localhost interface."
		echo "Finally, did you supply the correct admin password?"
		echo "   Don't know the admin password for your Postgres installation?"
		echo "   - then simply set the access level to \"trust\" temporarily (for localhost only!)"
		echo "   in your pg_hba.conf file."
		exit 1
	else 
		echo "We haven't been able to connect to the remote Postgres server as the admin user."
		echo "(Or you simply don't have psql installed on this server)"
		echo "Also, please make sure the Postgres server is accepting connections from this IP address!"
		echo "   - this may involve editing your pg_hba.conf file on the remote machine."
		exit 1
	fi

echo "Your credentials seem to be in order. Moving on to configuring the CPL database."

read -p "Postgres CPL database name: " POSTGRES_DATABASE
read -p "Postgres CPL username: " POSTGRES_USER
read -p "Postgres CPL user password: " POSTGRES_USER_PASSWORD

# configure Postgres database
$PSQL -U $POSTGRES_ADMIN -h POSTGRES_SERVER -p POSTGRES_PORT \
	-v db_name=$POSTGRES_DATABASE \
	-v user_name=$POSTGRES_USER \
	-v user_password=$POSTGRES_USER_PASSWORD < ../postgresql-setup-conf.sql

echo "Great. Moving on to configuring ODBC settings."

# TODO: find psqlodbcw.so

# Add Postgres ODBC driver
echo "[PostgreSQL Unicode]
Description = PostgreSQL ODBC driver (Unicode version)
Driver = psqlodbcw.so \n" > CPL_odbcinst.txt

$ODBCINST -i -d -f CPL_odbcinst.txt

# Verify 
if ! $($ODBCINST -q -d | grep -q '[PostgreSQL Unicode]') ; then
  echo "Postgres ODBC driver configuration failed."
  echo "Check that you've downloaded the dependency"
  echo "from https://odbc.postgresql.org/."
  echo "You can always manually modify odbcinst.ini"
  echo "or use a ODBC administrator tool."
fi

# Configure CPL ODBC connection
echo "[CPL]
Description     = PostgreSQL Core Provenance Library
Driver          = PostgreSQL Unicode
Server          = $POSTGRES_SERVER
Database        = $POSTGRES_DATABASE
Port            = $POSTGRES_PORT
Socket          =
Option          =
Stmt            =
User            = $POSTGRES_USER
Password        = $POSTGRES_USER_PASSWORD \n" > CPL_odbc.txt

$ODBCINST -i -s -f CPL_odbc.txt

# Verify 
if ! $($ODBCINST -q -s | grep -q '[CPL]'); then
  echo "CPL ODBC configuration failed."
  echo "You can always manually modify the ~/.odbc.ini file"
  echo "or use a ODBC administrator tool."
fi

echo "Configuration successful."
