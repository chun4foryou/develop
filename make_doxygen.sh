# Developer : ���� ������(������)kim jin kwon
# date : 2016-12-17 
#
#!/bin/bash	

#TODO ������ ��� �Ұ��ΰ�.
WHOAMI=`whoami`
SERVER="10.0.8.206"
STORAGE="/var/www/doxygen/project"

##################################
# User Setting Option �����
##################################
function make_user_option()
{
#TODO dialog�� ����� �� ������
	echo "PROJECT_NAME           = ${PRO_NAME}" >> ${D_OPTION}
	echo "PROJECT_NUMBER         = ${VERSION}" >> ${D_OPTION}
	echo "OUTPUT_DIRECTORY       = ./${PRO_NAME}-${VERSION}" >> ${D_OPTION}
	echo "INPUT                  = ./" >> ${D_OPTION}
	echo "FILE_PATTERNS          = *.c *.h" >> ${D_OPTION}

	echo -n "Exclude File or Directory(delimiter is space): "
	read EXCLUDE_LIST
	if [ "$EXCLUDE_LIST" == "" ];then
		EXCLUDE_LIST=""
	fi	
	echo "EXCLUDE                = ${EXCLUDE_LIST}" >> ${D_OPTION}
	echo "EXCLUDE_PATTERNS       =" >> ${D_OPTION}
}

##################################
# Default Setting Option �����
# �ɼ� ������ ���� Ȩ������ ����
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
	echo "CALL_GRAPH             = YES" >> ${D_OPTION}
	echo "CALLER_GRAPH           = YES" >> ${D_OPTION} 
	echo "DOT_PATH               = /usr/bin/dot" >> ${D_OPTION}
	echo "DOT_GRAPH_MAX_NODES    = 5" >> ${D_OPTION}
}

#########################
# �ʱ�ȭ
########################
function clear_project()
{
	if [ -e ./${D_OPTION} ];then
		rm -rf ./${D_OPTION}
	fi 

	if [ -e ./${DOX_CONF} ];then
		rm -rf ./${DOX_CONF}
	fi 
}

###############################
# ������Ʈ ������ �Է� �޴´�.
###############################
function get_project_info()
{
	echo -n "Project Name : "
	read TMP_NAME
	if [ "$TMP_NAME" == "" ];then
		TMP_NAME="TEST"
	fi	

	PRO_NAME="${TMP_NAME//[[:space:]]/}" # ���� ����

	echo -n "Version : "
	read TMP_VERSION
	if [ "$TMP_VERSION" == "" ];then
		TMP_VERSION="1.0"
	fi	

	VERSION="${TMP_VERSION//[[:space:]]/}" #���� ����

	D_OPTION="${PRO_NAME}-${VERSION}_dox.conf"
	DOX_CONF="${PRO_NAME}-${VERSION}_doxygen.conf"
	clear_project
}




###########################################
# doxygen config �� Extra tag �߰� �����
###########################################
function add_option()
{
	echo "INPUT_ENCODING         = euc-kr" >> $DOX_CONF
}

#########################
# Doxygen ���� �����
########################
function make_config()
{
	echo "Doxygen Configuration Setting"
	make_user_option
	make_default_option

}

#########################
# Doxygen ���� �����
########################
function make_doxygen()
{
	doxygen -g $DOX_CONF
	make_config

# �ڵ������� Doxygen �ɼ� �����ϱ�
	while read line; do
		key=`echo $line | awk '{print $1}'`
		sed -i -e "/^$key/ c\
			$line" $DOX_CONF
	done < $D_OPTION

	add_option

# ���� ����� doxygen �ɼ����� ����ȭ ����
	doxygen $DOX_CONF

}

#########################
# ������ Doxygen �����ϱ�
#########################
function compress_doxygen()
{
	rm -rf ${PRO_NAME}-${VERSION}.tar.gz
	echo "tar cvfz ${PRO_NAME}-${VERSION}.tar.gz ${PRO_NAME}-${VERSION}"
	tar cvfz ${PRO_NAME}-${VERSION}.tar.gz ./${PRO_NAME}-${VERSION} > /dev/null	
}

#########################
# �������� ���� �����ϱ�
#########################
function send_to_webserver()
{
	scp ./${PRO_NAME}-${VERSION}.tar.gz $WHOAMI@${SERVER}:$STORAGE
	ssh $WHOAMI@${SERVER} <<EOF
	cd $STORAGE;
	rm -rf ${PRO_NAME}-${VERSION};
	tar xvfz ${PRO_NAME}-${VERSION}.tar.gz > /dev/null
	chmod -R 775 ${PRO_NAME}-${VERSION}
	rm -rf ${PRO_NAME}-${VERSION}.tar.gz;
EOF
	rm -rf ${DOX_CONF}
	rm -rf ${D_OPTION}
	rm -rf ${PRO_NAME}-${VERSION}
	rm -rf ${PRO_NAME}-${VERSION}.tar.gz

	RED='\033[0;31m'
	NC='\033[0m' # No Color
	echo ""
	echo "===================== Doxygen link======================"
	printf "\n   ${RED}http://${SERVER}:1443/project/${PRO_NAME}-${VERSION}/html/${NC}\n"
	echo ""
	echo "===================== Finish ==========================="

}

#########################
######### MAIN ##########
#########################

get_project_info
make_doxygen
compress_doxygen
send_to_webserver
