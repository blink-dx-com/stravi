<?php


/**
 * sub functions for edit.tmpl.php => edit.sub.inc
 * - specific FORM sub functions for view/edit
 * $Header: trunk/src/phplib/edit.sub.assoc.inc 59 2018-11-21 09:04:09Z $
 * @package edit.sub.assoc.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class fObjFormAssoc {
	

	
	function __construct() {
		$this->_isInit=0;
		
	}
	
	function setObj( $tablename, $id ) {
		$this->tablename= $tablename;
		$this->id = $id;
		if ($this->id!=NULL) $this->_isInit=1;
		
		
	}
	
	/**
	 * show one entry for associated elements
	 * @param string $icon
	 * @param string $colname
	 * @param string $value
	 * @param string $notes
	 * @param boolean $showRemarks
	 * @param boolean $extracol 0|1 : show extra column (in editmode)
	 * @param string $edittext text for $extracol
	 * @return -
	 */
	public function assocShowCol( $icon, $colname, $value, $notes, $showRemarks, $extracol, $edittext=NULL ) {
	    // bgcolor=#E8E8FF
	    echo '<tr bgcolor=#EFEFEF><td class="special2"><img src="'.$icon.'"></td><td class="special2">'.$colname.'</td><td>';
	    echo $value."</td>";
		if ($extracol)  echo "<td>".$edittext."</td>";   // due to extra column in edit mode
	    if ($showRemarks) {
	        echo '<td>&nbsp;<I>'.$notes.'</I></td>';
	    }
	    echo "</tr>\n";
	}
	
	/**
	 * 
	 * @param object $sql
	 * @param int $slim
	 * @param array $rowopt
	 *    'sh.link'   : 0,1
	 *    'sh.intern' : 0,1
	 * @return array $assoc_cache
	 *  [] = array($icon, $nice_tmp_table, "the nice text", table notes, edit extend text );
	 */
	public function get_assoc_info($sql, $slim, $rowopt) {
	    
	    $tablename = $this->tablename;
	    $id = $this->id;
	    
	    $assoc_tabs = &get_assoc_tables($tablename);
	    
	    if ( !empty($assoc_tabs) ) {
	        
	        $assoc_cache=array();
	        foreach( $assoc_tabs as $tmp_table=> $featarr ) {
	            
	            $nice_tmp_table   = $featarr['nice_name'];
	            $is_view          = $featarr['is_view'];
	            $is_bo            = $featarr['is_bo'];
	            $prim_col         = PrimNameGet2($tmp_table);
	            $prim_col_maintab = PrimNameGet2($tablename);
	            
	            if ($is_bo) { // if this is a BO, the searched object ID is NOT the primary key so search for this object in the table
	                $prim_col = $prim_col_maintab;
	            }
	            
	            $showit=1;
	            if ( $featarr['is_intern']>0 and !$rowopt['sh.intern'] ) $showit=0; // do not show INTERNAL tables !   
	            
	            if ( $is_view ) $showit=0;
	            
	            if ( $showit ) {
	                if ($prim_col=="") continue; // TBD: ERROR !!!
	                
	                $showrow=1;
	                $sql->query('SELECT COUNT(1) FROM '.$tmp_table.' WHERE '.$prim_col.' = '.$sql->addQuotes($id)); // calculate number of values
	                $cnt = ($sql->ReadRow() && $sql->RowData[0] != '') ? $sql->RowData[0] : 0;
	                if ($slim AND !$cnt) {
	                    $showrow=0; // do not show
	                }
	                
	                
	                if (!$showrow) {
	                    continue; // do not show
	                }
	                
	                $extra_info = "";
	                // extend the elements info, e.g. by a link to a view
	                if ( $rowopt[$tmp_table]['tab']!=NULL ) {
	                    $rowOptNow  = &$rowopt[$tmp_table];
	                    $extra_info = " [<a class=xl href=\"view.tmpl.php?t=".$rowOptNow['tab']."&searchMothId=" .$id
	                    . "\">".$rowOptNow['txt']."</a>]";
	                }
	                if ( $rowopt[$tmp_table]['url']!=NULL ) {
	                    $extra_info = ' [<a class=xl href="' .	$rowopt[$tmp_table]['url'] . '">'. $rowopt[$tmp_table]['txt'].'</a>]';
	                }
	                
	                
	                $icon = 'images/icon.'.$tmp_table.'.gif';
	                if ( !file_exists($icon) ) $icon="images/icon.UNKNOWN.gif";
	                
	                $tmpout='';
	                if ( $rowopt['sh.link'] ) {
	                    if ( $is_bo ) {
	                        $tmpout = '<a class=xl href="view.tmpl.php?t='.$tmp_table.'&searchCol='.$prim_col_maintab.'&searchtxt='.$id.'&condclean=1">';
	                    } else {
	                        $tmpout = '<a class=xl href="view.tmpl.php?t='.$tmp_table.'&searchMothId='.$id.'">';
	                    }
	                }
	                $tmpout .= 'elements: <B>'.$cnt.'</B>';
	                if ( $rowopt['sh.link'] ) $tmpout .= '</A>';
	                $tmpout .= $extra_info;
	                
	                // extend the elements edit info, e.g. by [add]
	                if ( $rowopt[$tmp_table]['edit']!=NULL ) {
	                    $editXtend = $rowopt[$tmp_table]['edit'];
	                } else $editXtend = NULL;
	                
	                $assoc_cache[] = array($icon, $nice_tmp_table, $tmpout, table_remark2($tmp_table), $editXtend);
	                
	            }
	        }
	    }
	    return $assoc_cache;
	}
	

	
	/**
	 * show number/info of asscociated elements of one object
	 * 
	 * - check for ASSOC_TABLES
	 * - check for ATTACHMENTS (BOs only)
	 * - check extended info (function x_assoc_show)
	 * 
	 * @return 
	 * @param object $sql
	 * @param int $showRemarks if 1=> show remark column
	 * @param int $showXcol  if 1=> show selection column
	 * @param int $slim[optional] if 1=> show only tables with count>0
	 * @param array  $rowopt[optional] array(TABLE) = info extension
	 *    'url'=> alternative URL
	 *    'txt'=> 
	 *    'tab'=> alternative table
	 *    'sh.link' => 0,1 # show link to the ASSOC table ???
	 */
	public function show_assoc_tabs( &$sql, $showRemarks, $showXcol, $slim=NULL, $rowopt=NULL	) {	
		//global $error;
		//$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		
		if (!$this->_isInit) sys_error_my( 'class "'.__CLASS__.'" not initialized.' ) ;
		$tablename=$this->tablename;
		$id = $this->id;
		
		$assoc_cache = $this->get_assoc_info($sql, $slim, $rowopt);

		if ( !empty($assoc_cache) ) {
		    foreach($assoc_cache as $row) {
		        $this->assocShowCol( $row[0], $row[1], $row[2], $row[3], $showRemarks, $showXcol, $row[4]);
		    }
		}
		
		$showAttach = 0;
		$tabIsBo    = cct_access_has2($tablename);
		if ($tabIsBo) $showAttach = 1;
		// if ($tablename=="LINK")   $showAttach = 0;
		
		if ( $showAttach )  { // can have attachments ???
			
		   $tmp_table = "SATTACH";
		   $sql->query("SELECT COUNT(1) FROM ".$tmp_table." WHERE TABLE_NAME='".$tablename."' AND OBJ_ID=".$sql->addQuotes($id)); // calculate number of values
		   $sql->ReadRow();
		   $cnt = $sql->RowData[0];
		   if ($slim AND !$cnt) { // do not show ???
		   } else {
			$icon      = "images/icon.".$tmp_table.".gif";
			$tmpout    = '<a class=xl href="obj.sattach.view.php?t='.$tablename.'&id='.$id.'">';
			$tmpout    .= 'elements: <B>'.$cnt.'</B></A>';
			$MAX_SHOW  = 7; // max shown attachments in feature form  NEW: request from Cindy to see 7 instead of 6 attachments
			$sql->query("SELECT NAME, REL_ID FROM ".$tmp_table." WHERE TABLE_NAME='".$tablename."' AND OBJ_ID=".$sql->addQuotes($id). " order by REL_ID"); // calculate number of values
		    $shCnt=0;
			while ($sql->ReadRow()) {
		    	// show HTML-Download-link to attachment
		    	if ($shCnt>=$MAX_SHOW) {
		    		$tmpout  .= "<br>\n&nbsp;... more ...";
		    		break;
		    	}
		    	$tmpout  .= "<br>\n".'<a href="obj.sattach.down.php?t='.$tablename.'&id='.$id.'&rel_id='.$sql->RowData[1].'">'.
		    		'<img src="images/icon.LINK.gif" border=0> '. htmlspecialchars($sql->RowData[0]).'</a>';
		    	$shCnt++;
			}
			$extratext = "[<a href=\"obj.sattach.edit.php?tx=$tablename&id=$id&action=insert\">Add</a>]";
			$this->assocShowCol( $icon, "attachments", $tmpout, "attached documents", $showRemarks, $showXcol, $extratext);
		   }
		}
		
		if (function_exists('x_assoc_show')) {
		  	$dataarr = x_assoc_show( $sql, $id );
		  	foreach( $dataarr as $datax ) {
				$this->assocShowCol( $datax[0], $datax[1], $datax[2], $datax[3], $showRemarks, $showXcol);
			}
			
		
		}
	}
}
