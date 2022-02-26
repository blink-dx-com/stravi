<?php
/**
 * - MixedVisu: mixed result visualization (2D, bargraph, raw image)
   - show mixed visualization graphics for a selection of experiment
   - new LAB-specific graphics can be added: example see function sh_blkpcr($sqlo, $expid)
   
   GLOBALS: $_SESSION['s_formState']["obj.exp.res_visu_arr"] - save variables
            "parx" => array()
  			"protoparm" => array("aprotoid"=>$parx["aprotoid"], "steps"=>$step)
 * @package obj.exp.res_visu_arr.php
 * @swreq UREQ:5224 o.EXP > MixedVisu > Result visualization for a list of experiments; FS-ID:FS-LIM-e6
 * @app_type_info will be overwritten by type:2021_abbott
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param   
 *       $go : 0,1
 * 	    [$actionx] 		
 *          ["show"] 
 *          "copy"
 *          "settings2" -- advanced settings
  		 $parx["nocache"]
  		 $parx["shownow"]   0|1
		 $parx["lab"] : array of lab options
		    
		 $parx["x_width"]   []    : NUMBER of pixels in X
		 $parx["showproto"] 0|1
		 
		 [$proj_id]         NUM if given, take experiments from project
		 [$selx]            array[exp_id] : if actionx="copy" => copy selx[exp_ids] to clipboard

 */
session_start(); 


require_once ('reqnormal.inc');

require_once ('func_form.inc');
require_once ("class.history.inc");
require_once 'f.rider.inc';
require_once ("glob.image.inc");
require_once ("f.head_proj.inc");
require_once ('f.clipboard.inc');
require_once ('gui/o.PROTO.stepout1.inc');
require_once ("o.EXP.proto.inc");
require_once('o.ABSTRACT_PROTO.stepx.inc');
require_once 'gui/f.plotly.inc';

class oEXPmixedVisu {

    function __construct() {
    	
        $this->mixedvisu_ext = NULL;
        $this->fields_ext    = array();
    	$this->initparams    = $_SESSION['s_formState']["obj.exp.res_visu_arr"];
    	$this->expHelpFuncs  = new oEXPprotoC();
    	$this->aProtoSublib  = new oABSTRACT_PROTO_stepx();
    	$this->protoOrgLib   =  new gProtoOrg( );
    	
    	$ext_file = $_SESSION['s_sessVars']['AppLabLibDir'].'/lablib/obj.exp.res_visu_arr.lab.inc';
    	if (file_exists($ext_file)) {
    	    require_once $ext_file;
    	    $this->mixedvisu_ext = new oEXPmixedVisu_Ext($this->initparams);
    	    $this->fields_ext = $this->mixedvisu_ext->form_params();
    	}
    	
    	
    	
    	/**
    	 *
    	 * @var oEXPmixedVisu $dia_defs
    	 */
    	$this->dia_defs=array(
    	    #'2d'=>array('title'=>'array visualization', 'img'=>'i20.2d.gif', 'notes'=>'2D array'),
    	    #'MeanBar'=>array('title'=>'Mean Bar', 'img'=>'i20.expres1.gif', 'notes'=>'Mean bar graph'),
    	    #'img'=>array('title'=>'Raw images', 'img'=>'i20.img1.gif', 'notes'=>'raw images'),
    	    #'TimeDia'=>array('title'=>'Timedia', 'img'=>'', 'notes'=>''),
    	    'blkpcr'=>array('title'=>'Raw Data 1', 'img'=>'i20.exprawpcr.png', 'notes'=>'raw data (PCR data)'),
    	    
    	);
    	
    }
    
    function set_parx($parx) {
        $this->parx = $parx;
        if($this->mixedvisu_ext!=NULL) $this->mixedvisu_ext->set_parx($parx);
    }
    
    function infoout($title, $text) {
    	echo "<font color=gray>$title:</font> <b>".$text."</b><br>\n";
    }
    
    function _config_tab( $mode ) {
        $xmodes=array(
            'main'     => array('Main config', $_SERVER['PHP_SELF'].'&show=all'),
            'blink'    => array('Result config', $_SERVER['PHP_SELF'].'&actionx=config2')
        );
        $rider_lib = new fRiderC();
        $rider_lib->riderShow($xmodes, $mode, 'Config-Mode: ');
    }
    
    function formshow( &$sql, $parx, $tmpExpid, $firstExpID, $numexp, $proj_id) {
    	
        //$this->_config_tab('main');
        //echo "<br>";
    	
    	$initarr   = NULL;
    	$initarr["action"]      = $_SERVER['PHP_SELF'];
    	$initarr["title"]       = "View options";
    	$initarr["submittitle"] = "Submit";
    	$initarr["tabwidth"]    = "AUTO";
    	$initarr["tabnowrap"]   = "1";
    	$hiddenarr = NULL;
    	$hiddenarr["parx[shownow]"] = 1;
    	$hiddenarr["proj_id"] = $proj_id;
    	
    	$protoinfo = NULL;
    	if (is_array($this->initparams["protoparm"])) {
    		$protoPnt = &$this->initparams["protoparm"];
    		if ($protoPnt["aprotoid"]) {
    			$protoinfo = "<br>\nprotocol (abstract): <b>".obj_nice_name( $sql, "ABSTRACT_PROTO", $protoPnt["aprotoid"])."</b> ";
    			$protoinfo .= "Steps: <b>".count($protoPnt["steps"])."</b>";
    		}
    		
    	} 
    	
    	$formobj = new formc($initarr, $hiddenarr, 0);
    
    	$fieldx = array ( "object" => "space"	 );
    	$formobj->fieldOut( $fieldx );
    	
    	// LAB
    	if (is_object( $this->mixedvisu_ext) ) {
    	
    	    $fields = $this->mixedvisu_ext->form_params();
    	    
    	    // echo "DDD".print_r($fields,1)."<br>";
        	
    	    foreach($fields as $field) {
        	    
    	        $tmp_name = $field['name'];
    	        $key    = $tmp_name;
    	        $fieldx = $field;
    	        
    	        if ($field['object']!='info') {
        	        $fieldx['name'] =  'parx[lab]['.$tmp_name.']';
            	    $fieldx["namex"]  = TRUE;
            	    $fieldx["val"] = $parx['lab'][$key];
    	        }
        	    $formobj->fieldOut( $fieldx );
        	}
    	}
  
    	$fieldx = array ( "object" => "space"	 );
    	$formobj->fieldOut( $fieldx );
    	
    	
    	$fieldx = array ( "title" => "Options", "object" => "info" );
    	$formobj->fieldOut( $fieldx );
    	
    	$fieldx = array ( "title" => "Show protocol params", "name"  => "showproto",
    			"object" => "checkbox",	"val"   => $parx["showproto"], "optional"=>1,
    			"notes" => "[<a href=\"obj.exp.res_visu_conf.php?expid=".$firstExpID."&backparam=".$proj_id."\">".
    			"Configure protocol parameters</a>]".$protoinfo
    			 );
    	$formobj->fieldOut( $fieldx );
    	
  
    	
    	$fieldx = array ( "title" => "X dimension", "name"  => "x_width",
    			"object" => "text",	"val"   => $parx["x_width"], "optional"=>1,
    			"fsize"=> 5,
    			"notes" => "[pixels] dimension of pictures e.g. 300" );
    	$formobj->fieldOut( $fieldx );
    	
    	if ($numexp>100) {
    		$fieldx = array ( "title" => "Show $numexp experiments", 
    			"object" => "info2", "val" => "yes",
    			"notes"  => "show them now" );
    		$formobj->fieldOut( $fieldx );
    	}
    
    	$formobj->close( TRUE );
    	
    	if ($numexp>100 AND !$parx["shownow"]) {
    		htmlFoot(); // stop now
    	}
    
    }
    
    function _setAProtoID($aProtoID) {
    	$this->aProtoSublib->setObj($aProtoID);
    }
    
    function _aproto_step_info( &$sqlo, $step_nr) { 
        return ( $this->aProtoSublib->step_info( $sqlo, $step_nr, 1) );
    }     
    
    private function _special_stepsPrep( &$sql ) {
    
    	$steps    = $this->initparams["protoparm"]["steps"];
    	$aProtoID = $this->initparams["protoparm"]["aprotoid"];
    	
    	
    	if ($aProtoID) {
    			$protoinfo = obj_nice_name( $sql, "ABSTRACT_PROTO", $aProtoID);
    			$protoinfo .= " </B>Steps:<b> ".count($steps);
    			$this->infoout("Show selected steps of protocol (abstract)", $protoinfo);
    	} else {
    		htmlInfoBox( "Protocol parameter problem", "Please select new protocol parameters", "", "WARN" );
    	}
    		
    	
    	$this->_setAProtoID($aProtoID);
        // get protocol step infos : name, unit, val
    	$compare_abstract_info=array();
    	foreach( $steps as $th0=>$tmp_step) { 
        //  (list($tmp_step)=eee($steps)) {
            $compare_abstract_info[$tmp_step] = $this->_aproto_step_info( $sql, $tmp_step);
        }  
        reset($steps);   
    	
    	$this->protoDetails =  $compare_abstract_info;          
    	
    }

    /**
     * show selected protocol steps ...
     * @param object $sql
     * @param int $expid
     */
    function _special_steps_out( &$sql, $expid ) {      
         $n_max_len = 40;
    	 $protoInfoarr = &$this->initparams["protoparm"];
    	 
    	 $aProtoID = $protoInfoarr["aprotoid"];
    	 if (!$aProtoID)  {
    	     echo "<font color=red>No protocol template defined.</font><br>";
    	     return;
    	 }
    	 $cproto = $this->expHelpFuncs->getConcProtoByAbstract ($sql, $expid, $aProtoID);
    	 if (!$cproto)  {
    	 	echo "<font color=red>Protocol not found.</font><br>";
    	 	return;
    	 }
    	 
    	 // spacer-table to show the protocol inserted
         echo "<table border=0 cellpadding=0 cellspacing=0><tr><td><img src=\"0.gif\" width=20 height=1></td><td>"; 
    	 echo "<table border=0 bgcolor=#E0E0FF cellpadding=1 cellspacing=1>";
         $tmp_info_arr = &$this->protoDetails;
         $rel_step_no=1;
         foreach( $tmp_info_arr as $stepno=>$info_arr) {
    		 
    		$tcolor = "";  
    		$subst  = "";
    		$quant  = "";               
    		$notes  = "";
    		$step_name = $info_arr["NAME"];
    		
    		if ($info_arr["ABSTRACT_SUBST_NAME"]!="") $subst = "S:".$info_arr["ABSTRACT_SUBST_NAME"];
    		if ($info_arr["QUANTITY"]!="")  $quant = "Q:".$info_arr["QUANTITY"];
    		
    		if ($cproto) { 
    			$c_arr = $this->protoOrgLib->cproto_step_info( $sql, $cproto, $stepno  );
    			
    			if ($c_arr["CONCRETE_SUBST_ID"]) {
    				$subst= "S:<B>".$c_arr["SUBST_NAME"]."</B>";
    			}
    			if ($c_arr["DEV_ID"]) {
    				$subst .= " D:<B>".$c_arr["DEV_NAME"]."</B>";
    			}
    			if ($c_arr["QUANTITY"]) {
    				$quant=  "Q:<B>".$c_arr["QUANTITY"]."</B>";
    			}
    			if ( $c_arr["NOTES"]!="" ) {   
    				$tmpnotes = $c_arr["NOTES"];        
    				$tmpmax = $n_max_len;
    				if ( $tmpmax>0 && (strlen($tmpnotes)> $tmpmax) )
    					$tmpnotes = substr($tmpnotes,0, $tmpmax). "...";
    				$notes= "<br><pre><I>".htmlspecialchars($tmpnotes)."</I></pre>";   
    			}
    		}
    		$meta_info = $subst." ".$quant.$notes;
    		if ($c_arr["NOT_DONE"]==1) {
    				$tcolor="bgcolor=#E0C0C0";
    				$meta_info="<font color=red>(inactiv)</font>"; 
    		}
    		echo "<tr $tcolor><td NOWRAP><font color=gray>$rel_step_no. $step_name</font> $meta_info</font></td></tr>\n";
    		$rel_step_no++;
         }
    	
              
         echo "</table>";
    	 echo "</td></tr></table>";
         
    }
    
    function init_analyseis($sql) {
        if ( $this->parx["showproto"] ) $this->_special_stepsPrep($sql);
    }
    
    function show_table($sql, $sql2, $sqlAfter) {
        
        $parx =  $this->parx;
      
        $sqlsLoop = "SELECT x.EXP_ID, x.NAME FROM ".$sqlAfter;
        $sql->query($sqlsLoop);
        
        $cnt=0;
        $infox=array();
        $infox["visucols"]=0;
        
        foreach($this->fields_ext as $row) {
            if ($row['object']=='checkbox') {    
                $varname = $row['name'];
                if ($parx['lab'][$varname]>0) {
                    $infox["visucols"]++;
                }
            }
        }
  
        
        echo "<form style=\"display:inline;\" method=\"post\"  name=\"editform2\"  action=\"".$_SERVER['PHP_SELF']."?actionx=copy\" >\n";
        echo "<table cellpadding=1 cellspacing=1 border=0>";
        
        while ( $sql->ReadRow() ) {
            
            $tmpid   = $sql->RowData[0];
            $tmpName = $sql->RowData[1];

 
            echo "<tr valign=top>\n";
            echo "<td colspan=". $infox["visucols"].">\n";
            $exphead = "<font color=gray>".($cnt+1).".</font>".
                " <input type=checkbox name=selx[".$tmpid."] value=1>".
                " <img src=\"images/icon.EXP.gif\"> <a href=\"edit.tmpl.php?t=EXP&id=$tmpid\">$tmpName</a> [EXP-ID:".$tmpid."]\n";
            echo $exphead;
            
            if ( $parx["showproto"] ) $this->_special_steps_out( $sql2, $tmpid );
            
            echo "</td></tr><tr valign=top>\n";
            
            $init_error_got=0;
            try {
                
                $this->mixedvisu_ext->row_init($sql2, $tmpid);

            } catch (Exception $e) {
                $init_error_got=1;
                echo "<td colspan=". $infox["visucols"].">\n";
                cMsgbox::showBox("warning", $e->getMessage());
                echo "</td>";
            }
            
            if(!$init_error_got) {
            
                foreach($this->fields_ext as $row) {
                    
                    if ($row['object']=='checkbox' and !$row['intern.option']) {
                        $varname = $row['name'];
                        if ($parx['lab'][$varname]>0) {
                            echo "<td>\n";
                            try {
                                $this->mixedvisu_ext->cell_row($sql2, $varname);
                            }  catch (Exception $e) {
                                cMsgbox::showBox("warning", $e->getMessage());
                            }
                            echo "</td>\n";
                        }
                    }
                }
            }
            
        	echo "</tr>";
        	echo "<tr><td>&nbsp;</td></tr>";
        	while (@ob_end_flush());
        	$cnt++;
        }
        echo "</table>\n";
        echo "<br>";
        echo '&nbsp;&nbsp;<input type=submit class="yButton" value="Copy selected experiments to clipboard"><br>'."\n";
        echo "</form>\n";
    
    }

}

$tablename = "EXP";
$infox = NULL;
$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] );
$sql2  = logon2(  );
if ($error->printLast()) htmlFoot();
//$varcol = & Varcols::get();

$proj_id=$_REQUEST['proj_id'];
$go=$_REQUEST['go'];
$actionx=$_REQUEST['actionx'];
$parx=$_REQUEST['parx'];
$selx=$_REQUEST['selx'];


$mainScriptObj = new oEXPmixedVisu();

$title = 'MixedVisu: mixed result visualization ( bargraph, raw image, ...)';
$infoarr = array();
$infoarr['help_url'] = "o.EXP.res_visu_mix.html";
$infoarr['title_sh'] = 'MixedVisu';
$infoarr['jsFile']   = f_plotly::get_js_link();
$pageHeadListObj = new gHtmlHeadProjC();
$obj_arr=array();
$answerArr = $pageHeadListObj->showHead( $sql, $tablename, $proj_id, $obj_arr, $title, $infoarr);

$sqlAfter  = $answerArr["sqlAfter"];

if ( !$go ) {
	$parx = $_SESSION['s_formState']["obj.exp.res_visu_arr"]["parx"];
}

if ( $parx["x_width"] > 0 ) {
	if ($parx["x_width"] > 500) $parx["x_width"] = 500;
}
$mainScriptObj->set_parx($parx);

  
gHtmlMisc::func_hist("EXP.array_vis", $title, $_SERVER['PHP_SELF'] );

$tablenice = tablename_nice2($tablename);

// 
//   check   R I G H T S
//
$t_rights = tableAccessCheck( $sql, "EXP" );
if ( $t_rights['read'] != 1 ) {
    tableAccessMsg( $tablenice, 'write' );
	htmlFoot();
}

if ( $actionx == "copy" ) {

 	echo "<ul>.. copy <b>".sizeof($selx)."</b> selected experiments to clipboard ...<br>";
	if ( !sizeof($selx) ) {
		 htmlInfoBox( "Copy failed", "No experiments selected!", "", "INFO" );
	} else {
		clipboardC::obj_put ( "EXP", $selx );
		echo ".. ready<br><br><hr>";
 	}
	echo "</ul>";
	echo "<br>\n";
}

// get first experiment
$sqlsLoop = "SELECT x.EXP_ID, x.NAME FROM ".$sqlAfter;
$sql->query($sqlsLoop);
$sql->ReadRow();
$infox["firstExpID"]  = $sql->RowData[0];
$infox["firstExpNAME"] = $sql->RowData[1];
$tmpExpid = $infox["firstExpID"];

if (!$go) {
	echo "<ul>";
	$mainScriptObj->formshow($sql, $parx, $tmpExpid, $infox["firstExpID"], $answerArr["obj_cnt"], $proj_id);
	echo "</ul>";
	htmlFoot();
} else {
	// save $parx
	$_SESSION['s_formState']["obj.exp.res_visu_arr"]["parx"] = $parx;
	$mainScriptObj->set_parx($parx);
	echo '[<a href="'.$_SERVER['PHP_SELF'].'?proj_id='.$proj_id.'">Show parameter form</a>]<br>'."\n";
}




// INFO
if ($parx["x_width"] > 0) echo "<font color=gray>Special dimension: </font> ".$parx["x_width"]." pixels<br>\n";
echo "<br>\n"; 

$mainScriptObj->init_analyseis($sql);



echo "<br>\n";
ob_end_flush ();

$mainScriptObj->show_table($sql, $sql2, $sqlAfter);


echo "<hr>\n";

htmlFoot();

