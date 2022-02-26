<?php
require_once ('down_up_load.inc');
require_once ('f.upload.inc');
require_once ("insertx.inc");
require_once 'f.directorySub.inc';


/**
 * manage upload/download of files through external API
 * config: $_SESSION['globals']['app.upload_zip'];
 * @package f.instr_upload_blk.inc
 * @swreq UREQ:FS-INT02.R01 upload ZIP-Archive and extract it to an DFS-directory
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/phplib/lev1/f.zip_instr_upload.inc 59 2018-11-21 09:04:09Z $
 * @unittest exists
 */
class f_instr_upload_blk {

	/**
	 * e.g. 'blinkone' => array('basedir'=>"Z:\\Forschung\\NGAi", 'year_basedir'=>'generated')
	 * @var array $inst_types_allow
	 */
	private $inst_types_allow;
	private $yearpath;
	private $relative_dir_from_year;

	private $ZIP_MAX_FILES=1000;         // max num of files in ZIP-file
	private $ZIP_MAX_SIZE_ALL=100000000; // max size of summary of all files in ZIP-file


	public function __construct($instrument_type) {
		$this->instrument_type  = $instrument_type;
		$this->inst_types_allow = $_SESSION['globals']['app.upload_zip'];

	}
	
	public function check_config() {
	    $instr_type_features = $this->get_instr_type_feats();
	    $basedir      = $instr_type_features['basedir']; // ['year_basedir']
	}
	
	public function get_instr_type_feats() {
	    return $this->inst_types_allow[$this->instrument_type];
	}

	public function get_types() {
	    return $this->inst_types_allow;
	}

	

	/**
	 * is $type allowed?
	 * @param string $type : instrument_type
	 * @return number 0,1
	 */
	public function instrument_type_allowed() {

		if (!is_array($this->inst_types_allow)) {
			return 0;
		}

		$types_allowed = array_keys($this->inst_types_allow);

		if (in_array($this->instrument_type, $types_allowed)) {
			return 1;
		}
		return 0;
	}

	


	/**
	 * main upload process
	 * @throws errors: please do NOT change the error numbers for unittests !
	 * @param string $exp_ext_id e.g. "15c06564-2966-11e9-ac6c-4cc"
	 * @param string $tmp_file temporary file to be uploaded  /tmp/phpk61OAI
	 * @param string $relpath   "segmentation/cycle_08_ramp_01_temp_058_row1_col1_z1_fluor_seg.png"
	 * @param int $allowOverWrite : 0,1
	 * @return array('exp_id'=>$exp_id)
	 *   $errorcode > 0: ERROR
	 */
	public function upload(&$sqlo, $exp_ext_id, $tmp_file, $relpath, $allowOverWrite=0 ) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
			
		$tablename='EXP';


		if ($exp_ext_id==NULL){
		    $error->set( $FUNCNAME, 11, "No ext-ID given."); 
		    return;
		}
		if ($relpath==NULL){
		    $error->set( $FUNCNAME, 13, "No relpath given."); 
		    return;
		}
		if (strstr($relpath,'..')!=NULL) {
		    $error->set( $FUNCNAME, 16, "relpath contains invalid characters (..)."); 
		    return;
		}

		if ($tmp_file==NULL)  {
			$error->set( $FUNCNAME, 12, "No upload-file given."); 
			return;
		}
		if (!file_exists($tmp_file)) {
		    $error->set( $FUNCNAME, 14, "Temp-file not found."); 
		    return;
		}
		
		$t_rights = tableAccessCheck( $sqlo, $tablename );
		if ( $t_rights["insert"] != 1 ) {
		    $answer = getRawTableAccMsg( $tablename, 'insert' );
		    $error->set( $FUNCNAME, 15, $answer);
		    return;
		}
		

		// check external-ID
		$exp_feats = glob_elemDataGet3( $sqlo, 'EXP', array('EXT_ID'=>$exp_ext_id), array('EXP_ID', 'DATAURL') );
		$expid   = $exp_feats['EXP_ID'];
		$DATAURL = $exp_feats['DATAURL'];
		
		if (!$exp_feats['EXP_ID']) {
		    
		    $error->set( $FUNCNAME, 25, 'No experiment found EXP:EXT_ID:'.$exp_ext_id);
		    return;
		    // $exp_feats = glob_elemDataGet3( $sqlo, 'EXP', array('EXP_ID'=>$expid), array('DATAURL') );
		    
		} else {
		    
		    $o_rights = access_check($sqlo, $tablename, $expid);
		    if ( !$o_rights["insert"]) {
		        $error->set( $FUNCNAME, 19, 'No insert right for user on object: EXP:'.$expid);
		        return;
		    }
		}
		
		$linux_path_dest = netfile2serverpath($DATAURL);

		if (!file_exists($linux_path_dest)) {
		    $error->set( $FUNCNAME, 22, 'Experiment DATAURL "'.$DATAURL. '" not exists.');
		    return;
		}

		$full_file_path = $linux_path_dest .'/'.$relpath;
		$basepath_file  = dirname($full_file_path);
		if (!file_exists($basepath_file) ) {
		    if ( !mkdir($basepath_file,0777, TRUE) ) {
		        $error->set( $FUNCNAME, 20, 'Error on creation of directory "'.$basepath_file.'" for file for Experiment ID:'.$expid);
		        return;
		    }
		}
		
		$filex_exists = file_exists($full_file_path);
		
		if ( !$allowOverWrite ) {
		    if ( $filex_exists ) {
    		    $error->set( $FUNCNAME, 21, 'File "'.$full_file_path.'" already exists for Experiment ID:'.$expid);
    		    return;
    		}
		}
		
		//$error->logx("INFO", $FUNCNAME, 'full_file_path:'.$full_file_path);
		if  ( !copy($tmp_file, $full_file_path) ) {
		    $error->set( $FUNCNAME, 23, 'Copy failed of File "'.$relpath.'"  for Experiment ID:'.$expid);
		    return;
		}
		

		return array('exp_id'=>$expid);
	}
	
	/**
	 * get absolute path onm server of file for download
	 * @param object $sqlo
	 * @param int $exp_id
	 * @param string $relpath "segmentation/cycle_08_ramp_01_temp_058_row1_col1_z1_fluor_seg.png"
	 * @return string full_path
	 */
	function download(&$sqlo, $exp_id, $relpath) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    if (!gObject_exists ($sqlo, 'EXP', $exp_id) ) {
	        $error->set( $FUNCNAME, 11, 'Experiment with ID:'.$exp_id. ' not exists.');
	        return;
	    }
	    
	    // check external-ID
	    $exp_feats = glob_elemDataGet3( $sqlo, 'EXP', array('EXP_ID'=>$exp_id), array('DATAURL') );
	    $DATAURL = $exp_feats['DATAURL'];
	    
	    if (!$DATAURL) {
	        $error->set( $FUNCNAME, 12, 'Experiment-ID:'.$exp_id. ': no DATAURL set.');
	        return;
	    }
	    
	    $DATAURL_on_server = netfile2serverpath($DATAURL);
	    
	    if (!file_exists($DATAURL_on_server)) {
	        $error->set( $FUNCNAME, 22, 'Experiment DATAURL "'.$DATAURL. '" not found.');
	        return;
	    }
	    
	    $full_file_path = $DATAURL_on_server .'/'.$relpath;
	    if (!file_exists($full_file_path) ) {
	        $error->set( $FUNCNAME, 23, 'File (relative) "'.$relpath.'" not exists for Experiment ID:'.$exp_id.' on DATAURL');
	        return;
	    }
	    
	    return $full_file_path;
	}

}
