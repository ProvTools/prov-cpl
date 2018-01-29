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

if ! find /usr -name psqlodbcw.so | egrep '.*' >/dev/null 2>&1; then
	echo "psqlodbcw.so not found. Please install psqlodbc and try again."
	exit 1
fi

if ! find /usr -name boost | egrep '.*' >/dev/null 2>&1; then
	echo "Boost libraries not found. Please install and try again."
	exit 1
fi

if ! find /usr -name nlohmann_json | egrep '.*' >/dev/null 2>&1; then
	echo "nlohmann_json not found. Please install and try again."
	exit 1
fi

if ! find /usr -name swig | egrep '.*' >/dev/null 2>&1; then
	echo "swig not found. Please install SWIG and try again."
	exit 1
fi



# get postgres DB connection info

echo "We'll need some information before we get started."

read -p "Postgres ADMIN username (default 'postgres'): " POSTGRES_ADMIN;
POSTGRES_ADMIN=${POSTGRES_ADMIN:-postgres};
echo "username: $POSTGRES_ADMIN"

read -s -p "Postgres ADMIN password (default ''): " POSTGRES_ADMIN_PASSWORD;
POSTGRES_ADMIN_PASSWORD=${POSTGRES_ADMIN_PASSWORD:-};
echo

read -p "Postgres server address (default 'localhost'): " POSTGRES_SERVER;
POSTGRES_SERVER=${POSTGRES_SERVER:-localhost};
echo "address: $POSTGRES_SERVER"

read -p "Postgres server port (default '5432'): " POSTGRES_PORT;
POSTGRES_PORT=${POSTGRES_PORT:-5432};
echo "port: $POSTGRES_PORT"

LOCAL_CONNECTION=0;
if [ "$POSTGRES_SERVER" == "localhost" ] || [ "$POSTGRES_SERVER" == "127.0.0.1" ]; then
	LOCAL_CONNECTION=1
fi

# check if connection is possible
export PGPASSWORD=$POSTGRES_ADMIN_PASSWORD
$PSQL -U $POSTGRES_ADMIN -h $POSTGRES_SERVER -p $POSTGRES_PORT -c 'SELECT version()' >/dev/null 2>&1
if ! [ $? = 0 ]; then
	if [ $LOCAL_CONNECTION = 1 ]; then
		echo "We haven't been able to connect to the local instance of PostgresQL as the admin user."
		echo "Is postgresql running?"
		echo "Also, please make sure that the daemon is listening to network connections"
		echo "on the localhost interface."
		echo "Finally, did you supply the correct admin password?"
		echo "   Don't know the admin password for your Postgres installation?"
		echo "   Just set the access level to \"trust\" temporarily (for localhost only!)"
		echo "   in your pg_hba.conf file."
		exit 1
	else 
		echo "We haven't been able to connect to the remote Postgres server as the admin user"
		echo "(or you simply don't have psql installed on this server)."
		echo "Also, please make sure the Postgres server is accepting connections from this IP address!"
		echo "   - this may involve editing your pg_hba.conf file on the remote machine."
		exit 1
	fi
fi

echo "Your credentials seem to be in order. Moving on to configuring the CPL database."

read -p "Postgres CPL database name (default 'cpl'): " POSTGRES_DATABASE
POSTGRES_DATABASE=${POSTGRES_DATABASE:-cpl};
echo "database name: $POSTGRES_DATABASE"
read -p "Postgres CPL username (default 'cpl'): " POSTGRES_USER
POSTGRES_USER=${POSTGRES_USER:-cpl};
echo "database user: $POSTGRES_USER"
read -s -p "Postgres CPL user password (default 'cplcplcpl'): " POSTGRES_USER_PASSWORD
POSTGRES_USER_PASSWORD=${POSTGRES_USER_PASSWORD:-cplcplcpl};
echo

# configure Postgres database
$PSQL -U $POSTGRES_ADMIN -h $POSTGRES_SERVER -p $POSTGRES_PORT \
	-v db_name=$POSTGRES_DATABASE \
	-v user_name=$POSTGRES_USER \
	-v user_password=$POSTGRES_USER_PASSWORD < ../postgresql-setup-conf.sql >/dev/null 2>&1

if ! [ $? = 0 ]; then
  echo "Something went wrong with database configuration."
  echo "Make sure your database and user names are valid strings."
  exit 1
fi


echo "Great. Moving on to configuring ODBC settings."

# TODO: find psqlodbcw.so

# Add Postgres ODBC driver
echo "[PostgreSQL Unicode]
Description = PostgreSQL ODBC driver (Unicode version)
Driver = psqlodbcw.so \n" > CPL_odbcinst.txt

$ODBCINST -i -d -f CPL_odbcinst.txt

rm CPL_odbcinst.txt

# Verify 
$ODBCINST -q -d | grep -q '[PostgreSQL Unicode]'
if ! [ $? = 0 ]; then
  echo "Postgres ODBC driver configuration failed."
  echo "Check that you've downloaded the dependency"
  echo "from https://odbc.postgresql.org/."
  echo "You can always manually modify odbcinst.ini"
  echo "or use a ODBC administrator tool."
  exit 1
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

rm CPL_odbc.txt

# Verify 
$ODBCINST -q -s | grep -q '[CPL]'
if ! [ $? = 0 ]; then
  echo "CPL ODBC configuration failed."
  echo "You can always manually modify the ~/.odbc.ini file"
  echo "or use a ODBC administrator tool."
  exit 1
fi

echo "Configuration successful."
