<?php
/**
 * [BulkUpdate] select a columns, update this column for a set of objects 
 * @package glob.objtab.edit.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   
 *     $tablename
        $go		   : 0, 1, 2, 3
        [$argu[]]   parameters array like argu[NOTES]="oieiei"
                    "EXTRA_OBJ_ID" will come with, must be filtered at update ...
        [$xargu["CLASS"]]  class ID
        [$arguobj[]]	extra atribute arguments
        $selcol    array[COLNAME] = "on" : selected columns
          'vario' : vario attributes ...
        $xobjatr   array[extra_attrib_id] : selected extra_attributes
        $vobj      VARIO select
        $vobjval   VARIO value
		$append    : DEPRICATED
		$infolevel
        TBD: remove all append functions !!!
 */

// extract($_REQUEST); 
session_start(); 


require_once ("reqnormal.inc");
require_once("object.subs.inc");
require_once("sql_query_dyn.inc");
require_once("edit.sub.inc");
require_once("db_x_obj.inc");
require_once("javascript.inc");
require_once("validate.inc");
require_once("edit.edit.inc");         
require_once('glob.objtab.edit.inc');
require_once('glob.obj.col.inc');
require_once('f.flushOutput.inc');
require_once ("f.visu_list.inc");  
require_once ("visufuncs.inc");
require_once ("func_form.inc");
require_once ('f.progressBar.inc');

class fBulkUpdateC {

    function __construct($tablename, $infolevel, &$flushLib, $numelements) {
    	global $error;
    	$FUNCNAME= "fBulkUpdateC";
    
        $this->infolevel    = $infolevel;
    	$this->tablename 	= $tablename;
    	
    	$this->primary_keys = primary_keys_get2( $tablename );
    	$this->colNames 	= columns_get_pos($tablename); 
    	$this->globColGuiObj = new globTabColGuiC();
    	$this->globColGuiObj->initTab($tablename);
    	$this->extra_obj_col_selected = 0; // $scriptLib
    	$sqlopt=array();
    	$sqlopt["order"] = 1;
    	$this->sqlAfter  = get_selection_as_sql( $tablename, $sqlopt);
    	$sqlopt = NULL;
    	$this->sqlAftNoOrd  = get_selection_as_sql( $tablename, $sqlopt);
    	$this->mainSubs  = new fBulkUpdateSub($tablename, $infolevel, $this->sqlAfter, 
    										  $this->sqlAftNoOrd, $flushLib, $numelements);
    	
    }
    
    function initUpdate() {
    	// init the SQL-update process
    	$this->mainSubs->initParams( $this->extra_obj_col_selected );
    }
    
    function set_x_action( $x_type ) {
        
        $this->x_type = $x_type;
        
        switch ($x_type) {
            case 'extra_obj_col':
                $this->extra_obj_col_selected =1;
                break;
        }
    }
    
    function addFormCols( 
    	&$selcol, 
    	$xobjatr,  
    	$infolevel,
     	$option=NULL  // "no_xobjatr" = [0] | 1
       ) {
        
        foreach( $selcol as $th0=>$th1) {
    		echo "<input type=hidden name=\"selcol[".$th0."]\" value=\"".$th1."\">\n";
    	}
    	
    	
    	if ( !$option["no_xobjatr"] ) {
    		if (!empty($xobjatr)) {
    		    foreach( $xobjatr as $th0=>$th1) {
    				echo "<input type=hidden name=\"xobjatr[".$th0."]\" value=\"".$th1."\">\n";
    			}
    			
    		}
    	} 
    	
    	echo "<input type=hidden name=\"infolevel\" value=\"$infolevel\">\n";
    }
    
    function head_out( $go ) {
      
    	$tablename = $this->tablename;
    	$goArray=array();
    	$goArray["0"] = "Select columns to update"; 
    		
    	if ( $this->extra_obj_col_selected ) {
    		$goArray["0.8"] = "Select CLASS"; // 1.2
    		$goArray["0.9"] = "Select CLASS parameters"; // 1.3
    	}
    	if ( $this->x_type=='vario' ) {
    	    $goArray["0.85"] = "Select VARIO data"; 
    	   
    	}
    	
    	$goArray[1] = "Edit new values"; 
    	$goArray[2] =  "Show/prepare values";
    	$goArray[3] = "Update NOW !!!";
    		
    	 
    	$extratext = "[<a href=\"".$_SERVER['PHP_SELF']."?tablename=".$tablename."\">Start again</a>]";
    	
    	$formPageLib = new FormPageC();
    	$formPageLib->init( $goArray, $extratext );
    	$formPageLib->goInfo( "$go" ); 
    	echo "<br>\n";
    	
    }
    
    function showelem($num_elem) {
    	echo "<font size=+1><B>$num_elem</B></font> objects selected. <br>\n";
    }
    
    private function _one_row($data) {
        
        echo "<tr bgcolor=#E0E0FF><td>";
        echo "<input type=checkbox name=selcol[". $data['col'] ."] > \n";
        echo "</td><td><b>\n";
        echo $data['icon']. $data['nice_name'];
        echo "</b></td><td>&nbsp;";
        echo "<I>". $data['colcomment'] ."</I>"; 
        echo "</td>";
        echo "</tr> \n";
    }
    
    function form0(&$sql) {
    	
    	
    	$tablename 	  = $this->tablename;
    	$primary_keys = $this->primary_keys;
    	$colNames	  = $this->colNames;
    	
    	$this->head_out( 0 );
    	echo "<font color=gray><B>WARNING:</B> This function changes many objects at the same time! ".
    		 "So take care!</font><br>\n";
        echo "<font color=gray><B>Advanced funtions:</B> ".
             "[<a href=\"glob.objtab.change_field.php?tablename=".$tablename."\">String replace</a>] ";
    		 
    	if ( sizeof($primary_keys)==1 ) {
    		echo " [<a href=\"glob.objtab.editinter.php?tablename=".$tablename."\">BulkEdit - Interactive bulk update</a>] ";
    	}
    
    	  
        echo "</font><br>\n";
    	
        $tabhasnotes=0;
        foreach( $colNames as $th0=>$th1) {
            $colName=$th1;
            if ($colName=="NOTES") $tabhasnotes=1;
        } 
    
    	
        echo "<br>\n";
        
        // $this->showelem($num_elem);
          
    	?>
    	<form method="post" name=editform action="<?echo $_SERVER['PHP_SELF']?>?tablename=<?echo $tablename?>&go=0.8" >   	
    	<table CELLSPACING=1 CELLPADDING=2 bgcolor=#c0d8e8 >
    	<tr bgcolor=#C0C0C0><td>Select</td><td>Column name</td><td>Comment</td></tr>
    	<?
           
        $cnt=0;
        foreach( $colNames as $th0=>$th1) {
    	
    		$colName   = $th1;
    		$colInfos  = "";
    	
    		$colInfos = $this->globColGuiObj->analyzeColumn( $sql, $colName ); 
    	
    		if ($colName=="EXTRA_OBJ_ID") $colInfos["showcol"]=1;
    		
    		if ( $colInfos["showcol"]>0 ) { 
    	
    			$nice_name  = $colInfos["nice"];
    			$colcomment = "";
    			$colcomment = column_remark2($tablename, $th1);
    			if ( $colInfos["mother"]!="" ) $icon = "<img src=\"". htmlObjIcon($colInfos["mother"], 1)."\"> ";
    			else $icon = "<img src=\"images/icon._empty.gif\"> ";
    			
    			if ($colName=="EXTRA_OBJ_ID") {
    				$nice_name="CLASS";
    				$colcomment="extra class";
    			} 
    			
    			$row_data = array( 'col'=>$colName);
    			$row_data['icon'] = $icon;
    			$row_data['nice_name'] = $nice_name;
    			$row_data['colcomment'] = $colcomment;
    			$this->_one_row( $row_data );
    			
    		}
    
    	} 
    	
    	if ( sizeof($primary_keys)==1 ) {
    	    $row_data = array( 'col'=>'vario');
    	    $row_data['icon'] = "<img src=\"images/icon._empty.gif\"> ";
    	    $row_data['nice_name']  = 'Vario data';
    	    $row_data['colcomment'] = 'Vario data';
    	    $this->_one_row( $row_data );
    	}
         
        echo '</tr></table>' . "\n"; 
        echo '<input type="submit" value="Select columns" class="yButton">' . "\n";
    
    	echo " &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<font color=gray>Infolevel</font> ";
    	echo "<select name=infolevel> \n";
    	echo "<option value=0 selected> normal";
    	echo "<option value=1> advanced";
    	echo "</select> <br>\n"; 
    
    	echo "<br>\n";
    	echo "</form>\n\n";  
    	
    	echo "<font color=gray>";
    	echo "<hr><br>\n";
    	echo "<B>Related tools:</B> \n";
    	echo "[<a href=\"glob.objtab.import.php?tablename=".$tablename."\">import objects from Excel</a>] ";   
    	if ( sizeof($primary_keys)==1 ) {
    		echo "[<a href=\"glob.obj.assocpaste.php?tablename=".$tablename."&parx[mode]=list\">insert associated elements from clipboard</a>] ";
    	}
    	echo "<br>\n";
    	
    	echo "<B>INFO:</B> Use <B>Bulk Feature Insert</B> to insert 'feature list' elements.<br>";
    	echo "<UL>"; 
    	echo " Possible for tables:<br>\n";
    	
    	
    	$assocnewarr = array(
    		"CONCRETE_PROTO"=>"CONCRETE_PROTO_STEP",
    		"EXP"=>"EXP_HAS_PROTO"
    		);
    		
    	foreach( $assocnewarr as $key=>$desttab) { 
    		echo "<li> ".tablename_nice2($key)." &gt;&gt; ".tablename_nice2($desttab)."</li>";
    	}
    	echo "<br>";
    	
    	if ( $assocnewarr[$tablename]!="" ) {
    		$desttab = $assocnewarr[$tablename];
    		$nicetab = tablename_nice2($desttab);
    		echo "&gt;&gt; <a href=\"glob.objtab.assocnew.php?tablename=".$desttab."\">".
    			"Insert '".$nicetab."' elements</a> for selected objects<br>\n";
    	
    	}
          
    
    }
    
    function go08_Class( &$sql, $selcol  ) {
    	// 1.2 Select CLASS
    	global $_SERVER;
    	
    	$tablename = $this->tablename;
    	
    	$this->head_out( 0.8 );
    	$tmpclassid = $this->mainSubs->getClassFromSel( $sql, $this->sqlAfter );
    	
    	echo "<br>\n";
    	echo '<form method="post" name="editform" action="',$_SERVER['PHP_SELF'],'?go=0.9&tablename=',$tablename,'">';
    		
    	$XFormLib = new fEditXobjForm( $tablename );
    	$XFormLib->h_select_out( $sql, $tmpclassid );
    	echo '<br><br><input type="submit" value="Next &gt;&gt;" class="yButton">';
    	
    	$this->addFormCols( $selcol, NULL, $this->infolevel );		
    	echo '</form>';
    }
    
    function go09_ClsParam( &$sql, $selcol, &$xargu) {
    	$tablename = $this->tablename;
    	
        $this->head_out( 0.9 );
        $tmpOpt=NULL;
    	$tmpOpt["no_xobjatr"] = 1;  // set $xobjatr in this form !!!
        echo "<br>";
        echo '<form method="post" name="editform" action="',$_SERVER['PHP_SELF'],'?go=1&tablename=',$tablename,'">';
        $this->mainSubs->classSelParam( $sql, $xargu["CLASS"] );
        echo '<br><input type="submit" value="Select parameters to update" class="yButton">';
        $this->addFormCols( $selcol, NULL,  $this->infolevel, $tmpOpt );
        echo "\n</form>\n";
    	
    }
    
    function go09_showBackLinks( &$sql, $retarr ) {
    	
    	
    	echo "<br>";
    	$tabobj = new visufuncs();
    	$dataArr= NULL;
    	$dataArr[] = array( 'Analysed elements:','<B>'. $retarr["cnt"].	'</B>');
    	$dataArr[] = array( 'Updated elements:','<B>'. ($retarr["cnt"]-$retarr["deny_cnt"]).		'</B>');
    	if ( $retarr["deny_cnt"] ) 
    		$dataArr[] = array( "<font color=red>Denied for manipulation:</font>",'<B>'. $retarr["deny_cnt"].		'</B>');
    	if ( $retarr["mothercnt"]>1 ) {
    		$dataArr[] = array( 'Mother objects:','<B>'. $retarr["mothercnt"] .'</B>');
    	} 
    	$headOpt = array( "title" => "Update-Summary", "headNoShow" =>1);
    	$headx   = array ("Key", "Val");
    	$tabobj->table_out2($headx, $dataArr,  $headOpt);
    	echo "<br>";
    
    	
    	echo "<img src=\"images/ic.navi.png\"> <b>You can go now to:</b><ul style=\"padding-left:25px; margin-left:9px;\" TYPE=SQUARE><br>\n";
    	if ( $retarr["backInfo"]!="" ) {
    	
    		$mothertable = $retarr["backInfo"]["table"];
    		$mother_nice =  tablename_nice2($mothertable);
    		$prim_id = $retarr["backInfo"]["id"];
    		$motherName = obj_nice_name    ( $sql, $mothertable, $prim_id );
    		if ($motherName=="")  $motherName="[".$prim_id."]";
    		$mothIcon = htmlObjIcon( $mothertable, 1);
    		echo "<li> <a href=\"edit.tmpl.php?t=".$mothertable."&id=".$prim_id."\">".
    			"<img src=\"".$mothIcon."\" border=0> mother object <B>$mother_nice</B>: ".
    			$motherName. "</a></li>\n";
    	} 
    	$mainIcon = htmlObjIcon( $this->tablename, 1);
    	echo "<li> <a href=\"view.tmpl.php?t=".$this->tablename."\"><img src=\"".$mainIcon."\" border=0> list view &gt;&gt;</a></li>\n";
    
    	if ( $this->tablename=="CONCRETE_PROTO_STEP" AND is_array( $_SESSION['s_formState']["proto.m_comp2e"]) ) {
    		// go back to protocol compare
    		echo "<li> <b><a href=\"obj.concrete_proto.m_comp2e.php?fromCache=1\"> protocol compare</a></b></li>";
    	}
    	
    	echo "</ul>\n";
    	echo "<br>\n";
    }
    
    function go_Vario( $sql, $selcol )  {
        
        $tablename = $this->tablename;
        
        $this->head_out( 0.85 );
        $tmpOpt=NULL;
        // $tmpOpt["no_xobjatr"] = 1;  // set $xobjatr in this form !!!
        echo "<br>";
        echo '<form method="post" name="editform" action="',$_SERVER['PHP_SELF'],'?go=2&tablename=',$tablename,'">';
        $this->mainSubs->vario_params( $sql );
        echo '<br><input type="submit" value="Select parameters to update" class="yButton">';
        $this->addFormCols( $selcol, NULL,  $this->infolevel, $tmpOpt );
        echo "\n</form>\n";
    }

}

// -----------------------------------------------------------------------------------------

$varcol= & Varcols::get();
$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] );
$sql2  = logon2( $_SERVER['PHP_SELF'] );

$tablename =$_REQUEST['tablename'];
$go	   =$_REQUEST['go'];
$argu  =$_REQUEST['argu'];
$xargu =$_REQUEST['xargu'];
$arguobj=$_REQUEST['arguobj'];
$selcol=$_REQUEST['selcol'];
$xobjatr=$_REQUEST['xobjatr'];
$vobj   =$_REQUEST['vobj'];
$vobjval=$_REQUEST['vobjval'];
$append=$_REQUEST['append'];
$infolevel=$_REQUEST['infolevel'];

if ( $go == 3 ) {
	$flushLib  = new fProgressBar( );
} else $flushLib = NULL;

$title    = 'Bulk update: Update values for a set of objects';
$title_sh = 'Bulk update';

$infoarr=array();
$infoarr["title"]    = $title;
$infoarr["title_sh"] = $title_sh;
$infoarr["form_type"]= "list";
$infoarr["obj_name"] = $tablename;
$infoarr["obj_cnt"]  = 1;
$infoarr["help_url"] = "g.Bulk_update.html";

$tmpcss  = fObjFormSub::datatab_css();
$pagelib = new gHtmlHead();
$opt=NULL;
$pagelib->_PageHeadStart ( $infoarr["title"], $opt);
echo "<style type=\"text/css\">\n";
echo $tmpcss;
if ($flushLib!=NULL) echo $flushLib->getCss();
echo "</style>\n";

js_formAll();
echo '<script language="JavaScript">'."\n";
echo '<!-- '."\n";
if ($flushLib!=NULL) echo $flushLib->getJS();
echo '//-->'."\n";
echo '</script>'."\n";
	
$pagelib->_PageHeadEnd($opt);
$headarr = $pagelib->_startBody($sql, $infoarr);

// ----------------

$listVisuObj = new visu_listC();
$tablename_nice =  tablename_nice2($tablename);

// check TABLE selection
$copt = array ("elemNum" => $headarr["obj_cnt"] ); // prevent double SQL counting
list ($stopFlag, $stopReason)= $listVisuObj->checkSelection( $sql, $tablename, $copt );
if ( $stopFlag<0 ) {
    htmlFoot("Attention", $stopReason." Please select elements of '".$tablename_nice."'!");
}

$numelements = $headarr['obj_cnt'];
$scriptLib   = new fBulkUpdateC($tablename, $infolevel, $flushLib, $numelements);
$sqlAfter    = $scriptLib->sqlAfter;
$editFormLib = new fFormEditC();

echo '<ul>';   
$t_rights = tableAccessCheck( $sql, $tablename );
if ( $t_rights['write'] != 1 ) {
	tableAccessMsg( $tablename_nice, 'write' );
	return;
}

$primary_keys = $scriptLib->primary_keys;
$colNames 	  = $scriptLib->colNames;
$colNames_ori = array();

$extra_obj_col_exists   = 0;


$mode_info = "<font color=\"#003399\"><B>INFO:</b></font> &nbsp;&nbsp;".
"<font size=+1><b>".$headarr["obj_cnt"]."</b></font> elements will be updated. ".
"The new parameters will overwrite the old parameters (also EMPTY values!)</font><br>&nbsp;<br>\n\n";



if ( !$go ) { 
	$scriptLib->form0($sql);
	htmlFoot("</ul>");  
}  

if ( $go >= 0.5 ) {

	
	if (empty($selcol)) {
		echo "ERROR: no column selected. Please start again.<br>";
		return;
	}
	
	if ( $selcol["EXTRA_OBJ_ID"]  ) {
	    $scriptLib->set_x_action('extra_obj_col');
	    $extra_obj_col_exists   =1;
	}
	
	
	if ( $selcol["vario"]  ) {
	    $scriptLib->set_x_action('vario');
	    unset($selcol["vario"]);
	}
	
}

if ( $go==0.8 ) {   // other sub types ?
    
    do {
        if ($scriptLib->x_type=='vario') {
            $go = 0.85;
            break;
        }
        
        if ( $scriptLib->extra_obj_col_selected ) {
           $go = 0.8;
    	   break;
        }
        $go = 1.0;
      
    } while (0);
    
}

if ( $go == 0.8 ) {
	$scriptLib->go08_Class( $sql, $selcol );
}  

if ( $go == 0.85 ) {
    $scriptLib->go_Vario( $sql, $selcol );
} 

if ( ($go == 0.9) AND ($xargu["CLASS"]=="") ) $go = 1; // no class parameter select ...
if ( $go == 0.9 ) {  
    $tmpattribs = $varcol->get_attrib_nice_names($xargu["CLASS"]);
    if ( empty($tmpattribs) )  $go = 1; // no attributes, continue
}

if ($go == 0.9) {   
	$scriptLib->go09_ClsParam( $sql, $selcol, $xargu );
}

if ($go >= 1) {
	if ( $extra_obj_col_exists ) {
		if ( $xargu["CLASS"] ) {
			$extra_class_id=$xargu["CLASS"];
        }
		$extraobj_o = array( "extra_obj_id"=>NULL, "extra_class_id"=>$extra_class_id,
		"arguobj"=>$arguobj, "subsel"=> $xobjatr ); // need "subsel" to define update of a SUBSET of parameters !!! 
	}
}

if ( $go == 1 ) {
    
   // give the VALUE parameters ...

  $scriptLib->head_out( 1);
  // $scriptLib->showelem($num_elem);
  echo '<form method="post" name="editform" action="',$_SERVER['PHP_SELF'],'?go=2&tablename=',$tablename,'">';
  echo '<table cellspacing="1" cellpadding="2" bgcolor="#c0d8e8" valign="top">';
       
  $cnt=0;
  if ( empty($selcol)) {
	info_out('ERROR', 'Please select columns for update!');
	return 0;
  }
  foreach( $selcol as $th0=>$th1) { 
	$colNames_ori[]=$th0;
  }
  
  $H_EXP_RAW_DESC_ID=0;
	
  $id=0;
  $editFormLib->setObject($tablename, $id);
  $edformOpt = array( 
	  "H_EXP_RAW"=>$H_EXP_RAW_DESC_ID,
	  "but.submit.txt"=>"Next &gt;&gt;" );
  
  $editAllow = 1;
  $action    = 'insert';
  $editFormLib->form_editx( $action, $sql, $colNames_ori, $primary_keys, $argu,
         $extraobj_o,  $editAllow, $edformOpt ); 
	
  $scriptLib->addFormCols($selcol, $xobjatr,  $infolevel);
  
  if ($scriptLib->extra_obj_col_selected) {
	echo '<input type="hidden" name="argu[EXTRA_OBJ_ID]" value="0">'; /* not provided by edit-form */
  }
  reset ($selcol);
  echo '</form>';
} 

if ( $go == 2 ) {
    
    // SHOW final data ...
    
  $scriptLib->head_out( 2);
  
  echo $mode_info;
    	
  //$colTypeTmp = '';
  $arguNew    = NULL;
  
  foreach( $selcol as $colname=>$dummy) {
		$colNames_ori[] = $colname;
  }
 
  
  foreach( $selcol as $colname=>$dummy) {
	$arguNew[$colname]    = $argu[$colname];
  }
    
 
  $extraobj_o["extra_obj_id"] = 1;	// set to one for the  'edit.show.inc' function
  
  // $extra_obj_col_exists
  require_once ("edit.show.inc"); 
  $showFormLib = new fFormShowC();
  $showFormLib->setObject($tablename, $id);
  $formopt = NULL;
  $formopt["colShowAdvanced"] = 1; 
  $formopt["H_EXP_RAW"]    = $H_EXP_RAW_DESC_ID;
  $formopt["do_not_close"] = 1;
  
  $colNames_depricated = NULL;
  $showFormLib->form_show( $sql, $sql2, $arguNew, $colNames_depricated, $extraobj_o, $formopt );
  
  if ($scriptLib->x_type=='vario') {
      
      
      $variolib = new oS_VARIO_sub($tablename);
      
      foreach( $_REQUEST['vobj'] as $vario_key => $flag ) {
          
          $v_features = $variolib->getColFetaures( $sql, $vario_key );
          
          $t_val = $_REQUEST['vobjval'][$vario_key];
          $vario_row = array(
             'iconimg'=>'',
             'colnice'=>'VARIO:'.$v_features['NICE'],
             'val'=>    $t_val,
             'notes'=>''
              );
          

          $showFormLib->row_out($vario_row);
      }
      
  }
  
  $showFormLib->formSubLib->datatable_close();
  
  // echo "</TD></TR></TABLE><br>";
  echo "<br>";
	
  echo '<form method="post" name="editform" action="',$_SERVER['PHP_SELF'],'?tablename=',$tablename,'&go=3">';
  echo "\n";
     
  //$cnt=0;
  foreach( $selcol as $colName=>$dummy) {
	//$value1 = htmlspecialchars ($argu[$colName]);
	echo '<input type="hidden" name="argu[',$colName,']" value="',rawurlencode ($argu[$colName]),'">';
    echo "\n";
  }
  
	
  if (!empty($arguobj)) {
    foreach( $arguobj as $th0=>$th1) { 
	  echo '<input type="hidden" name="arguobj[',$th0,']" value="',rawurlencode ($th1),'">';
      echo "\n";
	}
  }
  echo '<input type="hidden" name="xargu[CLASS]" value="',$extra_class_id,'">';
  echo "\n";
  
  if ($scriptLib->x_type=='vario') {

      echo '<input type="hidden" name="selcol[vario]" value="1">'."\n";
      foreach( $_REQUEST['vobj'] as $vario_key => $flag ) {
          $v_features = $variolib->getColFetaures( $sql, $vario_key );
          $t_val = $_REQUEST['vobjval'][$vario_key];
          echo '<input type="hidden" name="vobjval['.$vario_key.']" value="'.rawurlencode($t_val).'">'."\n";
      }
      
  }
  
  
		
  $scriptLib->addFormCols($selcol, $xobjatr, $infolevel);
  echo '<input type="submit" value="Update objects now!" class="yButton"><P></form>';
} 

if ( $go == 3 ) {
    
  // SAVE data in DB

  $scriptLib->head_out(3);
  // $scriptLib->showelem($num_elem);  
  if ($infolevel > 0) echo "Infolevel: $infolevel<br>\n";
  
  // DECODE variables
  if (!empty($arguobj)) {
      $arguobj_copy = $arguobj;
      foreach( $arguobj_copy as $th0=>$th1) { 
		$arguobj[$th0] =  rawurldecode($th1);
	  }
  }
  if (!empty($argu)) {  // TBD: need the CHECKBOX-argu, but won't be passed ???
      $argu_copy = $argu;
      foreach( $argu_copy as $th0=>$th1) { 
		$argu[$th0] =  rawurldecode($th1);
	  }
  }
  
  
  $scriptLib->initUpdate();
  $scriptLib->mainSubs->appLogAction($argu, $arguobj);
  
  if ( !empty($primary_keys)>1 ) {
	$retarr = $scriptLib->mainSubs->assoc_update( $sql, $sql2, $argu );
  } else {  
	$retarr = $scriptLib->mainSubs->singlePK_update(
	    $sql, $sql2, $argu, $arguobj, $extra_class_id, $_REQUEST['vobjval'] ); 
  }

  
  if ( $error->Got(READONLY) ) {
	echo "<br>";
	$error->printAll();
	echo "<br>";
  }

  $scriptLib->go09_showBackLinks( $sql, $retarr );
  
}  // END: if ($go==3)

htmlFoot('</ul><hr>');
