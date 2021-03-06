#!/bin/sh
set -e
# check to see if thrift folder is empty
if [ ! -d "$HOME/thrift/lib" ]; then
  wget http://archive.apache.org/dist/thrift/0.9.3/thrift-0.9.3.tar.gz;
  tar xfz thrift-0.9.3.tar.gz;
  cd thrift-0.9.3  &&  ./configure CFLAGS=-fPIC CXXFLAGS=-fPIC --without-java --without-c_glib --without-python --without-ruby --without-erlang --without-nodejs --without-go --without-php --prefix=$HOME/thrift && make install
else
  echo 'Using cached directory.';
fi
