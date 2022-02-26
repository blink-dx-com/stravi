<?php
/**
 * add READ access for group "all" to all BOs
 * @package access_all.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   $go [=1: add permission]
		 $parx["cctaccmax"]
 * @version0 2001-05-12
 */
session_start(); 

require_once ('reqnormal.inc');
require_once("access_mani.inc");

class oCCT_ACCESS_readmod {
    
    function __construct($parx) {
    	
    	$this->cctaccmax = $parx["cctaccmax"];
    	$this->opt_accid_sql = NULL;
    	$this->opt_accidAfterWhere = NULL;
    	if ( $this->cctaccmax>0 ) {
    		$this->opt_accidAfterWhere = "cct_access_id<".$this->cctaccmax." AND ";
    		$this->opt_accid_sql = " where cct_access_id<".$this->cctaccmax;
    	}
    	
    }
    
    function init(&$sql) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
       
        $this->group_name='everyone';
        
        $sqls = "select USER_GROUP_ID from USER_GROUP where NAME=".$sql->addQuotes($this->group_name);
    	$sql->query($sqls);	
    	$sql->ReadRow();
    	$group_id_tmp=$sql->RowData[0];
    	if (!$group_id_tmp) {
    	    $error->set( $FUNCNAME, 1, 'Group with name "'.$this->group_name.'" not found.' );
    	    return;
    	}
    	
    	$this->allGroupID = $group_id_tmp;
    }
    
    
    function moreInfo( &$sql ) {
    	$group_id_tmp = $this->allGroupID;
    	
    	
    	$sqls  = "select count(db_user_id) from DB_USER";
    	$retVal= $sql->query("$sqls");	
    	$retVal= $sql->ReadRow();
    	$user_num=$sql->RowData[0];
    	
    	$sqls = "select count(u.db_user_id) from DB_USER_IN_GROUP u where USER_GROUP_ID=".$group_id_tmp;
    	$retVal= $sql->query("$sqls");	
    	$sql->ReadRow();
    	$users_in_group=$sql->RowData[0];
    	
    	
    	$this->tmp_info_out("Available users", $user_num);
    	$this->tmp_info_out('Users in group '.$this->group_name, $users_in_group);
    	
    	$sqls  = "select count(cct_access_id) from cct_access".$this->opt_accid_sql;
    	$retVal= $sql->query("$sqls");	
    	$retVal= $sql->ReadRow();
    	$bos_num=$sql->RowData[0];
    	
    	if ($this->cctaccmax>0) $this->tmp_info_out("Max CCT_ACCESS_ID", $this->cctaccmax);
    	$this->tmp_info_out("Number of BOs", $bos_num);
    	
    	$sqls  = "select count(cct_access_id) from CCT_ACCESS_RIGHTS where ".$this->opt_accidAfterWhere."USER_GROUP_ID=".$group_id_tmp;
    	$retVal= $sql->query("$sqls");	
    	$retVal= $sql->ReadRow();
    	$bos_grp_num=$sql->RowData[0];
    	$this->tmp_info_out('Number of BOs with group '.$this->group_name, $bos_grp_num);
    }
    
    function doit( &$sqlo, &$sqlo2 ) {
    	global $error;
    	$FUNCNAME= "doit";
    	
    	$group_id = $this->allGroupID;
    	$sqls  = "select cct_access_id from cct_access ".$this->opt_accid_sql." order by cct_access_id";
    	$retVal= $sqlo->query("$sqls");
    	
    	$colcnt	=0;
    	$cnt	=0;
    	$exist_cnt=0;
    	echo "Start action now. Show current CCT_ACCESS_ID.<br><br>";
    	flush(); 
    	while ( $sqlo->ReadRow() ) {
    		
    			$access_id = $sqlo->RowData[0];
    			$retval    = $this->new_right_tmp( $sqlo2, $group_id, $access_id);
    			if ($retval<=0) {
    				$error->set( $FUNCNAME, 1, "Error occurred." );
    				break;
    			}
    			
    			if ($retval==2) $exist_cnt++;
    			
    			if ( ($access_id/1000) == ceil($access_id/1000) ) {
    				echo "$access_id ";
    				$colcnt++;
    				if ($colcnt>20) {
    					echo "<br>";
    					flush();
    					$colcnt=0;
    				}
    			}
    			$cnt++;
    	}
    	echo "<br>Ready.<br> $cnt objects reviewed.<br>";
    	echo "$exist_cnt objects already had ".$this->group_name." permissions.<br>";
    	echo ($cnt-$exist_cnt)." objects updated with new \"read\"-permissions.<br>";
    	
    }
    
    function new_right_tmp( &$db_h, $group_id, $access_id) {
      /*
      RETURN: 1 OK - inserted
      	  2 OK - exists, no update
      	  0 error
      */
      $retval=1;
      
      $mquery = "select * from cct_access_rights where cct_access_id=$access_id and user_group_id=".$group_id;
      $retval = $db_h->Query($mquery);
      if ( $db_h->ReadRow() ) {
      	$has_rights=1;
    	$retval=2;
      }
      if( !$has_rights ) {
      	//Create New Access Rights Entry
      	$colstr="CCT_ACCESS_ID, USER_GROUP_ID, SELECT_RIGHT, INSERT_RIGHT, UPDATE_RIGHT, DELETE_RIGHT";
    	$valstr= $access_id.",$group_id, 1, 0, 0, 0";
      	$retval= $db_h->Insertx ( "CCT_ACCESS_RIGHTS", $colstr, $valstr);
      }
      return $retval;
    }
    
    function tmp_info_out($info_head, $txt) {
    	echo "<font color=gray>$info_head:</font> <B>$txt</B><br>";
    }
    
    function form1() {
    	require_once ('func_form.inc');
    	
    	
    	$initarr   = NULL;
    	$initarr["action"]      = $_SERVER['PHP_SELF'];
    	$initarr["title"]       = "Prepare";
    	$initarr["submittitle"] = "Submit";
    	$initarr["tabwidth"]    = "AUTO";
    
    	$hiddenarr = NULL;
    
    	$formobj = new formc($initarr, $hiddenarr, 0);
    
    	$fieldx = array ( 
    		"title" => "Max CCT_ACCESS_ID", 
    		"name"  => "cctaccmax",
    		"object"=> "text",
    		"val"   =>  $parx["cctaccmax"],
    		"notes" => "give number of 0"
    		 );
    	$formobj->fieldOut( $fieldx );
    
    	$formobj->close( TRUE );
    }

}


global $error, $varcol;

$error = & ErrorHandler::get();
$sql = logon2( $_SERVER['PHP_SELF'] );
$sql2 = logon2( $_SERVER['PHP_SELF'] );

$go  = $_REQUEST["go"];
$parx= $_REQUEST["parx"];
$title="Read-Access modification for all BOs" ;
$infoarr=array();
$infoarr["title"] = $title;
$infoarr["form_type"]= "tool";
$infoarr["locrow"]   = array( array("index.php", "transform DB versions") );

$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);

$mainlib = new oCCT_ACCESS_readmod($parx);


if ( $_SESSION['sec']['appuser']!="root" ) {
	echo "Sorry, you must be root or have su_flag.";
	return 0;
}

ob_end_flush(); /* do to time consuming loops */

?>
<blockquote>
This function adds "read"-permission for group "all" to all business objects.<br><br>
<?

$mainlib->init($sql);
$pagelib->chkErrStop();

$group_id_tmp = $mainlib->allGroupID;


$mainlib->moreInfo( $sql );

echo "<br>";

if ( !$go ){
	?>	
	<LI> go to group <a href="../edit.tmpl.php?t=USER_GROUP&id=<?echo $group_id_tmp?>">GROUP</a> </LI>
	<?
	$mainlib->form1(); 
	htmlFoot();
} 

if ( $go == 1 ) {
		
		echo "BOs with existing permissions for this ALL-group  will be ignored!<br><br>";
		echo "<form method=\"post\" action=\"".$_SERVER['PHP_SELF']."?go=2&parx[cctaccmax]=".$parx["cctaccmax"]."\" > 	";
		echo "<input type=submit value=\"Add read-permission to all BOs NOW!\" >";
		echo "</form>";
		
		htmlFoot();
} 
	
if ( $go == 2 ) {
	
	$mainlib->doit( $sql, $sql2 );
	$error->printAll();
}

?>
<hr size=1>
</blockquote>
<?
htmlFoot();

