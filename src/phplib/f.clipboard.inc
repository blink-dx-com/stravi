<?PHP

/**
 * clipboard functions
 * 
 * - can be called without instancing
 * @package f.clipboard.inc
 * @swreq   UREQ:0000031: g > clipboard > HOME 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @var  $_SESSION['s_sessVars']["clipActCutProj"] -- contains current CUT-project
 * @var  $_SESSION['s_clipboard'] = array[] = array("tab"=>, "ida"=> "idb"=> "idc"=> )
 * @version $Header: trunk/src/phplib/f.clipboard.inc 59 2018-11-21 09:04:09Z $
 */
class clipboardC {

    // clear clipboard
    function resetx() {
    	
    	$_SESSION['s_clipboard'] = array();
    }
    
    // reset clipboard-pointer
    function reset() {
    	
    	if (sizeof($_SESSION['s_clipboard'])) reset ($_SESSION['s_clipboard']);
    }
    
    function &getClipBoard() {
    	
    	return ($_SESSION['s_clipboard']);
    }
    
    /**
     * get one object of table-type $tablename
     * @param string $tablename
     */
    static function obj_get ( $tablename ) {
      
    
      $found_id = 0;
      if (empty($_SESSION['s_clipboard'])) return;
      $tab_id   = NULL;
    
     
      foreach( $_SESSION['s_clipboard'] as $th0=>$th1) {
      
    	$tmp_tablename=current($th1);
    
    	$id0=next($th1);
    	//$id1=next($th1);
    	//$id2=next($th1);
    	if ( $tmp_tablename==$tablename ) {
    	  $found_id=$id0;
    	  $tab_id = $th0;
    	  break;
    	}
    	
      }
     
      return ( array ($found_id, $tab_id) );
    }
    
    /**
     * get all objects of table-type
     * @param string $tablename
     * @return array of $objectid
     */
    function getAllObjects( $tablename ) {
        
        if (!isset($_SESSION['s_clipboard'])) return;
        
        $num_obj    = sizeof($_SESSION['s_clipboard']);
        if (!$num_obj)    return;
       
        $objarr = array();
        foreach( $_SESSION['s_clipboard'] as $th1) {
    
    		$tmp_tablename=current($th1);
    	
    		$id0=next($th1);
    		//$id1=next($th1);
    		//$id2=next($th1);
    		if ( $tmp_tablename==$tablename ) {
    		    $objarr[] = $id0;
    		}
    	
        }
        return ( $objarr );
    }
    
    /**
     * get all objects , any table
     * @return array of array('t'=>, 'id'=>);
     */
    function getAll( ) {
        if (empty($_SESSION['s_clipboard']))    return;     
        $objarr = array();
        foreach( $_SESSION['s_clipboard'] as $th1) {
            
            $tmp_tablename=current($th1);
            $id0=next($th1);
            $objarr[] = array('t'=>$tmp_tablename, 'id'=>$id0);
        }
        return $objarr;
    }
    
    /**
     * get current clipboard element
     * - clipboard pointer increased !
     * @return array (table, $id0, $id1, $id2)
     */
    function getcurrElem() {		
    	$num_obj  = sizeof($_SESSION['s_clipboard']);
    	if (!$num_obj)  return;
    		
    	$th1 = current( $_SESSION['s_clipboard'] );
    	next( $_SESSION['s_clipboard'] ); // increment pointer ...
    	
    	if ($th1==NULL) return;
    	 
    	$tmp_tablename = current($th1);
    	$id0=next($th1);
    	$id1=next($th1);
    	$id2=next($th1);
    	return array($tmp_tablename, $id0, $id1, $id2);
    }
    
    /**
     * - put a number of objects to the clipboard
       - clear $_SESSION['s_sessVars']["clipActCutProj"]
     * @param  $tablename
     * @param  $idarr array[$objid] = 1
     * @param  int $add   if 1: add to clipboard
     */
    static function obj_put ( $tablename, $idarr, 	$add=0	) {
    	
    	$_SESSION['s_sessVars']["clipActCutProj"] = NULL;
    	if ($add<1) $_SESSION['s_clipboard'] = array();
    	
    	foreach( $idarr as $tmpid=>$dummy) {
    		$cliparr = array("tab"=>$tablename, "ida"=>$tmpid );
    		$_SESSION['s_clipboard'][] = $cliparr;
    	}
    	
    
    }
    
    function setCutProject($projid) {
    	
    	$_SESSION['s_sessVars']["clipActCutProj"] = $projid;
    }
    
    /**
     * put one object to the clipboard
     * @param  $tablename
     * @param  $objid
     */
    function obj_addone ( $tablename, $objid ) {
    	
    	
    	$_SESSION['s_clipboard'][] = array("tab"=>$tablename, "ida"=>$objid );
    }

}

