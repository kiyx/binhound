#!/bin/bash -eu
$CXX $CXXFLAGS -std=c++20 \
  -I$SRC/binhound/src \
  $SRC/binhound/tests/fuzz/fuzz_elf_header.cpp \
  $SRC/binhound/src/util/error.cpp \
  $SRC/binhound/src/util/reader.cpp \
  $SRC/binhound/src/util/bytes.cpp \
  $SRC/binhound/src/parser/elf/header.cpp \
  -o $OUT/fuzz_elf_header \
  $LIB_FUZZING_ENGINE

zip -j $OUT/fuzz_elf_header_seed_corpus.zip $SRC/binhound/tests/fuzz/corpus/*
