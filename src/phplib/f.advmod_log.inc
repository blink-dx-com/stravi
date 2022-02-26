<?php 
/**
 * ADVMOD_LOG library
 * active, if $_SESSION['globals']['app.advmod']>0
 * @package f.advmod_log.inc 
 * @swreq   SREQ:0003141: o.CCT_ACCESS > ADVMOD_LOG > create a library to manage it 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @unittest exists
 */
class f_advmod_log {
    
    public static $MAX_FIELD_SIZE=256;
	private $advmod_file;
	
	function __construct() {
		$this->advmod_file = $_SESSION['globals']['data_path'] .'/advmod_log.txt';
	}
	
	public function getFileName()  {
		return $this->advmod_file;
	}
	
	
	/**
	 * RECURSIVE: REDUCE data in array use self::self::$MAX_FIELD_SIZE
	 * @param array $entryDict
	 */
	static function _reduce_dict_REC(&$entryDict) {
	    $keys=array_keys($entryDict);
	    foreach($keys as $key) {
	        if (is_array($entryDict[$key])) {
	            self::_reduce_dict_REC($entryDict[$key]);
	        } else {
	            if (strlen($entryDict[$key])>self::$MAX_FIELD_SIZE) {
	                $entryDict[$key] = substr($entryDict[$key],0, self::$MAX_FIELD_SIZE).' [CUT]';
	            }
	        }
	    }
	}

	/**
	 * write line
	 * convert $entryDict => base64(json($entryDict))
	 * @param int $cct_access_id
	 * @param string $date_str  YYYY-MM-DDTHH:MM:SS
	 * @param array $entryDict
	 */
	public function writeLine($cct_access_id, $date_str, $entryDict) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		self::_reduce_dict_REC($entryDict);
		
		$content = $cct_access_id ."\t". $date_str . "\t" . base64_encode(json_encode($entryDict)) ."\n";
		
		$fp = fopen($this->advmod_file, 'a');
		if (!$fp) {
			// try five times, if file handle is occupied ...
			for ($i=0; $i<5; $i++) {
				usleep(1000); // 0.1 sec sleep
				$fp = fopen($this->advmod_file, 'a');
				if ($fp) break;
			}
			
			if (!$fp) {
			    error_log( 'APP-ERROR: '.$FUNCNAME.': Cant write advmod_file: '.$this->advmod_file );
				//$error->set( $FUNCNAME, 1, 'Cant write advmod_file: '.$this->advmod_file );
				return;
			}
			
		}
		
		
		
		$retVal = fputs( $fp, $content ); /* write data */
		fclose( $fp );
	}
	
	/**
	 * get $entryDict of ($cct_access_id,$date_str)
	 * @param int $cct_access_id
	 * @param string $date_str
	 * @return array ($found, $entryDict)
	 * 	$found : 0,1
	 *  $entryDict : the PHP-dict
	 */
	public function searchLine($cct_access_id, $date_str) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$entryDict = NULL;
		$found	   = 0;
		
		if (!$cct_access_id or $date_str==NULL) {
			$error->set( $FUNCNAME, 1, 'Input params missing.' );
			return;
		}
		
		if (!file_exists($this->advmod_file)) {
			return; // file does not exist
		}
		
		$LINE_LENGTH = 100000;
	    $fp = fopen($this->advmod_file, 'r');
		if (!$fp) {
			$error->set( $FUNCNAME, 1, 'Cant open advmod_file for read: '.$this->advmod_file );
			return;
		}
		
		$i=1;
		while( !feof ( $fp ) ) { 
	        $line     = fgets($fp, $LINE_LENGTH);
	        $firstPos = strpos($line, "\t");
	        $cctid_loop = substr($line,0,$firstPos);
	        if ($cctid_loop==$cct_access_id) {
	        	// check deeper
	        	$linearr = explode("\t",$line);
	        	$date_loop = $linearr[1];
	        	if ($date_loop==$date_str) {
	        		// match !
	        		$entryDict = json_decode(base64_decode($linearr[2]), TRUE);
	        		$found=1;
	        		break;
	        	}
	        }
		    $i++;
		}
		fclose ( $fp);
		
		return array($found,$entryDict);
		
	}
	
	/**
	 * start query
	 * @return $ok: 0, 1
	 */
	public function queryStart($cct_access_id) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$this->query_cctid = $cct_access_id;
		
		//$entryDict = NULL;
		//$found	   = 0;
		
		if (!file_exists($this->advmod_file)) {
			return 0; // file does not exist
		}
		
		
	    $this->fp = fopen($this->advmod_file, 'r');
		if (!$this->fp) {
			$error->set( $FUNCNAME, 1, 'Cant open advmod_file for read: '.$this->advmod_file );
			return 0;
		}
		
		return 1;
	}
	
	/**
	 * get next row
	 * @return array('f'=>$found_flag, 'date'=>date-string, 'dict'=>$entryDict)
	 */
	public function queryRow() {
		
		if ($this->fp==NULL) return;
		
		$cct_access_id = $this->query_cctid;
		$i=1;
		$found=0;
		$LINE_LENGTH = 100000;
		while( !feof ( $this->fp ) ) { 
	        $line     = fgets($this->fp, $LINE_LENGTH);
	        $firstPos = strpos($line, "\t");
	        $cctid_loop = substr($line,0,$firstPos);
	        
	        if ($cctid_loop==$cct_access_id) {
	        	// check deeper
		        if ( $_SESSION["userGlob"]["g.debugLevel"]>1 ) {
					debugOut($line, 'queryRow:found_line'); 
				}
	        	$linearr = explode("\t",$line);	
	        	$entryDict = json_decode(base64_decode($linearr[2]), TRUE);
	        	$found=1;
	        	break;
	        	
	        }
		    $i++;
		}
		return array('f'=>$found, 'date'=>$linearr[1], 'dict'=>$entryDict);
	}
	
	public function queryClose() {
		fclose($this->fp);
	}
	
}
