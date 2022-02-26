<?php
require_once("view.tmpl.inc");

/**
 * handle CSV or XLS data export into row-arrays
 * @author steffen
 * @version $Header: trunk/src/www/pionir/view.sub.csv.inc 59 2018-11-21 09:04:09Z $
 */
class viewSubCsv extends viewSubC {
	
var $viewmode; /*
	'pureids'
		0=>'name + ID'
		1=>'only ID'
		2=>'only name' 
*/

function __construct($tablename) {
	parent::__construct($tablename);
}

	
/**
 * get STANDARD TABLE columns
 * @global $this->_headShowSum
 */
function headRow(&$colNames_show, $sortcritX) {
	$cnt   = 0;
			
	foreach( $colNames_show as $tmpColSh) {
    
		$tmp_col_pure = $tmpColSh["cc"];
		$tmp_col_ext  = $tmpColSh["ce"];
		$show_flag    = $tmpColSh["show"];
		$colNicename  = $tmpColSh["nice"];
        
		if ( $this->view_opt["colCoNa"] ) $colNicename = $tmp_col_pure;	  // show column code name
		if ($show_flag) { 

			if (($tmp_col_ext != "c.NAME") && ($tmp_col_ext != "x.EXTRA_OBJ_ID") ) {
				//
			} else {
				$colNicename = "class";
			}
			$this->_headShowSum[]= $colNicename;
		}	
		$cnt++;
	} 
	
}

/**
 * - show for EXTRA_OBJ => Atribute header
 * - allow also sorting of column
 * @param object $xobNameArr
 * @global $this->c_attrib	
 */
function _xobj_para_showhead( $xobNameArr ) {
	
	//$sortcritX = $this->sortcritX;	
	
	if (empty($xobNameArr)) return;
	    
	$lener = sizeof($xobNameArr);
	$i=0;                  
	while ($i<$lener) {
		$oneArr = $xobNameArr[$i];
		$theName = 'c:'.$oneArr['na']; // add Prefix to EXTRA_CLASS attribute name (needed for usage as IMPORT template)
		//$theMap  = $oneArr['map'];
		
		$this->_headShowSum[] = $theName;
		
		$i++;
	}
    
}

function _outRowHead(&$dataarr) {
	// do the output ...
	$this->_outRowArr($dataarr);
}

function _outRowArr(&$oneRowArr) {
	echo "<tr>";
	foreach( $oneRowArr as $val) {
		echo "<td>".$val."</td>";
	}
	 
	echo "</tr>";
	echo "\n";
	
}

/**
 * print out HEADER-row

 * @return array of column names
 * @param object $sql2
 * @param object $sortcritX
 * @global $this->xcols, $this->colNames_show
 */
function prHeadRow( &$sql2 ) {
	 
	
	$colNames_show = &$this->colNames_show;
	$sortcritX = $this->sortcritX;
	
	$this->colPosPrep( );
	$xobNameArr = $this->xobj_para_prep( $sql2 );
	
	$this->_headShowSum=array();
	
	
	$this->headRow($colNames_show, $sortcritX);
	$this->_xobj_para_showhead( $xobNameArr );
	
	if (!empty($this->xcols)) {
		foreach( $this->xcols as $text) {
			$this->_headShowSum[] = $text;
		}
		
	}
	
	return ($this->_headShowSum);
	
}

/**
 * manage output of one row
 * @global INPUT: 
 * 		$this->prColpos, $this->cctgoba, $this->primas, $this->foreign_keys, $this->primast
	    $this->objFuncLib, $this->tablename, $this->colNames_show, $this->userSqlObj
	    $this->xcols
 */
function print_row( &$sql, &$arr, $viewcnt ) {
  
  
  $optpureids = $this->view_opt["pureids"]; // 0,1 : show pure IDs, not the NAME of the foreign object!
  $primas     = &$this->primas;
  $foreign_keys = &$this->foreign_keys;
  $primast 	  = &$this->primast;
  // get Foreign object names ?
  if (!$optpureids or $optpureids==2) $getFkNames=1;
  else  $getFkNames=0;

  $oneRowArr  = NULL; // final output array

  $arr_len= sizeof($arr); 

  for ( $cnt=0; $cnt < $arr_len; $cnt++ ) {
	  
    $colval	   = $arr[$cnt];
	$text	   = "";
	$show_arr  = $this->colNames_show[$cnt];
	$col_name  = $show_arr["ce"];
	$show_flag = $show_arr["show"];
    //$app_data_type = $show_arr["app"];
	
    if ( $show_flag ) {
		$primar_name_tmp = $foreign_keys[$cnt][0];
		$text		= $colval;
		$text_extra = "";
		
		if ( $primar_name_tmp and $colval and $getFkNames ) {
			if ( $col_name=="x.EXTRA_OBJ_ID" ) {
				$sqls="select c.name from extra_obj o, extra_class c where o.extra_obj_id=".$colval. 
					" AND ".
					"o.extra_class_id=c.extra_class_id";
				$sql->query($sqls);
 				$sql->ReadRow();
 				$text = $sql->RowData[0];
			} else {
				$sqls="select ".$primar_name_tmp." from ".$foreign_keys[$cnt][1].
					" where ".$foreign_keys[$cnt][2]."='".$colval."'";
				$sql->query($sqls);
 				$sql->ReadRow();
 				$text = $sql->RowData[0];
 				
				if ( !$optpureids and ($primar_name_tmp!="NICK") ) $text_extra = " [$colval]";
				if ( $text=="" and $text_extra=="") $text="[$colval]"; // show PK, if no name (e.g. for IMG)
				
			}
			if (!$optpureids) $text .= $text_extra;
		} 
		
		$oneRowArr[] = $text;
  	}
  }
  

  if ( !empty($this->class_params_show[1]) ) { 
        $lener = sizeof($this->class_params_show[1]);
		$sqls="select ".$this->class_params_show[2]." from extra_obj where extra_obj_id=".
			$arr[$this->prColpos["extra_obj"]];
		$sql->query($sqls);
		$sql->ReadRow();
        $i = 0;
        while ($i<$lener) {
			$oneRowArr[] = $sql->RowData[$i]; 
            $i++;
        }
  }
  
  if ( !empty($this->xcols) ) {
    $primapur=array();
  	$primapur[0] = isset($primas[0]) ? (isset($arr[$primast[0]]) ? $arr[$primast[0]] : NULL) : NULL;
  	$primapur[1] = isset($primas[1]) ? (isset($arr[$primast[1]]) ? $arr[$primast[1]] : NULL) : NULL;
  	$primapur[2] = isset($primas[2]) ? (isset($arr[$primast[2]]) ? $arr[$primast[2]] : NULL) : NULL;
	
	$cct_access_id = 0;
	if ( isset($this->prColpos["CCT_ACCESS_ID"]) ) $cct_access_id = $arr[$this->prColpos["CCT_ACCESS_ID"]];
	if ( $cct_access_id ) $primapur["CCT_ACCESS_ID"] = $cct_access_id;
	
	foreach( $this->xcols as $codename=>$dummy) {

		$ret = $this->objFuncLib->xColDataGet ($sql, $codename, $primapur);
  		//if ($codename!="xSQL") 
		//else {
			// @swreq UREQ:0000983:xSQL
			// $retarr = $this->userSqlObj->exeSqlReplace ($sql, $primas[0], $arr[$primast[0]]);
			//$ret    = 'DEPRECATED'; // $retarr[0];
			
		//}
		$oneRowArr[] = $ret;
		
	}
	
  }

  return ($oneRowArr);
  
}


}
