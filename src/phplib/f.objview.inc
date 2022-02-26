<?php

/**
 * object feature view functions, generate links to "object single form" edit.tmpl.php; 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @package f.objview.inc
 */
class fObjViewC {
  
    // @param $addPath example: '../lab/'
    function __construct($addPath=NULL) {
      $this->addPath = $addPath;
    }
    	
    // get icon of object
    function _getIcon($tablename) {
    	return self::getIcon($tablename, $this->addPath);
    }
    
    // get icon of object [STATIC]
    static function getIcon($tablename, $addPath='') {
        $icon     = file_exists($addPath."images/icon.".$tablename.".gif") ?
        $addPath."images/icon.".$tablename.".gif":
        $addPath."images/icon.UNKNOWN.gif";
        return ($icon);
    }
    
    // show just an link to "single object form": shown link-text: ID or NAME (if given)
    static function bo_IDLink( $tablename, $id, $name=NULL) {
    	$addPath  = '';
    	$showname = $id;
    	if ($name!=NULL) $showname=$name;
    	$htmlText =  "<a href=\"".$addPath."edit.tmpl.php?t=".$tablename."&id=".$id."\" >".$showname."</a>";
    	return ($htmlText);
    }
    
    /* get URL of object
     * @param $tablename
     * @param $id
     * @return string $url
     */
    static function bo_ID_URL( $tablename, $id, $id2=NULL, $id3=NULL) {
    	$url =  "edit.tmpl.php?t=".$tablename."&id=".$id;
    	if ($id2) {
    	    $url .= '&primasid[1]='.$id2;
    	}
    	if ($id3) {
    	    $url .= '&primasid[2]='.$id3;
    	}
    	return ($url);
    }
    
    /**
     * get NICE name of object
     * @param object $sqlo
     * @param string $tablename
     * @param int $id
     * @return string
     */
    static function bo_nice( $sqlo, $tablename, $id) {
        $nice_name = obj_nice_name($sqlo, $tablename, $id);
        return $nice_name;
    }
    
    /**
     * get edit-link of BO + NAME + icon 
     * @param $sql
     * @param $tablename
     * @param $id
     * @param $opts 
     *   "relPath" => "", "LAB"
    	 "format"  => ["html"] : normal link
    				     "csv"   : print only the object-name !!! (no HTML-tags)
    	 "noIcon"  => 0,1
    	 "absName" => 0,1     : show also abstract name
    	 "showID"  => 0,1     : show also ID of object
    	 "url_xparam" => string : extra URL parameter; e.g. 'xmode=x'
     * @return $htmlText
     */
    static function bo_display( &$sql, $tablename, $id, $opts=NULL	) {
    	$htmlText = "...???...";
    	
        if ($tablename=="" || !is_numeric($id) ) {
            return ($htmlText);
        }
    	
    	$addPath = "";
    	if ($opts["relPath"]=="LAB") $addPath = "../pionir/"; 
                            
        $nice_name = tablename_nice2($tablename);
    	
    	if ($nice_name!="") {
       		$nicopt=NULL;
    		if ($opts["absName"]) $nicopt["absName"] = $opts["absName"];
            $tmp_name = obj_nice_name($sql, $tablename, $id, $nicopt);
    		
    		if ($opts["format"]=="csv") {
    			$htmlText = $tmp_name . " [ID:".$id."]";
    		} else {
    			$iconTag = '';
    			if (!$opts["noIcon"]) {
    				$icon     = file_exists($addPath."images/icon.".$tablename.".gif") ? 
    						$addPath."images/icon.".$tablename.".gif":
    						$addPath."images/icon.UNKNOWN.gif";
    				$iconTag = '<img src="'.$icon.'" border=0>';
    			}
    			if ($opts["showID"]>0) $tmp_name .= ' [ID:'.$id.']';
    			$url_param="t=".$tablename."&id=".$id;
    			if ($opts["url_xparam"])  $url_param .= '&'.$opts["url_xparam"];
    			$htmlText =  '<a href="'.$addPath.'edit.tmpl.php?'.$url_param.'" >'.$iconTag. ' '.$tmp_name.'</a>';
    		}
        }     
        return ($htmlText);
    }
    
    
    /**
     * get nice presentation of BO + NAME + icon 
     * @param $sql
     * @param $tablename
     * @param $id
     * @param $opts 
    	 "format"  => ["html"] : normal link
    				     "csv"   : print only the object-name !!! (no HTML-tags)
    	 "noIcon"  => 0,1
    	 "absName" => 0,1     : show also abstract name
     * @return $htmlText
     */
    function bo_display2( &$sqlo, $tablename, $id, $opts=NULL 	) {
    	$htmlText = "...???...";
    	$addPath='';
    	
        if ($tablename=="" || !is_numeric($id) ) {
            return ($htmlText);
        }                 
        $nice_name = tablename_nice2($tablename);
    	
    	if ($nice_name=="") {
    		return ($tablename.':[ID:'.$id.']');
    	}
    	
    	$nicopt=NULL;
    	if ($opts["absName"]) $nicopt["absName"] = $opts["absName"];
        $tmp_name = obj_nice_name($sqlo, $tablename, $id, $nicopt);
    	
    	if ($opts["format"]=="csv") {
    		$htmlText = $tmp_name . " [ID:".$id."]";
    	} else {
    		$iconTag = '';
    		if (!$opts["noIcon"]) {
    			$icon     = file_exists($addPath."images/icon.".$tablename.".gif") ? 
    					$addPath."images/icon.".$tablename.".gif":
    					$addPath."images/icon.UNKNOWN.gif";
    			$iconTag = '<img src="'.$icon.'" border=0>';
    		}
    		$htmlText =  $iconTag." ".$tmp_name;
    	}
          
        return ($htmlText);
    }
    
    
    /**
     * show link to list-view
     * info: LIST-icon: images/but.list2.gif
     * @param string $table
     * @param array $opt
     *   'url.params' : string : e.g. 'condclean=1'
     * @return string
     */
    function tableViewLink($table, $opt=array() ) {
    	
    	$addPath = $this->addPath;
    	$nice_name = empty($table) ? "" : tablename_nice2($table);	
    	$icon  = $this->_getIcon($table);
    	$ext_param = $opt['url.params']!=NULL ? '&'.$opt['url.params'] : '';
    	
    	
    	$htmlText =  "<a href=\"".$addPath."view.tmpl.php?t=" . $table . $ext_param . "\" ><img src=\"$icon\" border=0> $nice_name</a>";
    	return ($htmlText);
    }

}
