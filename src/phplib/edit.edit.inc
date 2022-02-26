<?php
/**
 * functions for the HTML-edit-form for single objects
 * @package edit.edit.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @var array typedef colopt_TYPE:column options : 
	 array( 'cols'=>COLARR, 'assoc'=>ASSOCARR, 'ftx'=> FEATURE_TAB_ARR )
		COLARR = array[COLNAME] = array($keys)
				 'selACondId' special for 'select by abstract'
				 'show'     : -1, 1 show the column ?
				 'EDITABLE' : 0,1
				 'colNameAdd' additional text for column name
				 'notes'      use alternative comment
		ASSOCARR = array(ASSOCTABLE=>array(keys) 	
		FEATURE_TAB_ARR = array() -- extend the feature table by new rows
 */

require_once ('edit.sub.inc');
require_once ('object.oneview.inc');
require_once ('f.prefvals.inc');
require_once ('javascript.inc');
require_once ('date_funcs.inc');
require_once ('glob.obj.advmod.inc');
require_once ('f.update.inc');
require_once ('glob.obj.update.inc');
require_once ('o.H_UNIT.siSubs.inc');


/**
 * abstract class for files o.{TABLE}.xedForm.inc
 *
 */
class gui_xedForm_ABS {
	
	var $colInfoArr =NULL; // - extra column meta definitions
	protected $obj_id;
	
	function set_id($obj_id) {
	    $this->obj_id = $obj_id;
	}
	/**
	 * define special column features
	 * - can set $this->colInfoArr
	 * @param object $sqlo
	 * @param array  $arguByCol
	 * @param object $extraobj_o
	 * @return array $colInfoArr - extra column meta definitions
	 *   e.g. $colInfoArr['QUANTITY']['NOT_NULL'] = 1;
	 *    $colInfoArr['class']['Kuerzel']['NOT_NULL'] = 1 
	 *    $colInfoArr['class']['Kuerzel']['EDITABLE'] = -1 
	 */
	function getColFeatures(&$sqlo, &$arguByCol, &$extraobj_o, $showCols) {}
	
	/**
	 * check input
	 * - throws errors
	 * - can manipulate $argu
	 * @throws array errors
	 * @return -
	 */
	function paramCheck(&$sql, &$argu, &$extraobj_o, $action, $showCols ) {}
	
	// get column specials
	function x_cols_extra(&$sqlo, &$arguByCol) {
	    return array();
	}
	
}

/**
 *  - functions for the HTML-edit-form for single objects
 *  
 *  - EXTEND table functionality: require file: o.{TABLE}.xedForm.inc
 *    - contains a class oTABLE_xedForm
 *    - contains function getColFeatures(), paramCheck() 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @global DB-TABLE:USER_RIGHT COL:NAME VAL:f.o.TABLE.editColAdv comment: use a role-right for advanced edit
 */
class fFormEditC {

var $_action;
var $_colopt; /* option array () keys:
		['cols'] = 
			(array of $colName) 
				= array of keys: ['selACondId'] = ABSTRACT_SUBST_ID (for predefined selection)
		*/
var $infox; /* array of collected info values: can be set in set_infox_val() 
	'f.editColAdv' : advanced right: allow edit of column with $colInfos["EDITABLE"]=2
	*/
private $has_single_pk;

function __construct() {
	$this->jsFormLib = new gJS_edit();
	$this->showRemarks = 0;
	$this->formSubLib = new fObjFormSub();
	$this->prefValLib = new fPrefValsGet();
	
	$this->infox=NULL;
}

function javaHead() {
	js_formAll();
	echo '<style type="text/css">'."\n";
	echo $this->formSubLib->datatab_css();
	echo '</style>'."\n";
}

/**
 * 
 * @param int $id  // can be NULL
 */
function setObject( $tablename, $id) {
	
	
	$this->tablename =$tablename;
	$this->id = $id;
	$this->object_is_bo = cct_access_has2($tablename);
	if ( countPrimaryKeys($tablename)==1) $this->has_single_pk=1;
	else $this->has_single_pk=0;
	
	$this->formSubLib->setObj( $tablename, $id );
	$editRemarks       = isset($_SESSION['userGlob']['editRemarks']) ? $_SESSION['userGlob']['editRemarks'] : 0;
	$this->showRemarks = $editRemarks;
	
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

function _colDateButNow( $colName ) {
	$UNIX_MIN_ONE = 60;
	$timeNowHuman = date_unix2datestr(time() , 2);
	
	$retx = "<img src=\"images/but.timenow.gif\" onclick=\"document.editform.elements['argu[". 
		$colName ."]'].value='" .$timeNowHuman. "';\" border=0 title=\"date + time now\"> ".
		"<a href=\"javascript:open_info('f.help.php?f=single_sheet_edit.html#datefield')\">?</a>\n";
	return ($retx);
}

/**
 * form initial checks
 * @param array $arguByCol data input from user (or origin data from database)
 * @param array $showCols  shown columns in form
 */
function _initForm(&$sql, &$arguByCol, &$extraobj_o, &$showCols) {
	$this->_xColInfos = null;
	
	// get special column-info for object
	if (is_object($this->objectClass)) 
		if (method_exists($this->objectClass,'getColFeatures'))
			$this->_xColInfos = $this->objectClass->getColFeatures($sql, $arguByCol, $extraobj_o, $showCols);
}

/**
 * output one ROW
 * @param array $colInfos : 'NOT_NULL'
 */
function oneRow($colNicename, $inputCol, $comment, $colInfos) {
	$iconimg=NULL;
	echo " <tr bgcolor=\"#EFEFEF\">";
	echo "<td>".$iconimg."</td>";
	echo '<td class="special2">';
	echo $colNicename;
	if ( $colInfos['NOT_NULL']  ) echo " <img src=\"images/redstar.gif\">";
	echo "</td><td>".$inputCol;
	echo "</td><td nowrap>&nbsp;</td>";
	if ( $this->showRemarks ) {
		echo "<td nowrap>".$comment."</td>";
	}
	echo '</tr>'."\n";
}

/**
 * show select box
 * @return 
 * @param object $sql
 * @param object $linked_tab
 * @param object $val_this
 * @param object $colName
 * @param object $extra_condition e.g. for EXTRA_CLASS to select 
 * 				special values for the table 			  
 */
function _h_select_out( &$sql, $linked_tab, $val_this, $colName, 
		$extra_condition  ) {

    $extra_tmp    = $extra_condition ? ' WHERE '.$extra_condition : '';
	$outname      = 'NAME';
	$h_pk_name    = PrimNameGet2($linked_tab);
	//$argu_id_name = 'argu';
	$argu_def     = "argu[$colName]";
	
	if ($linked_tab=="EXTRA_CLASS") {
		$outname = "NAME";
		$argu_def = "xargu[CLASS]";
	}
	
	
	$sql->query('SELECT '.$h_pk_name.', '.$outname.' FROM '.$linked_tab.$extra_tmp.' ORDER BY '.$outname);
	
	$found=0;
	$cnt=0;
	while ( $sql->ReadRow() ) {
		$h_id  = $sql->RowData[0];
		$h_name= $sql->RowData[1];
		if (!$cnt) {
			echo "<select size=0 name=".$argu_def." > \n";
			echo "<option value=\"\" > &nbsp;</option>\n";
		}
		echo "<option value=\"".$h_id."\" ";
		if ( $h_id == $val_this) {
			echo " selected";
			$found=1;
		}			
		echo "> ".$h_name ."</option>\n";
		$cnt++;
	}
	if ( $cnt ) {
		if ( !$found ) {
			echo "<option value=\"".$val_this."\" selected";	
			echo "> ". $val_this ."</option>\n";
		}
		echo "</select>\n";
	} else echo "&nbsp;<input type=hidden name=$argu_def  value=\"\" >" ; /* no values available -> hidden argument */
	echo "</td><td>&nbsp;";	
}

/**
 * handle LINKED-table
 */
function _linkedTab( &$sql, $colName, $linked_tab, $select_allowed,
		&$colInfos, $this_colval, $viewcnt, $colNicename ) {
	
	
	$tablename = $this->tablename;
	$id 	   = $this->id;
	
	$linked_isbo = cct_access_has2($linked_tab);
	
	echo "<td class=\"special2\">";
	echo $colNicename;
	if ( $select_allowed && $colInfos['NOT_NULL']  ) echo " <img src=\"images/redstar.gif\">";
	
	echo "</td><td>";
	
	if ( substr($linked_tab, 0,2) != 'H_' ) { // "H_"-table or normal link ?
		
		if ( $this_colval != NULL ) { 
		    $typic_name  = obj_nice_name($sql, $linked_tab, $this_colval);
			$button_value= "[".$this_colval."]";
			if ($typic_name!=NULL) $button_value= $typic_name;	
		} else $button_value=NULL;

		$this->jsFormLib->setTable( $linked_tab, $viewcnt );
		$out =  $this->jsFormLib->getObjLnk( 'argu['.$colName.']', $this_colval, $button_value);
		echo $out;
				 
		echo "</td><td nowrap>";

		if ( $select_allowed ) {
			$osopt = NULL;
			$selvar = "o.".$tablename.".".$colName.".sel";
			
			if ($_SESSION['userGlob'][$selvar]!="") {
				$linked_tab_take = $_SESSION['userGlob'][$selvar];
				$bgcol_tmp       = $colName;
				$osopt['liTab'] = $linked_tab_take;
				$osopt['liCol'] = $bgcol_tmp;
			}
			
			$out =  $this->jsFormLib->getObjSelector( $osopt );
			$selACondId = $this->_colopt['cols'][$colName]['selACondId'];
			if ( $selACondId != NULL ) {
				$butIdTmp = $this->jsFormLib->butid;
				$tmp_cond = '&condclean=1&tableSCond=x.ABSTRACT_SUBST_ID='.$selACondId; // TBD: make it generic for other tables ...
				echo "<a href=\"javascript:openwin('CONCRETE_SUBST','".$butIdTmp."','".$tmp_cond."','')\">";
				echo "<IMG src=\"images/select_abs.svg\" TITLE=\"select by abstract\"></a> ";
			}
			echo $out;
			
		} else {
			echo "<font color=gray>identifier</font>";
		}
	} else {
	    
	    do {
	        if ( ($tablename=='ABSTRACT_SUBST' or $tablename=='PUR') and $colName=='H_UNIT_ID') {
    			$tmp_si_obj = new oH_UNIT_siSubs();
    			$extraWhere = $tmp_si_obj->get_si_sql_cond($sql);
    			$this->_h_select_out( $sql, $linked_tab, $this_colval, $colName, $extraWhere );
    			break;
    		} 
    		if ($tablename=='ABSTRACT_SUBST' and $colName=='CONC_UNIT_ID') {
    		    $tmp_si_obj = new oH_UNIT_siSubs();
    		    $extraWhere = $tmp_si_obj->get_concentration_sql_cond($sql);
    		    $this->_h_select_out( $sql, $linked_tab, $this_colval, $colName, $extraWhere );
    		    break;
    		} 
    		    
    		// default
    		$this->_h_select_out( $sql, $linked_tab, $this_colval, $colName, "" );
    		
	    } while(0);
	} 
}

/**
 * show table header
 * @param array $initarr 'action', 'colModeSplit', 'HasAdvCols'
 * @return -
 */
function formHead( $initarr=NULL ) {
	$this->formSubLib->tEdit_jsadd();
	if ($initarr['action']!=NULL) {
		echo '<form name="editform" style="display:inline;" method="post" action="'.$initarr['action'].'">';
		echo "\n";
	}
	$headoption=NULL;
	if ($initarr['colModeSplit']) $headoption = array('colmode'=>1, 'HasAdvCols'=>$initarr['HasAdvCols'] );
	$this->formSubLib->dataheader_show( "EDIT", $this->showRemarks, $headoption );
}


/**
 * show SELECT box
 * @param string $colName
 * @param string $colval
 * @param int $viewcnt
 * @param array $selectVals
 *   of array(key, val)
 */
private function _selvals( $field_name, $colval, $viewcnt, &$selectVals ) {
	
	$butid = $this->jsFormLib->getID($viewcnt);
	echo '<select size=0 name="' .$field_name. '" id="'.$butid.'" > '."\n";
	$found=0;
	foreach( $selectVals as $valis) {
		echo "<option value=\"".$valis[0]."\" ";
		if ( $valis[0] == $colval) {
			echo " selected";
			$found=1;
		}	
		echo "> ". $valis[1] ."</option>\n";
	}
	
	if ( !$found ) {
		echo "<option selected";	
		echo "> ". $colval ."</option>\n";
	}
	echo "</select>\n";

	echo "</td><td>";
	$this->jsFormLib->shSelectAlter($viewcnt);
}

// HTML > INPUT > DATALIST
private function _field_datalist( $colName, $valueNew, $viewcnt, $selectVals ) {
    $butid = $this->jsFormLib->getID($viewcnt);
    echo '<input name="argu[' .$colName. ']" value="'.$valueNew.'" list="'.$butid.'"> '."\n".
        '<datalist id="'.$butid.'">'."\n";
    
    foreach( $selectVals as $valis) {
        echo '<option value="'.$valis[0].'">'.
           $valis[1] .'</option>'."\n";
    }

    echo "</datalist>\n";
    echo "</td><td>";
}

/**
 * FUTURE: a SIMPLE column, still not tested
 * all configs are given in arguments, no database calls here
 * @param object $sqlo
 * @param mixed $valueNew
 * @param int $viewcnt
 * @param string $fieldname
 * @param array  $colInfos
 *   ... used fields ...
 *   'NOT_NULL'
 *   'EDITABLE'
 *   'COMMENTS'
 *   'column_type' :
 *   'select_arr' : array for select field
 * @param array $colopt
 */
function oneColumnSimple( object $sqlo, $valueNew, int $viewcnt, string $fieldname, array $colInfos, $colopt = array() ) {
    
    $editAllow = $this->editAllow;
    if ( !$colInfos["EDITABLE"] )   $editAllow = 0; // denied
    $column_type = $colInfos["column_type"];
    
    echo " <tr bgcolor=\"#EFEFEF\">";
    echo "<td></td>"; // icon column
    $colNicename = $colInfos['NICE_NAME'];
    echo '<td class="special2">';
    echo $colNicename;
    if ( $colInfos['NOT_NULL']  ) echo " <img src=\"images/redstar.gif\">";
    echo "</td>";
    echo "<td>";
    
    $html_txt = htmlspecialchars ($valueNew);
    
    if ($editAllow>0) {
   
        switch ($column_type) {
            case "password":
                echo '<input type=password name="'.$fieldname.'"  value="'.$valueNew.'" >' ;
                echo '</td><td>&nbsp;';
                break;
            case "boolean":
                $valCheck='0';
                if ($valueNew>0) $valCheck='1';
                // need this javascript-technology, because values of NON-CHECKED boxes are NOT send
                echo '<input type=hidden name="'.$fieldname.'"  value="'.$valCheck.'">';
                echo '<input type=checkbox name=dummy  value="1" '.
                    'onClick="if (this.checked == false) val=0; else val=1;',
                    ' this.form.elements[\''.$fieldname.'\'].value=val"';
                if ( $valueNew>0 ) echo " checked";
                echo ">";
                echo '</td><td>&nbsp;';
                break;
                
            case "textarea":
                $this->formSubLib->notesFieldOut($fieldname, $html_txt);
                break;
            case "select":
                $this->_selvals( $fieldname, $valueNew, $viewcnt, $colInfos['select_arr'] );
            default:
                echo '<input name="'.$fieldname.'"  value="'.$html_txt .'">';
                echo '</td><td>&nbsp;';
                break;
        }
    } else {
        switch ($column_type) {
            case "password":
                echo '***' ;
                echo '</td><td>&nbsp;';
                break;
            case "boolean":
                $valCheck='0';
                if ($valueNew>0) $valCheck='1';
                echo $valCheck;
                echo '</td><td>&nbsp;';
                break;
                
            case "textarea":
                echo '<pre>';
                echo $html_txt;
                echo '</pre>';
                echo '</td><td>&nbsp;';
                break;
            default:
                echo $html_txt;
                echo '</td><td>&nbsp;';
                break;
        }
    }
    
    echo "</td><td nowrap>&nbsp;</td>";
    if ( $this->tabInfo['showRemarks']>0 ) {
        if ( $column_type != "textarea" ) { // no comment
            echo "<td>&nbsp;";
            $colcomment = $colInfos['COMMENTS'];
            echo "<I>". $colcomment ."</I>"; // comment
            echo "</td>";
        }
    }
    echo '</tr>'."\n";
        
}

/**
 * analyse/show ONE column of edit form
 * @param array $colopt : typedef colopt_TYPE : several options 'colNameAdd'
 * @global 
	$this->editAllow
	$this->tabInfo['showRemarks']
	$this->_action
	$this->pk_arr
	$this->prefValLib
 * @return array($valueNew, $shown)
 
 */
private function oneColumn( &$sql, $cnt, $valueIn, $viewcnt, $colName, $H_EXP_RAW_DESC_ID, $colopt = array() ) {

	$tablename = $this->tablename;
	$editAllow = $this->editAllow;
	$action	   = $this->_action;
	$pk_arr	   = $this->pk_arr;
	
	$col_show = 1; // show column ?
	$APP_DATA_TYPE_name="";
	
	/* EDIT/SELECT allowed ??? */
	$select_allowed = $editAllow; /* allow select ? */
	$colInfos = colFeaturesGet($sql, $tablename, $colName, $H_EXP_RAW_DESC_ID);
	if (is_array($this->_xColInfos[$colName])) {
		// overload the original
		$colInfos = array_merge($colInfos, $this->_xColInfos[$colName]);
	}
	
    $valueNew = $valueIn; // set temporary variable
			
	$ret_pkname   = NULL;
	$ret_imp_name = NULL;		
    $linked_tab = fk_check2($colName, $tablename, $ret_pkname, $ret_imp_name); 
	
	$colIsSingelPK=0;
	if ( $pk_arr[0] == $colName) {
		$select_allowed = 0;
		if ($pk_arr[1]==NULL) {
			$colIsSingelPK=1;  // is a singlePK-Table
		}
	}
    if ( $action != 'insert' )  {
	    if ( !empty($pk_arr) ) {
		    foreach( $pk_arr as $tmp_id=>$primname) {
			    if ( $primname == $colName ) $select_allowed = 0; 
		    }
		    
	    }
	}
    	
	$gSofColShowAdvanced = 1;  // always on on edit, otherwise to many complications, if wrong params are copied and so on
	$col_show            = $this->formSubLib->col_show_test( $col_show, $linked_tab, $colName, $colInfos, $gSofColShowAdvanced, $colopt['show']);
	
    if ( $colInfos["EDITABLE"] <=0 )     $select_allowed = 0; // denied
    if ( $colInfos["EDITABLE"] == 2 ) {
    	$select_allowed = 0;
    	if ($this->infox['f.editColAdv']>0) $select_allowed = 1; // advanced edit
    }
    
    if (is_array($colopt)) {
        if (array_key_exists('EDITABLE', $colopt)) {
            $select_allowed = $colopt['EDITABLE']; 
        }
    }
    
	if ( $_SESSION['sec']['appuser'] == "root" )           $select_allowed = 1; // allow all for root
	if ( $colIsSingelPK and  $action != 'insert' )  $select_allowed = 0; 
    
 	if ( !$col_show ) { //  show column ??? 
		return array($valueNew,$col_show);
	}	
	
	$field_name = 'argu['.$colName.']';
	
	
	echo " <tr bgcolor=\"#EFEFEF\">";
	if ( $linked_tab ) {
		$icon="images/icon.".$linked_tab.".gif";
		if ( !file_exists($icon) ) $icon="images/icon.UNKNOWN.gif";
		$iconimg = "<img src=\"".$icon."\">";
	} else $iconimg="&nbsp;";
	
	echo "<td>".$iconimg."</td>";
	$colNicename = $colInfos['NICE_NAME'].$colopt['colNameAdd'];

	if ( $linked_tab ) { /* is it a FOREIGN key to an other table ??? */

		$this->_linkedTab( $sql, $colName, $linked_tab, $select_allowed,
						$colInfos, $valueNew, $viewcnt, $colNicename );
						
	} else { // common FORM input field (text, text area, checkbox, ...)

		$APP_DATA_TYPE_ID   = appDataTypeGet2($tablename, $colName);
		$APP_DATA_TYPE_name = appDataTypeNameGet2($APP_DATA_TYPE_ID);

		if ( $APP_DATA_TYPE_name == "notes" ) {
			$notes_colspan = $this->formSubLib->getColspan();
			echo "<td colspan=".$notes_colspan."><font color=gray>" . $colNicename."</font> ";
		}  else {
			echo "<td class=\"special2\">" . $colNicename; 
			// don't show RED_STAR for booleans, because the user gets confused otherwise
			if ( ($APP_DATA_TYPE_name!="boolean") && $select_allowed && !empty($colInfos['NOT_NULL'])  ) 
				echo " <img src=\"images/redstar.gif\">"; 
			echo "</td><td>\n   ";
		}

		$columnType=""; 					
		if ( $APP_DATA_TYPE_name == "notes" )   $columnType="textarea";
		if ( $APP_DATA_TYPE_name == "boolean" ) $columnType="boolean";
		if ( $APP_DATA_TYPE_name == "date" )   $columnType="date";
		if ( $colName == "PASS_WORD" ) $columnType="password";
		if ( $colIsSingelPK ) { // contains FIRST Primary key ??? (editable by root)
			$columnType="firstpk";
		}
		$html_txt = htmlspecialchars ($valueNew);

		if ( !$select_allowed ) { // select NOT allowed ???
		
			echo "<input type=hidden name=argu[" .$colName. "]  value=\"$html_txt\">";

			switch ($columnType) {
			case "textarea":
				echo "<br>"; 
				echo "<pre>". $html_txt."</pre>";
				break;
			case "firstpk":
				$primval = $valueNew;
				if ( $primval==0 ) echo "*";
				else echo $html_txt."&nbsp;";
				break;
			default: echo $html_txt."&nbsp;";
				break;
			}
			if ( $columnType == "textarea" ) $dummy=0;/* no other column */
			else echo "</td><td>&nbsp;";
			
			
		} else {  // select allowed   
			
			
			if ( $columnType ) { /* special INPUT type */
				switch ($columnType) {
					case "password": 
						echo "<input type=password name=argu[" .$colName. "]  value=\"".$valueNew."\" >" ;
						echo '</td><td>&nbsp;';
						break;
					case "boolean":
						$valCheck='0';
						if ($valueNew>0) $valCheck='1';
						// need this javascript-technology, because values of NON-CHECKED boxes are NOT send
						echo '<input type=hidden name=argu[' .$colName. ']  value="'.$valCheck.'">';
						echo '<input type=checkbox name=dummy  value="1" '.
							'onClick="if (this.checked == false) val=0; else val=1;',
							' this.form.elements[\'argu[' .$colName. ']\'].value=val"';
						if ( $valueNew>0 ) echo " checked";
						echo ">";
						echo '</td><td>&nbsp;';
						break;
					case 'date':
						$DATE_MIN_LEN=10;
						$HHMM_len=5;
						$dateValOut=$valueNew;
						if ($valueNew!=NULL) {
							if (strlen($valueNew)>$DATE_MIN_LEN) {
								$hhmm=substr($valueNew,$DATE_MIN_LEN+1,$HHMM_len);
								// cut '00:00' for user convinience
								if ($hhmm=='00:00') $dateValOut=substr($valueNew,0,$DATE_MIN_LEN);
							}
						}
						
						echo '<input name=argu[' .$colName. '] size=19 class="datepicker" value="' . $dateValOut . '" >';
						//echo $this->_colDateButNow( $colName ).'</td><td>';
						echo '</td><td>'.$this->_colDateButNow( $colName );
						break;
					
					case "textarea": 
						$this->formSubLib->notesFieldOut("argu[" .$colName. "]", $html_txt);
						break;
					case  "firstpk":
					    
						$butid= $this->jsFormLib->getID($viewcnt);
						echo '<input type=text id='.$butid.' name=argu['.$colName.']  value="'.$valueNew.'" >' ;
						echo '</td><td>&nbsp;';
						// $this->jsFormLib->pkInput($viewcnt);
						break;	
					/* default action for "date" */	
					default: 
						echo '<input name=argu['.$colName.'] value="'.$html_txt .'">';
						echo '</td><td>&nbsp;';
						break;
				}

			} else { // common input fields

				$selectVals= $this->prefValLib->getValInitList($sql, $tablename, $colName); // get pre-initialised values 
				if ( !empty($selectVals) ) {
				    // TBD: datalist ???
				    if ($colInfos['DATAX_arr']['field.type']=='datalist') {
				        $this->_field_datalist( $colName, $valueNew, $viewcnt, $selectVals );
				    } else {
				        $this->_selvals( $field_name, $valueNew, $viewcnt, $selectVals );
				    }
				} else {
					echo "<input name=argu[" .$colName. "] size=30 value=\"" .htmlspecialchars($valueNew) . "\" >" ;
					echo "</td><td>&nbsp;";
				}
				
			}

		} 
	}	
	echo "&nbsp;</td>";

	// COMMENTS
	if ( $this->tabInfo['showRemarks']>0 ) {
		if ( $APP_DATA_TYPE_name != "notes" ) { // no comment
			echo "<td>&nbsp;";
			$colcomment = $colInfos['COMMENTS']; //column_remark2($tablename, $colName);  
			echo "<I>". $colcomment ."</I>"; // comment
			echo "</td>";
		}
	}
	echo "</tr>\n";

	return array($valueNew,$col_show);
}

function formSubmitClose($action,$edopt) {
	$action_text="SUBMIT";
	if ( $action=="update" ) $action_text="UPDATE";
	if ( $action=="insert" ) $action_text="CREATE";
	if ( $edopt["but.submit.txt"]!=NULL ) $action_text = $edopt["but.submit.txt"];

	echo "<tr class=\"nobord\"><td colspan=2>&nbsp;</td><td>&nbsp;"; // bgcolor=#D0D0D0  bgcolor was: #6699FF
	echo "<input class='yButton' type=submit value=\"".$action_text."\" ></td><td>&nbsp;</td>";
	
	$editRemarks = $this->showRemarks;
	if ( $editRemarks ) echo "<td>&nbsp;</td>";
	echo "</tr>";
	
	$this->formSubLib->datatable_close();
}

function formTableClose() {
	$this->formSubLib->datatable_close();
}

/**
 * optional init the form lib, especially $this->infox
 * @param  $sqlo
 */
public function init(&$sqlo) {
	// check for special EDIT role-right
	if ($this->tablename==NULL) sys_error_my('CLASS: tablename missing.');
	$role_right_name = 'o.'.$this->tablename.'.editColAdv';
	$role_right      = role_check_f($sqlo, $role_right_name);
	if ( $role_right=="execute" ) {
		$this->infox['f.editColAdv'] = 1;
	}
}

/**
 * set e.g. 'f.editColAdv' = 0,1
 * @param string $key
 * @param string $val
 */
public function set_infox_val($key, $val) {
	$this->infox[$key] = $val;
}

/**
 *	- show an HTML form to edit basic object parameters
 *	- does NOT close with "</form>"
 *
 *	- used by edit.tmpl.php, obj.cart_batch.cartLiUp.php
 *
 * @param string $action,        "update", "insert"
 * @param object &$sql,
 * -- @param object &$sql2, DEPRECATED
 * @param array $colNames_ori,   column names
 * @param array $pk_arr,         PrimaryKey array [OPTIONAL, can be NULL]
 * @param array $arguByCol,      ORIGINAL data: array[COL_NAME];  OLD: $argu_load[POS]
 * @param array $extraobj_o,     extraobj_o_STRUC  : extra_object structure 
 * @param array $editAllow,      flag: 0|1
 * @param array $edopt=NULL, form options <pre>	
 * 		"H_EXP_RAW" => $H_EXP_RAW_DESC_ID 
 *		"but.submit.txt" => text for SUBMIT 
 *		"assocShow" = 0,1
 * </pre>
 * @param array colopt_TYPE $colopt=NULL						
 */
function formBody( $action, &$sql, $colNames_ori, $pk_arr, $arguByCol, 
		$extraobj_o, $editAllow,  $edopt=NULL, $colopt=NULL	) {
	global   $varcol;
	
	$this->_action = $action;
	$this->_colopt = $colopt;
 
  
	$tablename 	   = $this->tablename;
	$id 	   	   = $this->id;
	$has_single_pk = $this->has_single_pk;
	
	$H_EXP_RAW_DESC_ID = NULL;
	if ( ($tablename=="EXP_RAW_RESULT") || ($tablename=="EXP_RAW_SPOT_RESULT" || $tablename=="EXP_CHAN_RES") ) {
	    $sql->Quesel("H_EXP_RAW_DESC_ID from EXP where EXP_ID=".$id);
	    $sql->ReadRow();
	    $H_EXP_RAW_DESC_ID=$sql->RowData[0];
	} 
	
	$editRemarks = $this->showRemarks;
	$viewcnt     = 0; // counter for viewed columns
	$xtraArgCnt  = 0; // count extra arguments (like joins from other tables)
	$numCols 	 = sizeof($colNames_ori);
	reset ($colNames_ori);
	
	$this->tabInfo   = NULL;
	$this->editAllow = $editAllow;
	$EXTRA_OBJ_dataExists = in_array  ( 'EXTRA_OBJ_ID'  , $colNames_ori );
	if ( $tablename == 'EXTRA_OBJ' ) $EXTRA_OBJ_dataExists = 0;
	if ($EXTRA_OBJ_dataExists) {
		// any classes defined for object type ?
		$classes = $varcol->get_class_names($tablename);
		if (empty($classes)) $EXTRA_OBJ_dataExists=0; // do not show the class area
	}
	
	$this->tabInfo['showRemarks'] = $editRemarks;
	$this->pk_arr = $pk_arr;
	
	$this->_initForm($sql, $arguByCol, $extraobj_o, $colNames_ori);
	
	$cnt = 0; // counter for column 
	foreach( $colNames_ori as $dummy=>$colName) { /* for each column */

		$valueIn   = $arguByCol[$colName];
	
		// to the end of table ... 
	  	if ( $EXTRA_OBJ_dataExists AND ( $cnt == ($numCols - 1) ) ) { 
			$XFormLib = new fEditXobjForm( $tablename );
			if (is_array($this->_xColInfos['class'])) $XFormLib->set_xColInfos($this->_xColInfos['class']);
			$viewcnt  = $XFormLib->showCols( $sql, $viewcnt, $editAllow, $extraobj_o, $editRemarks, 1 ); // do it!
		} 
		list($newval, $shown) = $this->oneColumn( $sql, $cnt, $valueIn, $viewcnt, $colName, 
				$H_EXP_RAW_DESC_ID, $colopt['cols'][$colName] );
		
		if ( $shown ) {
			// save only data, if shown here
			$viewcnt++;
		}
	  	$cnt++;
	
	}
      
	if ( $edopt["assocShow"]>0 ) {
	    $sh_slim=1;
	    if ( glob_isAdmin() ) $sh_slim=0; // the admin should see ALL possible associated tables
	    $this->formSubLib->show_assoc_tabs($sql, $editRemarks, 1, $sh_slim, $colopt['assoc']);
	}
	
	$this->formSubmitClose($action,$edopt);
	
}

/**
 * show easy object  edit form
 * DEPRECATED:  old param &$sql2
 * @todo used_by:  www/pionir/glob.objtab.assocnew.php 
    	  www/pionir/glob.objtab.edit.php
 */
function form_editx( $action, &$sql, &$colNames_ori, $pk_arr, &$arguByCol, &$extraobj_o, $editAllow, $edopt=NULL, $colopt=NULL ) {
    
	$this->init($sql);
	$this->formHead();
	$this->formBody( $action, $sql, $colNames_ori, $pk_arr, $arguByCol, 
					$extraobj_o, $editAllow, $edopt, $colopt	 );
}

function addHidden($hiddenarr) {
	if (empty($hiddenarr)) return;
	
	foreach( $hiddenarr as $idx=>$valx) {  
		   echo '<input type=hidden name="'. $idx .'" value="'.$valx.'">'."\n";
	}
	
	echo "\n";
     
}

function close() {
	echo '</form>'."\n";
}

/**
 * translate indices NAME => ID
 * @return 
 */
function _extraAtribTrans($arguobj, $obj_class_id) {
	global $varcol, $error;
	
	$arguobj_id = array();
	if (!empty($arguobj)) {
	    foreach( $arguobj as $th0=>$th1) { 
			$newid = $varcol->attrib_name_to_id($th0, $obj_class_id);
			if ( $error->Got(READONLY) ) {
				reset($arguobj);
				return;
			}
			$arguobj_id[$newid] = $th1;
		}
		
	}
	return ($arguobj_id);
}

/**
 * check params
 * @param object $sqlo
 * @param array  $argu
 * @param array  $extraobj_o
 * @param string $action
 * @param array  $showCols expected columns
 */
function formParamCheck(&$sqlo, &$argu, &$extraobj_o, $action, $showCols=NULL) {
	if (is_object($this->objectClass))  {
		if (method_exists($this->objectClass,'paramCheck')) {
			$this->_xColInfos = $this->objectClass->paramCheck($sqlo, $argu, $extraobj_o, $action, $showCols);
		}
	}
}

/**
 * update one object/element 
 * @return 
 * @param object $sql
 * @param object $sql2
 * @param object $colNames_ori
 * @param object $argu <pre>
 * 		no check of NON-allowed columns (TBD: change this?)
 * 		for all users:
 * 			- CCT_ACCESS_ID will be removed from $argu
 * 			- EXTRA_OBJ_ID  will be removed from $argu
 *  </pre>
 * @param array $extraobj_o extraobj_o_STRUC  additional key/val: 'CLASS' : class-id
 * @param object $xargu
 * @param object $arguobj input-arguments for EXTRA_OBJECT
 * 	 - if type ARRAY : set class atribs
 *   - if type NULL  : ignore class attributes
 */	
function formUpdate( &$sql, &$sql2, $colNames_ori, &$argu, &$extraobj_o) {	
	global $error, $varcol;
	$FUNCNAME='formUpdate';
	
	$tablename = $this->tablename;
	$id 	   = $this->id;
	$idname    = PrimNameGet2($tablename);
	$extra_obj_col_exists = 0;
	
	if (is_array($extraobj_o)) $extra_obj_col_exists = 1;    
	
	// get special column-info for object
	if (is_object($this->objectClass)) 
		if (method_exists($this->objectClass,'paramCheck')) {
			$this->_xColInfos = $this->objectClass->paramCheck($sql2, $argu, $extraobj_o, "update", $colNames_ori);
			if ($error->Got(CCT_ERROR_READONLY)) {
				$error->set( $FUNCNAME, 2, "Input-parameter check failed." );
				return;
			}
			if ($error->Got(CCT_WARNING_READONLY)) {
			    // set warning
			    $error->set( $FUNCNAME, 2, "Input-parameter check warning.", 1 );
			    // continue ...
			}
		}
	
	if ( $id != $argu[$idname] ) {
		$error->set( $FUNCNAME,100, "ID does not fit with the input arguments!" );
		return;
	}
	
	$advmodLib   = NULL;
	$advmod_info = array();
	
	if ( $this->object_is_bo ) {
		// not allowed !
		unset ($argu["EXTRA_OBJ_ID"]);
		unset ($argu["CCT_ACCESS_ID"]);
		

	}
	
	// checks for ADVMOD : <start>
	$tmpDo_advmod = 0;
	$advmod_table = $tablename;
	do {
	    if ($this->has_single_pk) break; // will be handled by update_meta
		$mothertab = mothertable_get2($tablename);
		if ($mothertab==NULL)     break; // has no mother
		if (!cct_access_has2($mothertab)) break; // ooch nich
		$tmpDo_advmod = 1;
		$advmod_table = $mothertab;	
	} while (0);
	
	if ($tmpDo_advmod) {
		$advmodLib   = new obj_advmod();
		$advmod_info = $advmodLib->setObj($sql, $advmod_table, $this->id);
		if ($advmod_info['dolog']>0) {
			$advmodLib->getOriData_ass($sql, $tablename);
		}
	}
	// checks for ADVMOD : <end>
	
	
// 	if ($extra_obj_col_exists) {
// 		$arguobj      = $extraobj_o['arguobj'];
// 		$obj_class_id = $extraobj_o["extra_class_id"];  // existing CLASS-ID   
// 		$arguobj_id   = $this->_extraAtribTrans($arguobj, $obj_class_id);
// 	} else $arguobj_id = array();

	$validLib = new fValidate();
	$dtypes   = $validLib->check_columns($sql, $tablename, $argu, true);
	if ($error->Got(READONLY)) {
		return;
	}
	
	$xopt = NULL;
	if ( !empty($dtypes)) {
		// check data type
		$xoptType = NULL;
		foreach( $dtypes as $col=>$dtype) {
			if ($dtype=='DATE1') {
				$xoptType[$col]='DATE1';
			}
		}
		
		if ( !empty($xoptType)) {
			$xopt = array('types'=>$xoptType);
		}
	}
	
	if ($this->has_single_pk) {
	    
	    $args = array('vals'=>$argu);
	    if ($extra_obj_col_exists and array_key_exists('CLASS', $extraobj_o)) {
	        $args['xobj'] = array('extra_class_id'=>$extraobj_o['CLASS'], 'values'=>$extraobj_o['arguobj'] );
	    }
	    
	    $up_obj = new globObjUpdate();
	    $up_obj->update_meta($sql, $tablename, $this->id, $args, $xopt);
	} else {
	    $retVal = gObjUpdate::update_row($sql, $tablename, $argu, $xopt);
	    if ( $retVal <= 0 ) {
	        $error->set( $FUNCNAME,3, "SQL update failed! (Code: $retVal)" );
	        return;
	    } 
	}
	if ($error->Got(READONLY))  {
	    $error->set( $FUNCNAME, 4, 'Error on update of OBJ('.$tablename.','. $this->id .')');
	    return;
	}
	
	
	
// 	if ( $extra_obj_col_exists ) {
// 	    debugOut("EXTRA_OBJ: update: args:".print_r($extraobj_o,1).' arguobj_id:'.print_r($arguobj_id,1), $FUNCNAME, 1);
// 	    $x_obj_lib = new fVarcolMeta($sql, $tablename, $id);
// 	    $x_obj_lib->extra_update($sql, $extraobj_o["extra_obj_id"], $extraobj_o["CLASS"], $arguobj_id );
// 	}
	
	if ($advmod_info['dolog']>0) {
		$touchInfo = array('timest'=>time() ); // TBD: must come in FUTURE from gObjUpdate::update_row !
		$advmodLib->getPostData_ass($sql, $touchInfo);
	}
	
	// Analysis of additional parameters; ... not used till now !!!          
	// if (function_exists('xfrom')) $retVal = xfrom($sql, , '', $id, $argu);

	
}


/* 
 * get extra column settings
 * @return colopt_TYPE
 * EXAMPLE:
 $assoc=array();
 $assoc['SPOT_RESULT'] = array('tab'=>'EXP_SPOT_RESULT', 'txt'=>'with probes');
 $assoc['EXP_RAW_RESULT']=array('tab'=>'EXP_RAW_SPOT_RESULT', 'txt'=>'with probes');
 $colopt = array('assoc'=> $assoc);
 return($colopt);
 }
 */
function x_cols_extra($sqlo, $arguByCol) {
    $colDefOpt = array();
    if (is_object($this->objectClass))
        if (method_exists($this->objectClass,'x_cols_extra'))
            $colDefOpt = $this->objectClass->x_cols_extra($sqlo, $arguByCol);
        
  
    return $colDefOpt;
}

}
