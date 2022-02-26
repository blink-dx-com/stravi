<?php
/**
 * sub functions for H_ALOG_ACT
 * @namespace core::obj::H_ALOG_ACT
 * @package o.H_ALOG_ACT.subs.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class oH_ALOG_ACT_subs {
    
    // background colors for table cells
    public static $auditColor = array(
        'assigned'  =>'#C8C8FF',
        'released'  =>'#C0FFC0',
        'D_released'=>'#E0FFE0',
        'R_released'=>'#E8FFE8',
        'finished'  =>'#FFD0D0',
        'produced'  =>'#ffffb0',
        'reopen'    =>'#ffd850',
        'QC checked'=>'#E0F0E0',
        'quarantine'=>'#FF8080',
        'rejected'  =>'#FF8080'
    );
    
    const STATUS_reopen  ='reopen';
    const STATUS_released='released';
    const STATUS_tech_ready='tech_ready';
    const STATUS_rejected  ='rejected';
   
    
    static $stateCacheID = array();   // cached array H_ALOG_ACT of ID=>NAME
    static $stateCacheNA = array();   // cache array [TABLENAME][NAME] = ID
    
    static function get_color_of_name($name) {
        return self::$auditColor[$name];
    }
    
    /**
     * transform H_ALOG_ACT_ID to NAME
     * - cache ID=>NAME
     * @param $sql
     * @param $statusid : can be 0
     * @return string name of H_ALOG_ACT
     */
    static function statusID2Name( &$sqlo, $statusid ) {
        
        if (!$statusid) return;
        
        if ( self::$stateCacheID[$statusid]!=NULL ) {
            // get from cache
            return self::$stateCacheID[$statusid];
        }
        
        $sqlsel = "NAME from H_ALOG_ACT where H_ALOG_ACT_ID=".$statusid;
        $sqlo->Quesel($sqlsel);
        $sqlo->ReadRow();
        $name = $sqlo->RowData[0];
        
        self::$stateCacheID[$statusid]=$name;
        
        return ($name);
    }
    
    /**
     * TBD: change definition of table: H_ALOG_ACT:TABLE_NAME: admin must set tables in "", example:"ABSTRACT_SUBST"
     * - get H_ALOG_ACT_ID by name
   
     * - result is cached in self::$stateCacheNA

     * @param  object $sql
     * @param  string $name name of H_ALOG_ACT
     * @return int $H_ALOG_ACT_ID
     */
    static function getH_ALOG_ACT_ID( &$sql, $table, $name ) {
        
        if ( !array_key_exists($table, self::$stateCacheNA) ) self::$stateCacheNA[$table]=array();
        if ( isset(self::$stateCacheNA[$table][$name]) ) {
            return ( self::$stateCacheNA[$table][$name] );
        }
        
        $addSQL = ' and (TABLE_NAME like '.$sql->addQuotes('%'.$table.'%'). ' or TABLE_NAME is NULL)';

        $sqls = "select H_ALOG_ACT_ID, TABLE_NAME  from H_ALOG_ACT where name='".$name."'".$addSQL;
        $sql->query($sqls);
        $sql->ReadRow();
        $retid    = $sql->RowData[0];
        $forTable = $sql->RowData[1];
        if ($forTable==NULL) {
            // only cache it, if the status-name is table independent !
            self::$stateCacheNA[$table][$name] = $retid;
        }
        return ($retid);
    }
	
    /**
     * get all entries in H_ALOG_ACT for a table 
     * @param $sqlo
     * @param [$tablename] optional only for this table
     * @return array of ID => NAME
     */
    static function getActionEntries(&$sqlo, $tablename=NULL) {
    	
    	$iniarr = NULL;
    	$sqlsel  = "H_ALOG_ACT_ID, NAME, TABLE_NAME from H_ALOG_ACT";
    	if ($tablename) {
    	    $sqlsel .= " where TABLE_NAME is NULL or TABLE_NAME like ".$sqlo->addQuotes('%'.$tablename.'%');
    	}
    	$sqlsel .= " order by NAME";
    	$sqlo->Quesel($sqlsel);
    	while ( $sqlo->ReadRow() ) {
    	    
    	    $loop_store = 1;
    	    $loop_tables = $sqlo->RowData[2];
    	    
    	    if ($tablename) {
    	        // detail analysis, search for komma separated tables ...
    	        $loop_store = 0;
    	        if (!$loop_tables) {
    	            $loop_store = 1;
    	        } else {
        	        $table_arr = explode(',',$loop_tables);
        	        if (in_array($tablename,$table_arr)) $loop_store = 1;
    	        } 
    	    }
    	    if ($loop_store) $iniarr[$sqlo->RowData[0]] = $sqlo->RowData[1];
    	}
    	return $iniarr;
    }

}
