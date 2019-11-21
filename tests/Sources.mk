vpath modules/%.cpp ../
vpath modules/%.c ../

TEST_FRAMEWORK_SOURCES := \
modules/unity/src/unity.c

#common include dirs
INCLUDE_DIRS := \
-I"../modules" \
-I"../src" \
-I"./"
