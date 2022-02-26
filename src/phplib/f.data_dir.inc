<?php

/**
 * functions for handling data in the directory $_SESSION['globals']["data_path"]
 * @package f.data_dir.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class datadirC {

    static function datadir_filename( $tablename, $id ,$ext ) {

    	$img_dir=$_SESSION['globals']['data_path'];
    	return ( $img_dir."/".$tablename .".".$id.".". $ext );
    }

}
