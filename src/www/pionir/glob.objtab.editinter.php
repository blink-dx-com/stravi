<?php
/**
 * BULK-EDITOR ( edit each elements by hand)
 * Refatoring: 2021-05-05
 * @package glob.objtab.editinter.php

 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   $tablename
         $go 0 - absolute start 
		 	 0.5 - text input
			 1 - prepare: if (!$inType) => $valtext will be saved as $vx
			 2 - insert
         $col = "" (default: important name)
		 $valtext = VALUES in a text field, each line is an object name
         $inType : 
			0 : text field, 
			1: array of values, 
			2: dblink
		 $vx : value array [OBJID] = value
		 $selcol ???
 */
session_start(); 


require_once ("reqnormal.inc"); 
require_once ("sql_query_dyn.inc");
require_once ("visufuncs.inc");
require_once ('func_form.inc');
require_once ('glob.obj.col.inc');
require_once ('f.update.inc');
require_once ("glob.obj.update.inc");

// $mainScriptObj->
// $tablename = $this->tablename;

class gEditInterC_GUI {

function __construct( $tablename, $col, $inType ) {

	if ( $col == "" ) $col = importantNameGet2($tablename);
	$this->tablename = $tablename; 
	$this->col = $col;
	$this->inType = $inType;

	$this->globColGuiObj = new globTabColGuiC();
	$this->globColGuiObj->initTab($tablename);
	$this->formobj = NULL;
}

function checkColumn( &$sqlo ) {
	//  
	
	$tablename	= $this->tablename;
	$col = $this->col;

	$infox   	= NULL;
	$infox["errorWas"] = 0;
	$primary_key = PrimNameGet2($tablename);
	
	$infox["nameCol"]     = importantNameGet2($tablename);
	$infox["valNotNull"]  = 0;
	
	$infox["updateCol"]   = $this->col;
	$infox["mainColNice"] = columnname_nice2($tablename, $infox["updateCol"]); 
	
	$colFeat = colFeaturesGet($sqlo, $tablename, $infox["updateCol"]);
	if ($colFeat["NOT_NULL"]>0) $infox["valNotNull"]  = 1;
	
	$appTypeID = $colFeat["APP_DATA_TYPE_ID"];
	$infox["appTypeName"] = appDataTypeNameGet2($appTypeID);
	
	if ( $infox["mainColNice"] == "" ) htmlFoot("Error", "Column '".$infox["updateCol"]. "' is not known to the system.");

	if ( $this->isDeniedColumn( $infox["updateCol"], $primary_key) ) {
		htmlFoot("Error", "Column '".$infox["mainColNice"]. "' is not allowed for update");
	}

	if ( $this->inType==2 ) {
		
		// get remote table
		$colFeatures = colFeaturesGet( $sqlo, $tablename, $col );
		if ( $colFeatures['CCT_TABLE_NAME']==NULL ) {
			htmlFoot("Error", "Column '".$infox["mainColNice"]. "' must be an object-link, if database-link was selected..");
		}

		$infox["colMotherTable"] = $colFeatures['CCT_TABLE_NAME'];
	
	}

	$this->infox = $infox;

	return ($infox);
}

function getMaxLenField( &$valArrx ) {
	// FUNCTION: calculate the max field length  for the HTML-Form
	// INPUT: array of selected values
	$maxlen = 20;
	foreach( $valArrx as $key=>$xval) {
		if (strlen($xval)>$maxlen) $maxlen = strlen($xval); // if empty, add a WHITESPACE, otherwise textarea ignores PURE newlines
	}
	reset ($valArrx);
	$maxlen = $maxlen + 3; // extend field
	if ($maxlen>100)  $maxlen=100;
	return ($maxlen);
}

function getTextareLen($formFieldLen) {
	$retval = $formFieldLen;
	if ( $retval<80 )  $retval = 80;
	return ($retval);
}

function isDeniedColumn($thisCol, $primary_key) {
	// RETURN: 1 : if this column is denied for update
	$badFound = 0;
	$badCols = array($primary_key, "CCT_ACCESS_ID", "EXTRA_OBJ_ID");
	foreach( $badCols as $key=>$val) {
		if ($thisCol == $val) {
			$badFound = 1;
			break;
		}
	} 
	return ($badFound);
}

function FromColumnOut( &$sql, $colName, $selflag ) {

	$tablename = $this->tablename;
	$colInfos  = $this->globColGuiObj->analyzeColumn( $sql, $colName ); 

	$showcol = $colInfos["showcol"];

	if ( $showcol ) { 

		$nice_name = $colInfos["nice"];
		$colcomment= "";
		$colcomment= column_remark2($tablename, $colName);

		$fieldx = array ( 
			"title"  => $nice_name, "name" => "col", "namex"=>1,
			"object" => "radio", "notes" =>  $colcomment, "inits" => $colName
			 );
		if ($selflag) $fieldx["val"] = $colName;
		$this->formobj->fieldOut( $fieldx );
	}
}

function FormColNames( &$sql, $selval) {

    $cnt=0;
	$tablename = $this->tablename;
    $colNames  = columns_get2($tablename); 
	
	$mainColName = 'NAME';
	$arguidarr = array_keys( $colNames, $mainColName );
	reset($colNames);
	$name_pos = "";
	if ( sizeof($arguidarr) ) {
		$name_pos = $arguidarr[0];
		$this->FromColumnOut( $sql, $mainColName, 1 );
		$fieldx = array ("object" => "space");
		$this->formobj->fieldOut( $fieldx );
		unset ($colNames[$name_pos]); // delete column from list
	}

	if (!sizeof($colNames)) return; 
	  	
    foreach( $colNames as $dummy=>$colName) {
      
			$this->FromColumnOut( $sql, $colName, 0 );

    } 
     
}

function Form2(&$sql, $col, $inType, $selcol) {
	
	
	
	$tablename = $this->tablename;
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "BulkEdit settings";
	$initarr["submittitle"] = "Next &gt;&gt;";
	$initarr["tabwidth"]    = "AUTO";
	$initarr["goNext"]      = "0.5";
	$initarr["tabnowrap"] = 1;
	
	$hiddenarr = NULL;
	$hiddenarr["tablename"] = $tablename;
	// $hiddenarr["col"]     	= $col;
	

	$this->formobj = new formc($initarr, $hiddenarr, "0");

	$fieldx = array ( "title" => "Type of input", 
			"name"  => "inType",
			"object" => "radio",
			"colspan"=>"2",   
			"optx"  => array("rowbr"=>1),
			"val"   => $inType, 
			"namex" => 1,
			"inits" => array (
				0=>"ONE big text-field for ALL", 
				1=>"one text-field per object",
				2=>"DB-Link-Selector",
				)
			 );
	$this->formobj->fieldOut( $fieldx );
	
	
	$fieldx = array ( "title" => "Select column", "object" => "info" );
	$this->formobj->fieldOut( $fieldx );
	
	$this->FormColNames($sql, $selcol);

	$this->formobj->close( TRUE );
}

function ShowForm(
	&$sqlo,   
	&$valtextNew,// text field
	&$valArrx,	 // value array
	&$valArrOld, // old values
	$inType,	 // "" : use $valtextNew, 1: use $valArrx
	$go, 
	&$infoArr,   // array[ID] = array(name, errortext, info)
	&$infox
	) {
	
	
	
	$tablename = $this->tablename;
	$updateCol = $this->col;
	
	
	if ( $go <= 1 ) {
	
		$tmpinf = " (one text field)";
		if ($inType) $tmpinf = " (table design)";
		if ($inType==2) $tmpinf = " (database-link-selector)";

		$initarr   = NULL;
		$initarr["action"]      = $_SERVER['PHP_SELF'];
		$initarr["title"]       = "Edit new values". $tmpinf;
		$initarr["submittitle"] = "Prepare update &gt;&gt;";
		$initarr["tabwidth"]    = "AUTO";
		$initarr["dblink"]		= 1;
		$initarr['tabnowrap']	= 1;
		if ( $go == 0.5 ) $initarr["goNext"]      = "1";
		
		if ( $go == 1 ) $initarr["submittitle"] = "Update now!";
	
		$hiddenarr = NULL;
		$hiddenarr["tablename"]     = $tablename;
		$hiddenarr["col"]     		= $updateCol;
		$hiddenarr["inType"]     	= $inType;
		
		if ( $go==1 ) {
			
			foreach( $valArrx as $key=>$xval) {
				$hiddenarr["vx[".$key."]"] = $xval;
			}
			reset ($valArrx);
			
		}
	
		$this->formobj = new formc($initarr, $hiddenarr, $go);
		
		$formFieldLen = $this->getMaxLenField( $valArrx );
		$textareaLen  = $this->getTextareLen($formFieldLen);
	
		if ( $go<1 ) {

			switch ($inType) {

				case 2:
					$MotherTable = $this->infox["colMotherTable"];
					$cnt=0;
					foreach( $valArrx as $key=>$xval) {

						if ($xval) $objname = obj_nice_name ( $sqlo, $MotherTable, $xval );
						else  $objname = ' --- ';
						$fieldx  = array ( 
							"title" => htmlspecialchars($infoArr[$key][0]). " [ID:$key]", 
							"name"  => "vx[".$key."]",
							"object"=> 'dblink', 
							"inits" => array( 
								"table"   =>$MotherTable, 
								"objname" =>$objname, 
								"pos" =>$cnt, 
								"projlink"=> 1),
                                             
							"val"   => $xval, 
							"namex" => 1,
							"fsize" => $fsize
							);
						$this->formobj->fieldOut( $fieldx );
						$cnt++;
					}
					reset ($valArrx);
					break;
				
				case 1:
			
					$tempInits = NULL;
					$objectType = "text";
					$fsize = $formFieldLen;
					if ( $infox["appTypeName"] == "notes" ) {
						$objectType = "textarea";
						$tempInits = array("rows" => 2, "cols" => $textareaLen);
						$fsize = "";
					}
					foreach( $valArrx as $key=>$xval) {
						$fieldx = array ( "title" => htmlspecialchars($infoArr[$key][0]). " [ID:$key]", "name"  => "vx[".$key."]",
							"object"=> $objectType, 
							"inits" => $tempInits,
							"val"   => $xval, 
							"namex" => 1,
							"fsize" => $fsize
							);
						$this->formobj->fieldOut( $fieldx );
					}
					reset ($valArrx);
					break;
			
				default:
					$valtextNew = "";
					$tmpNewline = "";
					foreach( $valArrx as $key=>$xval) {
						if ($xval=="") $xval = " "; // if empty, add a WHITESPACE, otherwise textarea ignores PURE newlines
						$valtextNew .= $tmpNewline . $xval;
						$tmpNewline = "\n";
					}
					reset ($valArrx);
				
					$fieldx = array ( "title" => "Values", "name"  => "valtext",
							"object" => "textarea",
							"val"   => $valtextNew, "namex" => 1,
							"inits" => array("rows" => $infox["numTextobj"], "cols" => $textareaLen),
							"notes" => "" );
					$this->formobj->fieldOut( $fieldx );
			}
			
		}
		
		$this->formobj->close( TRUE );
		
		
		if ( $go<1 AND $infox["errorWas"] > 0 ) {
			// show errors in an extra field
			echo "<br>";
			htmlInfoBox( "Detected problems", "", "open", "ERROR" );
			foreach( $infoArr as $key=>$xarr) {
				$tmpinfo  = $xarr[1];
				if ($tmpinfo!="") {
					echo "object-ID: ".($key).": ".$tmpinfo."<br>\n";
				} 
			}
			htmlInfoBox( "", "", "close" );
		}
	
	} 
	
	if ( $go >= 1) {
	
		echo "<b>INFO:</b><br>";
		$tabobj = new visufuncs();
		$headx = array("ID", "Name of object");
		
		$mainName = importantNameGet2($tablename);
		if ($mainName!=$updateCol) {
			$headx[] = "OldVal";
		}
		 
		$headx[] = "NewVal";
		$headx[] = "Info";
		$tabobj->table_head($headx);
		$anaArr = &$valArrx;
		$MotherTable = $this->infox["colMotherTable"];
		
		$i=0;
		while ( $i < $infox["numTextobj"] ) {
		
			$topt = NULL;
			
			$objid = key($infoArr);
			$xarr  = current($infoArr);
			next($infoArr);
			
			$tmperror = $xarr[1];
			$tmpinfo  = $xarr[2];
			
			$objid2 = key($valArrx);
			$newval = current($valArrx);
			next($valArrx);
			
			
			if ( !$inType ) {
				$dummy=0;
			} else {
				
				if ($objid2!=$objid) htmlFoot("Error", "function thisShowForm();".
					" IDs of objects do not match with Info-Obj-ID.!");
			}
			if ($tmperror!="")  $tmperror = "<font color=red>Error:</font> $tmperror; ";
			$tmpinfoAll = $tmperror . $tmpinfo;
			if ($tmperror!="") $topt["bgcolor"] = "#FFD0D0";
			
			$dataArr = array($objid, $xarr[0]);
			
			if ($mainName != $updateCol) {
				// add old-val
				$dataArr[] = $valArrOld[$objid];
			}
			if ( $MotherTable!=NULL and $newval!=NULL ) {
				$valout = obj_nice_name ( $sqlo, $MotherTable, $newval ). ' [ID:'.$newval.']';
			} else {
				$valout = $newval;
			}

			$dataArr[] = $valout;
			$dataArr[] = $tmpinfoAll;
			
			$tabobj->table_row($dataArr, $topt);
			$i++;
			
		}
		reset ($valArrx);
		$tabobj->table_close();
	
	}

	
}

function GoInfo( $go ) {
	
	$goArray   = array( 
			"0"=>"Select column", 
			'0.5'=>"Edit values of column",
			 1=>   "Prepare update of column",
			 2=>   "Do update of column"
		);

	$extratext = '[<a href="'.$_SERVER['PHP_SELF'].'?tablename='.$this->tablename.'">Start again</a>]';
	
	$formPageLib = new FormPageC();
	$formPageLib->init( $goArray, $extratext );
	$formPageLib->goInfo( $go ); 

}

}

class gEditInter_C {
    
    function __construct( $tablename, $sqlAfter, &$vx, &$vxNew, $inType ) {
       $this->tablename = $tablename;
       $this->primary_key = PrimNameGet2($tablename);
       $this->sqlAfter=$sqlAfter;
       $this->vx = $vx;
       $this->vxNew= $vxNew;
       $this->inType=$inType;
    }
    
    function update($sql, $sql2, &$infox, $go) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $flagProduceText = 1;
        $flagGotText = 0;
        if ( sizeof($this->vx) ) {
            $flagProduceText = 0;
            $flagGotText     = 1;
        }
        
        $tablename=$this->tablename;
        $inType = $this->inType;
        $vx = &$this->vx;
        
        $primary_key = $this->primary_key;
        $sqlAfter=$this->sqlAfter;
        
        $sqlsLoop = "SELECT x.".$primary_key.", x.".$infox["nameCol"].", x.".$infox["updateCol"]." FROM ".$sqlAfter;
        $sql->query($sqlsLoop);
        
        //$showObj 	= 1;
        //$tmpNewline = "";
        $objcnt  	= 0;
        $this->infoArx 	= NULL;
        $this->valArrOld  = array();
        
        while ( $sql->ReadRow() ) {	// for EACH object ...
            
            $loopError = "";
            $loopInfo  = "";
            $doUpdate  = 0;
            $objid  = $sql->RowData[0];
            $tmpName= $sql->RowData[1];
            $tmpCol = $sql->RowData[2];
            
            if ( $flagGotText ) {
                $thisNewVal = trim($vx[$objid]);
            }
            
            if (!$inType) {
                if ( strstr($tmpCol,"\n") != NULL ) {
                    $error->set( $FUNCNAME, 1, "Object (ID:$objid) name: '".$tmpCol."' contains a NEWLINE in column ".
                        $infox["mainColNice"].". This is not allowed for this tool!" );
                    return;
                }
            }
            $o_rights = access_check( $sql2, $tablename, $objid);
            if ( !$o_rights["write"] ) $loopError = "No write access";
            
            if ( $go>=1 AND $infox["valNotNull"] AND $thisNewVal=="") {
                
                $loopError = "Value missing.";
            }
            
            if ( $thisNewVal !== $tmpCol ) { // real compare (alos leading ZEROs)
                $loopInfo = "update";
                $doUpdate = 1;
            }
            
            if ($go==2 AND $loopError=="" AND $doUpdate) {
                
                $args=array(
                    'vals'=>array($infox["updateCol"] => $thisNewVal)
                );
                
                $UpdateLib = new globObjUpdate();
                $UpdateLib->update_meta( $sql2, $tablename, $objid, $args );
                if ($error->Got(READONLY))  {
                    $errLast   = $error->getLast();
                    $error_txt = $errLast->text;
                    $loopError = "Update failed: ".$error_txt;
                    $error->reset();
                }
                
            }
            
            if ( $flagProduceText ) {
                $this->vxNew[$objid] = $tmpCol;
            }
            $this->valArrOld[$objid] = $tmpCol;
            
            // collect infos, errors
            
            if ($loopError!="") {
                $infox["errorWas"] ++;
                $loopInfo = "";
            }
            $this->infoArx[$objid] = array($tmpName, $loopError, $loopInfo);
            $objcnt ++;
        }
        $infox["numTextobj"] = $objcnt;
        
        if ($infox["errorWas"]>0) {
            $xopt = array("vspace"=>1);
            htmlInfoBox( "Errors detected", "Erros detected in data analysis.", "", "ERROR", $xopt );
            
        }
        
        echo "<br>";
        
        $outInfx = "Analysed objects: <b>".$objcnt."</b>&nbsp;";
        if ($infox["errorWas"]>0) $outInfx .=  ", &nbsp;&nbsp;<font color=red>Errors:</font>&nbsp;<b>".$infox["errorWas"]."</b>&nbsp;";
        echo "Analysis:&nbsp;".$outInfx;
        echo "<br>";
    }
}
// --------------------------------------------------- 
global $error, $varcol;

$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] );
$sql2  = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();

$tablename = $_REQUEST['tablename'];
$go = $_REQUEST['go'];

$col = $_REQUEST['col'];
$valtext = $_REQUEST['valtext'];
$inType = $_REQUEST['inType'];
$vx = $_REQUEST['vx'];
// $selcol= $_REQUEST['selcol'];


$title       = "BulkEdit - Interactive bulk update";
$title_sh    = "BulkEdit";
$infoarr=array();
$infoarr["help_url"] = "g.Bulk_edit.html";

$infoarr["title"]    = $title;
$infoarr["title_sh"] = $title_sh;
$infoarr["form_type"]= "list";
$infoarr["obj_name"] = $tablename;
$infoarr["obj_cnt"]  = 1;          // show number of objects

$MAX_ELEM	 = 500;
$title_short = "BulkEdit";
$flagAct 	 = "";

if ( $go <= 0 ) $go = 0;

$pagelib = new gHtmlHead();
$headarr = $pagelib->startPage($sql, $infoarr);

$mainScriptObj = new gEditInterC_GUI($tablename, $col, $inType);
$col = $mainScriptObj->col;

echo "[<a href=\"".$_SERVER['PHP_SELF']."?tablename=".$tablename."&go=0\">Start</a>]";
echo " <i>Info: This tool allows to edit data of a selection of objects. You can edit values of ONE column (e.g. name of object).</i>";
echo "<br>";
echo "<ul>";

if ($tablename=="") htmlFoot("Error", "Tablename missing"); 

gHtmlMisc::func_hist("glob.objtab.editinter", $title_short, $_SERVER['PHP_SELF']."?tablename=".$tablename );

$tablenice = tablename_nice2($tablename);

// 
//   check   R I G H T S
//
$t_rights = tableAccessCheck( $sql, $tablename );
if ( $t_rights["write"] != 1 ) {
	tableAccessMsg( $tablenice, "write" );
	htmlFoot();
}

$tab_isBo  = cct_access_has2($tablename);
if ( !$tab_isBo ) {
	htmlFoot("Error", "Only Business objects are supported for this function!");
}
$sqlopt=array();
$sqlopt["order"] = 1;
$sqlAfter  = get_selection_as_sql( $tablename, $sqlopt);


$stopReason = "";
$tmp_info   = $_SESSION['s_tabSearchCond'][$tablename]["info"];
if ($tmp_info=="") $stopReason = "No elements selected.";
if ($headarr["obj_cnt"] <= 0) $stopReason = "No elements selected.";
if ($headarr["obj_cnt"] > $MAX_ELEM) $stopReason = "Too many elements (".$headarr["obj_cnt"].") selected. Please select less than ".$MAX_ELEM;

if ($stopReason!="") {
    htmlFoot("Attention", $stopReason." Please select elements of '".$tablenice."'!");
}

$infox = $mainScriptObj->checkColumn( $sql );
$mainScriptObj->GoInfo($go, $infox["mainColNice"]);

if ( !$go ) {
	$col = $infox["nameCol"];
	$mainScriptObj->Form2($sql, $infox["updateCol"], $inType, $col);
	htmlFoot();
}

$primary_key = PrimNameGet2($tablename);

$errArr     = NULL;
$valtextNew = "";
$vxNew      = NULL;

if ( !$inType AND $go<2 ) {
	if ($valtext!="")  $valtextNew = $valtext; // text area given
	if ( $go >= 1 ) {
		$anaArr     = explode ("\n", $valtext);
		$numElems   = sizeof($anaArr);
		$valtextNew = $valtext;
		do {
			if (  $numElems != $headarr["obj_cnt"]) {
				$xopt = array("vspace"=>1);
				htmlInfoBox( "Error", 
							 "Number of Elements in Editor (".$numElems.") do not match with number of selected objects (".$headarr["obj_cnt"].")", "open", "ERROR", $xopt );
				echo "<pre>";
				echo htmlspecialchars($valtext);
				echo "</pre>";
				htmlInfoBox( "", "", "close");
				// $flagAct = "NOQUERY";
				$go = 0.5;
				
				$infox["numTextobj"] = $numElems;
				break;
			}
			
			if ( $go == 1 ) {
				// ransform TEXT-field to value-array
				
				$sqlsLoop = "SELECT x.".$primary_key.", x.".$infox["nameCol"]." FROM ".$sqlAfter;
				$sql->query($sqlsLoop);
				
				$i=0;
				while ( $sql->ReadRow() ) {
					$objid  = $sql->RowData[0]; 
					$vx[$objid] = trim($anaArr[$i]);
					$i++;
				}
			}
			
		} while (0);
	}
}



if ( sizeof($vx) ) {
    $vxNew 		  = $vx;
}

// echo "DEBUGXX: flagProduceText: $flagProduceText inType: $inType <br>";

$upd_lib = new gEditInter_C($tablename, $sqlAfter, $vx, $vxNew, $inType);

if ( $flagAct == "NOQUERY" ) {
	// just show $valtext again ...
} else {
	// produce $valArr
    $upd_lib->update($sql, $sql2, $infox, $go);
    $error->printAll();
    
    $valArrOld = &$upd_lib->valArrOld;
    $infoArx   = &$upd_lib->infoArx;
    $vxNew     = &$upd_lib->vxNew;
}

// if (sizeof($vx))  $inType = 1;


$mainScriptObj->ShowForm( $sql, $valtextNew, $vxNew, $valArrOld, $inType, $go, $infoArx,  $infox);

htmlFoot("<hr>");