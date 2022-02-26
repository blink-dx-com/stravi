<?php
/**
 * sub functions for edit.tmpl.php
 * - specific FORM sub functions for view/edit
 * $Header: trunk/src/phplib/edit.sub.inc 59 2018-11-21 09:04:09Z $
 * @package edit.tmpl.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @var VARSTRUCT extraobj_o_STRUCT = array (
			    "extra_obj_id"   -> val, 
				"extra_class_id" -> val, 
				"arguobj" -> array[] 
				"subsel"  -> array[attrib_ids] shows only a sub selection of attrib_ids (OPTIONAL)
			) 
 */


require_once('db_x_obj.inc'); 
require_once('f.objview.inc');
require_once('edit.sub.assoc.inc');
require_once("f.text_html.inc");
/**
 * sub -functions for edit.edit.inc and edit.show.inc
 */
class fObjFormSub {
	var $colmode; // define the column-mode: 'NORM','adv','dyn'

    function __construct() {
    	$this->colModeArr = array(
    		'NORM' =>array('n'=>'1','c'=>'normal attributes'), 
    		'adv'  =>array('n'=>'2','c'=>'advanced attributes'), 
    		'vario'=>array('n'=>'vario','c'=>'vario attributes'),
    	    'att'  =>array('n'=>'attachments','c'=>'attachments', 'i'=>'icon.LINK.gif')
    		);
    	$this->_isInit=0;
    	$this->ObjFormAssocLib = new fObjFormAssoc();
    	
    }
    
    function setObj( $tablename, $id ) {
    	$this->tablename= $tablename;
    	$this->id = $id;
    	$this->_isInit=1;
    	$this->colmode = $_SESSION['s_sessVars']['o.'.$tablename.'.editColMode'];
    	if ($this->colmode==NULL) $this->colmode = 'NORM';
    	$this->ObjFormAssocLib->setObj( $this->tablename, $this->id );
    }
    
    function getColMode() {
    	return $this->colmode;
    }
    
    function _InitTest() {
    	if (!$this->_isInit) sys_error_my( 'class "fObjFormSub" not initialized.' ) ;
    	else return 0;
    }
      
    function tEdit_jsadd() {
    	// FUNCTION: adds javascript code
    	js_formSel();
    }
    
    function getColspan() {
    	return ( $this->colspan );
    }
    
    static function datatab_css() {
      // input.button { border:solid 1px; text-align:left; background-color:#DFDFDF; border-color:#808080}
        
      return (
     "
      .xe { color: gray; text-decoration:underline;}
      .xl { color: #202060;}
      textarea { width:100% }
      .special   { color: #FFFFFF; font-weight:bold; padding-left:0px !important; }
      .specThin   { color: #FFFFFF; font-weight:normal; text-align:right;} 
      td.special2  { color: gray; }
      table.dataTab td  { border-bottom: 1px solid #DFDFDF;}
      table.dataTab tr td:first-child {
         padding-left: 5px;
         padding-right: 3px;  
      }
      tr.nobord td  { border: 0px}
      .xBoxDark {color: #FFFFFF; font-weight:bold; background-color: #3366cc; padding-right:8px; padding-left:8px;}
      .xBoxGray {background-color: #EFEFEF; font-weight:normal; padding-right:8px; padding-left:8px;}
      " 
      );
    }
    
    function datatable_close() { // opened by dataheader_show()
    	echo "</table>\n";
    	//echo "</td></tr></table>\n"; 
    }
    
    /**
     * get primary keys for URL
     * @todo bring into the CLASS !!!
     * @return
     * @param object $id
     * @param object $primas
     * @param object $primasid
     * @return string
     */
    static function urlGetPrimkeys( $id, &$primas, &$primasid ) {
        
        $this_obj="id=".$id;
        if (!empty($primas[1])) {
            $this_obj=$this_obj."&primas[1]=" . $primas[1] . "&primasid[1]=".$primasid[1];
            if (!empty($primas[2])) $this_obj=$this_obj. "&primas[2]=" . $primas[2] . "&primasid[2]=".$primasid[2];
        }
        return ($this_obj);
    }
    
    /**
     * show tabs of main form
     * @param string $now
     * @global $this->colModeArr
     * @return -
     */
    function _shoBoxLink($now, $hasAdvCols) {
    	
    	$backurl = urlencode('edit.tmpl.php?t='.$this->tablename.'&id='.$this->id);
    	
    	
    	foreach( $this->colModeArr as $colmode=> $infoarr ) {
    		$nice=$infoarr['n'];
    		$info=$infoarr['c'];
    		$icon='';
    		
    		if ($hasAdvCols<=0 and $colmode=='adv') continue; // do not show ...
    		if ($infoarr['i']) {
    		    $icon='<img src="images/'.$infoarr['i'].'" height=13> ';
    		}
    		if ($colmode==$now) {
    		    echo '<font class="xBoxDark" title="'.$info.'">'.$icon.$nice.'</font>';
    		} else {
    			echo '<a class="xBoxGray" href="sessVars_set.php?variable=o.'.$this->tablename.
    				'.editColMode&val='.$colmode.'&backurl='.$backurl.'" title="'.$info.'">'.
    				$icon.$nice.'</a>';
    		}	
    	}
    
    }
    
    /**
     * show data-table header
     * @param string $mode   ["show"], "edit"
     * @param int $remarksShow
     * @param array $option
     *   'colmode'    => '0,1 : show advancedColSplit ?
     *   'HasAdvCols' => 0,1
     * @return -
     */
    function dataheader_show( 
        $mode,
        $remarksShow,
    	$option=NULL
        ) {
    	
       $colspan = 3;   // icon, key, val
       $colwidth=array();
       $colwidth["icon"]= "1";
       $colwidth["key"] = "19";
       $colwidth["val"] = "80";
       
       $colwidth["rem"] = "40";
       $colwidth["sel"] = "40";
       
       if ( $remarksShow ) {
         $colspan++;
         $colwidth["val"] = $colwidth["val"] - $colwidth["rem"];
       }
       
       if ( $mode=="EDIT" ) {
            $colspan++;
            $colwidth["val"] = $colwidth["val"] - $colwidth["sel"];
            if ( $remarksShow ) {
                $colwidth["rem"] = 30;
                $colwidth["sel"] = 10;
                $colwidth["val"] = "40";
            }   
            
       }
       
       // border color: was #7777EF, now #C0C0C0
       echo '<table cellspacing="0" cellpadding="1" bgcolor=#DFDFDF class="dataTab" style="border: 1px solid #C0C0C0;">'."\n";
       // echo '<tr><td><table border="0" cellspacing="1" cellpadding="1">'."\n";
        
       echo '<colgroup>'."\n";
       echo '<col width="'.$colwidth["icon"].'%">'."\n";
       echo '<col width="'.$colwidth["key"].'%">'."\n";
       echo '<col width="'.$colwidth["val"].'%">'."\n";   // was 40%
       if ($mode=="EDIT") {
         echo '<col width="'.$colwidth["sel"].'%">'."\n";  // "select"
       }
       if ( $remarksShow ){
          echo '<col width='.$colwidth["rem"].'%>'."\n";
       }
       echo "</colgroup>\n";
       
       echo "<tr bgcolor=\"#C0C0C0\">";   // #CFCFEF
       if ( $option['colmode']>0) {
    		$colmodeSh=1;
       } else $colmodeSh = 0;
       
       $tmp_comments = '&nbsp;&nbsp;&nbsp;&nbsp;<a class=specThin href="preferences.php?variable=editRemarks&val=1">[Comments&gt;&gt;]</a>';
       
       if ( !$colmodeSh ) {
    		echo '<td class="special"><img src="0.gif" width="16" height="1"></td>';  // icon
    		echo '<td class="special" nowrap align=center><img src="0.gif" width="23" height="1">';  // images allow minimum size of column
    		echo 'Column<img src="0.gif" width="23" height="1"></td>';
    		echo '<td class="special">'; //&nbsp;Value
    		if (!$remarksShow)  echo $tmp_comments;
    		echo "</td>";
       } else {
    		// show advanced column SPLIT
    		echo '<td class="special" colspan=3>';  // icon
    		$table   = $this->tablename;
    		$colmode = $_SESSION['s_sessVars']['o.'.$table.'.editColMode'];
    		if ($colmode=='')  $colmode='NORM';
    
    		$this->_shoBoxLink($colmode, $option['HasAdvCols'] );
    		
    		if (!$remarksShow)  echo $tmp_comments;
    		echo '</td>';
       }
       
       if ($mode=="EDIT") echo '<td class="special">&nbsp;Select </td>';
       if ($remarksShow) {
            echo '<td class="special">&nbsp;Comments <a class=specThin href="preferences.php?variable=editRemarks&val=0" title="hide comments">[&lt;&lt;]</a></td>'; // <img src="images/but.infoless.gif" border="0"  TITLE="no comments">
       }
       echo "</tr>\n";
    	$this->colspan = $colspan;
       return ( $colspan ); 
    }
    
    /**
     * reduce the visibility
     * @param int $col_show  is input: 0 or 1  
     * @param string $linked_tab
     * @param string $colName
     * @param array $colInfos
     * @param int $ColShowFlag identifies the Form-Tab: 1 or 2
     * @param int $col_show2 other dynamic column show flag
     * @return number
     */
    function col_show_test(  $col_show, $linked_tab, $colName, 	$colInfos, 	$ColShowFlag, $col_show2=0	) {
    
    	if ( $colInfos['VISIBLE']=="0" ) {
    		$col_show = 0;
    	}	
    	
    	if ($col_show2<0) $col_show=0; // overruled
     	if ( $_SESSION['sec']['appuser']  == "root" ) $col_show = 1; // show ALL 	
    	if ( $colInfos['VISIBLE']=="2" ) {
    		if (!$ColShowFlag) $col_show=0;
    	}
    	
    	// also for root NOT visible !!!
     	if ( $linked_tab == "CCT_ACCESS"   && !$colInfos["PRIMARY_KEY"] ) $col_show = 0;
    	if ( $colName    == "EXTRA_OBJ_ID" && !$colInfos["PRIMARY_KEY"] ) $col_show = 0;
    	if ( $colName    == "PASS_WORD" ) $col_show = 0; // even not root
    	
    	return ( $col_show );
    }
    
    /**
     * show notes field
     * @return -
     * @param string $colName
     * @param string $html_txt
     */
    function notesFieldOut($colName, &$html_txt) {
        
    //   $notescolId  = "g.sof.colNotesSize";
    //     if ( $_SESSION['userGlob'][$notescolId] != "" )  {
    //         $notescolTmp = explode(":",$_SESSION['userGlob'][$notescolId]);
    //         $nwidth = $notescolTmp[0];  // depricated
    //         $nheight= $notescolTmp[1];
    //     }
        
        $nheight = substr_count ( $html_txt, "\n" );
        
        $nwidth=100;
        if ( $nheight<=2 ) $nheight = 3;
        if ( $nheight>30 ) $nheight = 30;
        
        #$nheightbigger=$nheight + 3;
        #if ( $nheightbigger>60 ) $nheightbigger=60;
        #$nheightless=$nheight -3;
        #if ( $nheightless<2 ) $nheightless=3;
        #echo " &nbsp;&nbsp;&nbsp;<font color=gray>size: [<a href=\"preferences.php?variable=".$notescolId."&val=\">reset</a>] ";
        #echo "[<a href=\"preferences.php?variable=".$notescolId."&val=$nwidth:$nheightless\">-</a>] "; 
        #echo "[<a href=\"preferences.php?variable=".$notescolId."&val=$nwidth:$nheightbigger\">+</a>] "; 
        #echo "[<a href=\"help/robo/single_sheet_edit.html#notesfield\" target=_help>?</a>]";             
        #echo "</font><br />\n" ; //  COLSPAN ! 
        
        echo "<textarea rows=".$nheight." cols=".$nwidth."  name=\"" .$colName. "\" >". $html_txt;
        echo "</textarea>\n";
    }
    
    function urlAdvQuery( $colcomment, $variable)  {
    	$tmp_url    = '';
    	$url_pos1   = strpos($colcomment, 'URL:"');
    	$url_pos2   = false;
    	$needle_len = 5;
    	$pos1_str   = substr($colcomment,$url_pos1, 5);
    	if ($pos1_str == 'URL:"' ) { // 'URL:"' in $colcomment
    	  $url_pos1n = $url_pos1 + 5;
    	  $url_pos2  = strpos( $colcomment, '"', $url_pos1n);
    	}
    	if ($url_pos2) {
    	  $tmp_url = substr($colcomment, $url_pos1n, ($url_pos2-$url_pos1n));
    	}
    	if ($tmp_url) {
    		$tmp_len_last = strlen($colcomment) - $url_pos2;
    		$colcomment = substr($colcomment, 0, $url_pos1).substr($colcomment,$url_pos2+1,$tmp_len_last);
    		$url_pos1   = strpos($tmp_url, '$VAL$');
    		if ($url_pos1) { // '$VAR$' in $tmp_url
    		  $url_len = strlen($tmp_url);
    		  $tmp_url = substr($tmp_url, 0, $url_pos1).$variable.substr($tmp_url, $url_pos1+$needle_len, $url_len-$url_pos1-$needle_len);
    		}
    		echo "<a href=\"$tmp_url\" target=help><img src=\"images/earth.gif\" border=0 TITLE=\"Link to\" ></a> ";
    	}
    	return;
    }
    
    
    /**
     * show one entry for associated elements
     * params see fObjFormAssoc::assocShowCol()
     */
    function _assocShowCol( $icon, $colname, $value, $notes, $showRemarks, $extracol, $edittext=NULL ) {
        $this->ObjFormAssocLib->assocShowCol( $icon, $colname, $value, $notes, $showRemarks, $extracol, $edittext );
    }
    
    /**
     * show number/info of asscociated elements of one object
     *
     */
    function show_assoc_tabs( $sqlo, $showRemarks, $showXcol, $slim=NULL, $rowopt=NULL	) {
    	$this->ObjFormAssocLib->show_assoc_tabs( $sqlo, $showRemarks, $showXcol, $slim, $rowopt	);
    }

        
}
        
/**
 * manage EXTRA_OBJ parameter edit/view form
 */
class fEditXobjForm {
	
	/**
	 * overload of attribute definition
	 * @var $_xColInfos
	 */
	private $_xColInfos;

    function __construct( $tablename ) {
    	$this->tablename  = $tablename;
    	$this->_xColInfos = NULL;
    	$this->rowcolor   = '#FFFFBB';
    }
    
    function set_xColInfos(&$xColInfos) {
    	$this->_xColInfos = $xColInfos;
    }
    
    function _get_Extra_pref_val( &$sql, $EXTRA_ATTRIB_ID, $value ) {
    	/* FUNCTION: - select notes from EXTRA_PREF_VAL
    				 - use this to get aliasNames for numberValues
    	   RETURN:   NOTES
    	   TBD:		 - cache once per session, if this attribute has EXTRA_PREF_VALs or not
    	   			 - if not, prevent a SQL-query !
    	*/
    	if ($value=="") return; 
    	$sql->query("SELECT notes FROM extra_pref_val WHERE extra_attrib_id = ".
    				$EXTRA_ATTRIB_ID." AND value=".$sql->addQuotes ($value) );
    	$sql->ReadRow();
    	$notes = $sql->RowData[0];
    	
    	return ($notes);
    }  
    
    function h_select_out(&$sql, $val_this) {
    # descr: create select-box for extra_classes using cache
    # input: val_this   ... preselected value
    #        table_name ... name of the table from which to show the extra_classes
    # return: void
    # errors:
    #         varcol   2 -- varcol-cache empty
    #                109 -- error in getting class_nice_names for table, tablename probably empty
        
        $varcol = & Varcols::get();
        $error  = & ErrorHandler::get();
        $table_name = $this->tablename;
    
        $class_nice_names = $varcol->get_class_nice_names($table_name);
        if ($error->got(READONLY)) return;
    
        if (count($class_nice_names)) {
            echo '<select size="1" name="xargu[CLASS]">'."\n";
            echo '<option value=""> --- no class --- </option>'."\n";
        }
        
        foreach( $class_nice_names as $class_id=> $class_name ) {
            $tmpsel="";
            if ( $val_this == $class_id ) $tmpsel=" selected";
            echo '<option value="'.$class_id.'" '.$tmpsel.'> '.$class_name.'</option>'."\n";
        }
        if (count($class_nice_names))
            echo "</select>\n";
        else
            echo '<input type="hidden" name="xargu[CLASS]" value="">'; // no classes available -> hidden argument
       
        
    }
    
    /**
     * select prefered values (+ notes) from EXTRA_PREF_VAL
     * @return array(CNT => array (VAL, NOTES))
     */
    function _selectValues(&$sql, $EXTRA_ATTRIB_ID ) {
    
    	$values = array();
    	$cnt    = 0;
    
    	$sql->query('SELECT value, notes FROM extra_pref_val WHERE extra_attrib_id = '.$EXTRA_ATTRIB_ID.' ORDER BY value');
    	while ( $sql->ReadRow() ) {
    		if ($cnt >= 40 ) {
    			$values[$cnt] = array('...','...');
    			break;
    		}
    		$values[$cnt] = array($sql->RowData[0], $sql->RowData[1]);
    		$cnt++;
    	}
    	
    	return $values;
    }  
    
    /**
     * manage one column
     */
    private function _one_col($sqlo, $e_attrib_id, $viewcnt) {
        global $_s_i_varcol;
        
        $attrib_edit_allow = $this->select_allowed; // general allow
        $e_attrib_vals = $_s_i_varcol[$this->extraobj_o['extra_class_id']][$e_attrib_id];
        $arguobj       = &$this->extraobj_o['arguobj'];
        $selectVals    = false;
        
        if (is_array($this->_xColInfos[$e_attrib_vals['name']])) {
            // overload the original $e_attrib_vals
            $e_attrib_vals = array_merge($e_attrib_vals, $this->_xColInfos[$e_attrib_vals['name']] );
        }
        
        echo '<tr bgcolor="'.$this->rowcolor.'">';
        echo '<td class="special2">&nbsp;</td>'; // icon-column
        
        $tempval = $arguobj[$e_attrib_vals['name']];
        $columnType_app = appDataTypeNameGet2($e_attrib_vals['app_data_type_id']);
        if ( ($e_attrib_vals['EDITABLE']<0) and ($_SESSION['sec']['appuser']!="root") ) $attrib_edit_allow = 0; // downgrade
        
        if ($attrib_edit_allow) {
            //Bugtrack-ID:0002864:Aenderung $sql2 in $sql
            $selectVals   = $this->_selectValues( $sqlo, $e_attrib_id );
            $html_colspan = "";
            $html_newcol  = "</td><td>";
            if ( $columnType_app == "notes") {
                $colcnt=2;
                if ($this->remarksShow)  $colcnt = $colcnt+1;
                if ($this->selexists)   $colcnt = $colcnt+1;
                $html_colspan = " colspan=".$colcnt;
                $html_newcol  = "";
            }
            $requiredHtml=NULL;
            if ( $this->selexists and $e_attrib_vals['NOT_NULL']>0 ) $requiredHtml = ' <img src="images/redstar.gif">';
            
            echo '<td class="special2"'.$html_colspan.'>'.$e_attrib_vals['nice_name'].$requiredHtml.$html_newcol;
            if (!count($selectVals)) {
                if ( $columnType_app == "notes") {
                    fObjFormSub::notesFieldOut("arguobj[". $e_attrib_vals['name'] ."]", $tempval); // TBD: $tempval must encoded ???
                } else {
                    echo '<input name="arguobj['.$e_attrib_vals["name"].']" value="'.$tempval.'" size=30>';
                }
                
            } else {
                $butid = $this->jsFormLib->getID($viewcnt);
                echo '<select size=0 name=arguobj['.$e_attrib_vals['name'].'] id="'.$butid.'">'."\n";
                $found=0;
                
                foreach( $selectVals as $valis ) {
                    
                    $tmpnote = "";
                    echo "<option value=\"".$valis[0]."\"";
                    if ( $valis[0] == $tempval) {
                        echo " selected";
                        $found=1;
                    }
                    if ($valis[1]!="")  $tmpnote = " --- ".$valis[1];
                    echo "> ". $valis[0] . $tmpnote ."</option>\n";
                }
                
                
                if ( !$found ) echo "<option selected> ". $tempval ."</option>\n";
                if ( $tempval!="" ) echo "<option></option> \n"; // add empty field here
                echo "</select>\n";
                
            }
            $data_type_ret = '';
        } else {
            if ($this->remarksShow) $tempColCom = $e_attrib_vals['comment'];
            else  $tempColCom = "";
            //Bugtrack-ID:0002864:Aenderung $sql2 in $sql
            $aliasval      = $this->_get_Extra_pref_val( $sqlo, $e_attrib_id, $tempval );
            //TBD: please test it $sqlo2 ??
            $data_type_ret = fEditContent_S::showByType($sqlo, $e_attrib_vals['app_data_type_id'], $tempval,
                $e_attrib_vals['name'], $tempColCom, $e_attrib_vals['nice_name'], $aliasval );
        }
        echo '</td>';
        if ($this->selexists && $columnType_app != "notes") {
            echo "<td>&nbsp;";
            if ($attrib_edit_allow) {
                if (count($selectVals)) {
                    $this->jsFormLib->shSelectAlter($viewcnt);
                }
            }
            echo '</td>';
        }
        if ($this->remarksShow && ($columnType_app != 'notes')) {
            echo '<td><I>&nbsp;',$e_attrib_vals['comment'],'</I></td>';
        }
        echo '</tr>',"\n";
    }
    
    /**
     * - show/edit CLASS and parameters
     * - OUTPUT: HTML-FORM: 
     * @param  &$sql, 
     * -- @param  &$sql2, DEPRECATED
     * @param  $viewcnt, 		 // counter of used Input-fields
     * @param  $select_allowed,  // general editallow ?
     * @param  $extraobj_o,	  
     *   'extra_class_id'
     *   'arguobj' : array(name=>val)
     * @param  $remarksShow, 
     * @param  $selexists       //  show extra column for EDIT help buttons like Select-helper button?
     * @REQUIRES: if use in EDIT-MODE: need to call tEdit_jsadd() on start of script!
     * @return:
     *		arguobj[]
     *		xargu[CLASS]
     */
    function showCols( &$sql, $viewcnt, $select_allowed,$extraobj_o, $remarksShow, $selexists) { 

        global $_s_i_varcol;
        $varcol     = & Varcols::get();
        $error      = & ErrorHandler::get();
        
      
        //$arguobj    = & $extraobj_o['arguobj'];
    
    	
    	// glob_printr( $extraobj_o, "showCols::extraobj_o info" );
    	
    
        echo '<tr bgcolor="' .$this->rowcolor .'">';
        echo '<td class="special2">&nbsp;</td>'; // icon-column
        echo '<td class="special2">CLASS</td><td>';
        if ($selexists) {
            $class_select_allowed = $extraobj_o['extra_class_id'] ? 0 : $select_allowed; // not allowed to select new class, if class already selected
            if ( $class_select_allowed ) {
                $this->h_select_out( $sql, $extraobj_o['extra_class_id'] );
            } else {
                $class_nice_name = $varcol->class_id_to_nice_name($extraobj_o['extra_class_id']);
                if ($error->printLast()) return; 
                
                echo '<select size="1" name="xargu[CLASS]">',"\n";
                echo '<option value="'.$extraobj_o['extra_class_id'].'" selected> '.$class_nice_name.'</option>',"\n";
                echo '<option value=""> --- unselect --- </option>',"\n";
                echo '</select>';
              
            }
            echo '&nbsp;</td><td>&nbsp;';
        } else { // view mode
            if ($extraobj_o['extra_class_id']) {
                $class_nice_name = $varcol->class_id_to_nice_name($extraobj_o['extra_class_id']);
                if ($error->printLast()) return;
                echo '<a class=xl href="edit.tmpl.php?t=EXTRA_CLASS&id='.$extraobj_o['extra_class_id'].'">'.$class_nice_name.'</a>';
            }
        }
        
        echo '&nbsp;</td>';
        if ($remarksShow) {
            echo '<td><I>&nbsp;'.($extraobj_o['extra_class_id'] ? $_s_i_varcol[$extraobj_o['extra_class_id']]['comment'] : 'Object class').'</I></td>';
        }
        echo '</tr>',"\n";
        $viewcnt++;
    	
    	
        if ( !$extraobj_o['extra_class_id'] ) {
            return $viewcnt;
        }
      
    	$e_attribs = $varcol->get_attrib_names($extraobj_o['extra_class_id']);
    	if ($error->printLast()) return;
        if (is_array($extraobj_o['subsel'])) $e_attribs = $extraobj_o['subsel']; // take sub selection
        
        $this->select_allowed = $select_allowed;
        $this->extraobj_o     = &$extraobj_o;
        $this->jsFormLib = new gJS_edit();
        $this->remarksShow=$remarksShow;
        $this->selexists=$selexists;
        
        foreach( $e_attribs as $e_attrib_id=>$dummy ) {
            $this->_one_col($sql, $e_attrib_id, $viewcnt);
    	    $viewcnt++;
    	}
       
      
        return $viewcnt;
    }

} // class:END

// ---------------

// STATIC class for content
class fEditContent_S {
    
    /**
     * show content of column (OLD: contentShowByType)
     * 
     * @return string $columnType (modified by $columnType_app)
     * @param object $sql2
     * @param object $APP_DATA_TYPE_ID APPL data type (opt)
     * @param object $content    data of column (mandatory)
     * @param object $colName
     * @param object $colcomment [IN,OUT] comment line for attribute, modified by function
     * @param object $colNicename
     * @param object $aliasval[optional] alias value from extra_pref_val or h_val_init
     */
    static function showByType( &$sql2, $APP_DATA_TYPE_ID, $content,  		
    	$colName, &$colcomment, $colNicename, $aliasval = NULL ) {
    
    	$columnType_app = appDataTypeNameGet2($APP_DATA_TYPE_ID);
    	$columnType     = $columnType_app;
    	
    	if ( $columnType == "notes" ) { // TBD: colspan ???
           // no $colcomment to notes, because they disturb the visual impression
    	   echo "<td colspan=3><table border=0 cellpadding=0 cellspacing=0 width=100%><tr><td width=20%><font color=gray>".$colNicename."</font></td><td width=40%>&nbsp;</td><td width=40%><I>".$colcomment."</I></td></tr></table>"; 
     	}  else echo "<td class=\"special2\">".$colNicename."</td><td>";
    	
    	$html_txt = htmlspecialchars ($content);	
    	if ($aliasval!="")  $html_txt .= " -- ".htmlspecialchars ($aliasval);
    	
    	
    	switch ($columnType) {
    		case 'date':
    			$DATE_MIN_LEN=10;
    			$HHMM_len=5;
    			$dateValOut=$content;
    			if ($content!=NULL) {
    				if (strlen($content)>$DATE_MIN_LEN) {
    					$hhmm=substr($content,$DATE_MIN_LEN+1,$HHMM_len);
    					// cut '00:00' for user convinience
    					if ($hhmm=='00:00') $dateValOut=substr($content,0,$DATE_MIN_LEN);
    				}
    			}
    			echo $dateValOut;
    			break;
    		case "email":
    			echo "<a class=xl href=\"mailto:".$content."\">".$content."</a>";
    			break;	
    		case "password": 
    			echo "***" ;
    			break;
    		case "notes":
    			echo '<pre style="white-space: pre-wrap;">'; // wrap, if line is too long for display
    			if ( $html_txt !="" ) {
    			    f_text_html::notes_out($sql2, $html_txt);
    			}			
    			echo "</pre>";	
    			break;
    		case "url":
    			if ( $content!="" ) {
    				$contentout = $content;
    				if (strstr($content,":")==NULL)  $contentout = "http://" . $contentout;
    				echo "<a class=xl href=\"".$contentout."\" target=help><img src=\"images/earth.gif\" border=0 TITLE=\"Link to\"> ".$content."</a>";
    			} else echo "&nbsp;";
    			break;
    		case "url_query_string":
    		  if ($content != '') { 
    		  	fObjFormSub::urlAdvQuery( $colcomment, $content);
    			echo $content;
    		  } else echo "&nbsp;";
    		  break;
    		default:  
    			echo  $html_txt."&nbsp;" ;
    			break;
    	}
    				
    	return $columnType;
    }
}
