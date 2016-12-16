#!/bin/bash

WHOAMI=`whoami`
D_OPTION="${WHOAMI}_dox.conf"
DOX_CONF="${WHOAMI}_doxygen.conf"

#########################
# 초기화
########################
function Init()
{

	if [ -e ./${DOX_CONF} ];then
		rm -rf ./${DOX_CONF}
	fi 

	if [ -e ./${DOX_CONF} ];then
		rm -rf ./${DOX_CONF}
	fi 

	if [ -e ./$WHOAMI ];then
		`rm -rf ./$WHOAMI`
	fi 
}

##################################
# Default Setting Option 만들기
# 옵션 설명은 공식 홈페이지 참조
# http://www.stack.nl/~dimitri/doxygen/manual/config.html
##################################
function make_default_option()
{
	echo "DOXYFILE_ENCODING      = UTF-8" >> ${D_OPTION}
	echo "OUTPUT_LANGUAGE        = Korea" >> ${D_OPTION}
	echo "EXTRACT_ALL            = YES" >> ${D_OPTION}
	echo "EXTRACT_STATIC         = YES" >> ${D_OPTION}
	echo "RECURSIVE              = YES" >> ${D_OPTION}
	echo "SOURCE_BROWSER         = YES" >> ${D_OPTION}
	echo "GENERATE_TREEVIEW      = YES" >> ${D_OPTION}
	echo "HAVE_DOT               = YES" >> ${D_OPTION}
	echo "UML_LOOK               = YES" >> ${D_OPTION}
	echo "CALL_GRAPH             = NO" >> ${D_OPTION}
	echo "CALLER_GRAPH           = NO" >> ${D_OPTION} 
	echo "DOT_PATH               = /usr/bin/dot" >> ${D_OPTION}
	echo "DOT_GRAPH_MAX_NODES    = 1" >> ${D_OPTION}
	echo "OUTPUT_DIRECTORY       = ./${WHOAMI}" >> ${D_OPTION}
}

##################################
# User Setting Option 만들기
##################################
function make_user_option()
{
	echo "PROJECT_NAME           = TEST" >> ${D_OPTION}
	echo "PROJECT_NUMBER         = 0.1" >> ${D_OPTION}
	echo "INPUT                  = test.c sn_worker.c" 
	echo "FILE_PATTERNS          = *.c *.h" >> ${D_OPTION}
	echo "EXCLUDE                =" >> ${D_OPTION}
	echo "EXCLUDE_PATTERNS       =" >> ${D_OPTION}
}

###########################################
# doxygen config 에 Extra tag 추가 만들기
###########################################
function add_option()
{
	echo "INPUT_ENCODING         = euc-kr" >> $DOX_CONF
}

#########################
# Doxygen 파일 만들기
########################
function make_config()
{
	echo "Doxygen Configuration Setting"
	make_default_option
	make_user_option
}

#########################
# Doxygen 파일 만들기
########################
function make_doxygen()
{
	doxygen -g $DOX_CONF
	make_config

# 자동생성된 Doxygen 옵션 변경하기
	while read line; do
		key=`echo $line | awk '{print $1}'`
		sed -i -e "/^$key/ c\
			$line" $DOX_CONF
	done < $D_OPTION

	add_option

# 최종 변경된 doxygen 옵션으로 문서화 진행
	doxygen $DOX_CONF

}

#########################
# 생성된 Doxygen 압축하기
#########################
function compress_doxygen()
{
	rm -rf ${WHOAMI}.tar.gz
	echo "tar cvfz ${WHOAMI}.tar.gz $WHOAMI"
	tar cvfz ${WHOAMI}.tar.gz ./$WHOAMI > /dev/null	
	rm -rf ../${WHOAMI}
	mv ${WHOAMI} ../
}

#########################
# 웹서버에 파일 전송하기
#########################
function send_to_webserver()
{
	scp ./${WHOAMI}.tar.gz $WHOAMI@192.168.22.128:/var/www/html/
}

#########################
######### MAIN ##########
#########################

Init
make_doxygen
compress_doxygen
send_to_webserver
