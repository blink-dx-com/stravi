<?php
require_once ('object.oneview.inc');

/**
 * SHOW object table, NO edit fields
 * sub-class used by edit.tmpl.php and others
 * - include code to show the feature-table of a single object
 * @package edit.show.inc
 * @author Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 *
 */
class fFormShowC {
    
    private $objectClass;

    function __construct() {
      $this->formSubLib = new fObjFormSub();
    }
    
    function setObject( 
    	$tablename, 
    	$id // can be NULL
    	) {
    	$this->tablename =$tablename;
    	$this->id = $id;
    	if ( countPrimaryKeys($tablename)==1) $this->has_single_pk=1;
    	else $this->has_single_pk=0;
    	$this->formSubLib->setObj( $tablename, $id );
    	
    	$tmpFileName='o.'.$this->tablename. '.xedForm.inc';
    	$specialFile=$_SESSION['s_sessVars']['AppLabLibDir'].'/objlib/'.$tmpFileName; // on lab
    	if (!file_exists($specialFile)) {
    	    $specialFile=$_SESSION['s_sessVars']['loginPATH'].'/objlib/'.$tmpFileName; // on pionir
    	}
    	
    	
    	if (file_exists($specialFile)) {
    	    // object dependend file
    	    require_once ($specialFile);
    	    $classname ='o'.$this->tablename. '_xedForm';
    	    $this->objectClass = new $classname;
    	    $this->objectClass->set_id($id);
    	}
    }
    
    /**
     * show row
     * @param array $data
     * 'iconimg'
     * 'colnice'
     * 'val'
     * 'notes'
     */
    function row_out( $data ) {
        
        $editRemarks = isset ($_SESSION['userGlob']['editRemarks']) ? $_SESSION['userGlob']['editRemarks'] : 0;
        
        echo " <tr bgcolor=\"EFEFEF\">";
        echo "<td>".$data['iconimg']."</td>";
        echo "<td>".$data['colnice'].'</td>';
        echo "<td>".$data['val'].'</td>';
        if ($editRemarks) {
            echo "<td><i>".$data['notes'].'</i></td>';
        }
        echo "</tr>\n";
        
    
    }
    
    /**
     * SingleForm for SHOW
     * @param object &$sql
     * @param object &$sql2
     * @param array &$arguByCol     arguments by COL_NAME;  OLD: $argu_load[POS]
     * @param array &$colNames    : DEPRICATED columnnames  !
     * @param array &$extraobj_o  : extra_obj params
     * @param array $formopt:
     			"colShowAdvanced" -- show also advanced columns
    			"colModeSplit"  : 0,1 -- show advanced columns as SPLIT
     			"colvSlim"
    			"assocShow" 0,1 -- show associated elements ?
    			"H_EXP_RAW" -- ID for special columns
    			"do_not_close" = 0,1
     * @param array colopt_TYPE $colopt=NULL column+assoc options : 
     * 			array( 'cols'=>COLARR, 'assoc'=>ASSOCARR )
     *				COLARR = array[COLNAME] = array($keys)
     *						 'selACondId' special for 'select by abstract'
     *						 'notes'      use alternative comment
     *				ASSOCARR = array(ASSOCTABLE=>array(keys)) 
     * @todo remove $colsDEPRICATE from parameter list
     */
    function form_show( &$sql, &$sql2, &$arguByCol, &$colsDEPRICATE, &$extraobj_o, $formopt, $colopt=NULL ) { 
    	
    	
    	$id 			= $this->id;
    	$tablename 		= $this->tablename;
    	$has_single_pk 	= $this->has_single_pk;
    	
    	$H_EXP_RAW_DESC_ID = NULL;
    	if ( ($tablename=="EXP_RAW_RESULT") || ($tablename=="EXP_RAW_SPOT_RESULT" || $tablename=="EXP_CHAN_RES") ) {
    	    $sql->Quesel("H_EXP_RAW_DESC_ID from EXP where EXP_ID=".$id);
    	    $sql->ReadRow();
    	    $H_EXP_RAW_DESC_ID=$sql->RowData[0];
    	} 
    	$editRemarks = isset ($_SESSION['userGlob']['editRemarks']) ? $_SESSION['userGlob']['editRemarks'] : 0;
    	$numCols     = sizeof($arguByCol);
    	$mode        = "SHOW";
    	$extra_obj_col_exists = array_key_exists( "EXTRA_OBJ_ID", $arguByCol );
    	
    	$headopt = NULL;
    	if ($formopt['colModeSplit']>0) $headopt = array('colmode' => 1, 'HasAdvCols'=>$formopt['HasAdvCols'] );
    	$this->formSubLib->dataheader_show( $mode, $editRemarks, $headopt ); 
    	
    	$cnt       = 0; /* counter for column */
    	$viewcnt   = 0; /* counter for viewed columns */
    	
    		
    	/***********************************************************************************************/
    	/*** Table view loop ***************************************************************************/
    	/***********************************************************************************************/
    	$colNames = array_keys($arguByCol);
    	$cnt	 = 0; /* counter for column */
    	     
    	foreach( $colNames as $colName ) {
    	
    		$data_val = $arguByCol[$colName];
    		if ($colopt['cols'][$colName]!=NULL) {
    			$colOptRow  = $colopt['cols'][$colName];
    		} else $colOptRow = NULL;
    		
    		$row_show  = 1;
    		$colcomment="";
    		$data_type_ret="";
    		
    		if ( $extra_obj_col_exists && ($cnt == ($numCols-1)) ) { /* to the end of table ... */
    			if ($extraobj_o["extra_obj_id"]) {
    				$XFormLib = new fEditXobjForm($tablename);
    				$XFormLib->showCols( $sql, $viewcnt, 0,  $extraobj_o, $editRemarks, 0 );
    			}		
    		}
    		
    		if ($data_val=="" and $formopt["colvSlim"]) {  // no data ?
    			$row_show = 0;      // do not show row
    		} else {
    			$colInfos   = colFeaturesGet($sql, $tablename, $colName, $H_EXP_RAW_DESC_ID);
    			$colNicename= "";
    			$linked_tab = fk_check2($colName, $tablename, $ret_primary_name, $ret_imp_name); 
    		
    			/* now reduce the visibility */	
    			$colShowAdvanced = 1; // always show ! former flag: $formopt["colShowAdvanced"];
    			$row_show = $this->formSubLib->col_show_test( $row_show, $linked_tab, $colName, $colInfos, $colShowAdvanced);
    		} 
    		// echo "DEBUG_XXX: colName:$colName colShowAdvanced:$colShowAdvanced showrow:".$row_show." data_val:$data_val<br>"; 
    		
    		if ( $row_show ) { /* show data-row ??? */ 
    			if ( $editRemarks ) {
    				$colcomment= column_remark2($tablename, $colName);
    			}
    			
    			$colNicename = $colInfos["NICE_NAME"].$colOptRow['colNameAdd'];
    			
    			echo " <tr bgcolor=\"EFEFEF\">";  // bgcolor=\"" .$rowcolor ."\"
    			if ( $linked_tab ) {
    				$icon="images/icon.".$linked_tab.".gif";
    				if ( !file_exists($icon) ) $icon="images/icon.UNKNOWN.gif";
    				$iconimg = "<img src=\"".$icon."\">";
    			} else $iconimg="&nbsp;";
    			echo "<td>".$iconimg."</td>";
    			
    			if ( $linked_tab ) { /* gibt es Link zu weiterem Table ??? */
    				$obj_tmp_id   = $data_val;
    				$typic_name   = '';
    				$linked_tab_l = strtolower($linked_tab);
    				
    				echo "<td class=\"special2\">";
    				echo $colNicename;
    				echo "</td><td>";	
    				
    				if ($obj_tmp_id!=NULL) {
    				    $typic_name = obj_nice_name($sql, $linked_tab, $obj_tmp_id);
    				}
    				
    				$link_dest = "javascript:return;";
    				if ( $obj_tmp_id ) { 
    					$link_dest = "edit.tmpl.php?t=" . $linked_tab . "&id=".$obj_tmp_id;
    				}
    				$button_value= "[".$obj_tmp_id."]";
    				if ($typic_name) $button_value= $typic_name;	
    				if ( $obj_tmp_id == "" ) { 
    					echo "&nbsp;";
    				} else { 
    					echo "<a class=xl href=\"". $link_dest . "\">";
    					echo "$button_value</a>";
    				}
    				
    				
    				$filename_tmp = "obj.".$linked_tab_l.".xfunc.inc";
    				$retu = file_exists($filename_tmp);
    				if ( $retu ) {
    				   
    					require_once($filename_tmp);
    					$tmp_func= "c".$linked_tab;
    					$tmp_obj = new $tmp_func();
    					if ( method_exists( $tmp_obj, 'inFormShowAtt') and $obj_tmp_id ) {
    					   $extra_txt = $tmp_obj->inFormShowAtt($sql, $obj_tmp_id );
    					   echo "&nbsp;&nbsp;$extra_txt";
    					}
    				}
    				
    				echo "</td>";
    			} else {
    				$APP_DATA_TYPE_ID= appDataTypeGet2($tablename, $colName);
    				$aliasVal = NULL;
    				
    				// check pref-valsin H_VAL_INIT; check only special data-types
    				if (in_array($APP_DATA_TYPE_ID, array(2,5,6,7,8,15))) {
    					$sql->Quesel('NOTES FROM H_VAL_INIT WHERE TABLE_NAME = '.$sql->addQuotes($tablename).
    					  ' AND COLUMN_NAME = '.$sql->addQuotes($colName).' and VALUE='. $sql->addQuotes($data_val) .' and NOTES is not NULL');
    			 		if ($sql->ReadRow() ) {
    			  			$aliasVal =  $sql->RowData[0];
    			 		}
    				}
    						
    				$data_type_ret = fEditContent_S::showByType($sql2, $APP_DATA_TYPE_ID, $data_val, $colName, $colcomment, $colNicename, $aliasVal);
    				echo "</td>";
    			
    			} // end: $linkedtab
    			
    			/* COMMENTS */
    			
    			if ( $editRemarks ) {
    			    $xNotes = $colOptRow['notes'];
    				if ( $data_type_ret != "notes" ) { /* no comment */
    					echo "<td>&nbsp;";
    					if ( $xNotes!=NULL ) {
    						$colcomment = $xNotes;
    					}
    					echo "<I>". $colcomment ."</I>"; /*comment */
    					echo "</td>";
    				}
    			}
    			echo "</tr>\n";
    			$viewcnt++;
    		}  /*end: show column ??? */	
    		$cnt++;
    	
    	}
    	
    	
    	if ( $has_single_pk AND $formopt["assocShow"]>0) {
    	    if ( glob_isAdmin() ) {
    	        $colopt['assoc']['sh.link']  =1; // ADMIN is allowed to see the links
    	        $colopt['assoc']['sh.intern']=1;
    	    }
    	    $this->formSubLib->show_assoc_tabs($sql, $editRemarks, 0, 1, $colopt['assoc']); //$formopt["colvSlim"]
    	}
    	
    	if ( !$formopt["do_not_close"] ) {
    	   $this->formSubLib->datatable_close();
    	}
    }
    
    function x_cols_extra($sqlo, $arguByCol) {
        $colDefOpt = array();;
        if (is_object($this->objectClass))
            if (method_exists($this->objectClass,'x_cols_extra'))
                $colDefOpt = $this->objectClass->x_cols_extra($sqlo, $arguByCol);
                
                
        return $colDefOpt;
    }

}