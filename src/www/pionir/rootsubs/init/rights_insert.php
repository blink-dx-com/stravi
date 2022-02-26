<?php

/**
 * insert role-rights to database (table USER_RIGHT)
 * 
 * - get rights from file rights_data.inc
 * - check in the LAB-folder for f.admin.rolerig.inc
 * - CCT_TABLE must be initialized !
 * - see also /www/lab/f.admin.rolerig.inc
 * @namespace core::init::rights_insert
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param $show_flag 0: no special infos
 * @param $go : 0,1
 * @version $Header: trunk/src/www/pionir/rootsubs/init/rights_insert.php 59 2018-11-21 09:04:09Z $
 */ 

session_start(); 


require_once ('reqnormal.inc');
require_once('object.subs.inc');
require_once('rights_data.inc');

class fRoleRigIns {


	function __construct($show_flag) {
		$this->show_flag = $show_flag;
	}
	
	function initData() {
	
		
		$this->object_rights=array(
			'read',
			'write',
			'insert',
			'delete',
			'admin'
			);
			
		$this->function_rights=array(
			'execute'); 
		$this->deny_function_rights=array(
			'deny');					
		
	                
	    list( $this->object_names, $this->function_names,
	    	$this->deny_function_names, $this->depricated_rig ) = rights_get();
							
		// get more data from "lab"
		$labextraFile = $_SESSION["s_sessVars"]["AppLabLibDir"] ."/root/f.admin.rolerig.inc";
		if ( file_exists($labextraFile) ) {
			echo "... get extra data from LAB-code-base.<br>\n";
			require_once($labextraFile);
			$adminRoles = f_admin_roleright();
			
			echo "<br>";
			
			$this->object_names = array_merge($this->object_names, $adminRoles["obj"]);
			$this->function_names = array_merge($this->function_names, $adminRoles["funcs"]);
			$this->deny_function_names = array_merge($this->deny_function_names, $adminRoles["deny"]);
			
			
		}
	}
	
	function perform_check( &$sql, $object_name, $object_type, $object_rights){
	  //Function which check a list of objects and rights against table user_right
	  //And creates the missing user_right entries
	  
	  //Input: $sql - db handle
	  //       $object_name - an array containing all the objects which has to cheched
	  //       $object_type - the class of the objects in the list ( eg: o - for database objects, f - for runctions
	  //       $object_rights - an array containing the list of rights which have to be checked for each object
	  //Return: 0 - ok , 1 - error
	  $show_flag = $this->show_flag;
	  
	  $sql->query("select name,cct_right from user_right");
	  
	  $i=0;
	  $current_rights = array();
	  while($sql->ReadRow()) {
		$current_rights[$i]["name"]=$sql->RowData[0];
		$current_rights[$i++]["cct_right"]=$sql->RowData[1];
	  };
	  
	  $inserted=0;
	  
	  foreach($object_name as $key => $value){
		if (is_numeric($key)) { // we now have two different notations
		  $my_obj = $value;
		  $my_obj_notes = 'NULL';
		} else {
		  $my_obj = $key;
		  $my_obj_notes = $sql->addQuotes($value);
		}
		// echo("<b>".$my_obj.":</b><p>");
		
		if ($object_type=="o") {
		  	// check, if table exists => NO => $right_ok=1;
			
			$tmpret = table_exists( $sql, $my_obj );
			if ($tmpret==NULL) {
				htmlInfoBox( "Warning", "table '$my_obj' is not defined. continued ...", "", "WARN" );
				continue; // next object ...
			}
			
		}
		  
		foreach($object_rights as $my_o_right) {
		
		  $right_ok = 0;
		  
		  foreach($current_rights as $c_r){
			if (($c_r['name'] == $object_type.'.'.$my_obj) and ($c_r['cct_right'] == $my_o_right)) $right_ok=1;
		  }
		  
		  $this->_display_status($my_obj,$my_o_right,$right_ok);
		  
		  if(!$right_ok) {
			$sql->query('INSERT INTO user_right (name, cct_right, notes) VALUES ('.
						$sql->addQuotes($object_type.'.'.$my_obj).', '.
						$sql->addQuotes($my_o_right).', '.
						$my_obj_notes.')');
			$inserted++;
		  }
		  
		}
		// echo("<p>\n");
	  }
	  echo("<br>Rights inserted: $inserted <br>\n");
	  
	  return 0;
	}
	
	function _display_status($obj, $right, $status) {
	  $show_flag = $this->show_flag;
	  if($status){
		if ($show_flag) echo($obj.' already has '.$right.'.<br>');
	  } else {
		echo($obj.' has no right "'.$right.'". <font color="#0000ff"><b> Inserting </b></font><br>');
	  }
	} 
	
	function perform_check_old( &$sql, &$sql2 ) { 
	    echo "<B>Test for old role-rights</B>";
	    echo "<blockquote>\n"; 
	                
		$bad_rights = &$this->depricated_rig;
	                         
	    if (sizeof($bad_rights)) {
	        foreach( $bad_rights as $key=>$val) {              
	            $retval = $sql2->query("select USER_RIGHT_ID, CCT_RIGHT from user_right where name='".$val."'");
	            while ($sql2->ReadRow()) {
		            $tmRightId  = $sql2->RowData[0];
					$tmRightval = $sql2->RowData[1]; 
					echo "delete <B>$val</B> : <B>$tmRightval</B>";
					       
					$sqls    = "DELETE from RIGHT_IN_ROLE where USER_RIGHT_ID=". $tmRightId;                      
	                $retval  = $sql->query($sqls);
	                $retval  = $sql->query("DELETE user_right where USER_RIGHT_ID='".$tmRightId."'");
	            	echo "<br>";
				}
	        }
	    } 
	    echo "</blockquote>\n";
	}
	
	
	function allChecks( &$sql, &$sql2 ) {
	
		echo "<b>Object rights:</B><blockquote>\n"; 
		$this->perform_check( $sql, $this->object_names, "o", $this->object_rights);
		echo("</blockquote>\n"); 
		
		
		echo "<b>POSITIVE Function rights:</B><blockquote>\n"; 
		$this->perform_check( $sql, $this->function_names, "f", $this->function_rights);
		echo("</blockquote>\n");
		
		echo "<b>DENY Function rights:</B><blockquote>\n"; 
		$this->perform_check( $sql, $this->deny_function_names,"f", $this->deny_function_rights);
		echo("</blockquote>\n");   
		
		$this->perform_check_old( $sql, $sql2 );
	}


	function this_formshow() {
		require_once ('func_form.inc');
	
		$initarr   = NULL;
		$initarr["action"]      = $_SERVER['PHP_SELF'];
		$initarr["title"]       = "Create missing rights";
		$initarr["submittitle"] = "Submit";
		$initarr["tabwidth"]    = "AUTO";
	
		$hiddenarr = NULL;
	
		$formobj = new formc($initarr, $hiddenarr, 0);
	
		$formobj->close( TRUE );
	}
	
}

$sql  = logon2( );
$sql2 = logon2( );

$back_url = 'index.php';
$back_txt = 'System data init';

$show_flag = $_REQUEST['show_flag'];
$go 	   = $_REQUEST['go'];


$title = 'Role: Insert missing rights';
$infoarr=array();
$infoarr["locrow"]  = array( array($back_url, $back_txt) );
$infoarr["title"] = $title;
$infoarr["form_type"]= "tool";

$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);
echo "<ul>";
echo "Info: this script inserts missing role-rights to the database.<br>";
if (!isset($show_flag)) $show_flag = 0;

$mainLib = new fRoleRigIns($show_flag);

if ( !glob_isAdmin() ) {
  htmlFoot('ERROR', 'You must be admin.');
}

if (!$go) {
	$mainLib->this_formshow();
	htmlFoot('<hr>');
}


echo "[<a href=\"../../view.tmpl.php?t=USER_RIGHT\">table user rights</a>]<br><br>\n"; 
$mainLib->initData();
$mainLib->allChecks( $sql, $sql2);

htmlFoot('<hr>');
