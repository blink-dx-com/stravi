<?php
require_once("class.obj.search_usage.inc");

/**
 * show usage of object
 * @package glob.obj.usagegui.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class gObjUsageGui {

    function __construct() {
    	$this->obj_num_all=0;
    }
    
    /**
     * show, where object is used
     * @param $sql
     * @param $sql2
     * @param $tablename
     * @param $id
     * @param $CCT_ACCESS_ID
     * @param array $opt 'show'  "-1" : hide, "1" : show
    
     */
    function show_usage( &$sql, &$sql2, $tablename, $id, $CCT_ACCESS_ID, $opt=NULL ) {
    
        $usage_txtout=0;
        $tmpheadtxt  = "<ul><table border=0 cellpadding=0 cellspacing=2 bgcolor=#C89800><tr><td><table border=0 cellpadding=2 cellspacing=0 bgcolor=#C89800><tr><td>\n";
        $tmpheadtxt  .= "<font color=#FFFFFF><B>Used by object(s)</B> (Modifications change their properties)</font></td></tr><tr><td bgcolor=#FFFFFF>"; 
    
    	// echo "<B>Show objects which rely on this one: </B><P>";
    	
        $showflag = 1;
        if ( $opt['show']<0 ) $showflag = 0;
    	$sopt = array( "showMotherObj"=>1 );
    	
    	$objSearch = new object_usage($sql, $tablename, $id, $showflag, $tmpheadtxt, $sopt );
    	$tab_track_info = $objSearch->start($sql);
    	foreach ($tab_track_info as $row) {
    	    $objSearch->getNumOneTab($sql, $row['pa_t'], $row['pa_pk']);
    	}
    	
    	$this->obj_num_all = $objSearch->obj_num_all;
    	
    	if ($objSearch->obj_num_all > 0 and $showflag) {
    		// echo "<B>WARNING:</B> Modifications on this object will change the meaning of related objects!<br>";
    		$usage_txtout=1;
    	}
    
        
        if ( $CCT_ACCESS_ID ) {
             $sqls="select WIID, ROID from CCT_ACCESS where CCT_ACCESS_ID=". $CCT_ACCESS_ID;
             $sql->query("$sqls");
             $sql->ReadRow();
             // $acc_wiid=$sql->RowData[0];
             $acc_roid=$sql->RowData[1];
             if ($acc_roid) {
                    if (!$usage_txtout)  echo "<ul><table border=0 cellpadding=0 cellspacing=2 bgcolor=#C89800><tr><td><table border=0 cellpadding=2 cellspacing=0 bgcolor=#C89800><tr><td>\n".
                        "<font color=#FFFFFF><B>Object comes from import</B></font></td></tr><tr><td bgcolor=#FFFFFF>"; 
        
                    else  echo "<br>";
                    echo "<font color=#A00000><B>INFO:</B></font> This object has been imported. See <I>access</I> for more information.  ";
                    $usage_txtout=1;
             }
        }
        if ($usage_txtout) {           
            echo "</TD></TR></TABLE></TD></TR></TABLE>\n</ul>\n";
        }
    }
    
    function getObj_num_all() {
    	return $this->obj_num_all;
    }

}
