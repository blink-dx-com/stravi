<?php
/**
 * edit an MODULE meta entry
 * on 'new' : expect $keyFamily
 * @package obj.module.metaEdit.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param number $id ID of MODULE
 * @param $action 'new', ['edit'], 'del'
 * @param $go
 *  -- for 'new' ---
 *   0 : select status-name
 *   1 : show   right-form
 *   3 : insert right-array 
 * @param $key key of meta entry
 * @param $parx[keyFamily] -- needed for $action=='new';
 *    'state.rights', 
 *    'state.emailuser', 
 *    'state.allowGrp'
 *    'low.def'
 *    'main.group' : ID of USER_GROUP
 *    'url.params' : for TYPE=3 (PLUGIN)
 *    'OTHER'
 * @param $parx[statusName] -- needed for $action=='new'; for keyFamily
 * @param $parx[value]    -- the input value
 * @param $garr[$group_id][$rightname] : if $group_id==-1 : new group in $parx['new_groupid']
 */

session_start(); 

require_once ("reqnormal.inc");
require_once ("insert.inc");
require_once ('func_form.inc');
require_once ("f.assocUpdate.inc");
require_once ('javascript.inc');
require_once ('o.CCT_ACCLOG.subs.inc');
require_once 'o.H_ALOG_ACT.subs.inc';
require_once 'o.MODULE.subs.inc';
require_once 'gui/o.MODULE.workflow_gui.inc';

// helper methods
class oMODULE_helpx {
    static function showHiddenParams($hiddenarr) {
        // show whole array
        
        if (!sizeof($hiddenarr)) return;
        
        foreach( $hiddenarr as $idx=>$valx) {
            echo "<input type=hidden name=\"". $idx ."\" value=\"".$valx."\">\n";
        }
        
        echo "\n";
        
    }
}

class guiAccessRights {
	
    
    
    /**
     * show form for rights
     * - form-parameters: $garr['.$group_id.']['.$rightname.']
     * @param string $destUrl
     * @param array $hiddenarr
     * @param array $groupRightArr $garr['.$group_id.']['.$rightname.']
     * @return -
     */
    function showRightsForm(&$sqlo, $title, $destUrl, $hiddenarr, $groupRightArr) {
    	
    	echo '<table border=0 cellspacing=1 cellpadding=0 bgcolor=#A0A0A0 ><tr><td>'.
    		     '<table border=0 cellspacing=0 cellpadding=2 bgcolor=#A0A0A0 width=100%><tr><td>';    
    	echo     '<font color=#FFFFFF><B>'.$title.'</b></font></td></tr></table>';
    	echo '</td></tr><tr><td>';
    	
    	echo '<table cellspacing=0 cellpadding=0 bgcolor=#EFEFEF><tr><td>'."\n";
    	echo '<form name="groups" style="display:inline;" method="post" action="'.$destUrl.'">'."\n";
    	
    	$rightNames = access_getRights ();
    	$colspan  = sizeof($rightNames);
    	
    	oMODULE_helpx::showHiddenParams($hiddenarr);
    	
    	echo '<table cellspacing="1" border="0" frame="void">'."\n";
    	echo '<tr valign=top bgcolor=#D0D0FF>'."\n";
    	echo '<th>user group</th>'."\n";
    	echo '<th colspan="'.$colspan.'">user group rights</th>'."\n";
    	echo '</tr>'."\n";
    
    	echo '<tr align="center" bgcolor=#D0D0FF>'."\n";
    	echo '<td>&nbsp;</td>';
    	foreach( $rightNames as $rightname) {
    		echo '<td>&nbsp;'.$rightname.'&nbsp;</td>';
    	}
    	reset ($rightNames); 
    	
    	echo '</tr>'."\n";
    	
    	echo "<tr align=center bgcolor=#D0D0D0>";
    	$jsFormLib = new gJS_edit();
    	
    	$fopt   = array( 'noshDel'=>1, 'selUseTxt'=>'[Add group]', 'emptyOnNoVal'=>1);
    	$ngroup = $jsFormLib->getAll('USER_GROUP', 'parx[new_groupid]', 'not_set', NULL,  0, $fopt);
    	echo "<td align=right>".$ngroup."</td>";
    	
    	foreach( $rightNames as $rightname) {
    		echo '<td><input type=checkbox name=garr[-1]['.$rightname.'] value=1';
    		echo "></td>";
    	}
    	reset ($rightNames); 
    	echo "</tr>\n";
    
    	if ( sizeof($groupRightArr) ) {
    		foreach($groupRightArr as $group_id => $rightArr) {
    	
    			$sqlo->query("SELECT name FROM user_group WHERE user_group_id=".$group_id);
    			$sqlo->ReadRow();
    			$tmpGrpName = $sqlo->RowData[0];
    			
    			echo "<tr align=center>";
    			echo "<td align=right><a href=\"edit.tmpl.php?t=USER_GROUP&id=".$group_id."\">".$tmpGrpName."</a></td>";
    			
    			foreach( $rightNames as $rightname) {
    				$val=$rightArr[$rightname];
    				echo '<td><input type=checkbox name=garr['.$group_id.']['.$rightname.'] value=1';
    				if ($val)
    					echo " checked";
    				echo "></td>";
    			}
    			
    			
    			echo "</tr>\n";
    		}  
    	}
    	echo "<tr bgcolor=#C0C0FF><td colspan=".($colspan+1)." align=center>";
    	echo "<img height=5 width=1><br>";
    	echo "<input type=submit value=\"Save\"><br>";
    	echo "<img height=5 width=1>";
    	echo "</td></tr>\n";
    	echo "</table>\n";
    
    	echo "</form>\n";
    
    	echo '</td></tr><table>';
    	echo '</td></tr><table>';
    	echo "<br>\n";
    	
    	
    }
}





class oModule_metaEdit {
	
	var $keyFamily;
	
    function __construct( $id, $action, $key, $parx ) {
    	
    	$this->parx=$parx;
    	$this->moduleid = $id;
    	$this->action=$action;
    	$this->key = $key;
    	$this->keyFamily = NULL;
    	$this->keyFamDefs = oMODULE_one::keyFamDefs;
    	
    }
    
    function _checkKeyExists(&$sqlo, $key) {
    	$sqlo->Quesel( "VALUE FROM MOD_META where MXID = " . $this->moduleid.' and KEY='.$sqlo->addQuotes($key) );
    	$exists = 0;
    	if ($sqlo->ReadRow()) {
    		$value = $sqlo->RowData[0];
    		$exists = 1;
    	}
    	return array($exists,$value);
    }
    
    function _getFamily($key) {
        return oMODULE_one::get_family_by_key($key);
    }
    	
    function _getValByKey(&$sqlo) {
    	$sqlo->Quesel( "VALUE FROM MOD_META where MXID = " . $this->moduleid.' and KEY='.$sqlo->addQuotes($this->key) );
    	$sqlo->ReadRow();
    	$val =  $sqlo->RowData[0];
    	return $val;
    }
    
    function _showAccRights(&$sqlo) {
    	
    	$groupArrUnserial = $this->_getValByKey($sqlo);
    	if ($groupArrUnserial!=NULL) $groupRightArr=unserialize($groupArrUnserial);
    	else $groupRightArr=NULL;
    	
    	$guiLib = new guiAccessRights();
    	$destUrl= $_SERVER['PHP_SELF'];
    	$hiddenarr=array(
    		'id'=>     $this->moduleid,
    		'key'   => $this->key,
    		'go'	=> 1
    	);
    	
    	echo  'key: <b>'.$this->key.'</b>'."<br>\n";
    	$title = 'Define right mask';
    	$guiLib->showRightsForm($sqlo, $title, $destUrl, $hiddenarr, $groupRightArr);
    }
    
    function _showStateConn(&$sqlo, $keyFamily) {
        
        $guiLib = new gui_MOD_Workflow();
    	
    	
        $guiLib->init_by_module($sqlo, $this->moduleid);
    	
    	
    	$destUrl= $_SERVER['PHP_SELF'];
    	$hiddenarr=array(
    		'id'=>     $this->moduleid,
    		'key'   => $keyFamily,
    		'go'	=> 1,
    		'parx[keyFamily]'   => $this->parx['keyFamily'],
    	);
    	$title = 'Define state connection';
    	$guiLib->showConnForm($sqlo, $title, $destUrl, $hiddenarr);
    	$guiLib->help('flowdef');
    }
    
    /**
     *
     * @param object $sqlo
     * @param string $value
     * @global $this->keyFamily
     * @return -
     */
    function _normalForm_KeyVal(&$sqlo, $key, $value) {
        $initarr   = NULL;
        $initarr["action"]      = $_SERVER['PHP_SELF'];
        $initarr["title"]       = "Meta Key/value form";
        $initarr["submittitle"] = "Update";
        $initarr["tabwidth"]    = "AUTO";
        $initarr["tabnowrap"]   = 1;

        $hiddenarr=array(
            'id'=>     $this->moduleid,
           
        );
        
        $formobj = new formc($initarr, $hiddenarr, 0);
        
        $fieldx = array (
            "title" => "Key",
            "name"  => "key",
            "object"=> "text",
            "namex" => TRUE,
            "val"   => $key,
            "fsize" => "60",
        );
        $formobj->fieldOut( $fieldx );
        $fieldx = array (
            "title" => "Value",
            "name"  => "value",
            "object"=> "text",
            "val"   => $value,
            "fsize" => "60",
            "notes" => ''
        );
        $formobj->fieldOut( $fieldx );

        $formobj->close( TRUE );
    }
    	
    
    /**
     * 
     * @param object $sqlo
     * @param string $value
     * @global $this->keyFamily
     * @return -
     */
    function _normalForm(&$sqlo, $value) {
    	$initarr   = NULL;
    	$initarr["action"]      = $_SERVER['PHP_SELF'];
    	$initarr["title"]       = "Meta Key/value form";
    	$initarr["submittitle"] = "Update";
    	$initarr["tabwidth"]    = "AUTO";
    	$initarr["tabnowrap"]   = 1;
    	
    	$parx = $this->parx;
    
    	$hiddenarr=array(
    		'id'=>     $this->moduleid,
    		'key'=>    $this->key,
    	);
    	
    	$formobj = new formc($initarr, $hiddenarr, 0);
    
    	$fieldx = array ( 
    		"title" => "Key", 
    		"name"  => "key",
    		"object"=> "text",
    		"namex" => TRUE,
    		"view"	=> 1,
    		"val"   => $this->key,
    		 );
    	$formobj->fieldOut( $fieldx );
    	$fieldx = array ( 
    		"title" => "Value", 
    		"name"  => "value",
    		"object"=> "text",
    		"val"   => $value,
    		"notes" => $valuenotes, 
    		"fsize" => "60",
    		"val"   => $value, 
    		"fsize" => "30",
    		"notes" => $this->keyFamDefs[$this->keyFamily]['value']
    		);
    	$formobj->fieldOut( $fieldx );
    	
    	$fieldx = array ( 
    		"title" => "Info", 
    		"object"=> "info2",
    		"val"   => '<i>'.$this->keyFamDefs[$this->keyFamily]['notes'] .'</i>'
    		
    		);
    	$formobj->fieldOut( $fieldx );

    	$formobj->close( TRUE );
    }
    
    // get all states
    function _form1SelState(&$sqlo, $keyFamily) {
    	
    	$sqlsel = "H_ALOG_ACT_ID, NAME from H_ALOG_ACT order by NAME";
    	$sqlo->Quesel($sqlsel);
    	$H_ALOG_ACT_arr=NULL;
    	while ( $sqlo->ReadRow() ) {
    	    $name = $sqlo->RowData[1];
    	    $H_ALOG_ACT_arr[$name]=$name;
    	}	
    	
    	$initarr   = NULL;
    	$initarr["action"]      = $_SERVER['PHP_SELF'];
    	$initarr["title"]       = "Select Status";
    	$initarr["submittitle"] = "Next &gt;";
    	$initarr["tabwidth"]    = "AUTO";
    
    	$hiddenarr=array(
    		'id'=>     $this->moduleid,
    		'action'=> $this->action,
    		'parx[keyFamily]'   => $this->parx['keyFamily'],
    	);
    
    	$formobj = new formc($initarr, $hiddenarr, 0);
    
    	$fieldx = array ( 
    		"title" => "Status", 
    		"name"  => "statusName",
    		"object"=> "select",
    		"val"   => $parx["statusName"], 
    		"inits" => $H_ALOG_ACT_arr,
    		"notes" => "select the status"
    		 );
    	$formobj->fieldOut( $fieldx );
    
    	$formobj->close( TRUE );
    }
    
    private function _formMainGroup($sqlo, $oldval) {
        
        
        
        $initarr   = NULL;
        $initarr["action"]      = $_SERVER['PHP_SELF'];
        $initarr["title"]       = "Select Main Group";
        $initarr["submittitle"] = "Submit";
        $initarr["tabwidth"]    = "AUTO";
        
        $hiddenarr=array(
            'id'=>     $this->moduleid,
            'action'=> $this->action,
            'parx[keyFamily]'   => $this->parx['keyFamily'],
        );
        
        $formobj = new formc($initarr, $hiddenarr, 0);
        
        $fieldx = array (
            "title" => "Status",
            "name"  => "grp_id",
            "object"=> "text",
            "val"   => $oldval,
            
            "notes" => "ID of user group"
        );
        $formobj->fieldOut( $fieldx );
        
        $formobj->close( TRUE );
    
    }
    
    // analyse $parx['keyFamily']
    function new0_AnaKey( &$sqlo ) {
    	global $error;
    	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
    	
    	$parx=$this->parx;
    	$keyFamily = $parx['keyFamily'];
    	switch ($keyFamily) {
    		case 'flow.def':
    			$this->_showStateConn($sqlo, $parx['keyFamily']);
    			break;
    		case 'state.rights':
    			$this->_form1SelState($sqlo, $parx['keyFamily']);
    			break;
    		case 'state.emailuser':
    			$this->_form1SelState($sqlo, $parx['keyFamily']);
    			break;
    		case 'state.otherRights':
    			$this->_form1SelState($sqlo, $parx['keyFamily']);
    			break;
    		case 'state.allowGrp':
    			$this->_form1SelState($sqlo, $parx['keyFamily']);
    			break;
    		case 'OTHER':
    		    $this->key='';
    		    $this->_normalForm_KeyVal($sqlo, '', '');
    		    break;
    		    
    		default:
    			$error->set( $FUNCNAME, 1, '$keyFamily: '.$keyFamily.' not supported.' );
    			return;
    	}
    }
    
    function new1_showForm( &$sqlo ) {
    	global $error;
    	$FUNCNAME= __CLASS__.':new1_showForm';
    
    	$parx=$this->parx;
    	$keyFamily = $parx['keyFamily'];
    	
    	if ($parx['statusName']==NULL) {
    		$error->set( $FUNCNAME, 1, 'statusName missing.' );
    		return;
    	}
    	
    	switch ($keyFamily) {
    		case 'state.rights':
    			$this->key = 'state.'. $parx['statusName'] .'.rights';
    			break;
    		case 'state.otherRights':
    			$this->key = 'state.'. $parx['statusName'] .'.otherRights';
    			break;
    		case 'state.emailuser':
    			$this->key = 'state.'. $parx['statusName'] .'.emailuser';
    			break;
    		case 'state.project':
    			$this->key = 'state.'. $parx['statusName'] .'.project';
    			break;
    		case 'state.unlink':
    			$this->key = 'state.'. $parx['statusName'] .'.unlink';
    			break;
    		case 'state.allowGrp':
    			$this->key = 'state.'. $parx['statusName'] .'.allowGrp';
    			break;
    		case 'flow.def':
    			$this->key = 'flow.def';
    			break;
    		default:
    			$error->set( $FUNCNAME, 1, 'keyFamily: '.$keyFamily.' not supported.' );
    			return;
    			
    	}
    	
    	$anser = $this->_checkKeyExists($sqlo, $this->key);
    	
    	if ( !$anser[0] ) {
    		echo '... create new entry.<br>';
    		$assoclib = new  fAssocUpdate();
    		$assoclib->setObj( $sqlo, 'MOD_META', $this->moduleid );
    		
    		$argu=array('KEY'=>$this->key, 'VALUE'=>NULL);
    		$assoclib->insert( $sqlo, $argu );
    		if ($error->Got(READONLY))  {
    			$error->set( $FUNCNAME, 1, 'insert of '.$this->key.' failed.' );
    			return;
    		}
    	}
    	
    	$this->edit1($sqlo);
    }
    
    function del0_Ask(&$sqlo) {
    	require_once ('func_formSp.inc');
    	
    	$value = $this->_getValByKey($sqlo);
    	echo '<b>key:</b> '.$this->key.'<br />';
    	echo '<b>value:</b> '.htmlspecialchars($value).'<br /><br />';
    
    	$specialLib = new formSpecialc();
    	$params=array('action'=>'del', 'go'=>1, 'id'=>$this->moduleid, 'key'=>$this->key);
    	$specialLib->deleteForm( 'Delete entry?', 
    			"Do you want to delete this entry?",
    			$_SERVER['PHP_SELF'], $params) ;
    }
    
    // delete one entry
    function del1_do( &$sqlo ) {
    	$assoclib = new  fAssocUpdate();
    	$assoclib->setObj( $sqlo, 'MOD_META', $this->moduleid );
    	$idarr=array('KEY'=>$this->key);
    	$assoclib->delOneRow( $sqlo, $idarr );
    }
    
    function edit1( &$sqlo ) {
    	global $error;
    	$FUNCNAME= __CLASS__.':edit1';
    
    	//$parx= $this->parx;
    	//$key = $this->key;
    	
    	$keyFamily = $this->_getFamily($this->key);
    	
    	$this->keyFamily=$keyFamily;
    	
    	switch ($keyFamily) {
    		case 'state.rights':
    			$this->_showAccRights($sqlo);
    			break;
    		case 'state.otherRights':
    			$this->_showAccRights($sqlo);
    			break;
    		case 'flow.def':
    			$this->_showStateConn($sqlo, $keyFamily);
    			break;
    		default:
    			$value = $this->_getValByKey($sqlo);
    			$this->_normalForm($sqlo, $value, $keyFamily);
    			return;
    	}
    }
    
    function edit2Save( &$sqlo, $garr ) {
    	global $error;
    	$FUNCNAME= $this->__CLASS__.':edit2Save';
    	
    	$parx=$this->parx;
    	$keyFamily = $this->_getFamily($this->key);
    	switch ($keyFamily) {
    		case 'state.rights':
    			if ($parx['new_groupid']>0) {
    				$subarr=$garr[-1];
    				$garr[$parx['new_groupid']]=$subarr;
    			}
    			unset($garr[-1]);
    			$value = serialize($garr);
    			break;
    		case 'state.otherRights':
    			if ($parx['new_groupid']>0) {
    				$subarr=$garr[-1];
    				$garr[$parx['new_groupid']]=$subarr;
    			}
    			unset($garr[-1]);
    			$value = serialize($garr);
    			break;
    		case 'flow.def':
    			if ($parx['new_state_id']>0) {
    				$subarr=$garr[-1];
    				$garr[$parx['new_state_id']]=$subarr;
    			}
    			unset($garr[-1]);
    			$value = serialize($garr);
    			break;
    		default:
    			$value = $parx['value'];
    	}
    	
    	echo '... save value.<br>';
    	$assoclib = new  fAssocUpdate();
    	$assoclib->setObj( $sqlo, 'MOD_META', $this->moduleid );
    	$anser = $this->_checkKeyExists($sqlo, $this->key);
    	if ( !$anser[0] ) {
    		$argu =array('KEY'=>$this->key, 'VALUE'=>$value);
    		$assoclib->insert( $sqlo, $argu );
    		if ($error->Got(READONLY))  {
    			$error->set( $FUNCNAME, 1, 'insert of '.$this->key.' failed.' );
    			return;
    		}
    		return;
    	}
    	$idarr=array('KEY'=>$this->key);
    	$argu =array('VALUE'=>$value);
    	
    	$assoclib->update( $sqlo, $argu, $idarr);
    	if ($error->Got(READONLY))  {
    		$error->set( $FUNCNAME, 1, 'update failed.' );
    		return;
    	}
    }

}

// --------------------------------------------------- 
global $error, $varcol;

$error = & ErrorHandler::get();
$sqlo  = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();

$id 		= $_REQUEST["id"];

$tablename	= "MODULE";
$title		= "add an module-UT-entry";

$infoarr			 = NULL;
$infoarr["scriptID"] = "";
$infoarr["title"]    = $title;
$infoarr["form_type"]= "obj";
$infoarr["obj_name"] = $tablename;
$infoarr["obj_id"]   = $_REQUEST["id"];
$infoarr["checkid"]  = 1;
$infoarr["help_url"]  = 'o.MODULE.trigger.html';

$pagelib = new gHtmlHead();
$pagelib->startPage($sqlo, $infoarr);

js_formAll();

echo "<ul>";

$garr= $_REQUEST["garr"];
$parx= $_REQUEST["parx"];
$key = $_REQUEST["key"];
$action = $_REQUEST["action"];
$go = $_REQUEST["go"];

if ($action==NULL) $action='edit';

$t_rights = tableAccessCheck( $sqlo, $tablename );
if ( $t_rights["insert"] != 1 ) {
    tableAccessMsg( tablename_nice2($tablename) , "insert" ); 
	htmlFoot();
}

$mainlib = new oModule_metaEdit( $id, $action, $key, $parx );
echo 'action: '.$action.'<br>';
$doForward=0;

switch ($action) {
	case 'new':
		if (!$go) {
			$mainlib->new0_AnaKey( $sqlo);
		}
		if ($go==1) {
			$mainlib->new1_showForm( $sqlo );
		}
		break;
	case 'del':
		if ( $key==NULL ) {
			$pagelib->htmlFoot('ERROR', 'key missing.');
		}
		if (!$go) {
			$mainlib->del0_Ask($sqlo);
		}
		if ($go==1) {
			$mainlib->del1_do($sqlo);
			$doForward=1;
		}
		break;
		
	default:

	    echo "keyFamily: ".$parx['keyFamily']."<br>";
		if ($go) {
		    if ( $key==NULL ) {
		        $pagelib->htmlFoot('ERROR', 'key missing. (3)');
		    }
			$mainlib->edit2Save( $sqlo, $garr );
			$error->printAll();
		}
		
		if ( $parx['keyFamily']=='OTHER') {
		    $mainlib->_normalForm_KeyVal($sqlo, '', '');
		} else {
		    if ( $key==NULL ) $pagelib->htmlFoot('ERROR', 'key missing. (4)');
		    $mainlib->edit1( $sqlo );
		}
	break;
	
}

$pagelib->chkErrStop();

if ($doForward) {
	js__location_replace('edit.tmpl.php?t=MODULE&id='.$id, "module" ); 
}
$pagelib->htmlFoot();

