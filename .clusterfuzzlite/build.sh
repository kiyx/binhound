#!/bin/bash -eu
# Configure once so FetchContent provides the pinned header-only dependencies.
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUILD_TESTING=OFF >/dev/null

$CXX $CXXFLAGS -std=c++20 \
  -I$SRC/binhound/src \
  -I$SRC/binhound/build/_deps/tl_expected-src/include \
  $SRC/binhound/tests/fuzz/fuzz_elf_header.cpp \
  $SRC/binhound/src/util/error.cpp \
  $SRC/binhound/src/util/reader.cpp \
  $SRC/binhound/src/util/bytes.cpp \
  $SRC/binhound/src/parser/elf/header.cpp \
  -o $OUT/fuzz_elf_header \
  $LIB_FUZZING_ENGINE

zip -j $OUT/fuzz_elf_header_seed_corpus.zip $SRC/binhound/tests/fuzz/corpus/*
