#!/bin/bash 
# create SQL-script
# import_dump.bash magasin-blk-2019-01-16.expdp.dmp blk XXX

if [ -z $3 ]
then
    echo "Usage: import_dump.bash -d DUMPFILE -u DBUSER -p NEWPASSWORD -oriu ORIGINAL_USER"
    echo "    DUMPFILE: oracle dump file"
    echo "    DBUSER: e.g. blk"	
	echo "    NEWPASSWORD : new DB-password"
	echo "Action: drop old user, create user, import dump file "
	echo "Prerequisites: you must be user ORACLE, Dump-file is in Oracle-Dir DATA_PUMP_DIR"
	echo "   DATA_PUMP_DIR: e.g. /data/magasin.backups/expdp"
    echo "- version: 2018-11-21"
	exit 0
fi 

POSITIONAL=()
while [[ $# -gt 0 ]]
	do
	key="$1"
	
	case $key in
	    -u)
	    DBUSER="$2"
	    shift # past argument
	    shift # past value
	    ;;
	    -d)
	    DUMPFILE="$2"
	    shift # past argument
	    shift # past value
	    ;;
	    -p)
	    NEWPASSWORD="$2"
	    shift # past argument
	    shift # past value
	    ;;
	    -oriu)
	    ORI_DBUSER="$2"
	    shift # past argument
	    shift # past value
	    ;;
	    
	    *)    # unknown option
	    POSITIONAL+=("$1") # save it in an array for later
	    echo "OPTION '$1' unknown!"
	    exit 0
	    ;;
	esac
done

## set -- "${POSITIONAL[@]}" # restore positional parameters


if [ -z $DBUSER ] || [ -z $DUMPFILE ] || [ -z $NEWPASSWORD ]
then
  echo "... some parameters were missing"
  exit 0
fi

echo "INPUT: DBUSER:$DBUSER  DUMPFILE:$DUMPFILE  NEWPASSWORD:$NEWPASSWORD ORI_DBUSER:$ORI_DBUSER"



TMP_SQL_FILE=/tmp/import_dump_01.sql

# drop old user
if [  -e $TMP_SQL_FILE  ]
then 
    echo "... remove old $TMP_SQL_FILE"
	rm $TMP_SQL_FILE 
fi

echo "DROP USER $DBUSER CASCADE;" >> $TMP_SQL_FILE

# echo "create tablespace blk2_tab datafile '/u01/app/oracle/oradata/XE/blk2_tab.dbf' SIZE 50M autoextend on next 50M MAXSIZE UNLIMITED;" >> $TMP_SQL_FILE
echo "CREATE USER $DBUSER IDENTIFIED BY ${NEWPASSWORD} ;" >> $TMP_SQL_FILE
echo "GRANT cct_user TO $DBUSER;" >> $TMP_SQL_FILE
echo "ALTER USER $DBUSER DEFAULT TABLESPACE ${DBUSER}_tab ;" >> $TMP_SQL_FILE
echo "ALTER USER $DBUSER QUOTA UNLIMITED ON  ${DBUSER}_tab ;" >> $TMP_SQL_FILE

# prepare impdp
echo "GRANT READ, WRITE ON DIRECTORY DATA_PUMP_DIR TO $DBUSER ;" >> $TMP_SQL_FILE
echo "quit" >> $TMP_SQL_FILE

sqlplus "/ as sysdba" @${TMP_SQL_FILE}

# import
IMPDP_STR="impdp $DBUSER SCHEMAS=$DBUSER  DIRECTORY=DATA_PUMP_DIR LOGFILE=import.log dumpfile=$DUMPFILE"

if [ -n $ORI_DBUSER ]
then
	IMPDP_STR="impdp $DBUSER SCHEMAS=$ORI_DBUSER REMAP_SCHEMA=$ORI_DBUSER:$DBUSER remap_tablespace=${ORI_DBUSER}_tab:${DBUSER}_tab DIRECTORY=DATA_PUMP_DIR LOGFILE=import.log dumpfile=$DUMPFILE"
fi

echo '... start import'

eval $IMPDP_STR

echo "OK"