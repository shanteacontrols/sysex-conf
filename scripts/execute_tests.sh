#!/bin/bash

set -e
export LLVM_PROFILE_FILE="build/test-%p.profraw"
for file in build/*.out; do $file 2>/dev/null; done
