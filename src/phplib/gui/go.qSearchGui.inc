<?php
require_once('o.CCT_TABLE.info.inc');
require_once 'f.objview.inc';
 
/**
 * for all objects: quick search form
 * - used by e.g. lay.mobile.inc
 * @namespace core::gui::objtab
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class go_qSearchGui {
 	
     /**
     * show select-field for some BO-tables
     * 
     * - show all tables, where: type is BO; user has "read"-right
     */
    function showTableSearchField(&$sqlo) { 
        
        $objLinkLib = new fObjViewC();
    	
        $TabLib = new oCCT_TABLE_info();
        $tab    = $TabLib->getTablesNiceByType($sqlo, 'BO');
    	$seltab = NULL; // selected table
    	
    	$tab = array_merge(array('PROJ'=>'folder'), $tab);
    	
    	// take preselected table from preferences
    	$tmparr = unserialize($_SESSION['userGlob']["f.glob.obj.qsearch"]);
    	if (is_array($tmparr)) $seltab = $tmparr["tablename"];
         
    	// selectpicker: https://developer.snapappointments.com/bootstrap-select/examples
        echo '<select id="tab_search_list"  name="tablename">'."\n";
    	echo '<option value="">--- object type  ---</option>'."\n";
    
    	// ignore these tables, can be searched on other places
    	$badTables = array( 'ABS_CONTAINER', 'BCBATCH_TMPL', 'DEVSIC', 'USER_GROUP', 'ABS_WORKLIST', 'AS_ASR' );
    	
    	foreach($tab as $table => $tab_nice) {
        	
        	if ( in_array($table,$badTables) ) continue;
        	
        	// user has "read"-role-right ?
        	$rights = tableAccessCheck( $sqlo, $table );
        	if ($rights['read']<=0) continue;
        	
    		$tmpselected = "";
    		if ($table==$seltab) $tmpselected = " selected";
    
    		$tab_nice_out = globTabMetaByKey($table, 'SHORT')  . ' '. $tab_nice;
    		//$icon = $objLinkLib->_getIcon($table);
    		echo '<option value="'.$table.'" '.$tmpselected.'>'.$tab_nice_out.'</option>'."\n";
    	} 
        echo "</select>";
    }	

}
