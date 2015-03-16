# nrop
#
# VERSION               1.1
FROM      debian:testing
MAINTAINER Anthony Verez <averez@google.com>

RUN apt-get update -qq
RUN apt-get install -y git make python gcc g++ python-pkgconfig libz-dev \
	libglib2.0-dev dh-autoreconf libc6-dev-i386 wget unzip llvm libncurses5-dev \
	clang \
	&& apt-get clean
ADD . /nrop
RUN cd /nrop \
	&& make clean \
	&& make prepare \
	&& cd libs/z3/build \
	&& make install \
	&& cd ../../capstone \
	&& ./make.sh \
	&& ./make.sh install \
	&& cd ../.. \
	&& make -j32

WORKDIR /nrop

