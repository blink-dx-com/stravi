#!/bin/bash 
# create SQL-script for POSTGRES
# import_dump.bash magasin-blk-2019-01-16.expdp.dmp blk XXX

if [ -z $3 ]
then
    echo "Usage: import_dump_PG.bash -d DUMPFILE -u DBUSER -p NEWPASSWORD "
    echo "    -d DUMPFILE: Postgres dump file"
    echo "    -u DBUSER: e.g. blk"	
	echo "    -p NEWPASSWORD : new DB-password"
    echo "    -oriu ORIGINAL_USER [optional]"
    echo "    -x drop old user and STOP, no import"
	echo "Action: drop old user, create user, import dump file "
	echo "Prerequisites: you must be user POSTGRES"
    echo "- version: 2020-09-02"
	exit 0
fi 

# stop on ERROR ...
set -e
ORI_DBUSER=""
DUMPFILE=""
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
        -x)
	    DROP_OLD="yes"
	    shift # past argument
	    ;;
	    
	    *)    # unknown option
	    POSITIONAL+=("$1") # save it in an array for later
	    echo "OPTION '$1' unknown!"
	    exit 0
	    ;;
	esac
done

## set -- "${POSITIONAL[@]}" # restore positional parameters

if [ "$DROP_OLD" == "yes"  ]
then 
    if [ -z $DBUSER ] 
    then
      echo "... some parameters were missing: DBUSER"
      exit 0
    fi

fi

if [ "$DUMPFILE" != ""  ]
then 
    if [ -z $DBUSER ] || [ -z $DUMPFILE ] || [ -z $NEWPASSWORD ]
    then
      echo "... some parameters were missing"
      exit 0
    fi 
fi




DB_TABLESPACE="${DBUSER}_tab"
DUMPFILE_MOD=/tmp/import_dump_tmp.sql
TMP_SQL_FILE=/tmp/import_dump_01.sql

echo "INPUT: DBUSER:$DBUSER  DUMPFILE:$DUMPFILE  NEWPASSWORD:$NEWPASSWORD TABLESPACE:${DB_TABLESPACE} DROP-OLD:$DROP_OLD"


# drop old user
if [  -e $TMP_SQL_FILE  ]
then 
    echo "... remove old $TMP_SQL_FILE"
	rm $TMP_SQL_FILE 
fi

if [ "$DROP_OLD" == "yes"  ]
then 
    echo "... drop old user"
    echo "/* DROP:START */" >> $TMP_SQL_FILE
    echo "DROP SCHEMA ${DB_TABLESPACE} cascade;" >> $TMP_SQL_FILE
    echo "REASSIGN OWNED BY $DBUSER TO postgres;" >> $TMP_SQL_FILE
    echo "DROP OWNED BY $DBUSER;" >> $TMP_SQL_FILE
    echo "DROP USER $DBUSER;" >> $TMP_SQL_FILE
    echo "/* DROP:END */" >> $TMP_SQL_FILE
    
    psql -v ON_ERROR_STOP -d magasin   < $TMP_SQL_FILE
    
    echo "READY"
    exit 0
fi



echo "/* CREATE:START */" >> $TMP_SQL_FILE
echo "CREATE ROLE ${DBUSER}_user; " >> $TMP_SQL_FILE
echo "create user ${DBUSER} with password '${NEWPASSWORD}';" >> $TMP_SQL_FILE
echo "grant all privileges on database magasin to ${DBUSER};" >> $TMP_SQL_FILE
echo "CREATE SCHEMA ${DB_TABLESPACE} AUTHORIZATION ${DBUSER};" >> $TMP_SQL_FILE
echo "ALTER SCHEMA ${DB_TABLESPACE} OWNER TO ${DBUSER};" >> $TMP_SQL_FILE
echo "alter role ${DBUSER} set search_path = ${DB_TABLESPACE}, pg_catalog;" >> $TMP_SQL_FILE



# create schema + user AS ROOT
echo '... create schema + user'
psql -v ON_ERROR_STOP -d magasin   < $TMP_SQL_FILE


if ["$ORI_DBUSER" == "" ]
then
    DUMPFILE_MOD=$DUMPFILE
else
    # FUTURE ...
    # create DUMPFILE_MOD
    ORI_tablespace=${ORI_DBUSER}_tab
    echo '... create modified dump-file ${DUMPFILE_MOD}!'
    cp $DUMPFILE ${DUMPFILE_MOD}
    sed --in-place  -e 's/SELECT pg_catalog.set_config.*//'  -e 's/${ORI_tablespace}\.//g'   -e 's/CREATE SCHEMA.*//' ${DUMPFILE_MOD}
fi


# import
#   IMPDP_STR="psql ..."
#   eval $IMPDP_STR

echo '... start import'

psql ON_ERROR_STOP -d magasin -U ${DBUSER}  < ${DUMPFILE_MOD}


echo "OK"