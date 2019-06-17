include GTest.mk
include Sources.mk
-include Objects.mk

pre-build:
	@chmod +x ./gen_tests_targets.sh && ./gen_tests_targets.sh

build/%.o: %.cpp $(GTEST_HEADERS)
	@mkdir -p $(@D)
	@echo Building $<
	@$(CXX) $(CPP_FLAGS) -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -c "$<" -o "$@"

exec:
	@echo Running all compiled tests.
	@chmod +x runtests.sh && ./runtests.sh

coverage:
	@echo Creating coverage report.
	@lcov --directory build/ --capture --output-file build/coverage.info > /dev/null 2>&1
	@lcov --remove build/coverage.info '*v1*' '/usr/*' '*crypto*' '*gtest*' '*stubs*' '*tests*' '*googletest*' --output-file build/coverage.info 2>&1
	@lcov --list build/coverage.info > /dev/null 2>&1
	@genhtml -o build/ build/coverage.info > /dev/null 2>&1

#doesn't remove gtest sources
clean:
	@echo Cleaning up.
	@if [ "$(wildcard build/)" != "" ]; then\
		find ./build ! -name 'gtest_main.a' ! -name 'gtest-all.o' ! -name 'gtest_main.o' -type f -exec rm -f {} +;\
	fi

clean-all:
	@echo Cleaning up.
	@rm -rf build/

#debugging
print-%:
	@echo '$*=$($*)'