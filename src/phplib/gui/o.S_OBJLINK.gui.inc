<?php
/**
 * for one object: show list of S_OBJLINK children
 * @package o.S_OBJLINK.gui.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/phplib/gui/o.S_OBJLINK.gui.inc 59 2018-11-21 09:04:09Z $
 */
class oS_OBJLINK_gui {
	
	private $objTable;
	
	function __construct($tablename=NULL, $objid=NULL) {
		$this->objTable = array();
		if ($tablename!=NULL) {
			$this->set_object($tablename, $objid);
		}
	}
	
	function set_object($tablename, $objid) {
		$this->tablename = $tablename;
		$this->objid = $objid;
		$this->objTable = array();
	}
	
	/**
	 * return number of objects
	 * @param unknown $sqlo
	 * @return number
	 */
	function obj_has_children(&$sqlo) {
		
		$id = $this->objid;
		$tablename  = $this->tablename;
		
		
		// select all types of children: SUC, CBC, ...
		$sqlsel = "CH_TABLE, CH_ID from S_OBJLINK where ".
				' MO_ID='.$id. ' and MO_TABLE= '.$sqlo->addquotes($tablename).
				' and KEY='.$sqlo->addquotes('ERPchild').
				' order by POS';
		$sqlo->Quesel($sqlsel);
		$cnt=0;
		
		$this->objTable=NULL;
		
		$this->table_cache=array();
		
		while($sqlo->ReadRow()){
			$tmptable = $sqlo->RowData[0];
			$tmpID    = $sqlo->RowData[1];
			$this->objTable[] = array($tmptable, $tmpID);
			$this->table_cache[$tmptable]=1;
			$cnt++;
		}
		
		return $cnt;
	}
	
	/**
	 * show S_OBJLINK children and status
	 * - can be called without calling obj_has_children before
	 * @param unknown_type $sqlo
	 * @param unknown_type $id
	 */
	function showChildren(&$sqlo) {
	
		if (!$this->objid) {
			sys_error_my( 'Class not initialized.' );
		}
		
		$id = $this->objid;
		$tablename  = $this->tablename;
		
		if (!sizeof($this->objTable)) {
			// call init method
			$cnt = $this->obj_has_children($sqlo);
		} else {
			$cnt = 1;
		}
		
		if (!$cnt) {
			echo '<span style="color:gray;">none</span>'."<br>\n";
			return;
		}
		
		if ( sizeof($this->table_cache)==1 ) {
			
			$child_table = key($this->table_cache);
			$pk_name =  PrimNameGet2($child_table); 
			// only select here children of the same object
			$selectStr = 'x.'.$pk_name.' in ('.
					' select CH_ID from S_OBJLINK where '.
					' MO_ID='.$id. ' and MO_TABLE= '.$sqlo->addquotes($tablename).' and CH_TABLE= '.$sqlo->addquotes($child_table).
					' and KEY='.$sqlo->addquotes('ERPchild').
					')';
			$urlParam = urlencode($selectStr);
			echo ' &nbsp; [<a href="view.tmpl.php?t='.$child_table.'&condclean=1&tableSCond='.$urlParam.'">Select children</a>]';
		
		}
		$objLinkLib = new fObjViewC();
		echo "<br>\n";
		echo '<ul><table><tr style="color:gray;"><th>Objekt</th><th>Status</td></tr>'."\n";
		$accLogLib = new oAccLogC();
	
		reset ($this->objTable);
		foreach( $this->objTable as $dummy=>$valarr) {
			$tmptable = $valarr[0];
			$tmpID    = $valarr[1];
			$statusName = NULL;
			echo  "<tr>";
			echo  "<td>" . $objLinkLib->bo_display( $sqlo, $tmptable, $tmpID ) . "</td>";
	
			// get status
			if (gObject_exists ($sqlo, $tmptable, $tmpID) ) {
				$accLogLib->setObject( $sqlo, $tmptable, $tmpID);
				$statusName = $accLogLib->getLastLogName($sqlo);
			} else {
				$statusName = '???';
			}
			echo '<td>'.$statusName.'</td>';
	
			echo "</tr>\n";
		}
	
		echo "</table>";
		echo "</ul>";
		return $cnt;
	}
	
}
