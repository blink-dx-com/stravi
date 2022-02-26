<?php
/**
 * [Pro-Feature-Search] combined feature search
 * @package glob.objtab.search_1.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   $tablename or $t
         $go  = 1 :: build new condition
		 	$adv_colname[CNT]   = column name
            $adv_col[CNT]   	= value
            $adv_bool[CNT]  	= condition
            $adv_fkval[CNT] 	= value
            $adv_fkbool[CNT]	= condition
		 	$parx[bool]			= "ADD", "OR"
		 	$xaction		= ['profeature'], 'wiid'
 * @version0 2008-08-19
 */
session_start(); 


require_once ("reqnormal.inc"); // includes all normal *.inc files
require_once ("sql_query_dyn.inc");
require_once ("view.tmpl.inc");
require_once ( "javascript.inc" );
require_once ('f.wiid_roid.inc');

class gObjTabSearch1 {
	var $info; /* 
		'prim_name'
		'access_id_has'
		'sel_mother'
				*/
	var $selectCols;
	var $actions; /* action defs
		array(actions) = array('req'=> array($ parxNames )  )
		*/
	
	function __construct($tablename) {
		$this->tablename=$tablename;
		$this->viSubObj = new viewSubC($tablename);
		
		$this->actions=array(
			'profeature'=>array(  ), 
			'wiid'=>array( 'req'=>array('wiid', 'roid') )
		);
	}
	
	/**
	 *
	 */
	function init(&$sql) {
		
		
		$tablename=$this->tablename;
		
		$colNames = columns_get2($tablename);
		$primas   = primary_keys_get2($tablename); // old parameter: $colNames
		$this->info['prim_name'] = $primas[0];                    // main primary key
		$this->info['access_id_has'] = cct_access_has2($tablename); 
		
		$class_tab_has=NULL;
		$classname = NULL;
		$exp_raw_desc_id = 0;
		list( $this->selectCols,  $this->useJoin) = $this->viSubObj->colinfoget( $sql, $tablename, $colNames, 
					$this->info['access_id_has'], $class_tab_has, $classname, $exp_raw_desc_id,  0);
		
		
		// $tableSCondM = $_SESSION['s_tabSearchCond'][$tablename]["w"];
		//$sel_info    = $_SESSION['s_tabSearchCond'][$tablename]["info"];
		$this->info['sel_mother']  = $_SESSION['s_tabSearchCond'][$tablename]["mothid"];
	}
	
	
	function do_feature( &$sql, &$parx, $adv_colname, $adv_col, $adv_bool, $adv_fkval, $adv_fkbool ) {
		
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
		$tablename = $this->tablename;
		$condclean = 1;
		$searchOp  = "";
		
		if ($parx["bool"]!="") {
		  $searchOp	    = "AND";	// add to old condition
		  $condclean	= 0;
		  echo "Info: add new filters to old condition.<br>";
		}
		
	    if (is_array($adv_col) ) { // build condition 
		
	        foreach( $adv_col as $cntid=>$tmpval) {
			
				$colname    = $adv_colname[$cntid];
	            $searchtxt  = $tmpval;
	            $searchCol  = $colname;
	            $searchBool = $adv_bool[$cntid];
	
	            if ($searchtxt!="") {
	                $tmpDoPrepare=1;
					if (strstr($searchBool,"LIKE")!=NULL)  $searchtxt = "%".$searchtxt."%"; // append wildcards
					
					if ($searchBool=="inlist") {
						$tmpDoPrepare=0;
						
						do {
							$tmparr = explode("\n", $tmpval);
							if (sizeof($tmparr)>1)  break;
							$tmparr = explode("\t", $tmpval);
							if (sizeof($tmparr)>1)  break;
							$tmparr = explode(" ", $tmpval);
						} while (0);
						$tmpor     = "";
						$tmpwhere  = "";
						foreach( $tmparr as $dummy=>$valx) {
							$valx = trim ($valx);
							
							if ($valx!="") {
								$tmpwhere .= $tmpor . $searchCol."='".str_replace ("'", "''",$valx)."'"; 
								$tmpor = " OR ";
							}
						}
						$searchCol  ="";
						$searchtxt  ="";
						$searchBool ="";
						$tableSCond = "(".$tmpwhere.")";
					}	
					
					$searchAlias=NULL;
					
					/*
					if ($searchCol=='a.user_name') {
						$error->set( $FUNCNAME, 2, 'Searching for "user" still not supported.' );
						return;
					}
					*/
					
					 $searchArr = array( 
				  		"alias"=>  $searchAlias, 
				  		"cond"=>   $tableSCond, 
				  		"column"=> $searchCol, 
				  		"stext"=>  $searchtxt, 
						"op"=>     $searchBool, 
						"condclean" => $condclean
						);
					
	                if ($tmpDoPrepare) $retval = $this->viSubObj->searchPrepare($sql, $tablename, $searchArr);
		            if ($error->Got(READONLY))  {
						$error->set( $FUNCNAME, 1, 'Error on prepare.' );
						return;
					}
					
	                
	                $topt = NULL; 
	                $topt["useJoin"] = $this->useJoin;
	                list ( $sqlfromXtra, $tableSCondM, $sqlWhereXtra, $sel_info, $classname, $mother_idM ) = 
	                         selectGet( $sql, $tablename, $searchArr["condclean"], $searchArr["cond"], $_SESSION['s_tabSearchCond'], $topt, 
	                         	$searchArr["stext"], $searchArr["column"],
					         	$sel, $searchArr["op"], $searchOp );
	
	                $condclean = 0; // after that AND
	                $searchOp  = "AND";
	
	                $_SESSION['s_tabSearchCond'][$tablename] = 
	 	                array ( "f"=>$sqlfromXtra, "w"=>$tableSCondM, "x"=>$sqlWhereXtra, "c"=>$classname, "info"=>$sel_info, "mothid"=>$mother_idM ); // save selection array 
	
	
	            } 
	            // echo "DEBUGGER: $searchCol::$searchBool::$searchtxt<br>";
	        }
	        
	    }
	    
	    if (is_array($adv_fkval) ) { // build condition 
	        foreach( $adv_fkval as $cntid=>$tmpval) {
			
	   			$colname   = $adv_colname[$cntid];
	            $searchtxt = "";
	            $searchCol = "";
	            $searchBool= "";
	            $tmpcond = $adv_fkbool[$cntid];
	            
	            if ($tmpval != "") {
	                
	                if (!$tmpcond) $tmpcond = "=";
	                $colname_pure = substr($colname,2); // remove "x."
	                $ret_primary_name = "";
	                $ret_imp_name = "";
	                $linked_tab = fk_check2( $colname_pure, $tablename, $ret_primary_name, $ret_imp_name);
	                if ( $tmpcond == "LIKE") {
	                      $ret_imp_name = "UPPER($ret_imp_name)";
	                      $tmpval = strtoupper($tmpval);
	                }  
	                $tableSCond = "$colname_pure in (select $ret_primary_name from $linked_tab 
	                               where ".$ret_imp_name ." ". $tmpcond. " '". $tmpval. "' )";
	                $topt = NULL; 
	                $topt["useJoin"] = $this->useJoin;
	                list ( $sqlfromXtra, $tableSCondM, $sqlWhereXtra, $sel_info, $classname, $mother_idM ) = 
	                         selectGet($sql, $tablename, $condclean, $tableSCond, $_SESSION['s_tabSearchCond'], $topt, $searchtxt, $searchCol,
					         $sel, $searchBool, $searchOp );
	
	                $condclean=0; // after that AND
	                $searchOp="AND";
	
	                $_SESSION['s_tabSearchCond'][$tablename] = 
	 	                array ( "f"=>$sqlfromXtra, "w"=>$tableSCondM, "x"=>$sqlWhereXtra, "c"=>$classname, "info"=>$sel_info, "mothid"=>$mother_idM ); /* save selection array */
	
	
	            }
	        }
	        echo "<br>\n";
	    }
	    
		if ( $_SESSION['userGlob']["g.debugLevel"]>0 ) {
	    	glob_printr( $_SESSION['s_tabSearchCond'][$tablename] , "s_tabSearchCond  info" );
		}
	    js__location_replace('view.tmpl.php?t='.$tablename, 'execute query' ); 
	    
	   
	    return;
	}
	
	function form_feature() {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
		
		$tablename  = $this->tablename;
		$selectCols = &$this->selectCols;
		
		if (!sizeof($selectCols)) {
			$error->set( $FUNCNAME, 1, 'no Columns exist. Bizarre!' );
			return;
		}
		
		echo "<form method=\"post\"  action=\"".$_SERVER['PHP_SELF']."?go=1&tablename=$tablename\" >";
		echo "<table border=0 cellpadding=1 cellspacing=0 bgcolor=#336699>\n";
		echo "<tr bgcolor=#336699><td colspan=3><font color=#FFFFFF>&nbsp;&nbsp;&nbsp;<B>Combined feature search</B></td></tr>\n";
		echo "<tr><td>";
		echo "<table border=0 cellpadding=1 cellspacing=1 bgcolor=#EFEFEF>\n";
		
		
		$tmp_arr=array();
		$tmp_arr["="]	="=";
		$tmp_arr["LIKE"]="LIKE";
		$tmp_arr[">"]	="&gt";
		$tmp_arr["<"] 	="&lt;";
		$tmp_arr[">="]	="&gt;=";
		$tmp_arr["<="] 	= "&lt;=";
		$tmp_arr["!="] 	= "!=";
		$tmp_arr["NOT LIKE"]="not LIKE";
		$tmp_arr["is NULL"] 	= "NULL";
		$tmp_arr["is NOT NULL"] = "not NULL";
		$tmp_arr["inlist"]  = "in list";
		$colsize = 40;
		
		for ( $rowid=0; $rowid<5; $rowid++ ) {
			
			
			echo "<tr valign=top>";
			echo "<td>";
			echo "<select name=adv_colname[$rowid]>\n";
			
			foreach( $selectCols as $th1) { 
		
				/**
				 * "show"	  =>1,
            "nice"	  =>"audit status",
            "key"     =>"NAME",
            "tab"     =>"H_ALOG_ACT",
            "fk_pname"=>"H_ALOG_ACT_ID",
            'tdAdd'   =>'class=importx',
				 * @var string $selected
				 */
				$selected=""; 
				$db_key  = key($th1);
				//$db_nice = current($th1);
				$db_show = $th1['show'];
				$db_conc = $th1['nice'];
				$db_fkf  = $th1['key'];
				$db_fktab= $th1['tab'];
				$db_fkprim=$th1['fk_pname'];
				
				
			
				if ( $db_key=="x.EXTRA_OBJ_ID" ) $db_show=0; 
				$db_conc_out= $db_conc;
				if ( $db_fktab ) {
					$db_conc_out= "ID of ".$db_conc;
				}
				if ( $db_key=="a.DB_USER_ID" ) {
					$db_conc_out= "user";
					$db_key = "a.user_name";
					$db_fkf = NULL;
				}
				
				if ($db_show) {
					
					$value_ini = "";
					// for assoc tables => set PRIM_ID of mother
					if ( $this->info['sel_mother'] != "" ) {
						if ( "x.".$prim_name == $db_key ) $value_ini = $this->info['sel_mother'];
					}
					echo "<option value=\"".$db_key."\">".$db_conc_out."</option>";
					
				}
			}
			echo "</select>\n";
			echo "</td><td>";
			$tmp2_arr = $tmp_arr;
					
			echo "<select name=adv_bool[$rowid]>\n";
			foreach( $tmp2_arr as $cond_tmp=>$cont_txt) {
				echo "<option value=\"".$cond_tmp."\" >".$cont_txt."\n";
			} 
			echo "</select>\n";
			echo "</td><td>";
			if (  $rowid==0 ) { 
				echo " <textarea name=\"adv_col[". $rowid . "]\" cols=".$colsize." rows=4>".$value_ini."</textarea>\n";
			} else {
				echo " <input type=text name=\"adv_col[". $rowid . "]\" value=\"".$value_ini."\" size=".$colsize."> \n"; 
			}
			echo "</td></tr>\n";
		}
		
		
		echo "<tr><td colspan=3><input type=checkbox name=\"parx[bool]\" value=\"1\"> <font color=gray>&nbsp;Add to last filter condition</font></td></tr>\n";
		echo "<tr><td>&nbsp;</td><td><input type=submit value=\"    QUERY    \"></td><td>&nbsp;</td></tr>\n";
		echo "</table>\n";
		echo "</td></tr></table>\n";
		echo "<input type=hidden name=\"xaction\" value=\"profeature\">";
		echo "</form>\n";
	}
	
	/**
	 * @param $inarr 
	 * 			0:key
	 */
	function _formOtherRow($action, $text, $notes=NULL) {
		echo '<tr><td>';
		echo "<input type=button value=\"Submit\" onClick=\"document.more.xaction.value='".$action."'; document.more.submit();\">";
		echo '</td>';
		echo '<td><font color=#336699><B>'.$action.'</B></font></td><td>';
		echo $text;
	    echo "</td></tr>";
	}
	
	function _formField($type, $name, $title) {
		return '<b>'.$title.'</b> <input type=text value="" name="parx['.$name.']" size=7>'."\n";
	}
	
	function form_more() {
		
		
		echo '<font color=#336699><B>More search forms ...</B></font><br><br>'."\n";
		echo '<form method="post" name="more" action="'.$_SERVER['PHP_SELF'].'">'."\n";
		echo '<input type=hidden name="xaction" value="">';
		echo '<input type=hidden name="go" value="1">'."\n";
		echo '<input type=hidden name="tablename" value="'.$this->tablename.'">';
		
	    echo "<table border=0 cellpadding=1 cellspacing=1 bgcolor=#EFEFEF>";
	    
	    $this->_formOtherRow( 'wiid', $this->_formField('text', 'wiid', 'WIID').' '.
	    	$this->_formField('text', 'roid', 'ROID') );
	    
	    
	    echo "</table>\n";
	    echo "</form>\n";
	}
	
	function _checkReqParams($action, $parx) {
		$tmparr = $this->actions[$action];
		
		if (!sizeof( $tmparr['req'] ) ) return;

		$retstr=NULL;
		$tmpkomma=NULL;
		foreach( $tmparr['req'] as $dummy=>$param) {
			if ( $parx[$param]==NULL ) {
				$retstr .= $tmpkomma . 'Missing '.$param.' ';
				$tmpkomma = ',';
			}
		}
		reset ($tmparr['req']);
		return $retstr;
	}
	
	function do_more( &$sql, $action, $parx ) {
		global $error;
		$FUNCNAME= 'do_more';
		$tablename = $this->tablename;
		
		$searchdone = NULL;
		$out_Error   = NULL;
		$out_Matches = 0;
		$out_Done	 = NULL;
		$out_Forward = NULL;
		
		echo 'Action : <b>'.$action.'</b><br>';
		
		if ( !isset($this->actions[$action]) ) {
			htmlErrorBox("Error", "Parameter check", 'Action '.$action.' not known.');
			return;
		}
		
		$answer = $this->_checkReqParams($action, $parx);
		if ( $answer!=NULL ) {
			htmlErrorBox("Error", "Parameter check", $answer );
			return;
		}
		
		switch ($action) {
			case 'wiid':
				 
				$out_Done = "object with WIID:ROID: ".$parx['wiid'].":".$parx['roid'];
				$roidSearchObj = new fWiidRoidC();
				$found_id      = $roidSearchObj->getObjID ($sql, $tablename, $parx['wiid'], $parx['roid']);
				if ($error->Got(READONLY))  {
					$errLast   = $error->getLast();
					$error_txt = $errLast->text;
					$error->reset();
					$out_Error   = $error_txt;
					$out_Matches = 0;
				} else {
					$out_Matches = 1;
					$out_Forward = 'edit.tmpl.php?t='.$tablename.'&id='.$found_id;
				}
				break;
				
			default:
				$error->set( $FUNCNAME, 1, 'action '.$parx['action'].' not known.' );
				return;
		}
		
		if ($out_Error!=NULL) {
			htmlErrorBox("Error", "Parse info", $out_Error);
			return;
		}
		echo 'Search done: <b>'.$out_Done.'</b><br>';
		echo 'Found objects: <b>'.$out_Matches.'</b><br>';
		
		if ( $out_Matches>0 and $out_Forward!=NULL ) {
			js__location_replace($out_Forward, "Forward search" );
		}
		return;
	}
}

// --------------------------------------------------- 
global $error, $varcol;
$FUNCNAME='MAIN';

$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] );

$tablename=$_REQUEST['tablename'];
$t=$_REQUEST['t'];
$go=$_REQUEST['go'];
$adv_colname=$_REQUEST['adv_colname'];
$adv_col   =$_REQUEST['adv_col   '];
$adv_bool  =$_REQUEST['adv_bool  '];
$adv_fkval =$_REQUEST['adv_fkval '];
$adv_fkbool=$_REQUEST['adv_fkbool'];
$parx=$_REQUEST['parx'];
$xaction=$_REQUEST['xaction'];


if ($t!="") $tablename = $t;
$title= " [Pro-Feature-Search] combined feature search";

$infoarr			 = NULL;
$infoarr["scriptID"] = "";
$infoarr["title"]    = $title;
$infoarr["title_sh"] = "Pro-Feature-Search";
$infoarr["form_type"]= "list";
$infoarr["help_url"] = "Search_advanced.html"; 
$infoarr["icon"]     = "images/i40.prosearch.gif";
if ($tablename!="") {
	$infoarr["obj_name"] = $tablename;
	$infoarr["obj_cnt"]  = 1; 
}


$pagelib = new gHtmlHead();
$pagelib->_PageHead ( $infoarr["title"],  $infoarr );
$retarr = $pagelib->_startBody( $sql, $infoarr );

$MainLib = new gObjTabSearch1($tablename);

if ( $tablename == "" ) {
  // echo "ERROR: No tablename given!<br>";
  echo "<ul>";
  require_once("f.visu_list.inc");
  visu_listC::selTable($sql, $_SERVER['PHP_SELF']); 
  htmlFoot();
}

$MainLib->init($sql);

if ($go==1 and $xaction=='profeature' ) {
   $MainLib->do_feature( $sql, $parx, $adv_colname, $adv_col, $adv_bool, $adv_fkval, $adv_fkbool );
	if ($error->Got(READONLY))  {
		$error->printAllEasy();
	}
   return;
}

if ($go==1  ) {
   $MainLib->do_more( $sql, $xaction, $parx );
   $pagelib->htmlFoot();
}

echo "<ul>\n";


if ($_SESSION['s_tabSearchCond'][$tablename]["info"]!="") {
	
	htmlInfoBox( "Current filter condition", "", "open", "CALM" );
	$tmpstr = $_SESSION['s_tabSearchCond'][$tablename]["info"];
	if (strlen($tmpstr)>80)  $tmpstr = substr($tmpstr,0,80) . " ...";
	$tmpstr = htmlspecialchars($tmpstr);
	echo $tmpstr;
	htmlInfoBox( "", "", "close" );
	echo "<br>";
}

$MainLib->form_feature();
if ($error->Got(READONLY))  {
	$error->set( $FUNCNAME, 1, 'Problem for table '.tablename_nice2($tablename) .' detected.' );
	$error->printAllEasy();
	return;
}

// ------------------------------------------------

htmlInfoBox( "Short help", "", "open", "HELP" );
?>
<ul><li> The LIKE-operator already adds WILDCARDS '%' around the pattern</li>
<li> The The "in list" operators selects all elements which have are equal to one of the entries. Entries are separated by NEWLINE, TAB or WHITE_SPACE.</li>
<li> The search cleans the old condition.</li>
<li> <a href="javascript:open_info('help/robo/Search_in_table.html')">... more help for syntax</a></li>
</ul>
<?php

htmlInfoBox( "", "", "close" );
echo "<br><br>\n";

 
if ( $MainLib->info['access_id_has'] ) {  // search for ROID
	
	$MainLib->form_more();
    
}


htmlFoot("<hr>");
