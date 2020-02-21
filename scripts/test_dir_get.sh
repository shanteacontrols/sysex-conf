#!/bin/bash

base_dir=$(make print-BUILD_DIR)
test_dir_var=TEST_DIR_$1
test_dir=$(make print-"$test_dir_var")

echo "$base_dir"/"$test_dir"