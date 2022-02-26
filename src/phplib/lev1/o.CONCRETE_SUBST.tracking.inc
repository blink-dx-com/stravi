<?php

require_once('db_access.inc');

/**
 * object tracking : support CONCRETE_SUBST 
 * - search for substance-components
 * - search substance in experiments
 * @package o.CONCRETE_SUBST.tracking.inc
 * @swreq SREQ:0002495: g > object tracking : support CONCRETE_SUBST 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class oCONCRETE_SUBST_trackC {
    
    function __construct($id) {
        $this->id = $id;
    }
    
    function gEXP_info($sql) {
        
        $sqls_after= $this->get_after();
        $sqls = "select count(*) from EXP_HAS_PROTO where ".$sqls_after;
        $sql->query($sqls);
        $sql->ReadRow();
        $num_exp = $sql->RowData[0];
        
        $condExp    = "x.EXP_ID in (select EXP_ID from EXP_HAS_PROTO where ".$sqls_after.")";
        
        return array('cnt'=>$num_exp, 'sql'=>$condExp);
    }
    
    /**
     * used in SUCs
     * @param object $sql
     * @return array
     */
    function gSUC_info($sql) {
        
        $subst_sql_after = $this->get_after();
        $subst_sql_after = 'CONCRETE_SUBST_ID in (select CONCRETE_SUBST_ID from CS_HAS_PR where '.$subst_sql_after.')';
        
        $sqls = "select count(*) from CONCRETE_SUBST where ".$subst_sql_after;
        $sql->query($sqls);
        $sql->ReadRow();
        $num_subst = $sql->RowData[0];
        
        return array('cnt'=>$num_subst, 'sql'=>$subst_sql_after);
    }
    
    private function get_after() {
        $id=$this->id;
        $sqls_after = "CONCRETE_PROTO_ID in (select CONCRETE_PROTO_ID from ".
            " CONCRETE_PROTO_STEP where CONCRETE_SUBST_ID=$id)";
        return $sqls_after;
    }

    function track_back( &$sql ) {
        $id=$this->id;
        $sqls_after = $this->get_after();
    	
    	
    	
    	echo "<font color=gray>Used in substance</font>:";
    	
    	$e_info    = $this->gSUC_info($sql);
    	$num_subst = $e_info['cnt'];
    	$subst_sql_after = $e_info['sql'];
      	
      	$condi_url  = urlencode($subst_sql_after);
    	
    	if ($num_subst>0) {
    		echo "<a href=\"view.tmpl.php?t=CONCRETE_SUBST&condclean=1&tableSCond=".$condi_url."\">";
    		echo " <B>$num_subst</B></a>";
    		
    		$sqls = "select CONCRETE_SUBST_ID, NAME from CONCRETE_SUBST where ".$subst_sql_after; 
      		$sql->query($sqls);
    		$cnt=0;
    		echo "<blockquote>\n";
      		while ($sql->ReadRow() && $cnt<=5 ) {
    			$subst_id = $sql->RowData[0];
    			$subst_name = $sql->RowData[1];
    			echo "<img src=\"images/icon.CONCRETE_SUBST.gif\" border=0> <a href=\"edit.tmpl.php?tablename=CONCRETE_SUBST&id=$subst_id\">$subst_name</a><br>\n";
    			$cnt++;
    		}
    		if ($num_subst>5) {
    			echo "<img src=\"images/icon.CONCRETE_SUBST.gif\" border=0> <font color=gray> ... more ...</font><br>\n";
    		}
      		echo "</blockquote>\n";
    		
    	} else echo " <font color=gray>none</font> <br>\n";
    			    
    	echo "<font color=gray>Used in experiment</font>:";
    	
    	$e_info  = $this->gEXP_info($sql);
      	$num_exp = $e_info['cnt'];
      	$condExp = $e_info['sql'];
      	$condExp_url= urlencode($condExp);
    	
    	if ($num_exp>0) {
    		echo "<a href=\"view.tmpl.php?t=EXP&condclean=1&tableSCond=".$condExp_url."\">";
    		echo " <B>$num_exp experiments</B></a>";
    	
    		$sqls = "select e.exp_id, e.NAME from EXP_HAS_PROTO p, EXP e where e.EXP_ID=p.EXP_ID AND ".$sqls_after; 
      		$sql->query($sqls);
    		$cnt=0;
    		echo "<blockquote>";
      		while (($haselem=$sql->ReadRow()) && $cnt<=5 ) {
    			$exp_id = $sql->RowData[0];
    			$exp_name = $sql->RowData[1];
    			echo "<a href=\"edit.tmpl.php?t=EXP&id=$exp_id\"><img src=\"images/icon.EXP.gif\" border=0> $exp_name</a></br>\n";
    			$cnt++;
    		}
    		if ($haselem) {
    			if ($sql->ReadRow()) echo "<img src=\"images/icon.EXP.gif\" border=0> <font color=gray> ... more ...</font><br>\n";
    		}
      		echo "</blockquote>\n";
    	} else echo " <font color=gray>none</font>  <br>\n";
    	
    }

}
