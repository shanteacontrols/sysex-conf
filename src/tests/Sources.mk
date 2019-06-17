vpath src/%.cpp ../

SOURCES_sysex := \
tests-all.cpp \
src/SysEx.cpp \

#make sure all objects are located in build directory
#also make sure objects have .o extension

OBJECTS_sysex := $(addprefix build/,$(SOURCES_sysex))
OBJECTS_sysex := $(OBJECTS_sysex:.c=.o)
OBJECTS_sysex := $(OBJECTS_sysex:.cpp=.o)