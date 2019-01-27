#!/bin/bash

# path to clang++, llvm-dis, and opt
LLVM_BIN=/LLVM_ROOT/build/bin
# path to CSE231.so
# LLVM_SO=/LLVM_ROOT/build/lib/231_solution.so
LLVM_SO=/LLVM_ROOT/build/lib/submission_pt1.so

# path to lib231.c
LIB_DIR=/lib231
# path to the test directory
TEST_DIR=/tests/test-example

FLAGS=`llvm-config --system-libs --cppflags --ldflags --libs core`
FLAGS="$FLAGS -Wno-unused-command-line-argument"

$LLVM_BIN/clang -c -O0 $TEST_DIR/test1.c -emit-llvm -S -o /tmp/test1-c.ll
$LLVM_BIN/clang++ -c -O0 $TEST_DIR/test1.cpp -emit-llvm -S -o /tmp/test1.ll
$LLVM_BIN/clang++ -c $LIB_DIR/lib231.cpp -emit-llvm -S $FLAGS -o /tmp/lib231.ll
$LLVM_BIN/clang++ -c $TEST_DIR/test1-main.cpp -emit-llvm -S -o /tmp/test1-main.ll

make --directory /LLVM_ROOT/build/lib/Transforms/CSE231_Project
$LLVM_BIN/opt -load $LLVM_SO -cse231-csi < /tmp/test1-c.ll > /dev/null 2> /tmp/csi.result
$LLVM_BIN/opt -load $LLVM_SO -cse231-cdi < /tmp/test1.ll -o /tmp/test1-cdi.bc
$LLVM_BIN/opt -load $LLVM_SO -cse231-bb < /tmp/test1.ll -o /tmp/test1-bb.bc

$LLVM_BIN/llvm-dis /tmp/test1-cdi.bc
$LLVM_BIN/llvm-dis /tmp/test1-bb.bc

$LLVM_BIN/clang++ /tmp/lib231.ll /tmp/test1-cdi.ll /tmp/test1-main.ll $FLAGS -o /tmp/cdi_test1
$LLVM_BIN/clang++ /tmp/lib231.ll /tmp/test1-bb.ll /tmp/test1-main.ll $FLAGS -o /tmp/bb_test1

/tmp/cdi_test1 2> /tmp/cdi.result
/tmp/bb_test1 2> /tmp/bb.result

