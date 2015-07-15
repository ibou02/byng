FROM ubuntu
MAINTAINER Adrien.F <adrien.fillon@gmail.com>

RUN echo "deb http://archive.ubuntu.com/ubuntu trusty main universe" > /etc/apt/sources.list
RUN echo "\ndeb http://ca.archive.ubuntu.com/ubuntu/ trusty-updates main universe" >> /etc/apt/sources.list
RUN apt-get update

RUN DEBIAN_FRONTEND=noninteractive apt-get install -y --force-yes libssl1.0.0/trusty libssl-dev/trusty openssl/trusty
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y build-essential
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y cmake
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y libboost-dev
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y libpqxx-dev
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y libboost-system-dev
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y git
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y libtool
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y autoconf

WORKDIR /tmp
RUN git clone https://github.com/google/gumbo-parser.git
WORKDIR /tmp/gumbo-parser
RUN sh ./autogen.sh
RUN ./configure
RUN make
RUN make install

RUN DEBIAN_FRONTEND=noninteractive apt-get install -y libboost-chrono-dev
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y librabbitmq-dev
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y wget
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y unzip
WORKDIR /tmp
RUN wget https://github.com/alanxz/SimpleAmqpClient/archive/v2.4.0.zip
RUN unzip v2.4.0.zip
RUN ls
RUN mkdir ampqbuild
WORKDIR /tmp/ampqbuild
RUN cmake ../SimpleAmqpClient-2.4.0 -G "Unix Makefiles"
RUN make -Wmissing-field-initializers
RUN make install

RUN DEBIAN_FRONTEND=noninteractive apt-get install -y libpoco-dev
RUN apt-get update
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y libcurl4-openssl-dev
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y libboost-thread-dev
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y libboost-filesystem-dev
ADD . /byng-crawler
WORKDIR /byng-crawler
RUN rm -rf CMakeFiles
RUN rm -f CMakeCache.txt
RUN cmake -G "Unix Makefiles"
RUN make byng-crawler
RUN cp bin/byng-crawler /usr/bin