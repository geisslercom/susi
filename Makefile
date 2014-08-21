# /*
#  * Copyright (c) 2014, webvariants GmbH, http://www.webvariants.de
#  *
#  * This file is released under the terms of the MIT license. You can find the
#  * complete text in the attached LICENSE file or online at:
#  *
#  * http://www.opensource.org/licenses/mit-license.php
#  *
#  * @author: Tino Rusch (tino.rusch@webvariants.de)
#  */


CC=g++-4.8

SERVERMAINFILE=$(shell find ./src/sources -name "main.cc")
SERVERMAIN_INTERMEDIATE=$(subst ./src/sources,./src/objects,$(SERVERMAINFILE))
SERVERMAIN=$(subst .cc,.o,$(SERVERMAIN_INTERMEDIATE))

CLIENTMAINFILE=$(shell find ./src/sources -name "client.cc")
CLIENTMAIN_INTERMEDIATE=$(subst ./src/sources,./src/objects,$(CLIENTMAINFILE))
CLIENTMAIN=$(subst .cc,.o,$(CLIENTMAIN_INTERMEDIATE))

CPPFILES=$(shell find ./src/sources -name "*.cpp")
HEADERFILES=$(shell find ./src/headers -name "*.h")
OBJECTS_INTERMEDIATE=$(subst ./src/sources,./src/objects,$(CPPFILES))
OBJECTS=$(subst .cpp,.o,$(OBJECTS_INTERMEDIATE))

TESTFILES=$(shell find ./test -name "*.cpp")
TESTS_INTERMEDIATE=$(subst ./test,./src/objects,$(TESTFILES))
TESTS=$(subst .cpp,.o,$(TESTS_INTERMEDIATE))

TEST_MAIN=$(shell find ./test/gtest*/lib -name "*.a")

DEBUG=-g -Wall
CCFLAGS=$(DEBUG) -O3 -I src/headers -I /opt/v8/include -I /usr/local/include/soci --std=c++11 -c
LDFLAGS=$(DEBUG) --std=c++11 -L /usr/local/lib64 -L /opt/v8/out/x64.release/lib.target
LIBS=-l PocoFoundation \
		-l PocoUtil \
		-l PocoJSON \
		-l PocoNet \
		-l soci_core \
		-l soci_sqlite3 \
		-l soci_firebird \
		-l v8 \
		-l icuuc \
		-l icui18n \
		-l event \
		-l event_pthreads

susi: ./bin/susi

client: ./bin/client

lib: bin/libsusi.a

bin/libsusi.a: $(OBJECTS)
	ar rvs bin/libsusi.a $(OBJECTS)



test: ./bin/test
	LD_LIBRARY_PATH=/usr/local/lib64:/usr/local/lib:./test/gtest-1.7.0/lib/.libs:/opt/v8/out/x64.release/lib.target ./bin/test

./bin/susi: $(OBJECTS) $(SERVERMAIN)
	$(CC) $(DEBUG) $(LDFLAGS) -o ./bin/susi $(OBJECTS) $(SERVERMAIN) $(LIBS)

./bin/client: $(OBJECTS) $(CLIENTMAIN)
	$(CC) $(DEBUG) $(LDFLAGS) -o ./bin/client $(OBJECTS) $(CLIENTMAIN) $(LIBS)	

./bin/test: $(OBJECTS) $(TESTS)
	$(CC) $(LDFLAGS) -L ./test/gtest-1.7.0/lib/.libs -o ./bin/test $(OBJECTS) $(TESTS) $(LIBS) -lgtest -lgtest_main

./src/objects/%.o: ./src/sources/%.cpp ./src/headers/**/* Makefile
	-@mkdir -p $(shell dirname $@)
	$(CC) $(CCFLAGS) -o $@ $<

./src/objects/%.o: ./src/sources/%.cc Makefile
	-@mkdir -p $(shell dirname $@)
	$(CC) $(CCFLAGS) -o $@ $<

./src/objects/%.o: ./test/%.cpp Makefile ./src/headers/**/*
	-@mkdir -p $(shell dirname $@)
	$(CC) $(CCFLAGS) -I ./test/gtest*/include -o $@ $<

clean:
	-rm -r ./src/objects
	-rm ./bin/susi
	-rm ./bin/test

run: susi
	LD_LIBRARY_PATH=/usr/local/lib64 ./susi -f ./config.json

cppcheck_output.txt: $(CPPFILES) $(HEADERFILES)
	cppcheck -I src/headers -I /usr/local/include --enable=all src -q  2>cppcheck_output.txt
	
check: cppcheck_output.txt
	@echo "$(shell echo "Errors      :" $(shell cat cppcheck_output.txt|grep '(error)'|wc -l))"
	@echo "$(shell echo "Warnings    :" $(shell cat cppcheck_output.txt|grep '(warning)'|wc -l))"
	@echo "$(shell echo "Style       :" $(shell cat cppcheck_output.txt|grep '(style)'|wc -l))"
	@echo "$(shell echo "Perfomance  :" $(shell cat cppcheck_output.txt|grep '(performance)'|wc -l))"
	@echo "$(shell echo "Information :" $(shell cat cppcheck_output.txt|grep '(information)'|wc -l))"
	@test 0 -eq $(shell cat cppcheck_output.txt|grep '(error)'|wc -l)

push: check
	$(MAKE) clean
	git checkout .
	$(MAKE) bin/test
	LD_LIBRARY_PATH=/usr/local/lib64:/usr/local/lib:./test/gtest-1.7.0/lib/.libs:/opt/v8/out/x64.release/lib.target ./bin/test
	git push
