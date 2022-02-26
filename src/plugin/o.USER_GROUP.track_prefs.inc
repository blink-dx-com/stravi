<?php
require_once 'o.USER_PREF.manage.inc';

/**
 * Analyse, where groups are used in User-Preferences
 * Delete preferences of selected groups
 *    
 * @package o.USER_GROUP.track_prefs.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param $go : 0,1
 * @version $Header: trunk/src/plugin/o.USER_GROUP.track_prefs.inc 59 2018-11-21 09:04:09Z $
 */
class o_USER_GROUP_track_prefs_XPL extends gPlugin {
	
	
	function register() {
		$tablename	= 'USER_GROUP';
		$title_sh		= 'Analyse/Delete User-Preferences';
		$title		  = 'Analyse/Delete User-Preferences, where groups are used';
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title_sh']    = $title_sh;
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'list'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_cnt']  = 1; // count current selection
		
		
		$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('read'), 'obj'=>array() ),
				'listChkSelect' => 1,
				'onlyAdmin' => 1
				);
		
	}
	
	/**
	 * - creates a form with one text field "Min-value"
	 * - creates a HTML-page parameter $_REQUEST['parx']['minval'] after SUBMIT
	 */
	function form1() {
		
		$initarr   = NULL;
		$initarr["title"]       = "Delete groups from User preferences"; // title of form
		$initarr["submittitle"] = "Delete!"; // title of submit button
		$initarr["tabwidth"]    = "AUTO";   // table-width: AUTO

		$hiddenarr = NULL; 

		// init form, uses class formc
		$formobj = $this->_html->formInit($initarr, $hiddenarr, 0);

		$formobj->close( TRUE ); // close form, sshow submit-button
	}
	
	private function get_varname($groupid) {
		$varname='umask.'.$groupid;
		return $varname;
	}
	
	function get_num_usages($sqlo, $groupid) {
		
		$varname = $this->get_varname($groupid);
		$sqlsel = 'count(1) from USER_PREF where VAR_NAME='.$sqlo->addQuotes($varname);
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$num = $sqlo->RowData[0];
		return $num;
	}
	
	function remove_preferences($sqlo, $groupid) {
		$varname = $this->get_varname($groupid);
		oUSER_PREF_manage::remove_var_all($sqlo, $varname);
	}

	function startMain() {
		global $error;
		
		$FUNCNAME='startMain';
		
		$sqlo  = &$this->_sqlo;
		$sqlo2 = &$this->_plug_regSql2();
		$go=$_REQUEST['go'];
		
		if (!$go) {
			$this->form1();
			echo "<br>";
		}
		
		$sqlAfter = $this->listLib->getSqlAfter();
		$sqlsel = "x.USER_GROUP_ID, x.NAME  from ". $sqlAfter;
		$sqlo2->Quesel($sqlsel);
		while ( $sqlo2->ReadRow() ) {
			$groupid = $sqlo2->RowData[0];
			$groupname = $sqlo2->RowData[1];
			
			$num = $this->get_num_usages($sqlo, $groupid);
			echo $groupid.':'.$groupname.':'.$num;
			
			if ($go>0) {
				$this->remove_preferences($sqlo, $groupid);
				if ($error->Got(READONLY))  {
					$error->set( $FUNCNAME, 1, 'Error on group '.$groupid );
					return;
				}
				echo '; &nbsp;&nbsp;&nbsp; DELETE_pref ok.';
			}
			echo "<br>";
			
			
		}
		
	}
	
}