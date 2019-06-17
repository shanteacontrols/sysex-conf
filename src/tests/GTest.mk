GTEST_DIR := ~/dev/googletest/googletest
GTEST_HEADERS := $(GTEST_DIR)/include/gtest/*.h \
                $(GTEST_DIR)/include/gtest/internal/*.h
GTEST_SRCS_ := $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)

#c++ compiler flags
CPP_FLAGS := -isystem $(GTEST_DIR)/include -isystem $(GTEST_DIR) -g -Og -pthread -std=c++11 --coverage

build/gtest-all.o: $(GTEST_SRCS_)
	@mkdir -p $(@D)
	@echo Building gtest-all.cc
	@$(CXX) $(CPP_FLAGS) -c $(GTEST_DIR)/src/gtest-all.cc -o "$@"

build/gtest_main.o: $(GTEST_SRCS_)
	@mkdir -p $(@D)
	@echo Building gtest_main.cc
	@$(CXX) $(CPP_FLAGS) -c $(GTEST_DIR)/src/gtest_main.cc -o "$@"

build/gtest_main.a: build/gtest-all.o build/gtest_main.o
	@mkdir -p $(@D)
	@echo Creating gtest-all.cc
	@$(AR) $(ARFLAGS) $@ $^ > /dev/null 2>&1