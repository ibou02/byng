FROM ubuntu
MAINTAINER Adrien.F <adrien.fillon@gmail.com>

RUN echo "deb http://archive.ubuntu.com/ubuntu trusty main universe" > /etc/apt/sources.list
RUN apt-get update

RUN apt-get install -y postgresql-9.3

ADD pg_hba.conf /etc/postgresql/9.3/main/pg_hba.conf
RUN chown postgres:postgres /etc/postgresql/9.3/main/pg_hba.conf
ADD postgresql.conf /etc/postgresql/9.3/main/postgresql.conf
RUN chown postgres:postgres /etc/postgresql/9.3/main/postgresql.conf