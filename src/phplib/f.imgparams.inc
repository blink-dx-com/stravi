<?php
/**
 * handle image thumbnail parameters
 *  FORMAT: $_SESSION['userGlob']["o.IMG.thumbnail_params"] = array[PROFILE] = array ("imgarr"=>$img_para_arr, "dim"=>$dim, "opt"=>$opt) 
  			PROFILE: "default", "galery", ...
 * @package f.imgparams.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class fImgParams {
	var $allparams;
	var $profiles;
	
function __construct() {
	
	$this->allparams = unserialize($_SESSION['userGlob']["o.IMG.thumbnailx"]);
	$this->profiles = array( "default"=>"default", "galery"=> "image gallery zoom" );
}

function getProfileParams($profile) {
	return ($this->allparams[$profile]);
}

function paramsSet( $profile, $opt, $img_para_arr, $dim ) {
	$savearr = array("imgarr"=>$img_para_arr, "dim"=>$dim, "opt"=>$opt );
	$this->allparams[$profile] = $savearr;
}

function saveDefParams() {
	
	$_SESSION['userGlob']["o.IMG.thumbnailx"] = serialize($this->allparams);
	$img_para_arr = $this->allparams["default"]["imgarr"];
	$_SESSION['userGlob']["o.IMG.thumbnail_params"] = $this->paramArray2str( $img_para_arr );
}

function paramArray2str( &$img_para_arr) { 
    $img_params = "";
	if (empty($img_para_arr)) {
		return;
	}
	
	foreach($img_para_arr as $th0=>$th1) {
		$xtraparam = "";
 		$tmp_str   = str_replace ( '"', '', $th0 );
		if ($th1 !="on" ) {
			// it is an parameter 
			// put them in high-commas to prevent bad system call intuders
			$xtraparam = "\"".$th1."\" ";   
        }
		if ($th1 !=""   ) $img_params = $img_params ." ". $tmp_str. " ".$xtraparam;

	}
  
    return  $img_params;
}

}