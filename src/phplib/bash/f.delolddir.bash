#!/bin/bash
# Module: f.delolddir.bash
# FUNCTION: delete old cache directories
# USAGE: typical cronjob-task for each day:
#	 
#	 

TERM=linux
export TERM

if [ -z $1 ]
then
	echo "f.delolddir.bash : Please give a parameter for DIR"
	exit 0
fi


DEST_DIR=$1
echo "... remove old directories from $DEST_DIR"

find  "$DEST_DIR" -type d -maxdepth 1 -ctime +7 -exec rm -r '{}' ';'
echo "o.k."
	


