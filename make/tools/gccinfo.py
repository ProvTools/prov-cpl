#!/usr/bin/env python

#
# The contents of this file are subject to the Mozilla Public License
# Version 1.1 (the "License"); you may not use this file except in
# compliance with the License. You may obtain a copy of the License a
# http://www.mozilla.org/MPL/ and license.txt. Software distributed
# under the License is distributed on an "AS IS" basis, WITHOUT WARRANTY
# OF ANY KIND, either express or implied. See the License for the specific
# language governing rights and limitations under the License.
#
# The Original Code is the File System Back Reference Simulator.
#
# The Initial Developer of the Original Code is NetApp, Inc.
# Portions created by NetApp, Inc. are Copyright (C) 2009.
# All Rights Reserved.
#
# Contributor(s): Peter Macko
#

#
# gccinfo.py
#
# A tool for determining useful properties of G++
#

import os
import sys


def usage():
	"""
	Print the usage information
	"""
	print "Usage: " + sys.argv[0] + " --include"


#
# The entry point to the application
#
if __name__ == "__main__":
	global config, prefix

	if len(sys.argv) <= 1:
		usage()
		sys.exit(-2)


	# Get the information about G++

	f = os.popen("/bin/bash -c 'g++ -v 2>&1'")
	lines = f.readlines()
	f.close()


	# Find the configuration line

	config_line = ""
	config_prefix = "Configured with:"
	for l in lines:
		if l.startswith(config_prefix):
			config_line = l[len(config_prefix):].strip()
			break

	if config_line == "":
		print "Could not find the gcc config line"
		sys.exit(1)


	# Extract the key/value pairs

	config = dict()
	for s in config_line.split(" "):
		i = s.find("=")
		if i <= 0:
			continue
		config[s[:i]] = s[i+1:]


	# Extract the basic properties

	prefix = config["--prefix"]


	# Depending on what we want...

	for arg in sys.argv[1:]:
		if arg == "-i" or arg == "--include":
			s = config["--with-gxx-include-dir"]
			if not s.startswith(prefix):
				s = prefix + s
			print s

		else:
			print "Invalid argument: " + arg
			sys.exit(1)
