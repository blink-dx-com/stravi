<?php
require_once("object.info.inc");
require_once("edit.sub.inc");

/**
 * show detailed features of one object
 * @package object.oneview.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class objOneviewC {

    var $table;
    private $_is_initialized;
    private $extend_lib;
    
    function __construct($table) {
        
        $this->_is_initialized=0;
        $this->table = $table;
        $this->extend_lib = NULL;
        
        $this->infoObj = new objInfoC();
        $this->objEdSub = new fObjFormSub();
        
        $ext_view_cls_name = 'o'.$table.'_oneviewC';
        if (class_exists ($ext_view_cls_name) ) {
            $this->extend_lib = new $ext_view_cls_name();
        }
    }
    

    
    function set_obj($id) {
    	
        $this->_is_initialized=1;
    	$this->id    = $id;
    	$this->objEdSub-> setObj( $this->table, $id );
    }
    
    function datatab_css() {
    	echo "<style type=\"text/css\">";
    	echo $this->objEdSub->datatab_css();
      echo "</style>\n";
    }
    
    
    
    function dataheader_show( ) {
        
       $colspan = 3;   // icon, key, val
       $colwidth=array();
       $colwidth["icon"]= "1";
       $colwidth["key"] = "19";
       $colwidth["val"] = "80";
       
       echo '<table cellspacing="0" cellpadding="1" bgcolor=#DFDFDF class="dataTab" style="border: 1px solid #6699FF;">'."\n";
       /*
       echo '<colgroup>'."\n";
       echo '<col width="'.$colwidth["icon"].'%">'."\n";
       echo '<col width="'.$colwidth["key"].'%">'."\n";
       echo '<col width="'.$colwidth["val"].'%">'."\n";   // was 40%
       echo "</colgroup>\n";
       */
       echo "<tr bgcolor=\"#CCDDEE\">";   //#6699FF #E8EFF8, #C0C0C0 
       echo '<td class="special"><img src="0.gif" width="16" height="1"></td>';  // icon
       echo '<td class="special" nowrap align=center><img src="0.gif" width="23" height="1">';  // images allow minimum size of column
       echo 'Column<img src="0.gif" width="23" height="1"></td>';
       echo '<td class="special">&nbsp;Value';
       echo "</td>";
       echo '<td class="special"></td>'; // notes
       echo "</tr>\n";
       
       return ( $colspan ); 
    }
    
    function datatable_close() { // opened by dataheader_show()
    	echo "</table>\n";
    }
    
    function headLine(
    	&$sql, 
    	$objid, 
    	$db_access_id, 
    	&$xopt		   // "headLinePreStr", 'showObjLink'
    	) {
    	
    	$table = $this->table;
    	$headLine = NULL;
    	if ( $xopt["showObjLink"]>0 ) {
    		$headLine = $xopt["headLinePreStr"] . "[<a href=\"edit.tmpl.php?t=".$table."&id=".$objid."\">ID: ".$objid."</a>]";
    	}
    	
    	if ( $db_access_id ) {
    		$access_data = access_data_getai( $sql, $db_access_id ); // get new access date, e.g. modifier	
            $headLine .= ' <font color="#999999"><img src="images/ic.usergr2.gif" title="owner" border=0 hspace=2> '.$access_data['owner'].
    			' <img src="images/ic.cread.gif" title="creation" border=0> '.$access_data['crea_date'];
            if ($access_data['mod_date']) $headLine .= ' <img src="images/ic.modd.gif" title="modification" border=0> '.$access_data['mod_date'];
            if ($access_data['modifier'])  $headLine .= ' <img src="images/ic.usergr3.gif" title="modifier" border=0> '.$access_data['modifier'];
            $headLine .= '</font>';
    		
        }
    	return ($headLine);
    }
    
    /**
     * show one object as HTML-table
     * @param  $sql
     * @param  $sql2
     * @param  $table
     * @param  $id
     * @param array $xopt
     *  "showAccess" => 0,1
    	"showObjLink"=> 0,1 : show link to object $id above the table
    	"showObjIDs" => 0,1 : show also ID of foreign objects
    	"headLinePreStr" = pre-string of headline
    	"withAssocs"     => [0],1 show associated elements, if exist
    	"pk2"   - primary key 2
    	"pk3"   - primary key 3
    	"cols"  - array of column names
    	"xcold" - extra column data; e.g. notes behind the value
    	          array(COL=>array('notes'=>string))
     */
    function oneOut( 
    	&$sql, 
    	&$sql2, 
    	$xopt = NULL 
    	) {
    	
    	global $error;
    	
    	if (!$this->_is_initialized) {
    	    throw new Exception('Class: Object not initialized.');
    	}
    	
    	$table = $this->table;
    	$id    = $this->id;
    	
    	$rawOpt = NULL;
    	if (is_array($xopt["cols"]) and sizeof($xopt["cols"])) $rawOpt['cols'] = $xopt["cols"];
    	if ($xopt["pk2"]!=NULL) $rawOpt['pk2'] = $xopt["pk2"];
    	if ($xopt["pk3"]!=NULL) $rawOpt['pk3'] = $xopt["pk3"];
    	$objFeatRaw = $this->infoObj->getParamsRaw($sql, $table, $id, $rawOpt);
    	if ($error->Got(READONLY))  {
        	$error->set("objOneviewC::oneOut", 1, "getting raw features failed.");
    		return;
    	}
    	
    	$objColVals = &$objFeatRaw["vals"];
    	$objColXobj = &$objFeatRaw["xobj"];
    	$extraobj_o = NULL;
    	$primary_key = PrimNameGet2($table);
    	
    	$objFeatNice = $this->infoObj->paramRaw2nice( $sql,$table, $objFeatRaw );
    	
    	if ( $objColVals["EXTRA_OBJ_ID"] )
    		$extraobj_o =  array( 
    			"extra_obj_id"   =>$objColVals["EXTRA_OBJ_ID"], 
    			"extra_class_id" =>$objColXobj['extra_class_id'], 
    			"arguobj" 		 => $objColXobj["values"]
    		);
    	
    	$objFeatNiceVals = &$objFeatNice['vals'];
    	$numCols     = sizeof($objColVals);
    	$cnt         = 0	; /* counter for column */
    	$viewcnt     = 0; /* counter for viewed columns */
    	$objid		 = $objColVals[$primary_key];
    	
    	$headLine = $this->headLine($sql, $objid, $objColVals["CCT_ACCESS_ID"], $xopt);
    	
    	if ( $headLine!=NULL ) echo $headLine. "<br>\n";
    	
    	
    	$this->dataheader_show();
    	$H_EXP_RAW_DESC_ID=0;
    	
    	foreach( $objColVals as $column => $data_val ) {
    	    
    		$row_show = 1;
    		
    		if ( $objColVals["EXTRA_OBJ_ID"] && ($cnt == ($numCols-1)) ) { //  to the end of table ...											   
    			$XFormLib = new fEditXobjForm($table);
    	  		$XFormLib->showCols($sql, $viewcnt, 0, $extraobj_o, 0,  0  );		
    		}
    		
    		if ($data_val=="" ) {   // no data ?
    			$row_show = 0;
    		} else {	
    			$colInfos    = colFeaturesGet( $sql, $table, $column, $H_EXP_RAW_DESC_ID);
    			$linked_tab  = $colInfos["CCT_TABLE_NAME"];
    			$colShowAdvanced = 1;
    			$row_show    = $this->objEdSub->col_show_test( $row_show, $linked_tab, $column, $colInfos, $colShowAdvanced);
    		}
    		
    		
    		if ( $row_show ) { // show data-row ??? 
    			$colNicename = $colInfos["NICE_NAME"];
    		
    			echo " <tr bgcolor=\"EFEFEF\">";  // bgcolor=\"" .$rowcolor ."\"
    			if ( $linked_tab!="" ) {
    				$icon="images/icon.".$linked_tab.".gif";
    				if ( !file_exists($icon) ) $icon="images/icon.UNKNOWN.gif";
    				$iconimg = "<img src=\"".$icon."\">";
    			} else $iconimg="&nbsp;";
    			
    			echo "<td>".$iconimg."</td>";
    			
    			if ( $linked_tab!="" ) { 
    			
    				$obj_tmp_id   = $data_val;
    		
    				
    				echo "<td class=\"special2\">";
    				echo $colNicename;
    				echo "</td><td>";	
    				
    				$typic_name = $objFeatNiceVals[$column]['valnice'];
    				
    				$link_dest = "javascript:return;";
    				if ( $obj_tmp_id ) { 
    					$link_dest = "edit.tmpl.php?t=" . $linked_tab . "&id=".$obj_tmp_id;
    				}
    				
    				$button_value= "[".$obj_tmp_id."]";
    				if ( $typic_name!="" ) $button_value = $typic_name . ' ' . $button_value;	
    				
    				if ( $obj_tmp_id=="" ) { 
    					echo "&nbsp;";
    				} else { 
    					echo "<a class=xl href=\"". $link_dest . "\">";
    					echo "$button_value</a>";
    					//if ($xopt["showObjIDs"]) {
    					//	echo ' [ID:'.$obj_tmp_id.']';
    					//}
    				}
    				echo "</td>";
    				
    			} else {
    				$APP_DATA_TYPE_ID= appDataTypeGet2($table, $column);			
    				$colComment = "";
    				fEditContent_S::showByType($sql2, $APP_DATA_TYPE_ID, $data_val, $column, $colComment, $colNicename);
    				echo "</td>";
    			}
    			echo "<td>";
    			if ( isset($xopt["xcold"][$column]['notes'] ) ) {
    				echo $xopt["xcold"][$column]['notes'];
    			}
    			echo "</td>\n";
    			echo "</tr>\n";
    			$viewcnt++;
    			
    		}
    		$cnt++;
    			
    	}
    	
    	if ( $xopt["withAssocs"] ) {
    		$this->objEdSub->show_assoc_tabs( $sql, 0, 0, 1);
    	}
    	
    	$this->datatable_close();
    	
    	
    	if ($this->extend_lib) {
    	  
    	    $this->extend_lib->set_obj($id);
    	    $this->extend_lib->show($sql, $sql2);
    	}
    
    }

}

class oCONCRETE_SUBST_oneviewC {
    
    function __construct() {
        require_once 'gui/o.CONCRETE_SUBST.protos_show.inc';
    }
    
    function set_obj($obj_id) {
        $this->obj_id = $obj_id;
    }
    
    function show($sql, $sql2) {
        
        echo "<br>Protocol-Log:<br>\n";
        $proto_out_lib = new oCONCRETE_SUBST_protos_show($sql, $this->obj_id);
        if ( $proto_out_lib->has_protos( ) ) {
            $proto_out_lib->writeJavascript();
            $proto_out_lib->show($sql, $sql2);
        }
    }
}
