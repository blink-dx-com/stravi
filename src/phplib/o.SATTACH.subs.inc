<?php

/**
 * attachment (table:SATTACH) analysis methods
 * @package o.SATTACH.subs.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @see o.SATTACH.mod.inc (modifications)
 * @version $Header: trunk/src/phplib/o.SATTACH.subs.inc 59 2018-11-21 09:04:09Z $
 */
class cSattachSubs {
	
	/**
	 * get filename on server; STATIC
	 * @param $t
	 * @param $obj_id
	 * @param $rel_id
	 * @return $path
	 */
	static function getDocumentPath($t, $obj_id, $rel_id) {
		$path = $_SESSION['globals']["data_path"]."/a.".$t.".".$obj_id.".".$rel_id;
		return ($path);
	}
	
	/**
	 * get number of attachments
	 * @param  $sqlo
	 * @param  $tablename
	 * @param  $id
	 * @return int number of attachments
	 */
	static function count_attach( &$sqlo, $tablename, $id) {
	    $sqlo->Quesel("count(1)  FROM SATTACH WHERE TABLE_NAME='".$tablename."' AND OBJ_ID=".$id);
	    $sqlo->ReadRow();
	    $obj_cnt = $sqlo->RowData[0];
	    return $obj_cnt;
	}
	
	/**
	 * get next FREE relative-ID (REL_ID)
	 * @param $sql
	 * @param  $tablename
	 * @param  $id
	 * @return int REL_ID (next)
	 */
	function getNextRelID( &$sql, $tablename, $id) {
		$sql->query("SELECT max(REL_ID) FROM SATTACH WHERE TABLE_NAME='".$tablename."' AND OBJ_ID=".$id);
		$sql->ReadRow();
		$infoval = $sql->RowData[0];
		if ( $infoval <= 0 ) $infoval = 0;
		$infoval++;  // next ID
		
		return ($infoval);
	}
	
	/**
	 * get data from one attachemnt entry
	 * @param  $sqlo
	 * @param  $tablename
	 * @param  $id
	 * @param  $rel_id
	 * @return array of (column-names,value)
	 */
	function getEntryByRelid( &$sqlo, $tablename, $id, $rel_id) {
		$sqlo->Quesel("* FROM SATTACH WHERE TABLE_NAME='".$tablename."' AND OBJ_ID=".$id.' and REL_ID='.$rel_id);
		$sqlo->ReadArray();
		$result = $sqlo->RowData;
		return ($result);
	}
	
	/**
	 * get RELID by KEY of attachment, 
	 * this method expects exactlty one answer, otherwise returns 0
	 * @param $sql
	 * @param $tablename
	 * @param $id
	 * @param string $key if 'search'=> 'like', can contain wildcards
	 * @param $options ['search'=> ['norm', 'like']]
	 * @return $relid
	 */
	function getRelIDbyKey(  &$sql, $tablename, $id, $key, $options=NULL ) {
		$condtion='=';
		$relid = 0;
		if ($options['search']=='like') $condtion='like';
		$sql->query("SELECT REL_ID FROM SATTACH ".
			" WHERE TABLE_NAME='".$tablename."' AND OBJ_ID=".$id. " AND KEY ".$condtion." ".$sql->addQuotes($key) );
		if ($sql->ReadRow() ) {
			$relid = $sql->RowData[0];
		} else return 0;
		if ($sql->ReadRow() ) return 0; // more attachments with this KEY will result 0
		return ($relid);
	}
	
	/**
	 * get RELID by NAME of attachment, 
	 * this method expects exactlty one answer, otherwise returns 0
	 * @param $sql
	 * @param $tablename
	 * @param $id
	 * @param string $key if 'search'=> 'like', can contain wildcards
	 * @param $options 
	 * 	'search' => ['norm'], 'like'
	 * @return $relid
	 */
	function getRelIDbyName(  &$sql, $tablename, $id, $name, $options=NULL ) {
		$condtion='=';
		$relid = 0;
		if ($options['search']=='like') $condtion='like';
		$sql->Quesel("REL_ID FROM SATTACH ".
			" WHERE TABLE_NAME='".$tablename."' AND OBJ_ID=".$id. " AND NAME ".$condtion." ".$sql->addQuotes($name) );
		if ($sql->ReadRow() ) {
			$relid = $sql->RowData[0];
		} else return 0;
		if ($sql->ReadRow() ) return 0; // more attachments with this KEY will result 0
		return ($relid);
	}
	
	/**
	 * check, if name exists, if yes create a new unqique name with version number
	 * e.g. test.pdf => test(2).pdf
	 * @param $sqlo
	 * @param $tablename
	 * @param $id
	 * @param $name
	 * @return $newname
	 */
	function checkName(&$sqlo, $tablename, $id, $name) {
		
		$rel_id  = $this->getRelIDbyName( $sqlo , $tablename, $id, $name );
		if (!$rel_id) {
			return $name;
		}
		
		$extstart   = strrpos($name,'.');
		$docbaseOri = $name; 
		if ($extstart>0) {
			 $docbaseOri = substr($name,0,$extstart);// test.pdf => test
		}
		$extOri = substr($name,$extstart); // ori extension
		
		// get last name of doc ...
		$docbaseSearch = $docbaseOri.'%'.$extOri;
		$sqlo->Quesel("REL_ID, NAME FROM SATTACH ".
			" WHERE TABLE_NAME='".$tablename."' AND OBJ_ID=".$id. " AND NAME like ".$sqlo->addQuotes($docbaseSearch).
			" order by REL_ID DESC" 
			);
			
		if ($sqlo->ReadRow() ) {
			$relid    = $sqlo->RowData[0];
			$lastName = $sqlo->RowData[1];
		}
		
		// change DOC-name
		// take extension, check vor .v1 , take next number
		$lastnumber = 1; // default
		$docbase   = basename($lastName);
		$extstart2 = strrpos($lastName,'.');
		$docbase   = $lastName; 
		if ($extstart2>0) {
			 $docbase = substr($lastName,0,$extstart2);// test.pdf => test
		}
		
		if (substr($docbase,-1,1)==')') {
			// has version string inside
			$posend  = strlen($docbase)-1;
			$posstart= strrpos($docbase,'(');
			if ($posstart>0) {
				$numberstr=substr($docbase,$posstart+1,$posend-1-$posstart);
				if (is_numeric($numberstr)) {
					$lastnumber=intval($numberstr);
				}
			}
		}
		
		$lastnumber = $lastnumber + 1;
		$newname = $docbaseOri.'('.$lastnumber.')'.$extOri; 
		return $newname;
	}
	
	/**
	 * check if NAME exists at REL_ID and SUB_POS_STR in $sub_pos (all entries of SUB_POS)
	 * - here REL_ID is not needed !
	 * @param object $sqlo
	 * @param string $tablename
	 * @param int $id
	 * @param int $sub_pos
	 * @param int $name -- the name to search
	 * @return number
	 */
	function checkNameExistsSubPos(object &$sqlo, string $tablename, int $id, int $sub_pos, string $name) {
	    $name_exists=0;
	    $sqlsel = "REL_ID from SATTACH where TABLE_NAME=".$sqlo->addQuotes($tablename).
	   	    " AND OBJ_ID=".$id. ' and NAME like '.$sqlo->addQuotes($name).
	        " and SUB_POS_STR like ".$sqlo->addQuotes($sub_pos.',%');
   	    $sqlo->Quesel($sqlsel);
   	    if ($sqlo->ReadRow()) {
   	        $name_exists = 1;
   	    }
   	    return $name_exists;
	}
	
	/**
	 * get all RELIDs by KEY of attachment, 
	 * this method returns a list of REL_IDs
	 * @param $sql
	 * @param string $tablename
	 * @param int $id
	 * @param string $key if 'search'=> 'like', can contain wildcards
	 *    if $key=NULL => get ALL attachments
	 * @param $options ['search'=> ['norm', 'like']]
	 * @return array of $relid
	 */
	function getRelArrByKey(  &$sql, $tablename, $id, $key, $options=NULL ) {
		$condtion='=';
		$relidArr=NULL;
		if ($options['search']=='like') $condtion='like';
		
		$sql_string = "SELECT REL_ID FROM SATTACH ".
			" WHERE TABLE_NAME='".$tablename."' AND OBJ_ID=".$id.
			" AND KEY ".$condtion." ".$sql->addQuotes($key) ;
		
		if ($key==NULL) {
			$sql_string = "SELECT REL_ID FROM SATTACH ".
				" WHERE TABLE_NAME='".$tablename."' AND OBJ_ID=".$id;
		}
		
		$sql->query( $sql_string .' order by REL_ID');
		while ($sql->ReadRow() ) {
			$relidArr[] = $sql->RowData[0];
		} return ($relidArr);
	}
	
	/**
	 * get HIGHEST rel_id of attachment by KEY, if exists
	 * @param $sqlo
	 * @param $table
	 * @param $objid
	 * @param $key
	 * @return int RELID or NULL
	 */
	public function get_LateRelIDByKey(&$sqlo, $table, $objid, $key) {
		$relidarr = $this->getRelArrByKey( $sqlo, $table, $objid, $key );
		if (empty($relidarr)) return NULL;
		
		end($relidarr); // get last version
		$relid = current($relidarr);
		return $relid;
	}
	
	/**
	 * get attachment name
	 * @param $sqlo
	 * @param string $table
	 * @param int $objid
	 * @param  $rel_id
	 * @return string
	 */
	public function getName(&$sqlo, $table, $objid, $rel_id) {
		if (!$rel_id) {
			sys_error_my( 'Input $rel_id missing.' );
		}
		
		$sqlo->Quesel("NAME FROM SATTACH ".
				" WHERE TABLE_NAME='".$table."' AND OBJ_ID=".$objid. " AND REL_ID=".$rel_id );
		$sqlo->ReadRow();
		$name = $sqlo->RowData[0];
		
		return $name;
	}
	
	static public function get_attach_down_url($table, $objid, $rel_id) {
		$attach_down_url = 'obj.sattach.down.php?t='.$table.'&id='.$objid.'&rel_id='.$rel_id;
		return $attach_down_url;
	}
	
	/**
	 * analyse documents by HASH
	 * @param object $sqlo
	 * @param string $table
	 * @param string $hash
	 * @return array
	 * 'cnt'=>number of hits
	 * 'ids'=>array of OBJ_ID; maximum: 10 entries !
	 */
	static function find_doc_by_hash(&$sqlo, $table, $hash) {
	    
	    $sqlsel_AFTER = 'from SATTACH where TABLE_NAME='.$sqlo->addQuotes($table). " and HASHX=".$sqlo->addQuotes($hash);
	    
	    $sqlsel='count(1) '.$sqlsel_AFTER;
	    $sqlo->Quesel($sqlsel);
	    $sqlo->ReadRow();
	    $obj_cnt = $sqlo->RowData[0];
	    
	    if (!$obj_cnt) {
	        return array('cnt'=>0);
	    }
	    
	    $objects=array();
	    $sqlsel='OBJ_ID '.$sqlsel_AFTER;
	    $sqlo->Quesel($sqlsel.' order by OBJ_ID');
	    $cnt=0;
	    $MAX_CNT=10;
	    while($sqlo->ReadRow() ) {
	        
	        if ($cnt>=$MAX_CNT) break;
	        $obj_id = $sqlo->RowData[0];
	        $objects[] = $obj_id;
	        $cnt++;
	    }
	    return array('cnt'=>$obj_cnt, 'ids'=>$objects);
	    
	}
	
	# get last index-pos1 of SUB_POS
	static function get_last_p1_of_SUB(object $sqlo, string $table, int $id, int $sub_pos) {
	    $last_index=0;
	    $sqlsel = 'SUB_POS_STR  from SATTACH where TABLE_NAME='.$sqlo->addQuotes($table). " and OBJ_ID=".$id.
	        " and SUB_POS_STR like ".$sqlo->addQuotes($sub_pos.',%').' order by REL_ID DESC';
	    
	    $sqlo->Quesel($sqlsel);
	    if ($sqlo->ReadRow() ) {
	        $SUB_POS = $sqlo->RowData[0];
	        $sub_arr=explode(',',$SUB_POS);
	        if ($sub_arr[1]>0) {
	            $last_index = $sub_arr[1];
	        }
	    }
	    return $last_index;
	}
	
	# get last index-pos1 of SUB_POS
	static function count_SUB_entries(object $sqlo, string $table, int $id, int $sub_pos) {
	    
	    $sqlsel = 'count(1)  from SATTACH where TABLE_NAME='.$sqlo->addQuotes($table). " and OBJ_ID=".$id.
	    " and SUB_POS_STR like ".$sqlo->addQuotes($sub_pos.',%');
	    $sqlo->Quesel($sqlsel);
	    $sqlo->ReadRow();
	    $cnt = $sqlo->RowData[0];
	    return $cnt;
	}
	
	
	/**
	 * 
	 * @param object $sqlo
	 * @param string $table
	 * @param int $id
	 * @param int $sub_pos
	 * @return array if SATTACH
	 */
	static function get_all_entries_of_SUB(object $sqlo, string $table, int $id, int $sub_pos) {
	    
	    $MAX_CNT=20;
	    $sqlsel_AFTER = 'from SATTACH where TABLE_NAME='.$sqlo->addQuotes($table). " and OBJ_ID=".$id. 
	    " and SUB_POS_STR like ".$sqlo->addQuotes($sub_pos.',%').' order by REL_ID';
	    
	    $objects = array();
	    $sqlsel='* '.$sqlsel_AFTER;
	    $sqlo->Quesel($sqlsel.' order by OBJ_ID');
	    $cnt=0;
	    $MAX_CNT=10;
	    while($sqlo->ReadArray() ) {
	        if ($cnt>=$MAX_CNT) break;
	        $objects[] =  $sqlo->RowData;
	        $cnt++;
	    }
	    return array('cnt'=>$obj_cnt, 'ids'=>$objects);
	}

}
