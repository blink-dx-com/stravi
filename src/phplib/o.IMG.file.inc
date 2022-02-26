<?php
/**
 * sub function of object IMG, related to attached file
 * $Header: trunk/src/phplib/o.IMG.file.inc 59 2018-11-21 09:04:09Z $
 * @package o.IMG.file.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
require_once ('glob.obj.touch.inc');

class oIMG_fileC {
	/**
	 * get full PATH of image-file on server
	 * @param $img_id
	 * @return unknown_type
	 */
	static function imgPathFull( $img_id ) {

		return  ( $_SESSION['globals']["img_path"]. "/Org_" . $img_id . ".tif" ) ;
	} 
	
	/**
	 * name is an URL ?
	 * @return unknown_type
	 */
	function nameIsUrl($name) {
		$isURL=0;
		if ( strstr($name, '/')!=NULL ) $isURL=1;
		if ( strstr($name, "\\")!=NULL ) $isURL=1;
		return ($isURL);
		
	}
	
	/**
	 * image exists on server ?
	 * @param $img_id
	 * @return int $exists 
	 * 		0 - image does not exist
	 *      1 - image exists  
	 */
	function onServerExists( $img_id ) {
	// RETURN: 
	    $retval = 0;   
	    $full_name = $this->imgPathFull( $img_id );    
	    if ( file_exists($full_name) ) {    
	         $retval = 1;
	         
	    }
	    return ( $retval );
	}
	
	/**
	 * image exists on NET ?
	 * @param $netname
	 * @return int $retval
	 * 		0 - image does not exist
	//      1 - image exists
	 */
	function imgOnNetExists( $netname ) { 
	    
	    $retval = 0;   
	    $serverName = netfile2serverpath( $netname );  
	    
	    if ( file_exists( $serverName ) ) {    
	         $retval = 1;
	    } 
	    if ( $_SESSION['userGlob']["g.debugLevel"]>2 ){
	        echo "DEBUG:imgOnNetExists(): net:'$netname' server:'$serverName'<br>\n";
	        echo "exists?: ".$retval."<br>\n";
	    }
	    
	    return ( $retval );
	} 
	
	/**
	 * delete file and touch object
	 * @param $sqlo
	 * @param $img_id
	 * @return unknown_type
	 */
	function deleteFile(&$sqlo, $img_id) {
		global $error;
		$FUNCNAME= 'deleteFile';
		
		$full_name = $this->imgPathFull( $img_id );
		if ( unlink ($full_name) !== TRUE ) {
         	$error->set( $FUNCNAME, 1, 'can not delete image-file of IMG-ID:'.$img_id );
         	return;
        }
        
        $touch_lib = new globObjTouch();
        $actarr = array('d'=>array( 'x'=>array('atx'=>NULL) ), 'a'=>array('key'=>'del'));
        $touch_lib->touch ( $sqlo, 'IMG', $img_id, $actarr );
        return 1;
	}
		
}

