<?
/*MODULE: o.IMG.upload.inc
  DESCR: functions for upload
  AUTHOR: qbi
*/
require_once ("glob.image.inc");
require_once ("insert.inc");
require_once ('f.update.inc');

function img_cp_file( &$sql, 
     $filename,          // SOURCE: original file path
     $id,                // IMG_ID or (NULL if "action" = "create")
     $userfile_name,     // short name (just info)
     $userfile_type,     // MIME_TYPE
     $opt = NULL         // "action" = "create" create an image object
                         // "argu"   = array of arguments
     ) {
    // RETURN: 0 OK
    //	      <0 error 
    
	global  $error;
	
    $main_ret=0;
	$error = & ErrorHandler::get();
    
	if ($filename == "none") {
        $error->set('img_cp_file', 1, "Your image is probably too big for upload. Please give a smaller image.");
  		return -1;
	}
	
	if ( file_exists($filename) && (filesize($filename)>0) ) {

		if ( $opt["action"] == "create" ) {
            $argu = NULL;
            $argu["NAME"]   = "";
            if (is_array( $opt["argu"] ))  $argu = $opt["argu"];
            
            $argu["IMG_ID"] = "";
            $id = insert_row( $sql, "IMG", $argu);
        }
        
        if (!$id) {
            $error->set('img_cp_file', 2, "Image object does not exist.");
  		    return -1; 
        }
  		$dest_name = imgPathFull( $id );

		if ( !copy( $filename, $dest_name ) ) {
			$error->set('img_cp_file', 3, "failed to copy uploaded file.");
			$main_ret=-2;
		} else {
            $argu = NULL;
            $argu["IMG_ID"]   = $id;
			$argu["MIME_TYPE"]= $userfile_type;
			$retval = gObjUpdate::update_row( $sql, "IMG", $argu);
		}
	} else {
		$error->set('img_cp_file', 4, "File '$filename' failed for upload!");
		$main_ret=-1;
	}
	return ($main_ret);
}	
