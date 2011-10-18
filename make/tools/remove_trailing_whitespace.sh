#!/bin/sh
cat $1 | sed 's/[ 	]*$//' > /tmp/__remove_trailing_whitespace || exit 1
diff -q $1 /tmp/__remove_trailing_whitespace > /dev/null \
	|| /bin/cp -f /tmp/__remove_trailing_whitespace $1
/bin/rm -f /tmp/__remove_trailing_whitespace
