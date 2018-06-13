TARGET := test_run
TEST_DIR := tests

# Flags passed to the preprocessor.
# Set Google Test's header directory as a system directory, such that
# the compiler doesn't generate warnings in Google Test headers.
CPPFLAGS += -isystem $(GTEST_DIR)/include --coverage

# Flags passed to the C++ compiler.
CXXFLAGS += -g -pthread

GTEST_DIR := ../googletest/googletest
GTEST_HEADERS := $(GTEST_DIR)/include/gtest/*.h \
                $(GTEST_DIR)/include/gtest/internal/*.h
GTEST_SRCS_ := $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)

TEST_SRCS := $(shell find $(TEST_DIR)/ -name "*.cpp") src/SysEx.cpp

all: $(TARGET).out

clean:
	@echo Cleaning up.
	@rm -f $(TARGET).out *.o *.info *.gcda *.gcno *.a *.html *.png *.infoclear *.css

gtest-all.o: $(GTEST_SRCS_)
	@$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c $(GTEST_DIR)/src/gtest-all.cc

gtest_main.o: $(GTEST_SRCS_)
	@$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c $(GTEST_DIR)/src/gtest_main.cc

gtest_main.a: gtest-all.o gtest_main.o
	@$(AR) $(ARFLAGS) $@ $^

tests-all.o: $(TEST_SRCS) $(GTEST_HEADERS)
	@$(CXX) $(CPPFLAGS) -std=c++11 $(addprefix -D,$(DEFINES)) $(CXXFLAGS) -c $(TEST_SRCS)

# A test should link with either gtest.a or
# gtest_main.a, depending on whether it defines its own main()
# function.
$(TARGET).out: $(notdir $(TEST_SRCS:.cpp=.o)) gtest_main.a
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -lpthread $^ -o $@

#debugging
print-%:
	@echo '$*=$($*)'
