#!/bin/bash -eu
# Keep -std=c++20 in sync with CMAKE_CXX_STANDARD in CMakeLists.txt.
# NOTE: src/cli/main.cpp is intentionally excluded (libFuzzer provides main),
# so keep this list explicit: globbing src/ would break the build.
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUILD_TESTING=OFF

# Seeds are generated (never committed) so the repo stays binary-free.
python3 "$SRC/binhound/tests/fuzz/generate_corpus.py" "$SRC/seed-corpus"

# Intentionally unquoted: CXXFLAGS and LIB_FUZZING_ENGINE are flag LISTS.
# shellcheck disable=SC2086
"$CXX" $CXXFLAGS -std=c++20 \
  "-I$SRC/binhound/src" \
  "-I$SRC/binhound/build/_deps/tl_expected-src/include" \
  "$SRC/binhound/tests/fuzz/fuzz_elf_header.cpp" \
  "$SRC/binhound/src/util/error.cpp" \
  "$SRC/binhound/src/util/reader.cpp" \
  "$SRC/binhound/src/util/bytes.cpp" \
  "$SRC/binhound/src/parser/elf/header.cpp" \
  -o "$OUT/fuzz_elf_header" \
  $LIB_FUZZING_ENGINE

if compgen -G "$SRC/seed-corpus/*" > /dev/null; then
  zip -j "$OUT/fuzz_elf_header_seed_corpus.zip" "$SRC"/seed-corpus/*
else
  echo "warning: fuzz seed corpus is empty, skipping zip" >&2
fi
