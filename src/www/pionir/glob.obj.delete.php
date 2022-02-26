<?php
/**
 * delete one object
 * @package glob.obj.delete.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq SREQ:0001708: g > GUI: delete one object 
 * @param
    $tablename
	$id[0] first PK
	$id[1]
	$id[2]
	[$forceflag] 0: wait
				 1: do it
				 2: go back
	[optdeep]    0|1
 */
session_start(); 


require_once ('reqnormal.inc');
require_once ('g_delete.inc');
require_once ('o.PROJ.subs.inc');
require_once ("o.PROJ.addelems.inc");

class gObjDeleteGui {
	
function __construct($tablename, $id, $forceflag, $optdeep) {
	$this->tablename=$tablename;
	$this->id=$id;
	$this->forceflag=$forceflag;
	$this->optdeep=$optdeep;
	$this->isBusinessObj = cct_access_has2($tablename) ;
	$this->projAskUnlink=0;
}

function init(&$sqlo) {
	$tablename=$this->tablename;
	$ids = $this->id;
	
	$pk_arr         = primary_keys_get2($tablename); // get primary keys
	$tmp_txt_arr    = array();
	if (sizeof($ids)) {	
		foreach( $ids as $pos=>$pk_id) {  
		  $tmp_txt_arr[] = columnname_nice2($tablename, $pk_arr[$pos]).'='.$pk_id;
		}
		reset ($ids);
	}
	
	if (sizeof($pk_arr)==1) {
		$objnice = obj_nice_name($sqlo, $tablename, $ids[0]);
		if ($objnice=="")  $objnice = "ID:".$ids[0];
		$objnice  = " object <B>'".$objnice."'</B>";
	} else $objnice  = " element with <B>[". implode(", ", $tmp_txt_arr)."]</B>";
	
	$inf=NULL;
	$inf[0] = "Delete ".$objnice;
	
	return $inf;
}

/**
 * check permissions
 * @return -
 * @param object $sqlo
 * @global $this->foundProjid
 * @global $this->projAskUnlink
 */
function check(&$sqlo) {
	global $error;
	$FUNCNAME= 'check';
	
	$tablename=$this->tablename;
	$id = $this->id;
	$this->projAskUnlink= 0;
	$this->foundProjid  = 0;
	
	$t_rights = tableAccessCheck($sqlo, $tablename);
	if ( $t_rights['delete'] != 1 ) {
	  $error->set( $FUNCNAME, 1, getRawTableAccMsg( $tablename, 'delete' ) );
	  return;
	}
	
	$o_rights = access_check($sqlo, $tablename, $id[0]);
	if (!$o_rights['delete']) {
		$error->set( $FUNCNAME, 2, 'No delete permission on this object!' );
		return;
	}
	
	if( $this->isBusinessObj ) {
		$objid = $id[0];
		$projSubLib = new cProjSubs();
		$projCnt = $projSubLib->cntProjectsByObj($sqlo, $tablename, $objid );
		if ($projCnt>1) {
			$error->set( $FUNCNAME, 3, 'Delete not allowed. Object is linked to more than one project! Remove the links first.' );
			return;
		}
		if ($projCnt==1) {
			$this->projAskUnlink=1;
			$this->foundProjid = $projSubLib->getProjByObject($sqlo, $tablename, $objid );
			
			$o_rights = access_check($sqlo, 'PROJ', $this->foundProjid );
			if (!$o_rights['insert']) {
				$error->set( $FUNCNAME, 4, 'No permission to remove this object from project ID:'.$this->foundProjid.'!' );
				return;
			}
		}
	}
}

function form0(&$sqlo, $inf) { 

	$tablename=$this->tablename;
	$id = $this->id;

	$iopt=array();
	$iopt["icon"] = "ic.del.gif";
	htmlInfoBox( "Delete object", "", "open", "INFO", $iopt );
	echo "<center>";
	echo $inf[0];
	if ($this->projAskUnlink) {
		$projNice=obj_nice_name ( $sqlo, 'PROJ', $this->foundProjid );
		echo "<br /> and remove it from project '<b>".htmlspecialchars($projNice)."</b>' ID:".$this->foundProjid;
	}
	echo ' ?<br>';
	
	echo "<form name=\"delform\" action=\"".$_SERVER['PHP_SELF']."\" method=post>\n";
	echo '<input type="hidden" name=tablename value="'.$tablename.'">'."\n";
	echo '<input type="hidden" name=forceflag value="0">'."\n";
	
	foreach( $id as $pos=>$pk_id) {
		echo '<input type="hidden" name="id['.$pos.']" value="'.$pk_id.'">'."\n";
	}
	
	
	echo "<br><input type=button value=\"YES\" onClick=\"document.delform.forceflag.value=1; document.delform.submit();\">\n";
	
	echo " &nbsp;&nbsp;&nbsp;<input type=button value=\"no\" onClick=\"document.delform.forceflag.value=2; document.delform.submit();\">\n";
	
	if ($tablename=="EXP") echo "<br><input type=checkbox name=optdeep value=\"1\"> allow remove of sub-objects (images, protocols)<br>\n";  
	echo "</form>\n";
	htmlInfoBox( "", "", "close");
	
}

function doDelete(&$sqlo) { 
	$tablename=$this->tablename;
	$id = $this->id;
	$optdeep=$this->optdeep;
	
	$tmp_arr_id = array();
	if (!empty($id[1])) $tmp_arr_id[] = $id[1];
	if (!empty($id[2])) $tmp_arr_id[] = $id[2];
	
	
	if ( $this->foundProjid ) { 
		// first unlink ..
		$projAddLib = new oProjAddElem($sqlo, $this->foundProjid);
		$projAddLib->unlinkObj( $sqlo, $tablename, $id[0]);
	}
	
	$dellib = new fObjDelC();
	$xopt	= NULL;
	$xopt["info"]=1;
	if ($optdeep>0) {
		$xopt["deep"]=1; 
	}
	$retval = $dellib->obj_delete ($sqlo, $tablename, $id[0], $tmp_arr_id, NULL, $xopt);
}

}

// ------------

$error = & ErrorHandler::get();
$sql   = logon2( );
if ($error->printLast()) htmlFoot();

$id = $_REQUEST["id"];
$tablename=$_REQUEST['tablename'];
$forceflag=$_REQUEST['forceflag'];
$optdeep=$_REQUEST['optdeep'];

if (empty($forceflag)) $forceflag = 0;
if (empty($id[0])) $id[0] = '';

$mainLib = new gObjDeleteGui($tablename, $id, $forceflag, $optdeep);

$title   = 'Delete object';
$backurl = 'edit.tmpl.php?t='.$tablename.'&id='.$id[0];

$infoarr=array();
$infoarr["scriptID"] = "";
$infoarr["title"]    = $title;
$infoarr['obj_name'] = $tablename;
$infoarr["obj_id"]   = $id[0];
if ($id[1]!="") $infoarr["obj_id2"] = $id[1];
if ($id[2]!="") $infoarr["obj_id3"] = $id[2];
$infoarr["form_type"]= "obj";
$infoarr['design']   = 'norm';

$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);

if (empty($tablename)) $pagelib->htmlFoot('ERROR', 'Tablename missing!');

$inf = $mainLib->init($sql);

if ($forceflag==2) {
	js__location_href( $backurl, 'go back to object' );
	return;
}

$mainLib->check($sql);
$pagelib->chkErrStop();

if ( !$forceflag ) {
	$mainLib->form0($sql, $inf);
	echo '</ul>';
	htmlFoot();
} else {
	echo $inf[0]."<br>";
	
}

if ($forceflag==1) {
  $mainLib->doDelete($sql);
  
  if ($error->Got(READONLY))  {
    $error->printAllEasy();
    $pagelib->htmlFoot('<br></ul><hr>');
  }
  
  js__location_href('view.tmpl.php?t='.$tablename, 'Go back');
}

echo '</ul>';
htmlFoot();

