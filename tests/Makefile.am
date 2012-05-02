TESTS = test_simple test_simple_exc
check_PROGRAMS = test_simple test_simple_exc

test_simple_SOURCES = test_simple.c
test_simple_CFLAGS = -I../src/
test_simple_LDADD = ../src/libstacktrace.la

test_simple_exc_SOURCES = test_simple_exc.cpp
test_simple_exc_CXXFLAGS = -I../src/
test_simple_exc_LDFLAGS = -Wl,--wrap=__cxa_throw
test_simple_exc_LDADD = ../src/libstacktracepp.la
