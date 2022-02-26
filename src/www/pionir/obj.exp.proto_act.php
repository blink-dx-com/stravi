<?
/**
 * experiment protocol actions 
 * @package obj.exp.proto_act.php
 * @author  steffen@blink-dx.com
 * @param 
 *   int $id (experiment id)
  	 $xaction : "new", "del"
  	 [$concrete_proto_id] for xaction "del"
	 [$abstract_proto_id] for xaction "new"
	 [$step_no] is a number or "all"
	 [$blueprint] for "new" ID of blueprint EXP
     [$newanyway] flag for generation of ALL protocols
	 $show_long_flag (define session var)
	 $go : 0,1 for "del"
 */

// extract($_REQUEST); 
session_start(); 


require_once ("reqnormal.inc"); 
require_once("g_delete.inc");  
require_once('glob.obj.proto_act.inc');
require_once 'o.EXP.proto_mod.inc';
require_once ( "javascript.inc" );

class oExpProtoActPg {

function __construct($expid) {
	$this->expid = $expid;
	// $this->exp_proto = new obj_proto_act();
}

function infoout($text) {
	echo "...".$text."<br>" ;
}

/**
 * create blue print of ALL protocols of experiment $blueprint
 * @param object &$sql
 * @param object&$sql2
 * @param int $blueprint
 * @param int $newanyway
 */
function _f_newBlue(&$sql, &$sql2, $blueprint, $newanyway) {
	global $error;
	$FUNCNAME= "_f_newBlue";

	 $id = $this->expid;
	 if (!$blueprint) {
          htmlFoot( "ERROR", "Please give a blueprint experiment."); 
     }

	$this->infoout("create all protocols from exp $blueprint");
        
	$sqls = "select e.exp_tmpl_id, et.name from exp e, exp_tmpl et where e.exp_id=".$id. " AND e.exp_tmpl_id=et.exp_tmpl_id";
	$sql->query("$sqls");
	$sql->ReadRow();
	$exp0_tmpl_id   = $sql->RowData[0];
	$exp0_tmpl_name = $sql->RowData[1];
	
	$sqls = "select e.exp_tmpl_id, et.name from exp e, exp_tmpl et where e.exp_id=".$blueprint. " AND e.exp_tmpl_id=et.exp_tmpl_id";
	$sql->query("$sqls");
	$sql->ReadRow();
	$exp1_tmpl_id   = $sql->RowData[0];
	$exp1_tmpl_name = $sql->RowData[1];
	
	if ($exp1_tmpl_id) { // if template at destination exists
		if ( ($exp1_tmpl_id!=$exp0_tmpl_id) && !isset($newanyway) ) {
			echo "<B>WARNING:</B> experiment templates are different.<br><br>\n";
			echo "template of blue print experiment: '$exp1_tmpl_name' [$exp1_tmpl_id]<br>\n";
			echo "template of destination experiment: '$exp0_tmpl_name' [$exp0_tmpl_id]<br>\n";
			echo "<br><br>\n";
			echo "Create protocols anyway? ";
			echo "[<a href=\"".$_SERVER['PHP_SELF']."?id=$id&xaction=new&step_no=all&blueprint=$blueprint&newanyway=1\"><B>YES</B></a>] &nbsp;&nbsp;&nbsp;";
			echo "[<a href=\"edit.tmpl.php?tablename=EXP&id=$id\">No</a>]<br>";
			htmlFoot( "<hr>");
		}
	}
	
	$sqls = "select concrete_proto_id, STEP_NO from exp_has_proto  
				where exp_id=".$blueprint. "  order by STEP_NO";
	$sql->query($sqls);
	
	$arr_protos=array();
	while ( $sql->ReadRow() ) {
		$blue_conc_prot = $sql->RowData[0];
		$step_no        = $sql->RowData[1];
		
		$sql2->query("select abstract_proto_id from concrete_proto where concrete_proto_id=".$blue_conc_prot);
		$sql2->ReadRow();
		$blue_abst_prot = $sql2->RowData[0];
		
		$arr_protos[]=array( 'pra'=>$blue_abst_prot, 'prc'=>$blue_conc_prot, 'st'=>$step_no  );
	}
	
	if ( !empty($arr_protos) ) { 
	    
	    $proto_lib = new oEXP_proto_mod();

		foreach( $arr_protos as $tmparr ) {
		    

		    $proto_lib->set_exp($sql, $id);
		    $proto_lib->create($sql, $sql2, $tmparr['pra'], $tmparr['st'], array(), $tmparr['prc']);
			// $this->exp_proto->exp_create_proto( $sql, $sql2, $id, $tmparr[0], $tmparr[1], $tmparr[2] );
			if ( $error->printAll() ) {
				htmlFoot( "ERROR", "Stopped due to previous error.");
			}
		}
			
	}
}


/**
 * create one new protocol 
 */
function f_new( &$sql, &$sql2, $step_no, $abstract_proto_id, $blueprint, $newanyway) {
	global $error;
	$FUNCNAME= "f_new";

	$id = $this->expid;
	$blue_conc_prot = 0;
    
    if ($step_no=="all") {
		$this->_f_newBlue($sql, $sql2, $blueprint, $newanyway);
		return;
	}

	if (!$step_no) {
		htmlFoot( "ERROR", "please give a step_no.");  
	}
	if (!$abstract_proto_id) {
		htmlFoot( "ERROR", "please give a planned protocol.");
	}

	if ( $blueprint ) {
		$sqls = "select c.concrete_proto_id, c.abstract_proto_id ".
			" from exp_has_proto e, concrete_proto c where e.exp_id=".$blueprint. " AND ".
			" e.STEP_NO=".$step_no. " AND e.concrete_proto_id=c.concrete_proto_id";
		$sql->query($sqls);
		$sql->ReadRow();
		$blue_conc_prot = $sql->RowData[0];
		$blue_abst_prot = $sql->RowData[1];
		if ( !$blue_conc_prot ) {
			$error->set( $FUNCNAME, 1, "Protocol on blue print experiment does not exist.<br>".
			   "INFO: You selected a blue print experiment to copy the concrete protocol steps.");
			return;
		}
		if ( $blue_abst_prot !=  $abstract_proto_id) {
			$abstract_proto_id=$blue_abst_prot;
			echo "WARNING: The planned protocol of blue print experimnent does not match the planned here.<br>";
		}
	}

	$creopt = array("checkStepNo"=>1);
	
	$proto_lib = new oEXP_proto_mod();
	$proto_lib->set_exp($sql, $id);
	$proto_lib->create($sql, $sql2, $abstract_proto_id, $step_no, array(), $blue_conc_prot, $creopt);
	// $retval = $this->exp_proto->exp_create_proto( $sql, $sql2, $id, $abstract_proto_id, $blue_conc_prot, $step_no, $creopt);
	if ($error->Got(READONLY))  {
		$error->printAll();
		htmlFoot( "ERROR", "Stopped due to previous error.");

	}

}

/**
 * delete one protocol
 */
function f_del(&$sql, $concrete_proto_id, $go) {
	//global $error;
	//$FUNCNAME= __CLASS__.':'.__FUNCTION__;

	$id = $this->expid;
	if (!$concrete_proto_id) {
		htmlFoot( "ERROR", "please give the ID of the performed protocol.");
	}
	
	$absprotoid   = glob_elementDataGet( $sql, "CONCRETE_PROTO", "CONCRETE_PROTO_ID", $concrete_proto_id, "ABSTRACT_PROTO_ID");
	$absProtoNice = obj_nice_name ( $sql, "ABSTRACT_PROTO", $absprotoid );

	$this->infoout( "Protocol: <b>$absProtoNice</b> [ID:$concrete_proto_id]");
	$back_url = "edit.tmpl.php?t=EXP&id=".$id;

	if ( !$go ){
		$o_rights = access_check( $sql, "CONCRETE_PROTO", $concrete_proto_id );
		echo "<br><br><B>Do you really want to delete the protocol?</B><br><br>";	  
		echo "<a href=\"".$_SERVER['PHP_SELF']."?id=".$id."&concrete_proto_id=".$concrete_proto_id."&xaction=del&go=1\" > <b>YES</B> </a> &nbsp;|&nbsp;";
		echo "<a href=\"".$back_url."\">NO</a><br>";
		
		if (!$o_rights["delete"]) {
			echo "<br><br>";
			echo "WARNING: You do not have the right to delete the protocol object.<br>";
			echo "WARNING: The object will remain, but is disconnected from the experiment.<br>";
		}
		htmlFoot("<hr>");
	}
	
	$proto_lib = new oEXP_proto_mod();
	$proto_lib->set_exp($sql, $id);
	$proto_lib->del_prot($sql, $concrete_proto_id);

}

}

// --------------------------

$show_long_flag = $_REQUEST['show_long_flag'];
$id      = $_REQUEST['id'];
$xaction = $_REQUEST['xaction'];
$concrete_proto_id= $_REQUEST['concrete_proto_id'];
$abstract_proto_id= $_REQUEST['abstract_proto_id'];
$step_no  = $_REQUEST['step_no'];
$blueprint= $_REQUEST['blueprint'];
$newanyway= $_REQUEST['newanyway'];
$go = $_REQUEST['go'];

$back_url = "edit.tmpl.php?t=EXP&id=".$id;

if ( $show_long_flag!="" ) {
	$_SESSION['userGlob']["o.EXP.tabmode_proto"]=$show_long_flag;
	js__location_replace($back_url, "back to experiment" );
	return;
	
} 	
	
global $error, $varcol;

$sql  = logon2( $_SERVER['PHP_SELF'] );
$sql2 = logon2( $_SERVER['PHP_SELF'] );

$error = & ErrorHandler::get();
if ($error->printLast()) htmlFoot();


$tablename			 = "EXP";
// $i_tableNiceName 	 = tablename_nice2($tablename);

$title       		 = "Experiment protocol actions";
$infoarr = NULL; 
$infoarr["title"] = $title;
$infoarr["scriptID"] = "";
$infoarr["form_type"]= "obj";
$infoarr["obj_name"] = $tablename;
$infoarr["obj_id"]   = $id;
$infoarr["checkid"]  = 1;

$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);


$mainlib = new oExpProtoActPg($id);


echo "<ul>";
echo " ACTION: ".$xaction;
echo "<br><br>\n";

$o_rights = access_check( $sql, "EXP", $id );

if( !$o_rights["read"] ){
	htmlFoot( "ERROR", "no read permissions!");
}

if ( !$o_rights["insert"] ) {
	htmlFoot( "ERROR", "not right to manipulate protocols of experiment.");
}

if ( $xaction=="new" ) {
	$mainlib->f_new( $sql, $sql2, $step_no, $abstract_proto_id, $blueprint, $newanyway);
}

if ( $xaction=="del" ) {
	$mainlib->f_del($sql, $concrete_proto_id, $go) ;
}

echo "<br>...ready<br>";
if ( $error->printAll() ) {
	htmlFoot();
}

js__location_replace($back_url, "back to experiment" );

htmlFoot();