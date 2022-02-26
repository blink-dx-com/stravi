<? 
require_once('o.LINK.c_queryAdv_sub.inc');
require_once('visufuncs.inc');

/**
 * contains sub functions for executing an advanced query
 * $Header: trunk/src/www/pionir/obj.link.c_queryAdv_sub2.inc 59 2018-11-21 09:04:09Z $
 * @package obj.link.c_queryAdv_sub2.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param string INPUT 
 */
class oLINK_c_queryAdv_gui {
	var $id; // LINK object ID
	
	//Constructor
	function __construct( $id, $go, $myquvarval ) {
		$this->setObj($id);
		$this->go = $go;
		$this->myquvarval=$myquvarval;
		
		$this->values = NULL;
		
		$this->queryObj = new oLINK_c_queryAdv();
	}	

	function setObj($id) {
		$this->id = $id;
	}
	
	/* builds and executes the advanced sql-query given by the value of "sqlArray"
	 * @param sql-handle-reference &$sqlo
	 */
	function doSearch( &$sqlo ) {
		global $error;
		$FUNCNAME= 'doSearch';
		
		$values  = &$this->values;
		$sqlsel  = $values["sqlArray"];
		$authKey = $values["authKey"];
		$cnt = 0;
		
		if ( !$this->queryObj->checkKey($sqlsel, $authKey) ) {
			//error handling and abortion
			$error->set( $FUNCNAME, 1, "The authorization key is not set or does not match the selected query.<br />\n".
					"Please create this key (Tool: Generate authorization key)!");
			return;
		}
		
		$numcols=0;
		
		$sqlo->Quesel($sqlsel);
		while ( $sqlo->ReadRow() ) {
			if (!$cnt) {
				$colName_list = $sqlo->ColumnNames();
				$numcols = sizeof($colName_list);
				
				$tabobj = new visufuncs();
				$headOpt= array( "title" => "Result");
				$tabobj->table_head($colName_list, $headOpt);
				
				//$colsout = implode( "\t", $colName_list);
				//echo $colsout."<br>";
			}
			
			//$colsout = implode( "\t",$sqlo->RowData);
			$tabobj->table_row ($sqlo->RowData);
			// echo $colsout."<br>";
			$cnt++;
		}
		$tabobj->table_close();
		
	}
	
	function initChecks( &$sqlo ) {
		$id = $this->id;
		
		$this->queryObj->setObj( $sqlo, $id );
		$this->queryObj->checkParams( $sqlo );
		$this->values = $this->queryObj->getValues();
	}
 
}
