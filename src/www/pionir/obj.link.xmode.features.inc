<?php
/**
 * document extension
 * @swreq UREQ:xxxxxxxxxxxx
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */

require_once("o.LINK.subs.inc");
require_once("o.LINK.versctrl.inc");
require_once("visufuncs.inc");



class obj_edit_ext extends gObj_edit_feat_ABS { 
    
    /**
     * check icono document
     * @param object $sql
     * @param int $id
     * @param string $mime
     * @param string $hash_code
     * @return int 0 ok: no other script
     //	  "hash" > 0 : at least an other document exist
     //	  "used" > 0 : used in an protocol
     //	  "infoHead"     : text
     //	  "info"     : text
     //		 = 1 : used by THIS user
     //		 = 2 : used by other users
     */
    static function this_icono_check( &$sql, $id, $mime, $hash_code) {
        
        
        $retval = NULL;
        if ( ($mime == "application/iconoclust-run") OR ($mime == "application/iconoclust-pck") ) {
            if ($hash_code!="") {
                // check, if HASH exists
                $sql->query("SELECT count(link_id) FROM LINK WHERE HASH='$hash_code' AND LINK_ID!=".$id);
                $sql->ReadRow();
                $numval = $sql->RowData[0];
                if ($numval>0) {
                    $retval["infoHead"]= "Check of 'application/iconoclust-run/pck'";
                    $retval["info"]= "<a href=\"view.tmpl.php?t=LINK&condclean=1&searchCol=HASH&searchtxt=$hash_code\">$numval
					document[s]</a> already exists in database (checked by HASH).\n";
                    $retval["hash"]=1;
                }
            }
            
            $sql->query("SELECT count(*) FROM CONCRETE_PROTO WHERE LINK_ID=".$id);
            $sql->ReadRow();
            $protonum = $sql->RowData[0];
            if ($protonum>0) {
                $retval["infoHead"]= "Check for usage of 'application/iconoclust-run/pck'";
                $retval["info"]    = "<a href=\"view.tmpl.php?t=CONCRETE_PROTO&condclean=1&searchCol=LINK_ID&searchtxt=$id\">$protonum
				protocol[s]</a> already use[s] this script.\n";
                
                $retval["used"]=1;
                
                $sql->query("SELECT count(*) FROM CONCRETE_PROTO c, CCT_ACCESS a WHERE c.LINK_ID=".$id. " AND
			c.CCT_ACCESS_ID=a.CCT_ACCESS_ID AND a.DB_USER_ID!=".$_SESSION['sec']['db_user_id']);
                $sql->ReadRow();
                if ($sql->RowData[0]>0) {
                    $retval["info"] .= "<br>Info: ".$sql->RowData[0]." protocol[s]</a> of <B>OTHER</b> users already use[s] this script.\n";
                    $retval["used"]=2;
                }
            }
        }
        return $retval;
    }
    
    function rightWindow( &$sqlo, &$sqlo2) {
        $id = $this->id;
        
        $argu  = &$this->argu;
        $o_rights = &$this->o_rights;
        $viewmode = $this->viewmode;
    
    	$infox=NULL;
        $varcol    = & Varcols::get(); 
    	echo "<blockquote>\n";
    	$editAllow = $o_rights["write"] || $o_rights["insert"];
    	if ($viewmode!= "edit" ) $editAllow = 0;
    	
    	$versionObj = new oLINKversC();
    	
    	$link_name = $argu['NAME'];
    	$mime_type = $argu['MIME_TYPE'];
        $extra_obj_id = $argu['EXTRA_OBJ_ID'];  
    	$hash_code = $argu['HASH'];
    	
    	$linkFullPath = linkpath_get( $id );
    	$upload_allow = 0;
    	
    	if ($editAllow) {
    		// do only check in edit mode
    		$retval = self::this_icono_check( $sqlo, $id, $mime_type, $hash_code );
    		
    		if ($retval["info"] != "") {
    			htmlInfoBox( "WARNING: ".$retval["infoHead"], $retval["info"], "", "WARN" );
    			echo "<br>";
    		} 
    		
    		if ($editAllow) 	 $upload_allow = 1;
    		if ($retval["used"]>1 AND $_SESSION['sec']['appuser']!="root" ) {
    			$upload_allow = 0; // do not allow uploads
    			echo "<br>Info: Upload/delete not allowed!<br>\n";
    		}
    	} 
        $attachment_info = 1; 
        $class_values    = array();
        
        if ( $extra_obj_id ) {
        	$extra_class = $varcol->obj_id_to_class_name ( $extra_obj_id );
        	$values_tmp = $varcol->select_by_name ($extra_obj_id);
        	$class_values    = &$values_tmp["values"];
        }
        
        if ( $extra_class=="query" ) {
            $upload_allow = 0;
            $attachment_info = 0;
        }
    	
    	if ( $extra_class=="query" ) {
            visufuncs::slimhead("SQL query functions"); 
    
            if ( $class_values["sqltext"]!="" && $class_values["table"]!=""  ) {          
                echo "[<a href=\"obj.link.c_query_exec.php?id=".$id."&myqu_go=1\"><B>Execute query</b></a>]\n";
               //TBD: may be this link ??? echo "[<a href=\"obj.link.c_query_exec.php?id=".$id."\">Execute query</a>]\n";
            }
    		echo " &nbsp;&nbsp;&nbsp;| <a href=\"obj.link.c_query_mylist.php\">my searches</a> ";
            echo " | <a href=\"help/robo/o.LINK.class.query.html\" target=help>help</a> ";                                                                                
            echo "|<br><br>\n";
        }   
        
        if ( $extra_class=="Buch" ) {
        	$isbn = $class_values['ISBN'];
        	if ($isbn!=NULL) {
        		$amazonSearch='<a href="http://www.amazon.com/s/ref=nb_sb_noss?url=search-alias%3Dstripbooks&field-keywords='.
        			$isbn.'" target=_new>Infos zum Buch bei Amazon</a>';
        		echo " ==> <b>".$amazonSearch."</b><br><br>\n";
        	}
        }
        
    
    	visufuncs::slimhead("Document File");
    		
    	if ( file_exists( $linkFullPath ) ) {
    		
    		$infox["fileexists"] = 1;
            echo "<table cellpadding=1 cellspacing=3 border=0><tr valign=top>\n";
    		$url_path = "obj.link.download.php?id=".$id;
    		
    		echo "<td><a href=\"". $url_path . "\" onmouseover=\"status='".$link_name.
    			"';return true;\"><img src=\"images/ic.docdown.big.gif\" border=0 TITLE=\"Download\"></a>".
    			"&nbsp;<b>Download</b></td>\n";
    		echo "<td><img src=0.gif width=30 height=1></td><td>";	
            echo "<a href=\"$url_path&mime=text/plain&inline=1\"><img src=\"images/ic.eye.gif\" border=0 TITLE=\"View\"></a> View as plain/text<BR>";
            
    		if ( $upload_allow ) {
    			echo "<a href=\"obj.link.delete.php?id=".$id."\" ><img src=\"images/ic.dbdel.gif\" border=0 TITLE=\"Delete\"></a> Delete document file<BR>";
    		}
    		$tmp_size = filesize ($linkFullPath);
    		$tmp_size_str = "$tmp_size bytes";
    		if ($tmp_size>1000000) $tmp_size_str = sprintf ("%.2f MB", $tmp_size/1048676 );
    		echo "<img src=\"images/ic.database.gif\"> <font color=gray>Size:</font> ".$tmp_size_str."\n";
    		echo "</td>";
    		echo "</tr></table>\n";
    		
    	} else {
            
    		echo "&nbsp; &nbsp; <img src=\"images/ic.dbno.gif\"> <font color=gray>Not on server.</font> <BR>";
            
    	}
    	
    	
    	if ( $upload_allow  ) { 
            ?>
    		<br><table cellpadding=0 cellspacing=0 border=0><tr valign=top><td>
    		<form ENCTYPE="multipart/form-data" ACTION="obj.link.upload.php?<?=SID?>&id=<?echo $id; ?>" METHOD=POST>
     		<INPUT TYPE="hidden" name="MAX_FILE_SIZE" value="<? echo $_SESSION['globals']["F.IMG.IMPORT.UPLOAD_MAX_SIZE"]; ?>">
     		<INPUT TYPE="submit" VALUE="Upload document">&nbsp;</td>
    		<td> <INPUT NAME="userfile" TYPE="file" accept="*.*" ><br>
    		<font color=gray>Max file size: <B><? echo $_SESSION['globals']["F.IMG.IMPORT.UPLOAD_MAX_SIZE"]/1E6; ?></B>&nbsp;MBytes.</font>
     		</FORM>
    		</td></tr></table>
    		<?      
    	} 
    	echo "<br>\n";
    	
    	
    	if ( $versionObj->isPossible() ) {
    		// create version control 
    		//	 OR
    		// link to history
    		$lastPos = $versionObj->getlastVersPos($sqlo, $id);
    		if ($lastPos>0) { 
    			visufuncs::slimhead("Version control"); // show header, if relevant
    			echo "<img src=\"images/ic.cvsok.gif\"> &nbsp;&nbsp;[<a href=\"obj.link.verhist.php?id=$id\">Version history</a>]";
    			$versCount = $versionObj->countVersions($sqlo, $id);
    			echo "&nbsp;&nbsp;&nbsp;<b>$versCount</b> version(s)";
    			echo "<br>";
    		} else {
    			$activateAllow=1;
    			if ( !$upload_allow  ) 		 $activateAllow = 0;
    			if ( !$infox["fileexists"] ) $activateAllow = 0;
    			
    			if ( $activateAllow  ) {
    				visufuncs::slimhead("Version control"); // show header, if relevant
    				echo "[<a href=\"obj.link.vernew.php?id=$id\">Activate control</a>]<br>";
    			} else {
    				//echo "<font color=gray>[Activate control]</font><br>";
    			}
    		}
    		
    	} else {
    		if ( $upload_allow ) echo "Version control:  not possible -- please ask the admin to update the system.<br><br>\n";
    	}
    	
        
         
       
    	echo "</blockquote>";
    	return 0;
    }  

}

