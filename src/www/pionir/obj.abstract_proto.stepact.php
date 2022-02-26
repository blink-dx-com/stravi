<?php
/**
 * actions for a selection of steps of one protocol
 * @package obj.abstract_proto.stepact.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param  
 *       $id (protocol id)
  	     $step[$i]  STEP_NR
		 $act = 
		    "copy"  "Copy steps to clipboard"
			"del"    selected steps
			"delall" delete all steps
			"paste"  paste steps
			"xargs"  extra args
	     $go : 0,1
	     $step_behind : int (for "paste")
	     $xargs
	       'inp.DATAURL' : 0,1  --on $act='xargs'
 */

session_start(); 


require_once ("reqnormal.inc"); // includes all normal *.inc files
require_once ( "javascript.inc" );
require_once ('func_form.inc');
require_once ('o.ABSTRACT_PROTO.stepMod.inc');
require_once ('o.ABSTRACT_PROTO.stepx.inc');
require_once ("subs/obj.abstract_proto.CLIPBOARD.inc");
require_once 'o.S_VARIO.mod.inc';

class oAbsProtoStepsAct {

    function __construct(&$sql, $act, &$step, $id) {
    	$this->id = $id;
    	$this->act = $act;
    	$this->id = $id;
    	$this->steps = &$step;
    	
    	$this->modLib = new oAbsProtoStepMod();
    	$this->modLib->init($sql, $this->id);
    }
    
    function _checkAcc(&$sql) {
    	global $error;
    	$FUNCNAME = "_checkAcc";
    	
    	$tablename = "ABSTRACT_PROTO";
    	$i_tableNiceName = tablename_nice2($tablename);
    	
    	$t_rights = tableAccessCheck( $sql, $tablename );
    	if ( $t_rights["write"] != 1 ) {
    		$error->set( $FUNCNAME, 1, "No 'write' access to table '".$i_tableNiceName."'"  );
    		return;
    	}
    	
    	$o_rights = access_check($sql, $tablename, $this->id);
    	if ( !$o_rights["write"]) {
    		$error->set( $FUNCNAME, 1, "You do not have write permission on this object ".$this->id ) ;
    		return;
    	}
    
    }
    
    function f_copy() {
    	
    	
    	if ( empty($this->steps) ) {
    		return;
    	}
    	
    	$_SESSION['s_clipboard'] = array();
    	foreach( $this->steps as $step_nr=>$dummy) {
    		echo "step_nr:".$step_nr."<br>";
    		$_SESSION['s_clipboard'][]= array ( 'tab'=>"ABSTRACT_PROTO_STEP", 'ida'=>$this->id, 'idb'=>$step_nr, 'idc'=>"" );
    	}
    	reset ($this->steps);
    }
    
    function f_del(&$sql) {
    	// delete steps
    	global $error;
    	$FUNCNAME = "f_del";
    	
    	$errorWas = 0;
    	
    	if ( !sizeof($this->steps) ) {
    		echo "... no steps selected.<br>\n";
    		return;
    	}
    	
    	$this->_checkAcc($sql);
    	if ($error->Got(READONLY))  {
        	//$error->set( $FUNCNAME, 1, "Error occurred." );
    		return;
    	}
    	
    	$this->modLib->set_log_level(fAssocUpdate::MASS_LOG_ALL_POS);
    	
    	foreach( $this->steps as $step_nr=>$dummy) {
    		echo "del step_nr:".$step_nr.": ";
    		$this->modLib->f_del($sql, $step_nr);
    		if ($error->Got(READONLY))  {
    			$errLast   = $error->getLast();
         		echo "Del-ERROR: ". $errLast->text;
    			$error->reset();
    			$errorWas = 1;
    		}
    		echo "<br>";
    	}
    	$this->modLib->close($sql);
    	
    	
    	if ($errorWas) {
    		$error->set( $FUNCNAME, 1, "Error(s) during delete occurred." );
    	}
    }
    
    function f_delall($sqlo) {
        // delete steps
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
       
        $this->_checkAcc($sqlo);
        if ($error->Got(READONLY))  {
            //$error->set( $FUNCNAME, 1, "Error occurred." );
            return;
        }
        
        // get all steps
        $pra_lib = new oABSTRACT_PROTO_stepx($this->id);
        $pra_steps= $pra_lib->get_steps($sqlo );
        
        $this->modLib->set_log_level(fAssocUpdate::MASS_LOG_ALL_POS);
        foreach($pra_steps as $step_nr) {
            $this->modLib->f_del($sqlo, $step_nr);  
            if ($error->Got(READONLY))  {
                $error->set( $FUNCNAME, 10, 'Error on Ste-delete (NR:'.$step_nr.')' );
                return;
            }
        }
        $this->modLib->close($sqlo);
 
    }
    
    function f_paste_form() {

        $initarr   = NULL;
        $initarr["action"]      = $_SERVER['PHP_SELF'];
        $initarr["title"]       = "Where to put the new steps";
        $initarr["submittitle"] = "Paste!";
        $initarr["tabwidth"]    = "AUTO";
        
        $hiddenarr = NULL;
        $hiddenarr["act"]    = 'paste';
        $hiddenarr["id"]     = $this->id;
        
        $formobj = new formc($initarr, $hiddenarr, 0);
        
        $fieldx = array (
            "title" => "Put new steps behind STEP-NR",
            "name"  => "step_behind",
            'namex' => TRUE,
            "object"=> "text",
            "val"   => -1,
            "notes" => "-1: to START, -2: at the END"
        );
        $formobj->fieldOut( $fieldx );
        
        $formobj->close( TRUE );
    }
    
    function f_delall_form() {
        
        $initarr   = NULL;
        $initarr["action"]      = $_SERVER['PHP_SELF'];
        $initarr["title"]       = "Do you really want to delete ALL steps of the protocol?";
        $initarr["submittitle"] = "Delete!";
        $initarr["tabwidth"]    = "AUTO";
        
        $hiddenarr = NULL;
        $hiddenarr["act"]    = 'delall';
        $hiddenarr["id"]     = $this->id;
        
        $formobj = new formc($initarr, $hiddenarr, 0);

        $formobj->close( TRUE );
    }
    
    function f_paste(&$sql, &$sql2, $step_behind) {
    	
    	 $pasteObj = new oAbsProtoStepPaste( $this->id );
    	 $pasteObj->doPaste( $sql, $sql2, $step_behind );
    }
    
    // save xargs
    function f_xargs($sqlo, $xargs) {
        
        // the key inp.DATAURL.set just identifies, that a value for inp.DATAURL is set
        if (array_key_exists('inp.DATAURL.set', $xargs) ) {
            $vario_mod_lib = new oS_VARIO_mod();
            $vario_mod_lib->setObject( $sqlo, 'ABSTRACT_PROTO', $this->id);
            $vario_mod_lib->updateKeyVal($sqlo, 'inp.DATAURL', $xargs['inp.DATAURL']);
            
            echo "OK: Param inp.DATAURL saved.<br>";
        }
    }

}

// --------------------------------------------------- 
global $error, $varcol;

$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] ); // URL-link for the first db-login; e.g. for object ($_SERVER['PHP_SELF']."?id=".$id)
$sql2  = logon2( );
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();

$id=$_REQUEST['id'];
$step=$_REQUEST['step'];
$act=$_REQUEST['act'];
$go=$_REQUEST['go'];

$tablename			 = "ABSTRACT_PROTO";
//$i_tableNiceName 	 = tablename_nice2($tablename);

$title       		 = "Protocol (abstract) step actions";
$infoarr			 = NULL;

$infoarr["title"] = $title;
$infoarr["form_type"]= "obj";
#$infoarr['help_url'] = "o.EXAMPLE.htm";
$infoarr["obj_name"] = $tablename;
$infoarr["obj_id"]   = $id;
$infoarr["show_name"]= 1;
$infoarr['checkid']  = 1;
$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);
echo "<ul>\n";

$mainLib = new oAbsProtoStepsAct($sql, $act, $step, $id);

echo "Action: <b>".$act."</b><br><br>\n";

switch ($act) {
	case "copy":
		$mainLib->f_copy($sql);
		break;
	case "del":
		$mainLib->f_del($sql);
		break;
		
	case "delall":
	    if (!$go) {
	        $mainLib->f_delall_form();
	        htmlFoot();
	    }
	    $mainLib->f_delall($sql);
	    break;
	case "paste":
	    if (!$go) {
	        $mainLib->f_paste_form();
	        htmlFoot();
	    }
	    $mainLib->f_paste($sql, $sql2, $_REQUEST['step_behind'] );
		break;
	case "xargs":
	    $mainLib->f_xargs($sql, $_REQUEST['xargs'] );
	    break;
	default:
		htmlFoot("Error", "Action '$act' unknown.");
}

if ($error->Got(READONLY))  {
     $error->printAll();
	 htmlFoot();
}


$newurl = 'edit.tmpl.php?t=ABSTRACT_PROTO&id='.$id;
if ( $_SESSION['userGlob']["g.debugLevel"]>0 ) {
	echo "... automatic page forward stopped due to debug-level.<br>";
	echo "[<a href=\"".$newurl."\">next page &gt;&gt;</a>]<br>";
	htmlFoot();
}



js__location_replace($newurl,"forward");

htmlFoot("<hr />");

