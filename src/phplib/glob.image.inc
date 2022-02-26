<?php
/**
 * - sub functions for image visualization
 * - please use package o.IMG.file.inc for future
 * $Header: trunk/src/phplib/glob.image.inc 59 2018-11-21 09:04:09Z $
 * @package glob.image.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */

require_once ("down_up_load.inc");

function imgPathFull( $img_id ) {
	/*FUNCTION: get full PATH 
	*/ 
	
	return  ( $_SESSION['globals']["img_path"]. "/Org_" . $img_id . ".tif" ) ;
} 

function imgVarioGetSize( 
	$imgOrgname,  // can be an image somewhere
	$option=null ) {
	// RETURN: $sizearr[] [0]=x, [1]=y
     
    $sizearr = NULL;
    
    $tmpdebug = $_SESSION['userGlob']["g.debugLevel"];
    if ($option["debug"]>0) $tmpdebug = 3; 
    
    $gostr  = $_SESSION['globals']['img_identify']. " -ping \"". $imgOrgname."\"" ;
    $answer = shell_exec ( $gostr );      
    
    if ( $tmpdebug>2 ) {
        echo "<B>DEBUG: imgGetSize</B> img:$imgOrgname [$answer]<br>";     
    }
    
   
    $sizename = strlen($imgOrgname);
    if ($answer==NULL) return (NULL);
     
    $pingstr = substr($answer,$sizename); // $pingstr: first CHAR is "WHITE SPACE"
    if ($pingstr=="") return (NULL);
    $pingarr = explode (" ",$pingstr);
    if ($pingarr[2]=="") return (NULL);
    $sizearr = explode ("x",$pingarr[2]);
    
    $tmppos = strpos($sizearr[1],"+");  // test for "300x400+20+3" or so
    if (strpos($sizearr[1],"+")) $sizearr[1] = substr($sizearr[1],0,$tmppos);
    
    return ($sizearr);
}  

function imgGetSize( $id, $option=null ) {
	// RETURN: $sizearr[] [0]=x, [1]=y
     
    $sizearr = NULL;
    
    $tmpdebug = $_SESSION['userGlob']["g.debugLevel"];
    if ($option["debug"]>0) $tmpdebug = 3; 
    $imgOrgname = imgPathFull($id);
	
    $sizearr = imgVarioGetSize( $imgOrgname, $option);
    return ($sizearr);
}  

function imgOnServerExists( $img_id ) {
// RETURN: 0 - image does not exist
//         1 - image exists  
    $retval = 0;   
    $full_name = imgPathFull( $img_id );    
    if ( file_exists($full_name) ) {    
         $retval = 1;
         
    }
    return ( $retval );
}

function imgOnNetExists( $netname ) {
// RETURN: 0 - image does not exist
//         1 - image exists  
    
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

function this_paramsCheck( &$img_params) {  
// FUNCTION: check parameters against hacker attacks in the form of:
//              -option 45x34; ls -l /etc
//          - the function allows the form: -OPTION "PARAM"
     $info = "";   
     $params = trim($img_params);
     
     if (!strlen($params)) return ($info);
     if (substr($params,0,1)!="-") return ("First character must be a '-'; identifying an option name like '-swirl'"); 
     
     // separate by "-" form the option '-dark "creme" ' 
     $tmparr = explode("-", $params);
     if ( sizeof($tmparr) ) {  
        foreach( $tmparr as $dummy=>$tmpstrBefore) {
            $tmpstr = trim($tmpstrBefore);
            $tmppos = strpos($tmpstr," ");
            if ( $tmppos!==false) { // parameter exists ?
                $tmpstrPar = trim(substr($tmpstr,$tmppos+1)); // the rest is real parameter
                // echo "debug:this_paramsCheck():   '$tmpstrPar' decode".urlencode($tmpstrPar)."<br>";
                $tmplen =  strlen($tmpstrPar); 
                if (substr($tmpstrPar,0,1)!='"' || substr($tmpstrPar,$tmplen-1,1)!='"') return ("Parameters must be in 'double quotes' for -".substr($tmpstr,0,$tmppos).": '".$tmpstrPar."'");
                if (strstr(substr($tmpstrPar,1,$tmplen-2),'"') !="") return ("Parameters must not contain 'double quotes': param:'".$tmpstrPar."'");
            }
        }
     
     }
     return $info;
}
/**
 * 
 * @param string $imgOrgname
 * @param string $imgCacheName   name without preface "i."; e.g. "SATTACH.3456.2"
 * @param array $dimension target dimension
 * @param array $maxOridim    maximum original size for copy as thumbnail
 * @param array $options
 * @return number
 */
function imgVarioShow( string $imgOrgname, string $imgCacheName, $dimension = NULL, $maxOridim = NULL, $options = NULL ) {
	
	$_debug=$_SESSION['userGlob']["g.debugLevel"];
	$MAX_SIZE = 700;
	$createit = 0;
	
	// FUNCTION: produce / show thumbnail of an image

	if ( !file_exists($imgOrgname) ) {
	
		 echo "<br>";
		 echo "&nbsp;&nbsp;<img src=\"images/ic.dbno.gif\"> <font color=gray>Image not on server.</font>\n";
		 $retval = -1;
		 return ($retval);
	} 
	 
	do {
		
		if (!$dimension[0] || !$dimension[1]) {
			$dimension[0]=350;
			$dimension[1]=250;
		}
		
		
		if ($dimension[0]> $MAX_SIZE OR $dimension[1]> $MAX_SIZE) {
		    $dimension[0]=$MAX_SIZE;
		    $dimension[1]=$MAX_SIZE;
		}
		
		$dimstr= $dimension[0]."x".$dimension[1];

		if ( $options["refresh"] )  $createit = 1;
		
		$cacheImgFile = $_SESSION['globals']['http_cache_path'] ."/i.". $imgCacheName."." .$dimstr .".jpg";
		if ($_debug>1) echo 'DEBUG: cacheImgFile:'.$cacheImgFile.'.<br>';
		$thumb_exists = file_exists($cacheImgFile);
		if ( !$thumb_exists ) {
		    if ($_debug>1) echo 'DEBUG: cacheImgFile not exists.<br>';
		    $createit=1;
		}
	
		if ( $createit ){
			// TBD: CACHE clear must be introduced !!! 
			// no PNG exists or create anyway ... 
			if ($maxOridim[0]>0 AND $maxOridim[1]>0) {
			    
				$dimIs = imgVarioGetSize( $imgOrgname );
				if ($_debug>1) echo "imgVarioShow copy?: MAX:".$maxOridim[0]."x".$maxOridim[1]." ORI:".$dimIs[0]."x".$dimIs[1]." <BR>";
				if ($maxOridim[0]>=$dimIs[0] AND $maxOridim[1]>=$dimIs[1]) {
					// just copy the image, NO resize ....
					
					copy ($imgOrgname, $cacheImgFile);
					break;
				}
			} 
			
			$gostr= '"'.$_SESSION['globals']['img_convert']. '" -geometry '. $dimstr.'  "'. $imgOrgname .'" "'. $cacheImgFile. '"' ;
			$dummy=NULL;
			$answer =  system ( $gostr , $dummy);
			if ($_debug>1) echo "Image thumbnail 'create' called! <BR>";
			if ($_debug>2) {
				echo "DEBUG: Convert command:  $gostr<BR>";
			}
		} else {
		    if ($_debug>1) echo "Image cacheImgFile not created, should exist.<BR>";
		}

		if ( !file_exists($cacheImgFile)  ){
		    if ($_debug>1) echo "Image cacheImgFile not_exists.<BR>";
			$cacheImgFile=$_SESSION['s_sessVars']["loginURL"]."/images/ic.img_unknown.gif";
			break;
		}
	} while (0);
		
	$dummy = rand();
	echo "<img src=\"". $cacheImgFile. "?dummy=$dummy\">\n";
				

}


function org_img_show ( 
	&$sql,
	$img_id,
	$dimension,	 // can be empty, [0]=x [1]=y    e.g.300,200
				 // can be overwritten by $options["sizeOri"]
	$img_params, /* must be in the form -OPTION "PARAMETER"
                 e.g.
			   = INVERS   show image invers 	
			   = NULL    no parameters */
	$name,	    /* name = optional URL of image  */
	$options    /* 
			"set_default" = 1 set as default thumbnail
			"noImageLogo" = 0|1
			"noImgBotTxt" = 0|1
			"refresh"	  = 0|1 refresh thumbnail
			"show_ref_points" =0|1 show ref spot positions (SPOT_ID<0), set in IMG.SPOT_POS_IN_IMG
			"show_mem_points" = array[SPOT_ID]=array(x,y) :: show crosses
			"segimg"    0|1 : try to show segmentation image SATTACH:KEY="SEG"
						- sets "sizeOri" = 1 (can only show this feature in original size)
			"sizeOri"   "" : use $dimension
					     1 : set size to original size, overwrite $dimension
			   */
	 ) {
/*FUNCTION: show thumbnail of image
	1. check, if thumbnail exists
	2. create thumbnail (produce different names for different parameter images)
	
	- if user has NO WRITE-permission: can produce thumb, if not exists
*/	   
	
	$MAX_SIZE = 2000; // max pixels show
	
	$imgOrgname = imgPathFull( $img_id );
	
	$_debug=$_SESSION['userGlob']["g.debugLevel"];
    
    if ( $_debug>2 ) {
        echo "<B>DEBUG: org_img_show ()</B> IMG_ID:$img_id param_dim:$dimension <br>\n";
        if (sizeof($options)) {
            echo "Parameters: \n";
            print_r($options);
            echo "<br>\n";
        }
    }

	if ($options["segimg"]) $options["sizeOri"]=1; // need to set size=ORI !
	$noImageLogo           = !empty($options["noImageLogo"]);
	$img_params_setdefault = !empty($options["set_default"]);
	
	$create_anyway   = 0;
	$img_extern_flag = 0;
	$set_hash 	     = 1;
	$retval   		 = 0;	
	$fileImgBadPar   = $_SESSION['s_sessVars']["loginURL"]."/images/ic.img_badpar.gif";
	
	$img_params_prefs = isset($_SESSION['userGlob']["o.IMG.thumbnail_params"]) ? $_SESSION['userGlob']["o.IMG.thumbnail_params"] : NULL;
	
	if ( !$noImageLogo ) {
		echo '<a href="edit.tmpl.php?t=IMG&id='.$img_id.'">';
		echo "<img src=\"images/icon.IMG.gif\" border=0> Image ". $img_id ."</a>";
        echo "<br>\n";
	}
		 
	if ( file_exists($imgOrgname) ) {
	  do {		
	  
	  		if ($options["sizeOri"]>0) {
				$dimension = imgGetSize( $img_id );
			} else {
				if (!$dimension[0] || !$dimension[1]) {
					$dimension[0]=350;
					$dimension[1]=250;
				}
			}
			if ($dimension[0]> $MAX_SIZE OR $dimension[1]> $MAX_SIZE) {
				 echo "<b><font color=red>Error</font></b> image dimensions to big:".$dimension[0]."x".$dimension[1]."<br>\n";
				 $cacheImgFile = $fileImgBadPar;
				 break;
			}
			
            $dimstr= $dimension[0]."x".$dimension[1];

            if ( $img_params=="" ) $img_params = $img_params_prefs; // if NOT set, $img_params take from preferences
            if ( $img_params==""  ) { // OLD: || ($img_params_prefs==$img_params)
                $take_default = 1; // if (no preferences or the parameters are equal to prefernces) 
            } 
            if ( ($img_params!="") && $img_params_setdefault) {
                $create_anyway = 1; // overwrite old thumbnail
                $take_default  = 1; // take as default image
            }
            if ( $options["refresh"] )  $create_anyway = 1;

            // CHECK the default thumbnail, if needed
            $cacheImgDefFile = $_SESSION['globals']['http_cache_path'] ."/img". $img_id . ".dim" .$dimstr  .".jpg"; 

            if ( $take_default && $create_anyway ) { 
                if ( file_exists($cacheImgDefFile) ) { // if exists, check PERMISSION
                    $o_rights = access_check($sql, 'IMG', $img_id);
                    if( !$o_rights['write'] ){
                        $take_default=0;
                        echo "<font color=red><b>WARNING:</b></font> No accession-right to set as default thumbnail.<br>";
                    }
                }
            }
            
            // chack for hacker attack !!!
            $tmpret = this_paramsCheck($img_params);
            if ($tmpret!="") { // WARNING: hacker attack !!!
                echo "<b><font color=red>Error</font></b> during thumbnail parameter check: ".$tmpret."<br>\n";
                echo "<b><font color=gray>Hint:</b> Please change your default settings in the function 'image beauty farm'</font><br>\n";  
                $cacheImgFile = $fileImgBadPar;
                break;
            }
            
            /* to produce different names for different parameter images */
            if ( $take_default ) {
                $param_hash = "";
            } else {
                $param_hash = md5($img_params);
                $param_hash = "." . substr($param_hash, 0, 5);
            }

            $act_time=filemtime($imgOrgname); // OLD: came after $img_id

            $cacheImgFile=$_SESSION['globals']['http_cache_path'] ."/img". $img_id . $param_hash. ".dim" .$dimstr  .".jpg";
            $thumb_exists = file_exists($cacheImgFile);

            if ( $thumb_exists ) {
                $act_time_thumb = filemtime($cacheImgFile);
                if ($act_time>$act_time_thumb) $create_anyway = 1; // ORIGINAL image newer than thumbnail
            }
			
			if ($_debug>1) echo "DEBUG: thumbnail_exists:".$thumb_exists."  thumbnail_create:".
					$create_anyway." create_params: ".htmlspecialchars($img_params)."<br>";

            if ( !$thumb_exists  || $create_anyway ){
                // TBD: CACHE clear must be introduced !!! 
                // no PNG exists or create anyway ... 
                // check parameters for hacker attacks
                $gostr= '"'.$_SESSION['globals']['img_convert']. '" -geometry '. $dimstr.' ' .$img_params.' "'. $imgOrgname .'" "'. $cacheImgFile. '"' ;
                $answer =  system ( $gostr , $tartra);
                if ($_debug>1) echo "Image thumbnail 'create' called! <BR>";
                if ($_debug>2) {
                    echo "DEBUG: Convert command:  $gostr<BR>";
                }
            }

            if ( !file_exists($cacheImgFile)  ){
                $cacheImgFile=$_SESSION['s_sessVars']["loginURL"]."/images/ic.img_unknown.gif";
                break;
            } 
	    
	    	$im = NULL; // image object for further calculation
			$cacheImgFileSave = $cacheImgFile;
			$show_mem_showSpotids = 1;
			
            if ( $options["show_ref_points"] ) {
				$spotmem = NULL;
				$sqls = "SELECT SPOT_ID, X_POS, Y_POS FROM SPOT_POS_IN_IMG WHERE IMG_ID=$img_id AND SPOT_ID<0";
				$sql->query($sqls); // get reference spots
				while($sql->ReadRow()) {
					$spotmem[$sql->RowData[0]] = array($sql->RowData[1], $sql->RowData[2]);
				}
				$show_mem_showSpotids = 0;
				$options["show_mem_points"] = &$spotmem; //
            }
			
			if ( $options["show_mem_points"]!=NULL ) {
				// show additional spots (RED)
                do {
					$ok=0;
					if ( ($im = @imagecreatefromjpeg ($cacheImgFile)) == NULL) break;
					
					$x=imagesx ( $im );
					$y=imagesy ( $im );
					$im_ori_size = getimagesize($imgOrgname);
					$ox = $im_ori_size[0];
					if ($ox<=0) break;
					if ( !($colred = imagecolorallocate( $im, 255,0,0)) ) break;
	
					$fac_t2o = $x / $ox;
					
					foreach( $options["show_mem_points"] as $spotid=>$xyarr) {
						$x_tmp = (int)($xyarr[0] * $fac_t2o);
						$y_tmp = (int)($xyarr[1] * $fac_t2o);
						imagerectangle( $im, $x_tmp-2, $y_tmp-2, $x_tmp+2, $y_tmp+2, $colred );
						if ($show_mem_showSpotids) imagestring( $im, 1, $x_tmp+2, $y_tmp, $spotid, $colred ); // print SPOT_ID
					}
					reset($options["show_mem_points"]);
					
					$cacheImgFile = $_SESSION['globals']['http_cache_path'] ."/img". $img_id . $param_hash. ".dim" .$dimstr. 
								    ".mem.jpg";
					$ok = 1;
					// keep $im, show at finish of while()
					
				} while (0);
				if (!$ok) {
					$im = NULL;
				}
            }
	    
			if ($options["segimg"]) { // SEGMENTATION image ?
				// only max size image ...
				require_once ("o.SATTACH.subs.inc");
				
				do {
					$ok=0;
					if ( !$im ) $im = @imagecreatefromjpeg ($cacheImgFile);
					if ( !$im ) break;
							
					$pct = 20;
					$x = imagesx ( $im );
					$y = imagesy ( $im );
					$im_ori_size = getimagesize($imgOrgname);
					
					
					// load segimage
					$segimgRel   = cSattachSubs::getRelIDbyKey($sql, "IMG", $img_id, "SEG");
					if (!$segimgRel) break;
					$segimPath = cSattachSubs::getDocumentPath("IMG", $img_id, $segimgRel);
					if ( !file_exists($segimPath) ) break;
					$imseg   = @imagecreatefrompng ($segimPath);
					$colorid = imagecolorclosest ( $imseg, 255, 0, 0 ); // get index for black
					
					
					$i=0;
					$segNumCols = 2;
					$memcol[0] = array(255,0,0);
					$memcol[1] = array(0,0,255);
					$colornew = NULL;
					// transfer PNG-colors ==> original image
					while ($i<$segNumCols) {
						 $tmpcolor = $memcol[$i];	
						 $rgb  = ($tmpcolor[0]<<16) + $tmpcolor[1]*256 + $tmpcolor[2]; 
						 $colind = imagecolorallocate( $im, $tmpcolor[0],$tmpcolor[1],$tmpcolor[2] );
						 $colornew[$rgb] = $colind;
						 $i++;
					}

					
					$yr=0;
					while ($yr<$y) {
						$xr = 0;
						while ($xr<$x) {
							$color_index = imagecolorat($imseg, $xr, $yr);
							if ($color_index>0) { 
								$newind = $colornew[$color_index];
								imagesetpixel ( $im, $xr, $yr, $newind );
							}
							$xr++;
						}
						$yr++;
					}
					
					$cacheImgFile = $_SESSION['globals']['http_cache_path'] ."/img". $img_id . $param_hash. ".dim" .$dimstr.".seg.jpg";
					$ok = 1;
				} while (0);
				
			}
	    
			if ($im != NULL) {
				if ($_debug>1) echo "DEBUG: save image '$cacheImgFile' to cache.<br>\n";
				Imagejpeg( $im, $cacheImgFile ); // save image to cache
				ImageDestroy($im);
			} else {
				$cacheImgFile = $cacheImgFileSave;
			}
	    
        } while (0);
		
		$dummy = rand();
		echo "<img src=\"". $cacheImgFile. "?dummy=$dummy\" ";
		echo " TITLE=\"show:".$img_params."\">"; /* add dimension information due to Browser caching */ 
		echo "</A><BR>\n";
				
	 } else {
		 echo "<br>";
		 echo "&nbsp;&nbsp;<img src=\"images/ic.dbno.gif\"> <font color=gray>Not on image-server.</font><BR>\n";
		 $retval = -1;
	 }
		
	if ( $name !="" AND $options["noImgBotTxt"]!="1" ) {
		$imgOrgname = $name;
		$imgOrgnameDo = $imgOrgname;
		if ( strchr($imgOrgname, ":") == "" ) {
			$imgOrgnameDo = "file://" .$name;
		}
		$lener=40;
		$imgOrgnameShow=$imgOrgname;
		if (strlen ($imgOrgnameShow)>$lener ) {
			$start = strlen ($imgOrgnameShow) - $lener;
			$imgOrgnameShow="...". substr($imgOrgnameShow, $start, $lener);
		}
		echo "&nbsp;&nbsp;<img src=\"images/earth.gif\" TITLE=\"extern URL\"> External URL ".
		     "<a href=\"". $imgOrgnameDo. "\" target=help>$imgOrgnameShow</a>";
		echo "<br>\n";
	}


		
	return 0;
}       

function image_series_get( $img_body, $extra_values) {          
	// RETURN: array[] = array(image_file_name, offset_time) 
    require_once ("o.IMG.c_series.inc");
    
    $errtext = "";
    $retval = 0;
    $image_arr = array();
    
    $values     = &$extra_values["values"];
    $checkit=array(); 
    $checkit[]="start-number";
    $checkit[]="number";
    $checkit[]="name-extension";

    // tmp_info_out("Image name body",$img_body);
    foreach( $checkit as $th0=>$th1) { 
   
        $indexer = $th1;
        $this_val = $values[$indexer];

        if ($this_val =="") {
            $errtext = $errtext . $indexer. ": need a value !\n";
            return array(-1, $errtext, NULL);
        }
    }
    // tmp_info_out("offset-time", $values["offset-time"]);

    if ($values["start-number"]<"0") {
        $errtext = "start-number must be >=0.";
        return array(-1, $errtext, NULL);
    }

    if ( $values["number"] <= 0 ) {
        $errtext = "number of images must be >0.";
        return array(-1, $errtext, NULL);
    }

    if ( $values["name-period"] != (int)$values["name-period"] ) {
        $errtext = "name-period must be an INTEGER.";
        return array(-1, $errtext, NULL);
    }

    if ( !$values["offset-time"] ) {
        $values["offset-time"]=0;
    }


    if ( !$retval ) {
        $i = 1; 
        while ( $i <= $values["number"] ) {
			$imagename = oIMGseriesC::getFilenameOfImage($img_body, $values, $i);
			$image_arr[] =  array( $imagename, $time_offs);
            $i++;
            
        }
    } 
    return array( $retval, $errtext, $image_arr );
}  


