<?
/**
 * functions for HTTP(S) download and upload
 * @package down_up_load.inc
 * @swreq UREQ:GLOBAL
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/phplib/down_up_load.inc 59 2018-11-21 09:04:09Z $
 */

/**
 * downlaod class 
 * @author Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 *
 */
class fDownloadC {

	/**
	 * for CSV-file header
	 * @param array $infoarr
				  additional:
					"title"
					"filename" = file...
					"mime"     =>"application/vnd.ms-excel"
	 * @param string $opt
	 */
	function pageCsv ( 	$infoarr, 	$opt = NULL	) {

		set_mime_type($infoarr["mime"], $infoarr["filename"]);
		echo "# Page: ".$infoarr["title"]."\n";
	}

}

/**
 * sets the mime-type. please use always this function to avoid problems on IE on windows using https
 * $filename: leave it empty for mime-types which are displayed on screen
 * @param string $mime_type
 *  typical mime-types:
					text/tab-separated-values
					application/vnd.ms-excel
					text/plain
					image/png
					text/xml
 * @param string $filename short name
 * @param array $options
 *  "size" : 1..N
	 "getsize" : 0|1
 */
function set_mime_type ($mime_type, $filename= "", $options = NULL ) {
 
  header("Content-type: $mime_type");
  header('Expires: 0');
  
  if ($options["size"]>0)  header("Content-length: ".$options["size"]);
  
  // if (array_key_exists("HTTPS", $_SERVER) &&
  //    ($_SERVER["HTTPS"] == "on") &&
  
  
  if ( strstr($_SERVER["HTTP_USER_AGENT"], "MSIE") && 
       strstr($_SERVER["HTTP_USER_AGENT"], "Win")) {
      
    // because of a bug in IE while downloading through https
    header("Pragma: public"); 
    header('Cache-Control: must-revalidate, post-check=0, pre-check=0');
    
    if ($filename != "")
	  header('Content-Disposition: attachment; filename="'.$filename.'"');
  } else {
    if ($filename != "")
      header('Content-Disposition: attachment; filename="'.$filename.'"');
  }

}##End function


/**
 * converts NET (WINDOWS-path) path to SERVER internal path
 * - convert "\" to "/"
 * - lookup in $_SESSION['globals']["sys.net2srv"]
 * e.g. "Z:\Data\2020\exp1" ==> "/mnt/data/2020/exp1"
 * @param string $filename
 * @return array 
 *  'srv_path'=>$filename_url, server path
 *  'mindex'=> -1 or number
 */
function netfile2server_raw($filename) {
    
    $filename_url     = str_replace("\\", "/", $filename);
    $filename_url_new = $filename_url;
    $match_index = -1;
    
    // take info from config.local.inc
    
    if ( is_array($_SESSION['globals']["sys.net2srv"]) )  {
        $tmparr = $_SESSION['globals']["sys.net2srv"];
        
        
        $loop_index = 0;
        foreach( $tmparr as $dummy=>$tmparr2) {
            $ori = key($tmparr2);
            $dest= current($tmparr2);
            if ( stristr( $filename_url, $ori) != "" ) {
                // produce a real server path
                $filename_url_new = $dest . substr ( $filename_url, strlen($ori) );
                $match_index = $loop_index;
                break;
            }
            $loop_index++;
        }
        
    }
    return  array('srv_path'=>$filename_url_new, 'mindex'=>$match_index);
}
 
/**
 * see netfile2server_raw
 * @param string $filename
 * @return string $filename_url_new
 */
function netfile2serverpath( $filename ) {
    $answer = netfile2server_raw($filename);
    return $answer['srv_path'];   
}


class fNetFile {
    
    // file exists on server ? 0,1
    static function netfile_exists($filename) {
        $out = 0;
        $answer = netfile2server_raw($filename);
        if (!$answer['srv_path']==NULL) return 0;
        
        if (file_exists($answer['srv_path'])) $out = 1;
        return $out;
    }
}

