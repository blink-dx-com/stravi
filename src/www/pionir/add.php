<?php
/**
 * - prepare insert of a new element into a table
 * - calls the edit.tmpl.php after
 * @package add.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param $tablename
 *	  $blueprint_val ( value of primary key of blueprint )
 *	  $remotepos  position TARGET-editfield in OPENER ( see edit.tmpl.php )
 */
session_start(); 

require_once ('reqnormal.inc');
require_once ( "javascript.inc" );

global $error;

$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();

$tablename=$_REQUEST['tablename'];
$blueprint_val =$_REQUEST['blueprint_val'];
$remotepos=$_REQUEST['remotepos'];


$title				 = 'add element';
$infoarr			 = NULL;
$infoarr['scriptID'] = '';
$infoarr['title']    = $title;
$infoarr['form_type']= 'tool';

$pagelib = new gHtmlHead();
$pagelib->startPageLight( $sql, $infoarr);
	
if (empty($tablename)) {
  htmlFoot('ERROR', 'table missing');
}

$tablename_nice = tablename_nice2($tablename);
$blueprint_col  = PrimNameGet2($tablename);

echo '&nbsp;<br><B>Prepare insert of  ('.$tablename_nice.') </B><br>';

if ( empty($blueprint_col) ) {
  $pagelib->htmlFoot('ERROR', 'table '.$tablename.' not described by cct_column. Contact the administrator please.');
}

$urlStr='edit.insert.php?tablename='.$tablename;
if (!empty($remotepos)) {
	$urlStr .= '&remotepos='.$remotepos;
}

$prim_cnt = countPrimaryKeys($tablename); // number of primary keys
$BO_is    = cct_access_has2 ($tablename);
$notest   = 0;


  $sel_pre = $blueprint_col;
  if ( $prim_cnt>1 ) {
	
	$blueprint_val = empty($_SESSION['s_tabSearchCond'][$tablename]["mothid"]) ? '' : $_SESSION['s_tabSearchCond'][$tablename]["mothid"];
	if (empty($blueprint_val)) {
		if ($_SESSION['sec']['appuser'] === 'root') {
            echo "Warning: no mother object ID given.<br>\n";
			$notest=1;
		} else {
			$mother_tab  = mothertable_get2($tablename);
			$mother_nice = tablename_nice2 ($mother_tab);
			echo '<center><br><B>ERROR:</B> To create an ASSOCIATED element, start from the mother object <B>'.$mother_nice.'</B>.<br>';
			echo 'Try to call the list from the mother object, where the new element should belong to!<br></center>';
			return;
		}
	}
  }
  

$ori_params = array();
if (!empty($blueprint_val)) { // vorlage?

    $sql->query('SELECT * FROM '.$tablename.' WHERE '.$blueprint_col.' = '.$sql->addQuotes($blueprint_val));
    if ( $sql->ReadArray() ) {
        $ori_params = $sql->RowData;
    }
	$urlStr .= '&argu_xtra['.$blueprint_col.']='.$blueprint_val;
} else {
	
}




js__location_replace( $urlStr, 'forward' );  // $stop=array(0=>flag, 1=>reason)); 

$pagelib->htmlFoot();

