#!/bin/bash

# Path to lib231
LIB_DIR=/lib231
# path to the test directory
TEST_DIR=.

FLAGS=`llvm-config --system-libs --cppflags --ldflags --libs core`
FLAGS="$FLAGS -Wno-unused-command-line-argument"

# Compile main.cpp into LLVM IR
clang++ -O0 $TEST_DIR/main.cpp -emit-llvm -S -o /tmp/main.ll
# Compile libe231.cpp into LLVM IR
clang++ $LIB_DIR/lib231.cpp -emit-llvm -S $FLAGS -o /tmp/lib231.ll

#### SECTION 1: Static Instruction Count #####
opt -load 231_solution.so -cse231-csi < /tmp/main.ll > /dev/null 2> /tmp/csi-main.result
#### END OF SECTION 1 ####

#### SECTION 2: Dynamic Instruction Count ####
# Instrument main.ll with calls to lib231 functions
opt -load 231_solution.so -cse231-cdi < /tmp/main.ll -o /tmp/main-instrumented.bc
# Convert main-instrumented.bc into a .ll file (disassemble)
llvm-dis /tmp/main-instrumented.bc
# Link instrumented IR with lib231 to create instrumented executable
clang++ /tmp/lib231.ll /tmp/main-instrumented.ll $FLAGS -o /tmp/cdi_main
# Execute instrumented executable and collect results in /tmp/cdi.result
/tmp/cdi_main 2> /tmp/cdi.results
#### END OF SECTION 2 ####

#### SECTION 3: Basic Blocks ####
# Instrument main.ll with calls to lib231 functions
opt -load 231_solution.so -cse231-bb < /tmp/main.ll -o /tmp/main-instrumented.bc
# Convert main-instrumented.bc into a .ll file (disassemble)
llvm-dis /tmp/main-instrumented.bc
# Link instrumented IR with lib231 to create instrumented executable
clang++ /tmp/lib231.ll /tmp/main-instrumented.ll $FLAGS -o /tmp/bb_main
# Execute instrumented executable and collect results in /tmp/cdi.result
/tmp/bb_main 2> /tmp/bb.results
#### END OF SECTION 3 ####
