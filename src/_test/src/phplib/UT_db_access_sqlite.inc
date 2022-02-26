<?php

class UT_db_access_sqlite_inc  extends gUnitTestSub {
	
function __construct() {
	$this->module_noPreLoad =0;  // 0,1 - do NOT preload ? can be set in constructor of class
	$this->GUI_test_flag    =0;
}

// return: 0 : not passed, 1: passed
function dotest( &$sqlo, $options ) {

    $_dbtype='CDBSqlite';
    $db='/tmp/goz_tmpsqlite.db'; 
    $user=NULL;
    $passwd=NULL;
    
    $sqlite_obj = logon_to_db( $user, $passwd, $db, $_dbtype);
    
    $sqlite_obj->Exec('CREATE TABLE IF NOT EXISTS UNITTEST (
            id INT PRIMARY KEY     NOT NULL,
		    schlag        TEXT     NOT NULL,
            datum_ein     CHARACTER(20)
            )'
        );
    
    $sqlsel = 'max(id) from UNITTEST';
    $sqlite_obj->Quesel($sqlsel);
    $sqlite_obj->ReadRow();
    $cnt = $sqlite_obj->RowData[0];
    $this->_infoNow('CURRENT_CNT:', $cnt);
    
    $sqlite_obj->Query("insert into UNITTEST (id, schlag) values (". ($cnt+1) .",'row".($cnt+1)."')" );
    
    $sqlsel = 'id, schlag from UNITTEST where id>0';
    $sqlite_obj->Quesel($sqlsel);
    $cntshow_MAX=3;
    $cntrow=0;
    while ( $sqlite_obj->ReadRow() ) {
        $id = $sqlite_obj->RowData[0];
        $schlag = $sqlite_obj->RowData[1];
        $this->_infoNow('result:', $id.':'.$schlag);
        if ($cntrow>=$cntshow_MAX) break;
        $cntrow++;
    }
    
    
    $sqlsel = 'max(id) from UNITTEST';
    $sqlite_obj->Quesel($sqlsel);
    $sqlite_obj->ReadRow();
    $cnt = $sqlite_obj->RowData[0];
    $this->_infoNow('CURRENT_CNT:', $cnt);
    
    $MAXINS=10000;
    $this->_infoNow('BEGIN, COMMIT:', $MAXINS. ' times');
    $sqlite_obj->Exec('BEGIN;');
    for ($i=0; $i<$MAXINS; $i++) {
        $sqlite_obj->Query("insert into UNITTEST (id, schlag) values (". ($cnt+1) .",'row".($cnt+1)."')" );
        $cnt++;
    }
    $sqlite_obj->Exec('COMMIT;');
	
	
	return (1);
}

}
