FROM gcr.io/oss-fuzz-base/base-builder:v1@sha256:8b4a73d83374b298a0a771eeec9a1d44ceff3416a678b7d7946303389d022aca
RUN apt-get update && apt-get install -y --no-install-recommends cmake ninja-build zip \
    && rm -rf /var/lib/apt/lists/*
COPY . $SRC/binhound
WORKDIR $SRC/binhound
COPY .clusterfuzzlite/build.sh $SRC/
