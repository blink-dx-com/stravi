#!/bin/bash
# Module: f.delold.bash
# FUNCTION: delete old cache files
# USAGE: typical cronjob-task for each day:
#	 
#	 

TERM=linux
export TERM

if [ -z $1 ]
then
	echo "f.delold.bash : Please give a parameter for DIR"
	exit 0
fi


DEST_DIR=$1
echo "... remove old files from $DEST_DIR"

# use xargs for long output lists !
find "$DEST_DIR" -ctime +7 | xargs rm
echo "o.k."
	


