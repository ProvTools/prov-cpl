.PHONY: all
all: build/release/cpl-odbc.o

build/release/cpl-odbc.o: cpl-odbc.cpp stdafx.h cpl-odbc-private.h \
  ../../include/backends/cpl-odbc.h ../../include/cpl-db-backend.h \
  ../../include/cpl.h ../../include/private/cpl-platform.h \
  ../../include/cplxx.h ../../include/cpl-exception.h \
  /usr/local/include/sql.h /usr/local/include/sqltypes.h \
  /usr/local/include/unixodbc_conf.h /usr/local/include/sqlext.h \
  /usr/local/include/sqlucode.h
	@echo "  CXX     backends/cpl-odbc/$@"
	@c++ -c -Wall -Wno-deprecated -m64 -O3 -fPIC -I../../include -g -DNDEBUG -o $@ $<

