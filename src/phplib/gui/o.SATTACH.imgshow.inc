<?php
// * $Header: trunk/src/phplib/gui/o.SATTACH.imgshow.inc 59 2018-11-21 09:04:09Z $
require_once('o.SATTACH.subs.inc');

/**
 * show a thumbnail of the attachment
 * @package o.SATTACH.imgshow.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class oSATTACH_imgshow {
    
    private $table;
    private $id;

    function __construct($table=NULL, $id=NULL) {
        
    	$this->_KEY='ObjThumbnail';
    	$this->_MAXSIZE = 2000000; // bytes
    	$this->subLib   = new cSattachSubs();
    	$this->table= NULL;
    	$this->id   = NULL;
    	
    	if ($table!=NULL) {
        	$this->table=$table;
        	$this->id = $id;
    	}
    }
    
    function getThumbAttach( &$sqlo, $table, $id) {
    	$this->table=$table;
    	$this->id = $id;
    	$relid = $this->subLib->getRelIDbyKey( $sqlo, $table, $id, $this->_KEY );
    	return ($relid);
    }
    
    /**
     * get HTML-tag
     * @param $relid
     * @return string
     */
    function getImgTag($relid, $width=0) {
    	
    	if (!$width) $width=300;
    	
    	// check size; must be smaller than 1MB
    	$docpath = $this->subLib->getDocumentPath($this->table, $this->id, $relid);
    	if ( !file_exists($docpath)) {
    		$retval  = '<font color=red>Warning:</font> Attachment-file does not exist.';
    		return $retval;
    	}
    	
    	if (filesize($docpath)>$this->_MAXSIZE ) {
    		$retval  = '<font color=red>Warning:</font> Thumbnail-file too big (max '.$this->_MAXSIZE.' bytes).';
    		return $retval;
    	}
    	
    	$retval  = 
    		'<img src="obj.sattach.down.php?t='.$this->table.'&id='.$this->id.'&rel_id='.$relid.'"'.
    		' width='.$width.'>';
    	return $retval;
    }
	
}

