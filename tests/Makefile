include GTest.mk
include Sources.mk
-include Objects.mk

CPP := clang
CXX := clang++
LLVM_PROFDATA := llvm-profdata-9
LLVM_COV := llvm-cov-9

TEST_BINARIES := $(addprefix -object ,$(shell find ./build -name "*.out" 2>/dev/null))

#passed to both c and c++ compiler
COMMON_FLAGS := \
-isystem $(GTEST_DIR)/include \
-isystem $(GTEST_DIR) \
-fprofile-instr-generate \
-fcoverage-mapping \
-Og \
-g \
-pthread \
-Wall

#c++ compiler only
CPP_FLAGS := \
-std=c++11

#c compiler only
C_FLAGS := \
-std=c11

#linker
LDFLAGS := \
-lpthread

#doesn't remove gtest sources
clean:
	@echo Cleaning up.
	@if [ "$(wildcard build/)" != "" ]; then\
		find ./build ! -name 'gtest_main.a' ! -name 'gtest-all.o' ! -name 'gtest_main.o' -type f -exec rm -f {} +;\
	fi

clean-all:
	@echo Cleaning up.
	@rm -rf build/

build/%.o: %.c $(GTEST_HEADERS)
	@mkdir -p $(@D)
	@echo Building $<
	@$(CPP) $(COMMON_FLAGS) $(C_FLAGS) $(addprefix -D,$(DEFINES)) $(INCLUDE_DIRS) $(INCLUDE_FILES) -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -c "$<" -o "$@"

build/%.o: %.cpp $(GTEST_HEADERS)
	@mkdir -p $(@D)
	@echo Building $<
	@$(CXX) $(COMMON_FLAGS) $(CPP_FLAGS) $(addprefix -D,$(DEFINES)) $(INCLUDE_DIRS) $(INCLUDE_FILES) -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -c "$<" -o "$@"

pre-build:
	@chmod +x ../scripts/gen_tests_targets.sh && ../scripts/gen_tests_targets.sh

exec:
	@echo Running all compiled tests.
	@chmod +x ../scripts/execute_tests.sh && ../scripts/execute_tests.sh

coverage:
	@echo Creating coverage report.
	@$(LLVM_PROFDATA) merge $(wildcard build/*.profraw) -o build/tests.profdata
	@$(LLVM_COV) report $(TEST_BINARIES) -instr-profile=./build/tests.profdata -ignore-filename-regex="test|modules" -use-color
	@$(LLVM_COV) show $(TEST_BINARIES) -instr-profile=./build/tests.profdata -ignore-filename-regex="test|modules" -use-color --format html > ./build/coverage.html
	@echo ***Generated HTML coverage report in ./build/coverage.html***

#debugging
print-%:
	@echo '$*=$($*)'