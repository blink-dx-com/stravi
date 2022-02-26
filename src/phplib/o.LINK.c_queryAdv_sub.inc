<? 
/**
 * contains sub functions for executing an advanced query
 * and generating/chcking for valid authorization keys
 * $Header: trunk/src/phplib/o.LINK.c_queryAdv_sub.inc 59 2018-11-21 09:04:09Z $
 * @package o.LINK.c_queryAdv_sub.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param string INPUT 
 */
 
require_once ('glob.obj.update.inc');

class oLINK_c_queryAdv {
	
	var $objID = NULL; // LINK object ID
	var $preSalt = "q2X§<jZ.)9*@"; //
	var	$postSalt = "O!>Rn|f&e#~0"; //
	var $extra_obj_id = 0;
	
	//Constructor
	function __construct() {
		global $varcol;
		$this->class_id	= $varcol->class_name_to_id( 'LINK', 'queryAdv' );
	}
	
	/*
	 *
	 */
	function setObj( &$sqlo, $objID ) {
		global $varcol;
		$this->id = $objID;
		$id = $this->id;
		$this->values = NULL;
		
		$sqls="select name, extra_obj_id, notes from LINK where LINK_ID=".$id;
		$sqlo->query($sqls);
		if ( $sqlo->ReadRow() ) {
			$link_name = $sqlo->RowData[0];
			$this->extra_obj_id = $sqlo->RowData[1];
			$notes = $sqlo->RowData[2];
		} 
		
		if ( $this->extra_obj_id ) {
			$values_all   	= $varcol->select_by_name($this->extra_obj_id);
			$this->values 	= $values_all['values'];
			$this->classid	= $values_all['extra_class_id'];
			$this->className= $varcol->class_id_to_name( $this->classid );
		}
	}

	/*
	 *
	 */
	function checkParams( &$sqlo ) {
		global $error;
		
		$FUNCNAME = 'checkParams';
		
		$values = $this->values;
		$extra_class = $this->className;
		$extra_obj_id = $this->extra_obj_id;
		
		if (!$extra_obj_id) {
			$error->set( $FUNCNAME, 1, 'document has no class.' );
			return;
		}	
		    
		if ( $extra_class!="queryAdv" ) {
			$error->set( $FUNCNAME, 1, 'not of  class "queryAdv"' );
			return;
		}
		
		if ( $values["sqlArray"]=="" ) { // || $values["authKey"]==""  ) {
			$error->set( $FUNCNAME, 1, 'class parameters missing' );
			return;
		}
	}
	
	/*
	 *
	 */
	function getValues() {
		return ($this->values);
	}
	
	/*	creates an MD5-Hash-Key out of the given static salts and the sqlArray
	 *	@param string $sqlArray
	 *	@return string MD5-Hash-Key
	 */
	function _getHash($sqlArray) {
		$md5Result = md5($this->preSalt . $sqlArray . $this->postSalt);
		return ($md5Result);
	}
	
	/*	gets an MD5-Hash-Key from the method _getHash(string) for a given sqlArray string
	 *	@param string $sqlArray
	 *	@return string MD5-Hash-Key
	 */
	 function createKey($sqlArray) {
		return $this->_getHash($sqlArray);
	}
	
	/*	creates an MD5-Hash-Key out of the given static salts and the sqlArray
	 *	@param string $sqlArray
	 *	@param string $key
	 *	@return boolean 1=(TRUE) if $sqlArray and $key map to the same hash, otherwise 0=(FALSE)
	 */
	function checkKey($sqlArray, $key) {
		$result    = 0;
		$md5Result = $this->_getHash($sqlArray);
		if ( $md5Result===$key ) $result = 1;
		return ($result);
	}
	
	/* writes a generated authorization key for an advanced query (extra class object) into the database
	 * @param sql-handle &$sqlo
	 * @param string $key
	 * @return boolean 1=(TRUE) if the authorization key update was successful, otherwise 0=(FALSE)
	 */
	function writeKeyToObject( &$sqlo, $key ) {
		global $error;
		$FUNCNAME = 'writeKeyToObject';
		
		$id = $this->id;
		$UpdateLib = new globObjUpdate();
		$args = NULL;
		$args['xobj'] = array (
				'extra_class_id' => $this->class_id,
				'extra_obj_id'   => $this->extra_obj_id,
				'values' => array('authKey'=> $key )	
				);
		
		$UpdateLib->update_meta( $sqlo, 'LINK', $id, $args );
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 1, 'update of LINK ID:'.$id.' failed.' );
			return 0;
		}
		return 1;
	}
}