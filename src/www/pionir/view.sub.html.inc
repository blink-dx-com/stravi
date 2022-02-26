<?php

require_once("view.tmpl.inc");
require_once("o.CCT_ACCLOG.subs.inc");

/**
 * sub functions to manage the HTML-output for view.tmpl.php
 * @namespace core
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/www/pionir/view.sub.html.inc 59 2018-11-21 09:04:09Z $
 */
class viewSubHtml extends viewSubC {
	
var $auditColor; // array of audit-state colors
var $auditCache; // cache audit-status-nice-names
	
function __construct($tablename) {
	parent::__construct($tablename);
	$this->auditCache=NULL;
	$this->auditColor = oH_ALOG_ACT_subs::$auditColor;
	$this->sort_link_flag=1;
}

public function sort_link_set_flag($flag) {
    $this->sort_link_flag=$flag;
}


/**
 *  add a link for new SORT criteria
 * @param array $sortcritX  format: $sortCondArr
 * @param string $colname
 * 
 */
private function sortlnk(&$sortcritX, $colname, $col_nice ) {
	$dirnew = "DESC";
	if ( $sortcritX[$colname] != NULL ) {
		if ( $sortcritX[$colname]["dir"] =="DESC" ) $dirnew = "ASC";
	}
	if ($this->sort_link_flag) echo "<a href=\"javascript:sort_pref('".$colname."', '".$dirnew."')\">";
	if ( $sortcritX[$colname] != NULL ) {
		if ( $sortcritX[$colname]["dir"] =="ASC" ) echo " <img src=\"images/but.asc.gif\" TITLE=\"switch to descending\"	border=0>\n"; 
		else echo " <img src=\"images/but.desc.gif\" TITLE=\"switch to ascending\" border=0>\n";
	}
	echo $col_nice;
	if ($this->sort_link_flag) echo "</a>";
}

	
/**
 * show HTML-table-head
 */
function headRow(&$colNames_show, $sortcritX) {
	$cnt   = 0;
		
	foreach( $colNames_show as $tmpColSh ) {
	
    
		$tmp_col_pure = $tmpColSh["cc"];
		$tmp_col_ext  = $tmpColSh["ce"];
		$show_flag    = $tmpColSh["show"];
		$colNicename  = $tmpColSh["nice"];
        
		if ( $this->view_opt["colCoNa"] ) $colNicename = $tmp_col_pure;	  // show column code name
		if ($show_flag) { 
			
            if ( substr($tmp_col_ext,0,2)=="a." ) echo "<th bgcolor=#C0EEC0>";
            else echo "<th>"; 
			
			if (($tmp_col_ext != "c.NAME") && ($tmp_col_ext != "x.EXTRA_OBJ_ID") ) {		
			    $this->sortlnk($sortcritX, $tmp_col_ext, $colNicename);
			} else {
				echo "class";
			}
			echo "</th>\n";
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
	
	$sortcritX = $this->sortcritX;	
	
	if (!is_array($xobNameArr)) {
	   return; 
	}
	
	$lener = sizeof($xobNameArr);
	$i=0;                  
	while ($i<$lener) {
		$oneArr = $xobNameArr[$i];
		$theName = $oneArr['na'];
		$theMap  = $oneArr['map'];
		
		echo "<th bgcolor=#F0F000>";
		$this->sortlnk( $sortcritX, "o.".$theMap, $theName );
		echo "</th>";
		
		$i++;
	}
    
}

/**
 * table HEAD: <colgroup>
 */
function prHeadPre( &$colNames_show ) {
	?> 
	<style type="text/css"> 
	table.xListView  {
	   background-color: #FFFFFF;
	   border: 0px solid #E0E0E0;
	}
	table.xListView  td {
	   white-space: nowrap;
	}
	table.xListView  th {font-size:80%; font-weight : normal;}
	td.importx {text-decoration: underline;}
	th.usx {font-size:80%; font-weight : normal; background-color:#B0B0FF }
	
	</style>   
	<?php
	echo '<table class="xListView" CELLSPACING=1 CELLPADDING=1>'."\n"; // #F0F0F0
	
	echo "<colgroup>";
	echo "<col>"; 		/* goto button */
	if ($this->internViewOpt['checkbox']) echo "<col width=20>"; 	/* checkbox */
	
	/*define table column number */
	foreach( $colNames_show as $tmparr) {  /* take names from selectstring !!! */
	
		$show_flag = $tmparr["show"];
		if ( $show_flag ) echo "<col>";
	}
	
	echo "</colgroup>\n";
}

/**
 * print out HEADER-row
 * @return 
 * @param object $sql2
 * @param object $sortcritX
 * @global $this->primast, $this->colNames_show, $this->xcols and other
 */
function prHeadRow( &$sql2 ) {
		
	$primast   = &$this->primast;
	$colNames_show = &$this->colNames_show;
	$sortcritX = $this->sortcritX;
	
	$this->colPosPrep( );
	$xobNameArr = $this->xobj_para_prep( $sql2 );
	
	$this->prHeadPre( $colNames_show );
	
	if ( $this->viewmode!="lean_csv") echo "<tr bgcolor=\"#C0C0C0\">"; 
	if ( $this->modebase!="lean") {
		echo "<th>GO</th>"; 
		if ($this->internViewOpt['checkbox']) echo "<th>&nbsp;</th>";      
	}
	
	$this->headRow($colNames_show, $sortcritX);
	$this->_xobj_para_showhead( $xobNameArr );
	
	if (!empty($this->xcols)) {
	    foreach( $this->xcols as $key=>$text ) {
		
			if ( $this->viewmode!="lean_csv" ) echo "<th class=usx>$text</th>";
			else echo $text."\t";
		}
		
	}
	
	if ( $this->viewmode!="lean_csv" ) {
			echo "</tr>\n";
			echo "<nobr>";     
	}
	echo "\n";  /* NEED FOR CSV OUTPUT */
}

/**
 * show content of one cell 
 * - this function should be optimized for fast performance (often called)
 * - USED_BY:  print_row()
 * @return -
 * @param object $text
 * @param object $text_extra e.g. the ID of a foreign object
 * @param object $cnt current Pos-in-Array 
 */
function view_textout( $text, $text_extra, $cnt, $tdAdd=NULL ) {
	
	$nameid 	= $this->prColpos["nameid"]; // Pos-in-Array of the name-column 
	$COL_MAXLEN = $this->view_opt["CelSizeOk"]; // is checked for value
     
	$textLen = strlen($text);
	
	// MAIN column value will NOT be truncated ...
	if ( ($textLen > $COL_MAXLEN) AND ($cnt != $nameid) ) 
		$texttmp = substr ($text, 0, $COL_MAXLEN ) . "..."; 
	else $texttmp    = $text;
	
	echo '<td NOWRAP '.$tdAdd.'>'; // TBD: remove NOWRAP: IE5.0 doesnt know CSS:NOWRAP
	echo htmlspecialchars($texttmp). $text_extra;
	echo "</td>\n";
	
}  

/**
 * show output of one data-row
 * @param array $arr value array, index: pos of column
 * @global INPUT: 
 * 		$this->prColpos, $this->cctgoba, $this->primas, $this->foreign_keys, $this->primast
	    $this->objFuncLib, $this->tablename, $this->colNames_show, $this->userSqlObj
	    $this->xcols
 */
function print_row( &$sql, $arr, $viewcnt ) {
		
	
	//$optpureids = $this->view_opt["pureids"]; // 0,1 : show pure IDs, not the NAME of the foreign object!
	$optShowFK  = $this->view_opt["ShowFK"];
	$primas	   = &$this->primas;
	$foreign_keys = &$this->foreign_keys;
	$primast 	  = &$this->primast;
	$prColpos	  = &$this->prColpos;

	$rowcolor="#F0F0F0"; // default row color
	
	if ( $prColpos["H_ALOG_ACT_ID"] ) {
		// color row by audit status
		// NO alternating row-colors!
		$val = $arr[$prColpos["H_ALOG_ACT_ID"]];
		if ($val) {
			if ( isset($this->auditCache[$val]) ) $valnice = $this->auditCache[$val];
			else {
				$sqls="select NAME from H_ALOG_ACT where H_ALOG_ACT_ID=".$val;
				$sql->query($sqls);
				$sql->ReadRow();
				$valnice = $sql->RowData[0];
				$this->auditCache[$val]=$valnice;
			}
			if ( $this->auditColor[$valnice] ) $rowcolor = $this->auditColor[$valnice];
		}
	} else {
		 // change row-color in every row, but only when NO audit-colors are shown
		if ( (int)$viewcnt&1 ) $rowcolor="#E0E0E0";
	}
	
	echo "<tr bgcolor=\"" .$rowcolor ."\" id=t".$viewcnt." onclick=\"th_snc( 't".$viewcnt."' )\" >\n"; 
	echo "<td>"; /* goto */
	
    /* this is code for for security level=='select_on'; is not needed this time ...
	if (isset($_SESSION['globals']['security_level']) && ($_SESSION['globals']['security_level'] == 'select_on' )) {
  	      $id_tmp = $arr[$primast[0]];
  	      $o_rights = access_check($sql, $tablename, $id_tmp);
	      if ( !$o_rights["read"] ) {
		      echo "<img src=\"images/but.selinact.gif\" border=0 TITLE=\"$id_tmp\">";
		      echo "</td>";
		      echo "</tr>\n";
		      return;
	      }
	}
	*/

	if ( $this->cctgoba != "" ) {
	  // show select-back-icon
	  $goObjectIcon='images/ic.zoom16_2.gif';
	  $cctgobacolid = $prColpos["cctgobacol"];
	  $tmp_id = $arr[$cctgobacolid];
	  $name=":";
	  if ( $prColpos["nameid"] >= 0 ) {
		$name = $arr[$prColpos["nameid"]];
	  }
	  
	  if ( $foreign_keys[$cctgobacolid][0] ) { /* ID to return is foreign key ??? */
		$fkBackData = &$foreign_keys[$cctgobacolid];							  
		$sqls="select ".$fkBackData[0]." from ".$fkBackData[1]." where ".$fkBackData[2]."='".$tmp_id."'";
		$sql->query("$sqls");
		$sql->ReadRow();
		$name=$sql->RowData[0];
	  }
	
	  if ( isset($prColpos['CCT_ACCESS_ID']) ) {
	  	$cct_access_id = $arr[$prColpos['CCT_ACCESS_ID']];
	  } else $cct_access_id = 0;
	  $this->_selShowOne( $sql, $name, $tmp_id, $cct_access_id);
	
	} else $goObjectIcon='images/arrow.but.gif';

	echo '<a href="edit.tmpl.php?t='.$this->tablename.'&id='.$arr[$primast[0]];
	if ( $this->primasNum > 1) {
		if (!empty($primas[1])) echo "&primas[1]=" . $primas[1] . "&primasid[1]=".$arr[$primast[1]];
		if (!empty($primas[2])) echo "&primas[2]=" . $primas[2] . "&primasid[2]=".$arr[$primast[2]];
	}
	echo '"><img src="'.$goObjectIcon.'" border=0 hspace=3></a>';
	echo "</td>\n";
	
	if ( $this->internViewOpt['checkbox'] ) {
		echo "<td>";
		// build multi-array, for multi-PK-table
		$check_sel_id="[".$arr[$primast[0]]."]";
	
		if (!empty($primas[1])) { /* 2. primary key exists */
	  	      $check_sel_id=$check_sel_id."[".$arr[$primast[1]]."]";
		}
		if (!empty($primas[2])) { /* 2. primary key exists */
	  	      $check_sel_id=$check_sel_id."[".$arr[$primast[2]]."]";
		}
		echo '<input type=checkbox name="sel' .$check_sel_id .'" value="1">';
		echo "</td>\n";
	}
   
  $arr_len= sizeof($arr); 

  for ( $cnt=0; $cnt < $arr_len; $cnt++ ) {
	  
    $colval	   = $arr[$cnt];
	$text	   = "";
	$show_arr  = $this->colNames_show[$cnt];
	$col_name  = $show_arr["ce"];
	$show_flag = $show_arr["show"];
    $app_data_type = $show_arr["app"];
	
    if ( $show_flag ) {
		$primar_name_tmp = $foreign_keys[$cnt][0];
		$text		= $colval;
		$text_extra = "";
		if ( $primar_name_tmp && $colval ) {
			if ( $col_name=="x.EXTRA_OBJ_ID" ) {
				$sqls="select c.name from extra_obj o, extra_class c where o.extra_obj_id=".$colval. 
					" AND o.extra_class_id=c.extra_class_id";
				$sql->query($sqls);
 				$sql->ReadRow();
 				$text=$sql->RowData[0];
			} else {
				$sqls="select ".$primar_name_tmp." from ".$foreign_keys[$cnt][1]." where ".$foreign_keys[$cnt][2]."='".$colval."'";
				$sql->query("$sqls");
 				$sql->ReadRow();
 				$foreign_val=$sql->RowData[0];
				$text=$foreign_val;
				if ( $optShowFK && ($primar_name_tmp!="NICK") ) $text_extra = " [$colval]";
				if ( $text=="" && $text_extra=="") $text="[$colval]"; // show PK, if no name (e.g. for IMG)
			}
		} 
		
		$this->view_textout( $text, $text_extra, $cnt, $show_arr["tdAdd"] );
  	}
  }
  
  $lener=0;
  if (!empty($this->class_params_show) ) {
      $lener = sizeof($this->class_params_show[1]);
  }
  
  if ( $lener ) { 
		$sqls="select ".$this->class_params_show[2]." from extra_obj where extra_obj_id=".$arr[$prColpos["extra_obj"]];
		$sql->query($sqls);
		$sql->ReadRow();
        $i = 0;
		$DUMMY = "";
		$DUMMY_VAL= -1;
        while ($i<$lener) {
            // $app_data_type = $this->class_params_show[1][$i][2];
			$this->view_textout(  $sql->RowData[$i], $DUMMY, $DUMMY_VAL ); 
            $i++;
        }
  }
  
  if ( !empty($this->xcols) ) {
      
    $primapur=array();
  	$primapur[0] = isset($primas[0]) ? (isset($arr[$primast[0]]) ? $arr[$primast[0]] : NULL) : NULL;
  	$primapur[1] = isset($primas[1]) ? (isset($arr[$primast[1]]) ? $arr[$primast[1]] : NULL) : NULL;
  	$primapur[2] = isset($primas[2]) ? (isset($arr[$primast[2]]) ? $arr[$primast[2]] : NULL) : NULL;
	
	$cct_access_id = 0;
	if ( isset($prColpos["CCT_ACCESS_ID"]) ) $cct_access_id = $arr[$prColpos["CCT_ACCESS_ID"]];
	if ( $cct_access_id ) $primapur["CCT_ACCESS_ID"] = $cct_access_id;
	
	foreach( $this->xcols as $codename=>$dummy ) {
	
	    if ($this->xcol_def[$codename][2]>0) {
	        $ret = $this->xcol_funcs($sql, $codename, $primapur);
	    } else {
		    $ret = $this->objFuncLib->xColDataGet ($sql, $codename, $primapur);
	    }
		echo "<td> ".$ret."</td>";
		
	}
	
  }
  echo "</tr>\n";
  
}

}
