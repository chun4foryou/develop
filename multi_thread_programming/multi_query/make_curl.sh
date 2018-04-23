#!/bin/bash
PWD=`pwd`
SETUPT_PATH="../library/curl"

## curl
cd ./curl-7.59.0/

if [ ! -e ${SETUPT_PATH} ]; then
	mkdir -p ${SETUPT_PATH}
else
	rm -rf ${SETUPT_PATH}/*
fi

autoreconf -f -i
./configure --enable-pthreads --disable-shared --disable-ldap --without-zlib  --with-ssl=${PWD}/../library/openssl --prefix=${PWD}/${SETUPT_PATH}
make clean
make -j 11
make install

