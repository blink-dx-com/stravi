<?php 
/**
 * ZIP directory + files
 * @package zip.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/phplib/zip.inc 59 2018-11-21 09:04:09Z $
 */
class myZip {
	
	/**
	 * ZIP it
	 * @param string $path - the TEMP_DIR path
	 * @param array $files
	 *   array of filename (with relative dir)
	 * @param string $tarfile
	 */
	function zipall( $path, $files, $tarfile) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
		$zip = new ZipArchive();
		if ($zip->open($tarfile, ZipArchive::CREATE)!==TRUE) {
			$error->set( $FUNCNAME, 1, 'can not create ZIP-file:'.$tarfile );
			return;
		}
	
	
		$dir_cache=array();
		$add_problem=0;
	
		// read file by file from dir and add to tar archive. catch readdir
		// returns '.', '..', xml file or zip file (?)
	
		foreach( $files as $dummy=>$filename_with_dir) {
	
			if (strstr($filename_with_dir,'/')!=NULL) {
				$patharr = explode("/",$filename_with_dir);
				$this_dir = $patharr[0];
				if (!array_key_exists($this_dir, $dir_cache)) {
					//echo "- DEBUG: NEW DIR: $this_dir<br>";
					$zip ->addEmptyDir($this_dir);
					$dir_cache[$this_dir]=1;
				}
			}
	
			$ori_file = $path . "/" . $filename_with_dir;
			//echo "- DEBUG: FILE: $filename_with_dir; ORI:$ori_file|<br>";
	
			if (!$zip->addFile( $ori_file,  $filename_with_dir )) {
				$add_problem++;
			}
	
		}
		
		$this->add_problem = $add_problem;
		
		$zip->close();
	}
}