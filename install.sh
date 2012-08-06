#!/bin/bash

#
# Core Provenance Library
#
# Contributor(s):
#      Peter Macko <pmacko@eecs.harvard.edu>
#
# Copyright 2012
#      The President and Fellows of Harvard College.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. Neither the name of the University nor the names of its contributors
#    may be used to endorse or promote products derived from this software
#    without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.


#
# Global variables
#

# Output prefix
P="`basename $0`: "

# Database
DATABASE=

# Build and install core CPL?
CORE=yes

# Build and install Java bindings?
L_JAVA=yes

# Build and install Perl bindings?
L_PERL=yes



#
# The function to show the program usage
#
usage() {
	echo "Usage: `basename $0` [OPTIONS]" >&2
	echo >&2
	echo "Options:" >&2
	echo "  -d DATABASE   Database to use (MySQL, PostgreSQL, 4store)" >&2
	echo "  -h            Show this help and exit" >&2
}


#
# Ensure that we have root privileges
#
ensure_root() {
	if [ `whoami` != "root" ]; then
		echo "${P}The following step requires root privileges." >&2
		echo "${P}If prompted, please enter your password below." >&2
		sudo true
		if [ $? != 0 ]; then
			echo "${P}Failed to authenticate as root" >&2
			exit 1
		fi
	fi
}




#
# ========== Program start ==========
#

echo "${P}Installer -- Core Provenance Library" >&2


#
# Check the command-line arguments
#

while getopts ":d:h" opt; do
	case $opt in
		d)
			DATABASE="$OPTARG"
			;;
		h)
			usage
			exit 0
			;;
		\?)
			echo "`basename $0`: Invalid option: -$OPTARG" >&2
			exit 1
			;;
		:)
			echo "`basename $0`: Option -$OPTARG requires an argument" >&2
			exit 1
			;;
	esac
done

shift $(($OPTIND - 1))

if [ "x$1" != "x" ]; then
	echo "`basename $0`: Too many arguments" >&2
	exit 1
fi


#
# Make sure that the proper packages are installed
#

echo "" >&2
echo "${P}Installing packages required for build" >&2

if [ "`uname`" = Linux ]; then
	if [ "`lsb_release -is`" = Ubuntu ]; then

		ensure_root
		sudo apt-get install \
			build-essential \
			uuid-dev \
			libssl-dev \
			libncurses5-dev \
			unixodbc \
			unixodbc-dev \
			odbcinst \
			libmyodbc \
			odbc-postgresql \
			libcurl4-openssl-dev \
			libxml2-dev \
			libperl-dev \
			swig

		if [ ${PIPESTATUS[0]} != 0 ]; then
			echo "${P}Package installation faild" >&2
			exit 1
		fi

	else
		echo "${P}I do not know how to install packages on `lsb_release -is`" >&2
		echo "${P}If any packages are missing, please install them manually" >&2
	fi
else
	echo "${P}I do not know how to install packages on `uname`" >&2
	echo "${P}If any packages are missing, please install them manually" >&2
fi


#
# Build and install core CPL
#

if [ $CORE = yes ]; then

	echo "" >&2
	echo "${P}Building CPL" >&2
	make release
	if [ ${PIPESTATUS[0]} != 0 ]; then
		echo "${P}Build failed" >&2
		exit 1
	fi

	echo "" >&2
	echo "${P}Installing" >&2
	ensure_root
	sudo make install
	if [ ${PIPESTATUS[0]} != 0 ]; then
		echo "${P}Install failed" >&2
		exit 1
	fi
fi


#
# Build and install Java bindings
#

if [ $L_JAVA = yes ]; then

	which javac 2>&1 | cat > /dev/null
	if [ ${PIPESTATUS[0]} = 0 ]; then
		echo "" >&2
		echo "${P}Building Java bindings " >&2
		# XXX Should not need this
		ensure_root
		sudo updatedb
		make -C bindings/java release
		if [ ${PIPESTATUS[0]} != 0 ]; then
			echo "${P}Build failed" >&2
			exit 1
		fi

		echo "" >&2
		echo "${P}Installing" >&2
		ensure_root
		sudo make -C bindings/java install
		if [ ${PIPESTATUS[0]} != 0 ]; then
			echo "${P}Install failed" >&2
			exit 1
		fi
		
		which mvn 2>&1 | cat > /dev/null
		if [ ${PIPESTATUS[0]} != 0 ]; then
			echo "" >&2
			echo "${P}I did not find Maven -- skipping" >&2
		else
			echo "" >&2
			echo "${P}Installing a Maven dependency" >&2
			make -C bindings/java/CPL maven-install
		fi
	else
		echo "" >&2
		echo "${P}I did not find JDK -- skipping Java bindings" >&2
	fi
fi


#
# Build and install Perl bindings
#

if [ $L_JAVA = yes ]; then

	echo "" >&2
	echo "${P}Building Perl bindings " >&2
	make -C bindings/perl release
	if [ ${PIPESTATUS[0]} != 0 ]; then
		echo "${P}Build failed" >&2
		exit 1
	fi

	echo "" >&2
	echo "${P}Installing" >&2
	ensure_root
	sudo make -C bindings/perl install
	if [ ${PIPESTATUS[0]} != 0 ]; then
		echo "${P}Install failed" >&2
		exit 1
	fi
fi


#
# Make sure that the database is properly installed and configured
#

echo "" >&2
echo "${P}Setting up the database" >&2

if [ "x$DATABASE" = "x" ]; then
	echo
	echo "Which database would you like to use with CPL?"
	select d in MySQL PostgreSQL 4store; do
		case $d in
			MySQL)
				DATABASE=$d
				break
				;;
			PostgreSQL)
				DATABASE=$d
				break
				;;
			4store)
				DATABASE=$d
				break
				;;
			*)
				continue
		esac
	done
fi

case $DATABASE in
	MySQL)
		if [ `odbcinst -q -d | grep -c MySQL` = 0 ]; then
			echo "${P}MySQL ODBC connector is not installed" >&2
			if [ "`uname`" = Linux ]; then
				if [ "`lsb_release -is`" = Ubuntu ]; then
					echo "${P}Attempting to fix it myself" >&2
					ensure_root
					sudo /bin/cp /etc/odbcinst.ini /etc/odbcinst.ini.old || exit 1
					echo "${P}Backed up your ODBC configuration to /etc/odbcinst.ini.old" >&2
					sudo bash -c 'echo "" >> /etc/odbcinst.ini'
					sudo bash -c 'echo "[MySQL]" >> /etc/odbcinst.ini'
					sudo bash -c 'echo "Description = MySQL driver" >> /etc/odbcinst.ini'
					sudo bash -c 'echo "Driver      = libmyodbc.so" >> /etc/odbcinst.ini'
					sudo bash -c 'echo "Setup       = libodbcmyS.so" >> /etc/odbcinst.ini'
					if [ `odbcinst -q -d | grep -c MySQL` = 0 ]; then
						echo "${P}Something went wrong..." >&2
						echo "${P}Please refer to backends/cpl-odbc/README.txt" >&2
						sudo /bin/cp /etc/odbcinst.ini.old /etc/odbcinst.ini || exit 1
						exit 1
					fi
				else
					echo "${P}Please refer to backends/cpl-odbc/README.txt" >&2
					exit 1
				fi
			else
				echo "${P}Please refer to backends/cpl-odbc/README.txt" >&2
				exit 1
			fi
		fi
		if [ `odbcinst -q -s | grep -c CPL` = 0 ]; then
			if [ "`uname`" = Linux ]; then
				if [ "`lsb_release -is`" = Ubuntu ]; then
					echo "${P}Adding DSN \"CPL\" using default MySQL settings" >&2
					ensure_root
					sudo /bin/cp /etc/odbc.ini /etc/odbc.ini.old || exit 1
					echo "${P}Backed up your ODBC DSN configuration to /etc/odbc.ini.old" >&2
					sudo bash -c 'echo "" >> /etc/odbc.ini'
					sudo bash -c 'echo "[CPL]" >> /etc/odbc.ini'
					sudo bash -c 'echo "Description     = MySQL Core Provenance Library" >> /etc/odbc.ini'
					sudo bash -c 'echo "Driver          = MySQL" >> /etc/odbc.ini'
					sudo bash -c 'echo "Server          = localhost" >> /etc/odbc.ini'
					sudo bash -c 'echo "Database        = cpl" >> /etc/odbc.ini'
					sudo bash -c 'echo "Port            = " >> /etc/odbc.ini'
					sudo bash -c 'echo "Socket          = " >> /etc/odbc.ini'
					sudo bash -c 'echo "Option          = " >> /etc/odbc.ini'
					sudo bash -c 'echo "Stmt            = " >> /etc/odbc.ini'
					sudo bash -c 'echo "User            = cpl" >> /etc/odbc.ini'
					sudo bash -c 'echo "Password        = cplcplcpl" >> /etc/odbc.ini'
					if [ `odbcinst -q -s | grep -c CPL` = 0 ]; then
						echo "${P}Something went wrong..." >&2
						echo "${P}Please refer to backends/cpl-odbc/README.txt" >&2
						sudo /bin/cp /etc/odbc.ini.old /etc/odbc.ini || exit 1
						exit 1
					fi
				else
					echo "${P}CPL data source is not installed." >&2
					echo "${P}Please refer to backends/cpl-odbc/README.txt" >&2
					exit 1
				fi
			else
				echo "${P}CPL data source is not installed." >&2
				echo "${P}Please refer to backends/cpl-odbc/README.txt" >&2
				exit 1
			fi
		else
			echo "${P}NOTE: I didn't check whether your DSN \"CPL\" uses MySQL." >&2
		fi
		echo "${P}Going to create a MySQL database for CPL." >&2
		echo "${P}If prompted, please enter your MySQL root password below." >&2
		mysql -u root -p < scripts/mysql-setup.sql
		if [ ${PIPESTATUS[0]} != 0 ]; then
			echo "${P}MySQL configuration failed." >&2
			exit 1
		fi
		;;
	PostgreSQL)
		echo "${P}Not yet implemented -- please refer to backends/cpl-odbc/README.txt" >&2
		break
		;;
	4store)
		./scripts/4store-setup.sh
		if [ ${PIPESTATUS[0]} != 0 ]; then
			echo "${P}4store configuration or installation failed." >&2
			exit 1
		fi
		;;
	*)
		echo "${P}Invalid or unsupported database \"$DATABASE\"" >&2
		exit 1
esac

