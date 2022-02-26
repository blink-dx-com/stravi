<?php
/*
 * extend edit.tmpl.php for MODULE
*/
require_once ("visufuncs.inc");
require_once ("f.objview.inc");
require_once ('o.CCT_ACCLOG.subs.inc');
require_once 'o.MODULE.subs.inc';
require_once ("lev1/gGraphRelWork.inc");

/**
 * 
 * @author skube
 * MOD_META:
 * ' state.rights'
    'state.otherRights'
    'flow.def'
    'state.project'
    'state.unlink'
    'state.allowGrp'
    'main.group' : the main group-ID of this workflow
 */
class oMODULE_xeditHelp {
	
	function __construct($viewmode) {
		$this->viewmode=$viewmode;
		$this->accRightNames = access_getRights ();
		$this->auditLib    = new oAccLogC();
	}
	
	function _getFamily($key) {
		$keyFamily = $key;
		$keyfamArr = explode('.',$key);
		if (is_array($keyfamArr)) {
			// if is an array, e.g. 'state.released.rights'
			$keyFamily=$keyfamArr[0].'.'.$keyfamArr[2];
		}
		if ($key == 'flow.def') $keyFamily = $key;
		if ($key == 'main.group') $keyFamily = $key;
		return $keyFamily;
	}
	
	// transfor right-mask to a short string, use first letter of right
	function _rightArr2ShortString($rightArr) {
		
		$oneGrpTxt="";
		foreach( $this->accRightNames as $dummy=>$rightName) {
			if ($rightArr[$rightName]==1)  $oneGrpTxt .= strtoupper(substr($rightName,0,1));
			else $oneGrpTxt .="-";
		}
		
		return $oneGrpTxt;
	}
	
// select only destination states from array
	function _stateArr2ShortString(&$sqlo, $stateArr) {
		$oneStateTxt="";
		$komma = "";
		foreach($stateArr as $state => $value) {
		    $statename = oH_ALOG_ACT_subs::statusID2Name($sqlo, $state);
			if ($state == -2)$statename = "flow end";
			if ($value==1)  $oneStateTxt .= $komma . $statename;
			$komma = '; ';
		}
		return $oneStateTxt;
	}
	
	function _getGroupInfo(&$sqlo, $group_id) {
		$group_id = intval($group_id);
		$sqlo->query("SELECT name FROM user_group WHERE user_group_id = ".$group_id);
		$sqlo->ReadRow();
		$groupname = $sqlo->RowData[0];
		if ($groupname==NULL) {
			$groupLink = '<span style="color:red;">ERROR: </span>ID: '.$group_id.' NOT FOUND!';
		} else {
			$groupLink= '<img src="images/icon.USER_GROUP.gif"> <b>'.$groupname."</b>";
		}
		return $groupLink;
	}
	
	function _getStateInfo(&$sqlo, $state_id) {
		$state_id = intval($state_id);
		$statename = oH_ALOG_ACT_subs::statusID2Name($sqlo, $state_id);
		if ($state_id == 0) $statename = "flow start";
		if ($statename==NULL) {
			$stateLink = '<span style="color:red;">ERROR: </span>ID: '.$state_id.' NOT FOUND!';
		} else {
			$stateLink= '<img src="images/ic.accAudit.gif"> <b>'.$statename."</b>";
		}
		return $stateLink;
	}
	
	/**
	 * transform right-array to user friendly overview for ONE status
	 * @param object $sqlo
	 * @param array $rigthArr
	 * @return string
	 */
	function _anaRightarr(&$sqlo, $rigthArr) {
		$infoout = NULL;
		$komma=NULL;
		foreach( $rigthArr as $group_id=>$rights) {
			$groupLink = $this->_getGroupInfo($sqlo, $group_id);
			$rightSummary = $this->_rightArr2ShortString($rights);
			$infoout .= $komma . $groupLink.': '.$rightSummary;
			$komma = '; ';
		}
		reset ($rigthArr); 
		
		return $infoout;
	}
	
/**
	 * transform state-array to user friendly overview for ONE status
	 * @param object $sqlo
	 * @param array $stateArr
	 * @return string
	 */
	function _anaStatearr(&$sqlo, $stateArr) {
		$infoout = NULL;
		$komma=NULL;
		foreach( $stateArr as $state_id=>$state) {
			$stateLink = $this->_getStateInfo($sqlo, $state_id);
			$stateSummary = $this->_stateArr2ShortString($sqlo, $state);
			$infoout .= $komma . $stateLink.': '.$stateSummary;
			$komma = '; ';
		}
		reset ($stateArr); 
		
		return $infoout;
	}
	
	
	function _anaGroupIds( &$sqlo, $groupStr) {
		$groupArr = explode(',',$groupStr);
		
		$infoout = NULL;
		$komma=NULL;
		foreach( $groupArr as $dummy=>$group_id) {
			
			$groupLink = $this->_getGroupInfo($sqlo, $group_id);
			$infoout .= $komma . $groupLink;
			$komma = '; ';
		}
		
		return $infoout;
	}
	
	function _anaMainGroup(&$sqlo, $group_id) {
	    $infoout = $this->_getGroupInfo($sqlo, $group_id);
	    return $infoout;
	}

	function _analyseKeyValue( &$sqlo, $key, $value ) {
		$funcerror = NULL;
		$info      = NULL;
		$keyFamily = $this->_getFamily($key);
		switch ($keyFamily) {
			case 'state.rights':
				$rigthArr = unserialize($value);
				if (!is_array($rigthArr)) {
					$funcerror='no valid values.';
					break;
				}
				$info = $this->_anaRightarr($sqlo, $rigthArr);
				break;
			case 'state.otherRights':
				$rigthArr = unserialize($value);
				if (!is_array($rigthArr)) {
					$funcerror='no valid values.';
					break;
				}
				$info = $this->_anaRightarr($sqlo, $rigthArr);
				break;
			case 'flow.def':
				$stateArr = unserialize($value);
				if (!is_array($stateArr)) {
					$funcerror='no valid values.';
					break;
				}
				$this->stateConnArr = $stateArr;
				$info = $this->_anaStatearr($sqlo, $stateArr);
				break;
			case 'state.project':
				if ($value>0) {	
					$objLinkLib = new fObjViewC();
					$info = $objLinkLib->bo_display( $sqlo, 'PROJ', $value );
				} else $info = $value;
				break;
			case 'state.unlink':
				if ($value>0) {	
					$objLinkLib = new fObjViewC();
					$info = $objLinkLib->bo_display( $sqlo, 'PROJ', $value );
				} else $info = $value;
				break;
			case 'state.allowGrp':
				if ($value!=NULL) {
					$info = $this->_anaGroupIds($sqlo, $value);
				}
				break;
			case 'main.group':
			    if ($value!=NULL) {
			        $info = $this->_anaMainGroup($sqlo, $value);
			    }
			    break;
			default:
				$info = $value;
		}
		
		// ana error
		
		if ($funcerror!=NULL) {
			$info .= '<span style="color:red;">Error: </span>'.$info;
		}
		
		return $info;
	}
	
	function showMetaList( &$sqlo, &$sqlo2, $id) {
		
		$tabobj = new visufuncs();
		$headOpt= array( "title" => 'List of '.tablename_nice2('MOD_META') );
		$headx  = array ("Key", "Value","&nbsp;", 'Info');
		$tabobj->table_head($headx,   $headOpt);
	
		$sqlo2->Quesel( "KEY, VALUE FROM MOD_META where MXID = " . $id.' order by KEY' );
		while ($sqlo2->ReadRow()) {
			
			$key   = $sqlo2->RowData[0];
			$value = $sqlo2->RowData[1];
			
			$info = $this->_analyseKeyValue( $sqlo, $key, $value );
			$keyInfo = '<a href="obj.module.metaEdit.php?id='.$id.'&key='.$key.'&action=edit">'.$key.'</a>';
			if ($this->viewmode=='edit') {	
				$actions =
					'<a href="obj.module.metaEdit.php?id='.$id.'&key='.$key.'&action=del">'.
					'<img src="images/but13.del.gif" border=0 title="delete"></a>';
			} else $actions=NULL;
			
			$notes='';
			$key_family = oMODULE_one::get_family_by_key($key);
			if ($key_family!=NULL) {
			    $fam_arr = oMODULE_one::get_notes_of_family($key_family);
			    if (!empty($fam_arr)) $notes = $fam_arr['notes'].'; '.$fam_arr['value'];
			}
			
			$dataArr=array( $keyInfo , $info, $actions, $notes );
			
			$tabobj->table_row ($dataArr);
		}
		$tabobj->table_close();
		$this->module_id = $id;
		if ($this->stateConnArr != NULL) $data_graph = $this->_creategraphtext($sqlo, $this->stateConnArr);
		if ($data_graph != NULL){
		    $ed_link='';
		    if ( $this->viewmode=='edit') 
		        $ed_link= '[<a href="obj.module.metaEdit.php?id='.$this->module_id.'&key=flow.def&action=edit">EDIT</a>]';
		    
		    echo "\n<br>Workflow transition graph: ".$ed_link."   <br />";
			$graphLib = new gGraphRelWork('graph1', $data_graph, $parx);
		    $graphLib->start($sqlo);
		}
	}

	function _creategraphtext(&$sqlo, $stateConnArr){
		$datex = "";
		if ($stateConnArr == NULL)return $datex;
		
		$this->auditLib    = new oAccLogC();
		foreach ($stateConnArr as $ini_state => $dest_arr){
		    $ini_state_name = oH_ALOG_ACT_subs::statusID2Name($sqlo, $ini_state);
			if ($ini_state != 0 and $ini_state != -2)$ini_state_name = $this->_getActionstring($sqlo, $ini_state_name);
			foreach ($dest_arr as $dest_state => $value){
				if ($ini_state == 0)$ini_state_name = "flow start";
				if ($value == 1){
					switch ($dest_state){
						case -2:
							$datex .= $colsep . $ini_state_name . "\tflow end";
							break;
						default:
						    $dest_state_name = oH_ALOG_ACT_subs::statusID2Name($sqlo, $dest_state);
							$datex .= $colsep . $ini_state_name . "\t" . $this->_getActionstring($sqlo, $dest_state_name);
					}
				}
				$colsep = "\n";
			}
		}
		return $datex;
	}
	
	function _getActionperState(&$sqlo, $ini_state_name){
		$query = "KEY, VALUE from MOD_META where KEY like 'state." . $ini_state_name . ".%' and MXID = ".$this->module_id;
		$sqlo->Quesel($query);
		$action = NULL;
		while ($sqlo->ReadRow()){
			$key = explode('.', $sqlo->RowData[0]);
			$action[$key[2]] = $sqlo->RowData[1];
		}
		return $action;
	}
	
	function _getActionstring(&$sqlo, $ini_state_name){
	$stateactionArr = $this->_getActionperState($sqlo, $ini_state_name);
	if ($stateactionArr != NULL){
		$ini_state_name .= " &rArr; ";
		$actionsep = NULL;
		foreach ($stateactionArr as $action => $key){
			$ini_state_name .= $actionsep . $action;
			$actionsep = ", ";
		}
	}
	return $ini_state_name;
	}
}


class obj_edit_ext extends gObj_edit_feat_ABS {
    
    function rightWindow( &$sqlo, &$sqlo2) {
        $id = $this->id;
        $argu  = &$this->argu;
        $viewmode = $this->viewmode;
        
    	echo '<ul>';
    	
    	if ($viewmode=='edit' ) {
    	    echo '<b>Add:</b> ';
    	    echo '[<a href="obj.module.metaEdit.php?id='.$id.'&action=edit&parx[keyFamily]=OTHER">OTHER</a>] ';
    	}
    	
    	$module_type = $argu['TYPE'];
    	switch ($module_type) {
    	    case 1:
    	       break;
    	    case 2: // workflow
    	        if ($viewmode=='edit' ) {
    	            echo '[<a href="obj.module.metaEdit.php?id='.$id.'&parx[keyFamily]=state.rights&action=new">status right-mask</a>] ';
    	            echo '[<a href="obj.module.metaEdit.php?id='.$id.'&parx[keyFamily]=state.otherRights&action=new">set other rights</a>]';
    	            echo '[<a href="obj.module.metaEdit.php?id='.$id.'&parx[keyFamily]=state.emailuser&action=new">status emails</a>]';
    	            echo '[<a href="obj.module.metaEdit.php?id='.$id.'&parx[keyFamily]=state.project&action=new">destination project</a>]';
    	            echo '[<a href="obj.module.metaEdit.php?id='.$id.'&parx[keyFamily]=state.unlink&action=new">unlink from project</a>]';
    	            echo '[<a href="obj.module.metaEdit.php?id='.$id.'&parx[keyFamily]=state.allowGrp&action=new">allow group</a>]';
    	            echo '[<a href="obj.module.metaEdit.php?id='.$id.'&key=main.group">main group</a>]';
    	            echo '[<a href="obj.module.metaEdit.php?id='.$id.'&parx[keyFamily]=flow.def&action=new">define state flow new</a>]';
    	            echo "<br />\n";
    	        }
    	        break;
    	    case 3: // plugin 
    	        break;
    	}
    	
    	
    	
    	$helpLib = new oMODULE_xeditHelp($viewmode);
    	$helpLib->showMetaList( $sqlo, $sqlo2, $id);
    	
      	//INACTIVE:  echo '<a href="f.exe.php?xid='.$id.'">Execute module</a><br />';
    	echo '</ul>';
    }


}
