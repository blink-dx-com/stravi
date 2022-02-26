<?php
/*MODULE:  obj.exp_tmpl.step_actio.php
  DESCR: - insert new protocol
	     - PROTO_ID automatic  INC
		 - jump back to exp_tmpl
  AUTHOR:  qbi
  INPUT:
  	   $id   ( EXP_TMPL_ID )
   	   $abstract_proto
	   $xaction  "add"
				 "del" -- delete a step
	   $proto_id -- for "del"
	   $proto_id_new
	   $step_no_new
	   $go : 0,1
  VERSION: 0.1 - 20020904
*/

session_start(); 


require_once ("reqnormal.inc");
require_once ('insert.inc');
require_once ("javascript.inc");
require_once ("func_formSp.inc");
require_once ("f.assocUpdate.inc");

class oExpTmplStAct {
	
    function __construct( $id, $xaction ) {
    	$this->id = $id;
    	$infoArr=array(
    		"add"=>"Add a protocol to protocol-log",
    		"del"=>"Delete a protocol-log-entry"
    		);
    	echo "Action: <font style=\"font-weight:bold; font-size:1.2em;\">".$infoArr[$xaction]."</font><BR><BR>";
    
    	if ( !isset($infoArr[$xaction]) ) {
    		htmlFoot("Error","Action '$xaction' not known.");
    	}
    }
    
    function getProtoId(&$sql, $proto_id) {
    	$sqls = "select PROTO_ID, ABSTRACT_PROTO_ID from EXP_TMPL_HAS_PROTO where EXP_TMPL_ID=".$this->id. 
    			" AND PROTO_ID=".$proto_id;
    	$sql->query($sqls);
    	$sql->ReadRow();
    	$retid = $sql->RowData[0];
    	$a_proto_id = $sql->RowData[1];
    	
    	return array($retid, $a_proto_id);
    }
    
    function delStep(&$sql, $proto_id, $go) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
    	
    	if (!$proto_id) {
    		htmlFoot("Error", "Proto_ID is missing!");
    	}
    	
    	list($retid, $a_proto_id) = $this->getProtoId($sql, $proto_id);
    	if (!$retid) {
    		htmlFoot("Error","Proto_id $proto_id does not exists.");
    	}
    	
    	if ( !$go ) {
    		$formLib = new formSpecialc();
    		$params = array( "id"=>$this->id, "proto_id"=>$proto_id, "xaction"=>"del", "go"=>1 );
    		$formLib->deleteForm( "Delete Step", "Do you want to delete Step ($proto_id) ".
    				"containing protocol <b>".
    				 obj_nice_name ( $sql, "ABSTRACT_PROTO", $a_proto_id )."</b> ?", $_SERVER['PHP_SELF'], $params );
    		htmlFoot("<hr>");
    	}
    	
    	echo "Delete step $proto_id<br>\n";
    	
    	$assoclib = new  fAssocUpdate();
    	$assoclib->setObj( $sql, 'EXP_TMPL_HAS_PROTO', $this->id );
    	if ($error->Got(READONLY))  {
    	    $error->set( $FUNCNAME, 1, 'Error on init.' );
    	    return;
    	}
    	$pkarr = array('PROTO_ID'=>$proto_id);
    	$assoclib->delOneRow($sql, $pkarr);
    	$assoclib->close($sql);
                    
    }
    
    function  add_Step(&$sql, $proto_id_new , $step_no_new, $abstract_proto) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        
        // get step with STEP_NO
        
        $id = $this->id;
        
        $sqls = "select PROTO_ID from EXP_TMPL_HAS_PROTO where EXP_TMPL_ID=".$id. " AND STEP_NO=".$step_no_new;
        $sql->query($sqls);
        $sql->ReadRow();
        $retid = $sql->RowData[0];
        if ($retid) {
            htmlInfoBox( "ERROR: Step-No exists", "", "open", "ERROR" );
            echo "Step-No <b>".$step_no_new."</b> already exists in protocol list.<br><br>".
                "What you could do:<ul>".
                "<li> choose another new number</li>".
                "<li> change the numbers in the list</li>".
                "<li> unlink the old protocol</li>";
            htmlInfoBox( "", "", "close" );
            htmlFoot("<hr>");
        }
        
        $argu=array();
        $argu['PROTO_ID']	=$proto_id_new;
        $argu['STEP_NO']	=$step_no_new;
        $argu['ABSTRACT_PROTO_ID']	= $abstract_proto;
    
        $assoclib = new  fAssocUpdate();
        $assoclib->setObj( $sql, 'EXP_TMPL_HAS_PROTO', $this->id );
        if ($error->Got(READONLY))  {
            $error->set( $FUNCNAME, 1, 'Error on init.' );
            return;
        }
        $assoclib->insert($sql, $argu);
        $assoclib->close($sql);
        
    }

}



global $error;

$error = & ErrorHandler::get();
$sql   = logon2(  ); // give the URL-link for the first db-login
if ($error->printLast()) htmlFoot();

$tablename			 = "EXP_TMPL";
$i_tableNiceName = tablename_nice2($tablename);

$id = $_REQUEST['id'];
$abstract_proto = $_REQUEST['abstract_proto'];
$xaction = $_REQUEST['xaction'];
$proto_id = $_REQUEST['proto_id'];
$go= $_REQUEST['go'];
$proto_id_new= $_REQUEST['proto_id_new'];
$step_no_new = $_REQUEST['step_no_new'];
		

$title       = "Protocol action for experiment template";

$infoarr			 = NULL;
$infoarr["title"] = $title;
$infoarr["form_type"]= "obj";
#$infoarr['help_url'] = 'o.EXAMPLE.htm';
$infoarr["obj_name"] = $tablename;
$infoarr["obj_id"]   = $id;
$infoarr["show_name"]= 1;


$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);
echo "<ul>";

$mainLib = new oExpTmplStAct($id, $xaction);

$t_rights = tableAccessCheck( $sql, $tablename );
if ( $t_rights["write"] != 1 ) {
	tableAccessMsg( $i_tableNiceName, "write" );  // $answer = getTableAccessMsg( $name, $right );
	htmlFoot();
}
$o_rights = access_check($sql, $tablename, $id);
if ( !$o_rights["write"]) htmlFoot("ERROR", "You do not have write permission on this ".$i_tableNiceName."!");

if ($xaction=='del') {
	$mainLib->delStep($sql, $proto_id, $go);
} 

if ($xaction=='add') {

	if (!$id || !$proto_id_new || !$step_no_new || !$abstract_proto) {
		htmlInfoBox( "ERROR: parameter missing.", "", "open", "ERROR" );
		echo 'Please give: ';
		if (!$id) echo 'experiment template, ';
		if (!$proto_id_new) echo 'protocol step identifier, ';
		if (!$step_no_new)  echo 'Step-No, ';
		if (!$abstract_proto) echo 'protocol (abstract), ';
		htmlInfoBox( "", "", "close" );
		
		htmlFoot("<hr>");
	}
	
	$mainLib->add_Step($sql, $proto_id_new , $step_no_new, $abstract_proto);

}

if ($error->Got(READONLY))  {
    $error->printAll();
} else  {
    $urlx = "edit.tmpl.php?t=EXP_TMPL&id=".$id;
    js__location_replace( $urlx, "object" ); 
}




htmlFoot("<hr>");