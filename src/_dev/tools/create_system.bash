#!/bin/bash 
# install the Gozilla code from a Source-TAR-Ball
# module: create_system.bash
# call-example: steffen> bash _dev/tools/create_system.bash LIMS .
# @version $Header: /src/_dev/tools/create_system.bash
# FUTURE: ZIP
# import shutil
# shutil.make_archive(output_filename, 'zip', dir_name)


# the running system
RUN_SYSTEM_DIR=/opt/gozilla
removeUnitTests=1
doInstall=0
APP_NAME=

function usage {
    echo "Usage: create_system.bash  -a APP -u -i -d /opt/gozilla"
    echo "    a APP = 'LIMS'"
    # echo  "   SRCDIR: directory of sources: e.g. /home/steffen/html/src"	
	echo  "   -u  : with unitTests"
	echo "    -i : install the package to DEST-dir $RUN_SYSTEM_DIR"	
    echo "    -d : install to given DEST-dir"
	echo "  example: create_system.bash LIMS . --withUT"
    echo "- copy gozilla to TEMPDIR ..."
    echo "- delete old TEMPDIR ..."

    echo "- version: 2021-06-02 : complete new call options"
}

if [ -z $2 ]
then
    usage
	exit 0
fi 

exit_abnormal() {
    echo "ERROR"
    exit 1
}

optstring=":a:uid:"

while getopts ${optstring} arg; do
  case ${arg} in
    a) APP_NAME=$OPTARG;;
    u) removeUnitTests=0;;
    i) doInstall=1;;
    d) RUN_SYSTEM_DIR=$OPTARG
        if [ -z $RUN_SYSTEM_DIR  ]
        then 
            echo "ERROR: Destination-Dir must be set."
            exit 1
        fi
        ;;
    ?)  
      echo "Invalid option: -${OPTARG}."
      exit 2
      ;;
  esac
done



INSTALL_DIR=`pwd`
SRC_TEMP_DIR=$INSTALL_DIR/src
DIRDEST="${SRC_TEMP_DIR}"

CURDATE=`date`
SOURCEDIR=.
SRC_ZIP_FILE=$SOURCEDIR/src.zip
TEMP_TAR_FILE=${DIRDEST}/FILE_tmp.ztar

echo "DEST-dir: $RUN_SYSTEM_DIR"
echo "APP-NAME: $APP_NAME"
echo "Do-INSTALL? $doInstall"
echo "UnitTests-Remove? $removeUnitTests"


if [ $doInstall -eq 1 ]
then
	
    if [ ! -e $RUN_SYSTEM_DIR  ]
    then 
        echo "ERROR: Destination-Dir $RUN_SYSTEM_DIR not exists"
        exit 1
    fi
fi


if [  -e $SRC_TEMP_DIR  ]
then 
    echo "Remove old SRC-Dir"
	rm -r $SRC_TEMP_DIR 
fi

if [ ! -e $SRC_ZIP_FILE  ]
then 
	echo "ERROR: $SRC_ZIP_FILE not found"
	exit 1
fi

if [ "$APP_NAME" == "" ]
then
	echo "ERROR: APP-type missing."
    exit 1
fi

echo "unpack ZIP"
unzip -q $SRC_ZIP_FILE

#
#
#
echo "... copy partisan from $SOURCEDIR to $DIRDEST";
echo "APPLICATION: $APP_NAME" 
echo "StartDate: $CURDATE" 

if [ ! -e $SRC_TEMP_DIR  ]
then 
	echo "ERROR: SRCDIR not found :$SRC_TEMP_DIR"
	exit 1
fi






echo "... goto $SRC_TEMP_DIR"
cd $DIRDEST

nowpwd=`pwd`
echo "dirnow: $nowpwd"

if [ -e FILE.ztar  ]
then
	rm -f FILE.ztar
fi



echo "... next commands copy special LIMS-version scripts"
 
if [ "$APP_NAME" == "LIMS" ]
then
	echo "!!! build Clondiag intern LIMS"
    echo "... copy ./_LIMS-dir"; 
    cd $DIRDEST/_app_types/LIMS
    cp -Rp . ..
fi


if [ "$APP_NAME" == "portal" ]
then 
	echo "!!! build common Portal (without lab-dir)"
    echo "... remove www/lab-dir";
    rm -R $DIRDEST/www/lab_blk 
    
    echo "... remove lab-dir";
    rm -R $DIRDEST/lab
    
fi

cd $DIRDEST

echo "... remove _app_types dir ..." 
rm -R _app_types

## servers
# TBD: just leave the needed server !!!
# rm -R _servers

#remove unit-tests (changed in 2017-09-26)
if [ $removeUnitTests -eq 1 ]
then
	echo "... remove UnitTest environment"
	rm -R www/_tests
	rm -R _test
else 
	echo "... keep UnitTest environment"
fi


#echo "... remove .svn, CVS, *.bck, *.*~ dirs now"
#rm -R `find . -name ".svn"`
#rm -R `find . -name "CVS"`
#rm -R `find . -name "*.bck"`


# echo "... cp -R /home/steffen/projekt/Partisan_pionir/robo_main/out/* www/pionir/help/robo/"
# cp -R /home/steffen/projekt/Partisan_pionir/robo_main/out/* www/pionir/help/robo/
 
echo "... remove local config-directory"

cd $DIRDEST
rm -r config

nowpwd=`pwd`
echo "... dirnow: $nowpwd"

echo "... build setup "
tar --exclude=FILE.ztar -czf  FILE.ztar .




if [ $doInstall -eq 1 ]
then
	echo "... start installation on DIR: $RUN_SYSTEM_DIR"
	cp FILE.ztar $RUN_SYSTEM_DIR
	cd $RUN_SYSTEM_DIR
	tar -xzf - < FILE.ztar
else 
	echo ""
	echo "MAY: compact: tar --exclude=FILE.ztar -czf  FILE.ztar . "
	echo "MAY: show content:  tar -tvzf FILE.ztar | more"
fi

#echo "cp FILE.ztar /opt/magasin"
#echo "cd /opt/magasin"
#echo "EXTRACT: tar -xzf - < FILE.ztar"

ENDDATE=`date`
echo "ReadyDate: $ENDDATE"
