#!/bin/bash 
# import EMPTY DB for  POSTGRES
# import_dump_EMPTY_PG.bash magasin-blk-2019-01-16.expdp.dmp blk XXX

if [ -z $3 ]
then
    echo "Usage: import_dump_EMPTY_PG.bash -d DUMP-nodata -u DBUSER -p NEWPASSWORD -x"
    echo "    DUMPFILE: Postgres dump file"
    echo "    DBUSER: e.g. blk"	
	echo "    NEWPASSWORD : new DB-password"
    echo "    -x drop old user"
	echo "Action: drop old user, create user, import dump file "
	echo "Prerequisites: you must be user POSTGRES"
    echo "- version: 2020-09-02"
	exit 0
fi 

# stop on ERROR ...
set -e

DROP_OLD='no'
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


if [ -z $DBUSER ] || [ -z $DUMPFILE ] || [ -z $NEWPASSWORD ]
then
  echo "... some parameters were missing"
  exit 0
fi

DB_TABLESPACE="${DBUSER}_tab"
DUMPFILE_MOD=/tmp/import_dump_tmp.sql
TMP_SQL_FILE=/tmp/import_dump_01.sql
TMP_SQL_FILE2=/tmp/import_dump_02.sql

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
    echo "DROP ROLE ${DBUSER}_user;" >> $TMP_SQL_FILE
    echo "/* DROP:END */" >> $TMP_SQL_FILE
fi

echo "/* CREATE:START */" >> $TMP_SQL_FILE
echo "CREATE ROLE ${DBUSER}_user; " >> $TMP_SQL_FILE
echo "create user ${DBUSER} with password '${NEWPASSWORD}';" >> $TMP_SQL_FILE
echo "grant all privileges on database magasin to ${DBUSER};" >> $TMP_SQL_FILE
echo "CREATE SCHEMA ${DB_TABLESPACE} AUTHORIZATION ${DBUSER};" >> $TMP_SQL_FILE
echo "ALTER SCHEMA ${DB_TABLESPACE} OWNER TO ${DBUSER};" >> $TMP_SQL_FILE
echo "alter role ${DBUSER} set search_path = ${DB_TABLESPACE}, pg_catalog;" >> $TMP_SQL_FILE



# create schema + user AS ROOT
echo '... create schema'
psql -v ON_ERROR_STOP=ON -a -d magasin  < $TMP_SQL_FILE

#OLD: create DUMPFILE_MOD
#echo '... create modified dump-file'
#cp $DUMPFILE ${DUMPFILE_MOD}
#sed --in-place  -e 's/SELECT pg_catalog.set_config.*//'  -e 's/${DB_TABLESPACE}\.//g'   -e 's/CREATE SCHEMA.*//' ${DUMPFILE_MOD}



# import
#   IMPDP_STR="psql ..."
#   eval $IMPDP_STR

echo '... start import'

psql -v ON_ERROR_STOP=ON -d magasin -U ${DBUSER}  < ${DUMPFILE}

# sequences are now ok ...
# sed -e 's/DBUSER_TAB/'${DB_TABLESPACE}'/'  reset_sequences.sql > $TMP_SQL_FILE2
#psql -v ON_ERROR_STOP=ON  -d magasin -U ${DBUSER}  < $TMP_SQL_FILE2

# create user root
psql  -v ON_ERROR_STOP=ON -d magasin -U ${DBUSER} -c "INSERT INTO DB_USER (LOGIN_DENY, FULL_NAME, NICK,  PASS_WORD, EMAIL) VALUES ('0', 'Root',    'root', 'nopasswd', 'root@noemail.com');"


echo "OK"