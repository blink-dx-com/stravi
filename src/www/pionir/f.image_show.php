<?php
/**
 *  provide functions to produce the thumbnail of an image 
       - output an image (JPG)
       - if $filename is a DFS-name like //clondiag.jena/dfs/test => convert it to the webserver-path e.g. /dfs/test/...
       - check, if file exists          
       - produce binary output of the image; this will be shown by the browser
   globals vars:
      $_SESSION['globals']['img_convert']
      $_SESSION['globals']['img.convert.jpg.tool'] : [0],1
      $_SESSION['userGlob']["o.IMG.thumbnail_params"]
      
 * @package f.image_show.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq SREQ:0001570: g > show image thumbnail of image on server 
 * @param 
 * 	$filename,
    $dim, [0]=x, [1]=y	"300x250" 
    $dim_ori = 1 # use original output size; but the system sets a max size: 2000x2000
	$img_params, [OPTIONAL]  urlencoded extra ImageMagick parameters; examples:
		-negate      : show image invers 	
		-rotate "45" : rotate the image	    
	$useDefParams = 0,1 use $_SESSION['userGlob']["o.IMG.thumbnail_params"] 
	
 */


// extract($_REQUEST); 
session_start(); 

require_once('globals.inc');
require_once('down_up_load.inc');

$MAX_IMG_LEN=2000;
 	   
$filename=$_REQUEST['filename'];
$useDefParams=$_REQUEST['useDefParams'];
$dim=$_REQUEST['dim'];
$img_params=$_REQUEST['img_params'];


$tmp_debug_lev  = $_SESSION['userGlob']['g.debugLevel'];


if ( $tmp_debug_lev > 0 ) {   
	echo "<B>DEBUG-LEVEL-INFO: level:".$tmp_debug_lev."</B><br>\n";
}

$img_type='';

$filename_upper=strtoupper($filename);
if (str_endswith($filename_upper, '.JPG')) {
    $img_type='JPG';
}

$use_jpg_tool=0;
if ($_SESSION['globals']['img.convert.jpg.tool']) {
    $use_jpg_tool=1;
}

//$img_extern_flag = 0;
//$set_hash 		 = 1;
//$retval   		 = 0;
if ( $useDefParams ) $img_params_prefs = isset($_SESSION['userGlob']["o.IMG.thumbnail_params"]) ? $_SESSION['userGlob']["o.IMG.thumbnail_params"] : NULL;

$filename_url_new = netfile2serverpath( $filename );

if ( $tmp_debug_lev > 0 ) {
	echo "DEBUG: Input_file_name: '".$filename."'<br>\n";
	echo "DEBUG: xx:try to load from file_url:'$filename_url_new' <br>\n";
}

$do_convert = 1; // default
		
if ( file_exists($filename_url_new) ) {
    
    if ($_REQUEST['dim_ori']>0) {
        
        $do_convert = 0;
        $path_parts = pathinfo($filename_url_new);
        $file_ext   = $path_parts['extension'];
        
        list($width, $height) = getimagesize($filename_url_new);
        
        if ($width>$MAX_IMG_LEN) {
            $width=$MAX_IMG_LEN;
            $do_convert = 1;
            
        }
        if ($height>$MAX_IMG_LEN) {
            $height=$MAX_IMG_LEN;
            $do_convert = 1;
        }
        
        $dim=array();
        $dim[0] = $width;
        $dim[1] = $height;
        
    }
    
    if ($do_convert) {
        
        if (!$dim[0] || !$dim[1]) {
            $dim[0]=350;
            $dim[1]=250;
        }
        
        //create a JPG now
        
        if ( $tmp_debug_lev <= 0 ) {
            Header("Content-type: image/jpg"); // set_mime_type('image/jpeg');
        }	
    
    	$dimstr= $dim[0]."x".$dim[1];
    	if ( $img_params=="" ) $img_params = $img_params_prefs; // if NOT set, $img_params take from preferences
    	if ( !$img_params || ($img_params_prefs==$img_params) ) {
    		$take_default = 1; // if (no preferences or the parameters are equal to prefernces) 
    	} 
    					
    		
    	$ori_gostr= "\"".$_SESSION['globals']['img_convert']. "\" -geometry ". $dimstr." " .$img_params;  // > /tmp/0test.jpg
    	
    	// Escape shell metacharacters, due to user input
    	if ($img_type=='JPG' and $use_jpg_tool) {
    	    $dimstr2= $dim[0]." ".$dim[1];
    	    $gostr  = 'djpeg "'. $filename_url_new . '" | pnmscale -xysize '.$dimstr2.' | cjpeg >&1';
    	} else {
    	    $gostr  = escapeshellcmd($ori_gostr) . ' "'. $filename_url_new . '" jpg:- '; 
    	}
    	
    	if ($_SESSION['userGlob']["g.debugLevel"]>2) {
    		echo "DEBUG: Convert command(2):  $gostr<BR>\n";
    	}
    	$return_var=NULL;
    	passthru ( $gostr , $return_var);
    	
    } else {
        
        if ( $tmp_debug_lev>0 ) {
            echo "<B>DEBUG: NO convert. Size: ".$width."x".$height."<br>\n";
        } else {
            Header("Content-type: image/'.$file_ext.'");
        }	
        readfile ($filename_url_new);   
    }
			
} else {
	 
	if ( $tmp_debug_lev > 0 ) {
		echo "<B>DEBUG: file not found.<br>\n";
	}  else {
		set_mime_type('image/gif');
	}
	readfile ("images/ic.dbno.gif");   

}

