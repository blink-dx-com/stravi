<?
require_once('glob.image.inc');
require_once('o.IMG.file.inc');

class obj_edit_ext extends gObj_edit_feat_ABS {
    
    function rightWindow( &$sqlo, &$sqlo2) {
        $id = $this->id;
        
        $argu  = &$this->argu;
        $o_rights = &$this->o_rights;
        $viewmode = $this->viewmode;
     

        $varcol    = & Varcols::get();
        $editAllow = $o_rights["write"];
        if ($viewmode!= "edit" ) $editAllow = 0;
        
    	$img_name     = $argu['NAME'];
    	$extra_obj_id = $argu['EXTRA_OBJ_ID'];
    	$imgSubLib = new oIMG_fileC();
        
        echo "<ul>\n";
    
    	$image_id=$id;
    	$imageExists = FALSE;
    	$imgOrgname  = imgPathFull( $image_id );
    	if ( file_exists( $imgOrgname ) ) $imageExists = TRUE;
    
    	$dimen=array();
        $dimen[0]=350;
        $dimen[1]=250;
        $options_tmp=array();
        $options_tmp["noImageLogo"]=1;
    
    	if ( $imageExists ) {
    		if ( $_SESSION['userGlob']["o.IMG.noThumbshow"]!=1 ) {
    			
    			$ret = org_img_show ( $sqlo, $image_id, $dimen, "", $img_name, $options_tmp );
    			echo "&nbsp;&nbsp;<a href=\"preferences.php?var=o.IMG.noThumbshow&val=1\">".
    				"<img src=\"images/ic.eyeNo.gif\" border=0> Do not show the thumbnail</a><br>";
    			echo "<br>\n";
    		} else {
    			echo "&nbsp;&nbsp;<a href=\"preferences.php?var=o.IMG.noThumbshow&val=\"><img src=\"images/ic.eye.gif\" border=0> Show image</a><br>";
    		}
    	} else {
    		echo "&nbsp;&nbsp;<img src=\"images/ic.dbno.gif\"> <font color=gray>Not on image-server.</font><br>\n";
    	}
    	
    	$nameIsUrl = $imgSubLib->nameIsUrl($img_name);
    	if ($nameIsUrl) {
    		if ( $imgSubLib->imgOnNetExists($img_name) ) {
    			echo "&nbsp;&nbsp;<img src=\"images/ic.network.gif\"> Exists on network <br />\n";
    		}
    	}
    
        if ( $extra_obj_id ) $extra_class = $varcol->obj_id_to_class_name ( $extra_obj_id );
        if ( $extra_class=="series" ) {
            echo "&nbsp;&nbsp;<img src=\"images/icon.IMG_OF_EXP.gif\"> This is an image series.\n";
            echo " [<a href=\"obj.img.series_show.php?id=".$id."\">View image list</a>] ";
    		echo " [<a href=\"obj.img.series_show.php?id=".$id."&showimages=1\">Show image thumbnails</a>]";
    		echo "<br>";
        }
        
        if ( $imageExists ) {
    
    
            $url_path = "obj.img.download.php?id=".$image_id;
            echo "&nbsp;&nbsp;<a href=\"". $url_path . "\"><img src=\"images/but.download.gif\" border=0> Download image</a><BR>";
            if ( $editAllow ) {
                echo '&nbsp;&nbsp;<a href="obj.img.delete.php?id=',$id,'"><img src="images/ic.dbdel.gif" border="0" TITLE="Delete"></a> Delete image file<BR>';
            }
            $tmp_size = filesize ($imgOrgname);
            $tmp_size_str = "$tmp_size bytes";
            if ($tmp_size>1000000) $tmp_size_str = sprintf ("%.2f MB", $tmp_size/1048676 );
            echo "&nbsp;&nbsp;<img src=\"images/ic.database.gif\"> <font color=gray>Size:</font> ".$tmp_size_str."<br><br>\n";
            
    		echo "&nbsp;&nbsp;<a href=\"obj.img.showHuge.php?img_id=".$image_id.
    			     "&opx[img_dim_flag]=1\"><img src=\"images/ic.zoom16.gif\" border=0> Zoom</a><br>";
            echo "&nbsp;&nbsp;<a href=\"obj.img.showHuge.php?img_id=".$id."\" ><img src=\"images/ic.beautfarm.gif\" border=0> Image beauty farm</a><br>\n";
            echo "&nbsp;&nbsp;<img src=\"res/img/info.svg\" width=20> <font color=gray size=-1><B>Image Info:</B> \n";
            $gostr = $_SESSION['globals']['img_identify']. " -ping ". $imgOrgname ;
            $answer =  system ( $gostr , $tartra);	     
            echo "</font><br>";
        }
        
        $imgOrgname2 = $img_name;
        
        if ( $editAllow ) { ?>
    
          <table cellpadding=1 cellspacing=1 border=0 bgcolor=#EFEFEF><tr><td>
          <form ENCTYPE="multipart/form-data" ACTION="obj.img.upload.php?id=<? echo $id; ?>" METHOD=POST>
          <INPUT TYPE="hidden" name="MAX_FILE_SIZE" value="<? echo $_SESSION['globals']["F.IMG.IMPORT.UPLOAD_MAX_SIZE"]; ?>">
          <INPUT TYPE="submit" VALUE="Upload image"></td><td>
    	  <INPUT NAME="userfile" TYPE="file" accept="*.*" size="50"></td></tr>
    	  <tr><td>&nbsp;</td><td>
          <font color=gray><I>Max file size: <B><? echo $_SESSION['globals']["F.IMG.IMPORT.UPLOAD_MAX_SIZE"]/1E6; ?></B>&nbsp;MBytes. Types: TIF, JPG, BMP, ...</font></I>
          </FORM>
          </td></tr></table> 
       
          <?
          if ( $extra_class=="series" ) {
            if ( $img_name != "" ) {
                $values_tmp = $varcol->select_by_name ($extra_obj_id);
    	        $values     = &$values_tmp["values"];     
                 
    			$laststripPos = strrpos( $img_name,"\\" );
    			$fulldir = substr($img_name,0,$laststripPos); 
            }
    		echo "<br>";
            ?>  
            <B>Create image-series parameters</B> from a directory (and upload last image)
    		<form ACTION="obj.img.series_crea.php?id=<? echo $id; ?>" METHOD=POST>
    		<table bgcolor=#EFEFEF border=0><tr><td>
            <INPUT TYPE="submit" VALUE="Create params"></td><td>
            <INPUT NAME="fulldir" TYPE="text" size="70" value="<?php echo $fulldir?>">
    		</td></tr><tr><td><td><I>Give the directory-path ...</I></td></tr>
    		</table>
            </FORM>
            
            <? 
          }
        } 
        if ( $editAllow ) {
           echo "<br><LI><B>Set reference spots</B>  <a href=\"obj.img.spotpos_gui.php?id=$id\" target=ximage>Open GUI</a><br>\n";
        }
        echo "</ul>\n";
        return 0;
     }

}

