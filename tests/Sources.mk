vpath src/%.cpp ../

SOURCES_sysexconf := \
src/tests.cpp \
src/SysExConf.cpp

#common include dirs
INCLUDE_DIRS := \
-I"../src"