#!/bin/bash

#
# Core Provenance Library
#
# Contributor(s):
#      Peter Macko <pmacko@eecs.harvard.edu>
#
# Copyright 2011
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

# Database name
DATABASE=cpl

# Default password (none)
PASSWORD=


#
# The function to show the program usage
#
usage() {
	echo "Usage: `basename $0` [OPTIONS]" >&2
	echo >&2
	echo "Options:" >&2
	echo "  -h            Show this help and exit" >&2
	echo "  -p PASSWORD   Set the database password" >&2
}


#
# Check the command-line arguments
#

while getopts ":hp:" opt; do
	case $opt in
		h)
			usage
			exit 0
			;;
		o)
			PASSWORD="$OPTARG"
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

which 4s-backend-setup 2>&1 | cat > /dev/null
if [ ${PIPESTATUS[0]} != 0 ]; then
	echo "`basename $0`: Package 4store is not installed" >&2
	echo "`basename $0`: On Ubuntu, please run:" >&2
	echo "`basename $0`:" '   "sudo apt-get install 4store"' >&2
	exit 1
fi


#
# Ensure that we have root privileges
#

if [ `whoami` != "root" ]; then
	echo "`basename $0`: This program requires root privileges. If prompted, please enter" >&2
	echo "`basename $0`: your password below." >&2
	sudo true
	if [ $? != 0 ]; then
		echo "`basename $0`: Failed to authenticate as root" >&2
		exit 1
	fi
fi


#
# 4store post-installation
#

sudo mkdir -p /var/lib/4store/ || exit 1


#
# Create the database
#

sudo 4s-backend-setup "$DATABASE" || exit 1


#
# Set the password
#

if [ "x$PASSWORD" != "x" ]; then
	sudo 4s-backend-passwd "$DATABASE" "$PASSWORD" || exit 1
fi

