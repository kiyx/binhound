FROM gcr.io/oss-fuzz-base/base-builder:v1
RUN apt-get update && apt-get install -y cmake ninja-build
RUN git clone https://github.com/kiyx/binhound.git binhound
WORKDIR binhound
COPY build.sh $SRC/
