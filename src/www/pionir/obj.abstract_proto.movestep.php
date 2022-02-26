<?
/**
 * insert new step, called by obj.abstract_proto.xedit.php
	step_nr ==0 at the end; 
	     	>0 after this step_nr
 * @package  obj.abstract_proto.movestep.php
 * @swreq   SREQ:0001801: o.ABSTRACT_PROTO > abstract protocol anzeigen/editieren 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param  $id   ( ABSTRACT_PROTO_ID )
   	   	   $action ( "NEW", "DEL", "UP", "DOWN", "LEFT", "RIGHT" )
	       [$actname:] name of step
		   $step_nr
 */
session_start(); 

 		
require_once ("reqnormal.inc");
require_once ('insert.inc');
require_once ("javascript.inc" );
require_once ('f.assocUpdate.inc');

class oAbsProtoStepMv {
	
function __construct( &$sqlo, $aproto_id, $step_nr, $actname ) {
	$this->proto_id = $aproto_id;
	$this->step_nr = $step_nr;
	$this->actname = $actname;
	
	$this->modLib = new  fAssocUpdate();
	$this->modLib->setObj( $sqlo, 'ABSTRACT_PROTO_STEP', $aproto_id );
	
	
}


/**
 * move BLOCK where MAIN_STEP_NR>$main_step_start FORWARD
 * @param $sql
 * @param $sql2
 * @param $main_step_start
 */
function _move_block_forward( &$sql, &$sql2, $main_step_start ) {

	$proto_id = $this->proto_id;
	$sqls = "select step_nr, MAIN_STEP_NR  from ABSTRACT_PROTO_STEP where".
		" ABSTRACT_PROTO_ID=".$proto_id. " AND MAIN_STEP_NR>".$main_step_start;
		
	$sql->query($sqls);
	while ( $sql->ReadRow() ) {
		
		$step_nr_tmp =$sql->RowData[0];
		$main_step_nr_tmp =$sql->RowData[1];
		$new_main_step=$main_step_nr_tmp + 1;
	
		$argu = array("MAIN_STEP_NR"=>$new_main_step);
		$idarr= array('STEP_NR'=>$step_nr_tmp);
		$this->modLib->update($sql2, $argu, $idarr);
	}
}

function _exchange_steps( &$sql, $st1, $st2 ) {
	
	echo "1. NR:".$st1[0]." MAIN:". $st1[1]."<br>"; 
	echo "2. NR:".$st2[0]." MAIN:". $st2[1]."<br>"; 

	$argu = array("MAIN_STEP_NR"=>$st2[1]);
	$idarr=array('STEP_NR'=>$st1[0]);
	$this->modLib->update($sql, $argu, $idarr);

	$argu = array("MAIN_STEP_NR"=>$st1[1]);
	$idarr=array('STEP_NR'=>$st2[0]);
	$this->modLib->update($sql, $argu, $idarr);
	
}

function getMaxStepnr(&$sql) {
	/*
	RETURN: $this->step_max, $this->main_step_nr
	*/
	$sqls= "select max( step_nr )  from ABSTRACT_PROTO_STEP where ABSTRACT_PROTO_ID=" . $this->proto_id;
	$sql->query("$sqls");
	$sql->ReadRow();
	$this->step_max = $sql->RowData[0];
	$this->main_step_nr=0;
	
	if ( $this->step_nr ) {	
		$sqls= "select main_step_nr, sub_step_nr from ABSTRACT_PROTO_STEP where ABSTRACT_PROTO_ID=" .
				$this->proto_id. " AND STEP_NR=".$this->step_nr;
		$sql->query($sqls);
		$sql->ReadRow();
		$this->main_step_nr= $sql->RowData[0];
		$this->sub_step_nr = $sql->RowData[1];
	}
}

function f_new(&$sql, &$sql2) {
	global $error;
	$FUNCNAME = "f_new";
	
	$step_max 		= $this->step_max;
	$step_nr 		= $this->step_nr;
	$main_step_nr 	= $this->main_step_nr;
	
	if ( $step_max<= 0) $step_max = 0;
	
	$step_new = $step_max+1;
	
	if ( $step_nr ) { /* put step between BLOCKS ?*/
	
		/*
		7: jubel
		   <== NEW (8:)
		8: next 1
		9: next 2
		*/
		$main_step_new = $main_step_nr + 1;
		
		/* now all steps must be moved FORWARD */
		
		$this->_move_block_forward( $sql, $sql2, $main_step_nr ); 
		
	} else { /* put step at the end ... */
	
		$sqls= "select max(main_step_nr) from ABSTRACT_PROTO_STEP where ABSTRACT_PROTO_ID=" . $this->proto_id;
		$sql->query($sqls);
		$sql->ReadRow();
		$main_step_nr  = $sql->RowData[0];
		$main_step_new = $main_step_nr + 1;
	} 
	
	$actname = $this->actname;
	if ( $actname == "" ) $actname="NEW_STEP";
	$argu=array();
	$argu['STEP_NR']		= $step_new;
	$argu['NAME']   		= $actname;
	$argu['MAIN_STEP_NR']	= $main_step_new;
	$argu['SUB_STEP_NR']	= $this->sub_step_nr;
	
	$retval = $this->modLib->insert($sql, $argu);
	if ( $retval<=0 ) {
		$error->set( $FUNCNAME, 1, "insert of new Protocol step failed." );
		return;
	}
}

function f_up (&$sql) {
	/*
	7: jubel
	8: next 1 ^^^^ 
	9: next 2
	*/
	/* search biggest MAIN_STEP_NR before this one */
	
	$main_step_nr 	= $this->main_step_nr;
	$step_nr		= $this->step_nr;
	$step_nr_tmp = 0;
	
	$sqls= "select step_nr, main_step_nr from ABSTRACT_PROTO_STEP where ABSTRACT_PROTO_ID=" .
			  $this->proto_id. " AND MAIN_STEP_NR<".$main_step_nr. 
			" order by MAIN_STEP_NR DESC";
	$sql->query($sqls);
	$sql->ReadRow();
	$step_nr_tmp	 =$sql->RowData[0]; /* step before */
	$main_step_nr_tmp=$sql->RowData[1]; /* step before */
	
	if ($step_nr_tmp) {  /* EXCHANGE MAIN_STEP_NR */
		
		$st1 = array($step_nr, $main_step_nr); 
		$st2 = array($step_nr_tmp, $main_step_nr_tmp); 
		$this->_exchange_steps( $sql, $st1, $st2 );
		
	} // ELSE: no action ... 
}

function f_right (&$sqlo, $step_nr) {
	$argu = array("SUB_STEP_NR"=>1);
	$idarr= array('STEP_NR'=>$step_nr);
	$this->modLib->update($sqlo, $argu, $idarr);
}

function f_left (&$sqlo, $step_nr) {
	$argu = array("SUB_STEP_NR"=>0);
	$idarr= array('STEP_NR'=>$step_nr);
	$this->modLib->update($sqlo, $argu, $idarr);
}

function f_down (&$sql) {
	/*
	7: jubel
	8: next 1 ^^^^ 
	9: next 2
	*/
	/* search smallest MAIN_STEP_NR after this one */
	
	$step_nr_tmp = 0;
	$main_step_nr 	= $this->main_step_nr;
	$step_nr		= $this->step_nr;
	
	$sqls= "select step_nr, main_step_nr from ABSTRACT_PROTO_STEP where ABSTRACT_PROTO_ID=" . 
			$this->proto_id. " AND MAIN_STEP_NR>".$main_step_nr.
			" order by MAIN_STEP_NR";
	$sql->query($sqls);
	$sql->ReadRow();
	$step_nr_tmp	 =$sql->RowData[0]; /* step before */
	$main_step_nr_tmp=$sql->RowData[1]; /* step before */
	
	if ($step_nr_tmp) {  /* EXCHANGE MAIN_STEP_NR */
		$st1 = array($step_nr, $main_step_nr); 
		$st2 = array($step_nr_tmp, $main_step_nr_tmp); 
		$this->_exchange_steps( $sql, $st1, $st2 );
		
	} /* ELSE: no action ... */

}

/**
 * delet one step
 * @param  $sqlo
 * @return array ('ok'=>, 'info'=> string)
 *  'ok'<=-10 : user error
 */
function f_del (&$sqlo) {
	global $error;
	$FUNCNAME = "f_del";
	
	$step_nr = $this->step_nr;
	$apid    = $this->proto_id;
	
	// check, if used
	echo "... check, if step-nr:".$step_nr." is used in ...<br />";
	echo "- concrete-protocol...<br>\n";
	
	$sqlsel = '1 from CONCRETE_PROTO_STEP where ABSTRACT_PROTO_ID='.$apid.' and STEP_NR='.$step_nr;
	$sqlo->Quesel($sqlsel);
	if ($sqlo->ReadRow()) {
		return array ('ok'=>-10, 'info'=> "Step-Nr ".$step_nr." already used in a protocol (concrete)!".
			" Solution: Delete these concrete-protocol steps first.");
	}
	
	echo "- acceptance-protocol...<br>\n";
	$sqlsel = '1 from ACCEPT_PROT_STEP where ABSTRACT_PROTO_ID='.$apid.' and STEP_NR='.$step_nr;
	$sqlo->Quesel($sqlsel);
	if ($sqlo->ReadRow()) {
		return array ('ok'=>-11, 'info'=> "Step-Nr ".$step_nr." already used in an ".
			tablename_nice2('ACCEPT_PROT') ."!".
			" Solution: Delete these steps first.");
	}
	
	$idarr = array('STEP_NR'=>$step_nr);
	$this->modLib->delOneRow($sqlo, $idarr);
	
	return;
}

function close(&$sqlo) {
	$this->modLib->close($sqlo);
}

}

$id   	= $_REQUEST["id"];
$action = $_REQUEST["action"];
$actname= $_REQUEST["actname"];
$step_nr= $_REQUEST["step_nr"];


$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] );
$sql2  = logon2( $_SERVER['PHP_SELF'] );

$title = "protocol (abstract) : move a step";


$pagelib = new gHtmlHead();
$optarr=array();
$pagelib->_PageHead ( $title,  $optarr );


$newurl = "edit.tmpl.php?t=ABSTRACT_PROTO&id=".$id."#plist";
echo "<ul>".$title." : ";
echo "[<a href=\"$newurl\">PRA-ID: $id</a>]";
echo "<br><br>";

$mainlib = new oAbsProtoStepMv( $sql, $id, $step_nr, $actname );

$o_rights = access_check( $sql, "ABSTRACT_PROTO", $id );
if ( !$o_rights["write"] ) {
	htmlFoot( "INFO", "no permission to write this protocol!");
}

if ( $action=="" ) {
	htmlFoot( "ERROR",  "no ACTION given");
}

echo "ACTION: $action <br>";
$mainlib->getMaxStepnr($sql);

if ( !$step_nr && ($action!="NEW") ) {
	htmlFoot( "ERROR",  "need a STEP_NR!");
}

// $mainlib->touchProt($sql);

switch ($action ) {
	case "NEW":
		$mainlib->f_new($sql, $sql2); //TBD: analyse error
	    break;
	
	case "LEFT":	
	    $mainlib->f_left ($sql, $step_nr);
		break;
	
	case "RIGHT":
	    $mainlib->f_right ($sql, $step_nr);
		break;
	
	case "UP":
		$mainlib->f_up ($sql);
	    break;
	
	case "DOWN":
		$mainlib->f_down ($sql);
	    break;
	
	case "DEL":
		//if ( !$go ) {
		//	echo " Do you really want to delete this step ?<br><br>";
		//	echo " <a href=\"".$_SERVER['PHP_SELF']."?id=$id&action=DEL&step_nr=$step_nr&go=1\"><B>[YES]</B></A>  NO<br>";
		//	return;
		//}
		$retarr = $mainlib->f_del ($sql);
		if (is_array($retarr)) {
			if ($retarr['ok']<=-10) {
				// do not show in the error log
				echo "<br />\n";
				$pagelib->htmlFoot('USERWARN', $retarr['info']);
			}
		}
		break;
	}

if ( $error->printAll() ) {
	htmlFoot('<hr>');
}

$mainlib->close($sql);


js__location_replace( $newurl, "protocol");


htmlFoot();