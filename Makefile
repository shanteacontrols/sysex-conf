# root of Google Test, relative to where this file is.
GTEST_DIR := ../googletest/googletest

# Flags passed to the preprocessor.
# Set Google Test's header directory as a system directory, such that
# the compiler doesn't generate warnings in Google Test headers.
CPPFLAGS += -isystem $(GTEST_DIR)/include

# Flags passed to the C++ compiler.
CXXFLAGS += -g -pthread

# All tests produced by this Makefile.  Remember to add new tests you
# created to the list.
TESTS := SysEx_Test_Build SysEx_Test_InvalidReq

# needed to build sysex library
DEFINES := \
SYSEX_MAX_BLOCKS=6 \
SYSEX_MAX_SECTIONS=10

# library source
LIB_SOURCES := $(shell find src/ -name "*.cpp")

# All Google Test headers.  Usually you shouldn't change this
# definition.
GTEST_HEADERS := $(GTEST_DIR)/include/gtest/*.h \
                $(GTEST_DIR)/include/gtest/internal/*.h

TEST_DIR := tests

all: $(TESTS)

clean :
	@echo Cleaning up.
	@rm -f $(TESTS) gtest.a gtest_main.a *.o

# Builds gtest.a and gtest_main.a.

# Usually you shouldn't tweak such internal variables, indicated by a
# trailing _.
GTEST_SRCS_ = $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)

# needed by all tests
COMMON_REQS := SysEx.o gtest_main.a

# For simplicity and to avoid depending on Google Test's
# implementation details, the dependencies specified below are
# conservative and not optimized.  This is fine as Google Test
# compiles fast and for ordinary users its source rarely changes.
gtest-all.o: $(GTEST_SRCS_)
	@$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c $(GTEST_DIR)/src/gtest-all.cc

gtest_main.o: $(GTEST_SRCS_)
	@$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c $(GTEST_DIR)/src/gtest_main.cc

gtest.a: gtest-all.o
	@$(AR) $(ARFLAGS) $@ $^

gtest_main.a: gtest-all.o gtest_main.o
	@$(AR) $(ARFLAGS) $@ $^

# A test should link with either gtest.a or
# gtest_main.a, depending on whether it defines its own main()
# function.

SysEx_Test_%.o: $(TEST_DIR)/$(@:.o=.cpp) $(LIB_SOURCES) $(GTEST_HEADERS)
	@$(CXX) $(CPPFLAGS) -std=c++11 $(addprefix -D,$(DEFINES)) $(CXXFLAGS) -c $(TEST_DIR)/$(@:.o=.cpp) $(LIB_SOURCES)

$(TESTS): %:%.o $(COMMON_REQS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -lpthread $^ -o $@

#debugging
print-%:
	@echo '$*=$($*)'