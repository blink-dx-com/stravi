<?php
require_once ('down_up_load.inc');
require_once ('f.upload.inc');
require_once('o.EXP.autocrea_log.inc');

/**
 * manage upload of files through external API
 * config: $_SESSION['globals']['app.upload_zip'];
 * @package f.zip_instr_upload.inc
 * @swreq UREQ:FS-INT02.R01 upload ZIP-Archive and extract it to an DFS-directory
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/phplib/lev1/f.zip_instr_upload.inc 59 2018-11-21 09:04:09Z $
 * @unittest exists
 */
class f_zip_instr_upload {

	/**
	 * e.g. 'NGAi' => array('basedir'=>"Z:\\Forschung\\NGAi", 'year_basedir'=>'generated')
	 * @var unknown
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



	public function errorout($errnum, $text) {
		global $error;
		header("err-code: ".$errnum);
		header("err-text: ".$text);
		echo("err-text: ".$errnum.':'.$text);

		$error->set(__CLASS__, $errnum, $text);
		$error->logError();

		exit($errnum);
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
	 * check, if filename is allowed
	 * @param string $filename 
	 *   A) YYYY-MM-DD.xxxxxx.xxxx.xxx.zip (extension ".zip" is optional)
     *   B) YYYYMMDD_time.xxxx.xxx.zip (extension ".zip" is optional)
	 * @return array('server_path', 'windows_path')
	 */
	function check_filename($filename) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;

		$this->zip_dir_pure = NULL;
		
		$pattern_ok=0;
		
		if (substr($filename,4,1)=='-') {
			// YYYY-MM-DD
			$pattern='YYYY-MM-DD';
			$regex_pattern='/^[0-9]{4}-[0-1][0-9]-[0-3][0-9]$/';
			$file_YYYMMDD = substr($filename,0,strlen($pattern));
			$match_answer = preg_match ( $regex_pattern , $file_YYYMMDD);
			if (!$match_answer) {
				$error->set( $FUNCNAME, 1, 'filename not valid; Expect YYYY-MM-DD as start.' );
				return;
			}
			$pattern_ok = 1;
			list($y, $m, $d) = array_pad(explode('-', $file_YYYMMDD, 3), 3, 0);
			
		} else {
			// YYYYMMDD
			$pattern='YYYYMMDD';
			$regex_pattern= '/^[0-9]{4}[0-1][0-9][0-3][0-9]$/';
			$file_YYYMMDD = substr($filename,0,strlen($pattern));
			$match_answer = preg_match ( $regex_pattern , $file_YYYMMDD);
			if (!$match_answer) {
				$error->set( $FUNCNAME, 1, 'filename not valid; Expect YYYY-MM-DD as start.' );
				return;
			}
			$pattern_ok = 1;
			$y = substr($file_YYYMMDD,0,4);
			$m = substr($file_YYYMMDD,4,2);
			$d = substr($file_YYYMMDD,6,2);
		}
		
		if (!$pattern_ok) {
			$error->set( $FUNCNAME, 2, 'filename not valid; Expect YYYY-MM-DD or YYYYMMDD as start.' );
			return;
		}

		
		if (! (ctype_digit("$y$m$d") and checkdate($m, $d, $y)) ) {
			$error->set( $FUNCNAME, 4, 'filename not valid; Date-part invalid.' );
			return;
		}
		
		$YYYYMMDD = $y.'-'.$m.'-'.$d;

		$tmp_type_info = $this->inst_types_allow[$this->instrument_type];

		$tmp_type_info['basedir'] = rtrim($tmp_type_info['basedir'],"\\");
		$yearpath = $tmp_type_info['basedir']."\\".$y;
		$relative_dir_from_year = NULL;
		if ($tmp_type_info['year_basedir']!=NULL) {
			$newpath = $yearpath . "\\" . $tmp_type_info['year_basedir'];
			$relative_dir_from_year = $tmp_type_info['year_basedir'];
		}

		$this->yearpath = $yearpath;

		$filename_wozip = $filename;
		$zip_ext_pattern='.zip';
		$substr_len=strlen($zip_ext_pattern);
		$lastpart=substr($filename,-$substr_len);
		if ($lastpart=='.zip' or $lastpart=='.ZIP') {
			$filename_wozip = substr($filename,0,-$substr_len); // cut ".zip" from name!
			$this->zip_dir_pure = $filename_wozip;
		}
		$newpath .= "\\". $YYYYMMDD ."\\". $filename_wozip;
		$relative_dir_from_year .= '/' . $YYYYMMDD ."/". $filename_wozip;
		$this->relative_dir_from_year = $relative_dir_from_year;

		$linux_path_dest = netfile2serverpath($newpath);

		return array('server_path'=>$linux_path_dest, 'windows_path'=>$newpath);

	}

	function save_log_file() {
		$explog_lib = new o_EXP_autocrea_log();
		$explog_lib->save_log_file($this->yearpath, $this->zip_dir_pure, $this->relative_dir_from_year);

	}

	/**
	 * main upload process
	 * @return array($errorcode, $text, $path_arr)
	 *   $errorcode > 0: ERROR
	 */
	public function upload($filename, $tmp_file) {
		global $error;

		

		if ($filename==NULL)  {
			return array(11, "No ZIP-Filename given!");
		}

		$path_arr = $this->check_filename($filename);
		if ($error->Got(READONLY))  {
			$errLast   = $error->getLast();
			$error_txt = $errLast->text;
			return array(5, "ZIP-Filename not allowed! ".$error_txt);
		}
		$linux_path_dest = $path_arr['server_path'];


		$testLib = new uploadC();
		$answer = $testLib->getZipfileInfo( $tmp_file );
		if ($error->Got(READONLY))  {
			return array(6, "Error on ZIP-File test!");
		}
		if ($answer['numfiles'] > $this->ZIP_MAX_FILES) {
			return array(7, "Too many files in ZIP!");
		}
		if ($answer['sizeall'] > $this->ZIP_MAX_SIZE_ALL) { // e.g. max 100MB
			return array(8, "Too much data in ZIP!");
		}

		if (file_exists($linux_path_dest)) {
			return array(12, "Dest_dir already exists: ".$path_arr['windows_path']);
		}

		// create directory on destination path
		if (!file_exists($linux_path_dest)) {
			$answer = mkdir($linux_path_dest,   0777, true);
			if (!$answer) {
				return array(10, "MKDIR of new dir failed! ".$linux_path_dest);
			}
		}

		$copt=array('verbose'=>'silent');
		$testLib->getZipFile( $tmp_file, $linux_path_dest, $copt);
		if ($error->Got(READONLY))  {
			return array(9, "Error on ZIP-File extraction!");
		}

		$this->save_log_file();
		if ($error->Got(READONLY))  {
			return array(13, "Error on Upload-Log-File.");
		}

		return array(0, "o.k.", $path_arr);
	}

}
