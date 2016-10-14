.PHONY: all
all: build/release/cpl-lock.o build/release/cpl-platform.o build/release/cpl-standalone.o

build/release/cpl-lock.o: cpl-lock.cpp stdafx.h ../include/private/cpl-lock.h \
  ../include/cpl.h ../include/cpl-exception.h cpl-platform.h
	@echo "  CXX     cpl-standalone/$@"
	@c++ -c -Wall -Wno-deprecated -m64 -O3 -fPIC -I../include -g -DNDEBUG -o $@ $<

build/release/cpl-platform.o: cpl-platform.cpp stdafx.h cpl-platform.h ../include/cpl.h
	@echo "  CXX     cpl-standalone/$@"
	@c++ -c -Wall -Wno-deprecated -m64 -O3 -fPIC -I../include -g -DNDEBUG -o $@ $<

build/release/cpl-standalone.o: cpl-standalone.cpp stdafx.h cpl-private.h \
  ../include/cplxx.h ../include/cpl.h ../include/cpl-exception.h \
  ../include/cpl-db-backend.h ../include/private/cpl-lock.h \
  cpl-platform.h
	@echo "  CXX     cpl-standalone/$@"
	@c++ -c -Wall -Wno-deprecated -m64 -O3 -fPIC -I../include -g -DNDEBUG -o $@ $<

