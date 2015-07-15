FROM ubuntu
MAINTAINER Adrien.F <adrien.fillon@gmail.com>

RUN echo "deb http://archive.ubuntu.com/ubuntu trusty main universe" > /etc/apt/sources.list
RUN apt-get update


RUN DEBIAN_FRONTEND=noninteractive apt-get install -y --force-yes libssl1.0.0/trusty libssl-dev/trusty openssl/trusty
RUN apt-get install -y nodejs nodejs-dev npm
RUN npm install -g node-static
RUN ln -s /usr/bin/nodejs /usr/bin/node

ADD . /byng-web