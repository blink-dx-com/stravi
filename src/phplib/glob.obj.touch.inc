<?php
/**
 * - touch an object (write simple action-info to CCT_ACCESS)
 * - doeas NOT do the ADVMOD (advanced mod log), managed in glob.obj.advmod.inc 
 * - manages tables: 
 *   CCT_ACCESS  - the current object status
 *   CCT_ACC_UP  - the full accmodification  log
 * structure in database: see document 1002_SDS_code : touch_row : touchRow_INFO_STRUCT
 * changes:
 *   #touch_row:IF_CHANGED
 * @package glob.obj.touch.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
 class globObjTouch {
 
 	/**
 	 * @deprecated
 	 * get the text
 	 * - special keywords:
 	 * 'txt': value is a STRING: will be surround by double quotes:  "text"
 	 * @param array touchRow_INFO_STRUCT $actarr
 	 * @param $opts
 	 */
 	static function getTouchText($actarr, $opts=NULL) {
 		$text=NULL;
		$mainKeys=array('d','a','x');
		
		foreach( $mainKeys as $dummy=>$mainkey) {
			if ( $actarr[$mainkey]!=NULL ) {
				$text .= '<'.$mainkey.':'.$actarr[$mainkey]['key'];
				$moreData = &$actarr[$mainkey]['data'];
				
				if (is_array($moreData)) {
					foreach( $moreData as $key=>$val) {
					    if ($key=='txt') {
					        $val = str_replace('"', '\\"', $val);
					        $val='"'.$val.'"';
					    }
						$text .= ' '.$key.':'.$val;
					}
					reset ($moreData); 
					
				}
				$text .= '>';
			}
		}
		
		
		return $text;
 	}
 	
 	/**
 	 * touch BO ( change modification time and user )
 	 *
 	 * - no right check !!!
 	 * - if object is not a BO, just return: -1
 	 *
 	 * @param object &$sql, 	  sql handle
 	 * @param string $tablename  name of table
 	 * @param int $id 	      id of object
 	 * @param string $reason=NULL (see external-document)
 	 * @return array ('up'=>0,1, 'timest'=>unix time stamp)
 	 *   - old return type was int (till 15.01.2013)
 	 * @see  file://CCT_QM_doc/89_1002_SDS_code.pdf
 	 */
 	private static function _touch_row ( &$sql, $tablename, $id, $reason=NULL ) {
 	    $ret = -1;
 	    
 	    if ( !cct_access_has2($tablename) ) return ( array('up'=>$ret, 'timest'=>0 ) );
 	    $cct_access_id = cct_access_get($sql, $tablename, $id);
 	    if (!$cct_access_id) return array(array('up'=>$ret, 'timest'=>0 ));
 	    $addInfo=NULL;
 	    
 	    if ($reason!=NULL) $addInfo = ", UPINFO=".$sql->addQuotes($reason);
 	    
 	    $timeStamp = time();
 	    
 	    $ret = $sql->query( 'update cct_access set mod_date='.$sql->Timestamp2Sql($timeStamp).
 	        ', DB__DB_USER_ID='.$_SESSION['sec']['db_user_id'].$addInfo.
 	        ' where cct_access_id = '.$cct_access_id);
 	    $retarr = array('up'=>$ret, 'timest'=>$timeStamp );
 	    
 	    return ($retarr);
 	}
 	
	 /**
	 * do a touch of an object
	 * @param  array touchRow_INFO_STRUCT $actarr : array 
	 *		'd' = array('key'=>'ass', 'data'=>array(key,val) )
	 *		'a' = array('key'=>'ass', 'data'=>array(key,val) )
	 *		...
	 * @return array ('up'=>0,1, 'timest'=>unix time stamp)
	 */
	static function touch( &$sqlo, $table, $objid, $actarr, $opts=NULL ) {
		
		// $text = self::getTouchText($actarr, $opts);
	    $text   = json_encode($actarr);
		$retarr = self::_touch_row( $sqlo, $table, $objid, $text );
		return $retarr;
	}

}