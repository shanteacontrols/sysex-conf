TARGETNAME := sysex-conf

CPP := clang
CXX := clang++

CLANG_FORMAT := clang-format
#when set to 1, format target will fail if there are any changes to the repository after formatting
CF_FAIL_ON_DIFF := 0

build/$(TARGETNAME).o:
	@mkdir -p $(@D)
	@$(CXX) -std=c++11 -Wall -c SysExConf.cpp -o "$@"
	@echo Finished building: SysExConf.cpp

format:
	@echo Checking code formatting...
	@find . -regex '.*\.\(cpp\|hpp\|h\|cc\|cxx\)' \
	-exec $(CLANG_FORMAT) -i {} \;
ifeq ($(CF_FAIL_ON_DIFF), 1)
	git diff -s --exit-code
endif

clean:
	@echo Cleaning up.
	@rm -rf build/

#debugging
print-%:
	@echo '$*=$($*)'