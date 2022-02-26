<?
/**
 * special functions for list view of object type
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @package obj.chip_reader.xfunc.inc 
 * @swreq SREQ:0002412: o.CHIP_READER > list view extensions 
 */
require_once ('gui/glob.xfunc_abs.inc');
require_once ('o.CHIP_READER.logs.inc');
require_once ('o.CHIP_READER.subs.inc');
require_once ('gui/o.SATTACH.imgshow.inc');

class cCHIP_READER extends xfunc_ABS  {
    
  var $ident="CHIP_READER";
  var $xcolarr;
  private $_current_devid;
 
	
  

  function __construct( ) {
      parent::__construct();
      
  	$this->xcolarr[]=array ("ok_stat"       => "OK-Status",      "rem" => "current OK-Status");
  	$this->xcolarr[]=array ("last_log_stat" => "last log",       "rem" => "last service log entry name");
  	$this->xcolarr[]=array ("last_log_adv"  => "last Service advanced",       "rem" => 'last service log "Service" : name+sub status');
  	$this->xcolarr[]=array ("qualiStatus"   => "quali status",   "rem" => "qualification status");
  	$this->xcolarr[]=array ("qualiDate"     => "last quali date","rem" => "last successful qualification");
  	$this->xcolarr[]=array ("Thumbnail"     => "Thumbnail",      "rem" => "show thumbnail of object");
  	
  	$this->dev_dir_lib = NULL;
  	$lab_extension=$_SESSION['s_sessVars']['AppLabLibDir'].'/objlib/o.CHIP_READER.xLabQC.inc';
  	if ( file_exists($lab_extension) ) {
  		$this->xcolarr[]=array ("device_dir"   => "Ger&auml;teordner", "rem" => "Path of electronic Device dir");
  		require_once ($lab_extension);
  		$this->dev_dir_lib = new oChipReader_LabQC(NULL);
  	}
  	
  	$this->logSubLib = new oREA_LOG_subs();
  	$this->attachLib = new oSATTACH_imgshow();
  	$this->devLib    = new oCHIP_READER_subs();
  	$this->_current_devid = 0;
  }
  
  function inProjShowAtt( &$sqlo, $prim_id, $options=NULL ) {
      $tmp_optinfo ="";
      
      $tmp_optinfo = $this->get_abs_object_name($sqlo, $prim_id);
      
      if ($options["location"]) {
          $sqlsel = 'LOCATION from CHIP_READER where CHIP_READER_ID='.$prim_id;
          $sqlo->Quesel($sqlsel);
          $sqlo->ReadRow();
          $loc=$sqlo->RowData[0];
          if ($loc!="") $tmp_optinfo .= "&nbsp;(loc: $loc)";
      }

      return ($tmp_optinfo);
  }

  
  function xColDataGet ( &$sqlo,$colname,$idarr ) {
  	global $error;
  	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
  	
  	$output = NULL;
  	
  	if ($this->_current_devid != $idarr[0]) {
  	    $this->devLib->set_dev($sqlo, $idarr[0]); // initialize device
  	    $this->_current_devid = $idarr[0];
  	}
  	
  	if ($colname=='last_log_stat') {
	 	$answer = $this->logSubLib->getCurrentKey($sqlo, $idarr[0]);
		$output = $answer[1];
  	}
  	
  	if ($colname=='last_log_adv') {
  	    $tmp = $this->logSubLib->getLastKey_entry($sqlo, $idarr[0], oREA_LOG_subs::KEY_Service);
  	    if(!empty($tmp)) {
      	    $acc_str=NULL;
      	    if ($tmp['ACCEPT']) $acc_str=oREA_LOG_subs::ACCEPT_pref[$tmp['ACCEPT']];
      	    $output = $tmp['NAME']. '; Status:'.$acc_str;
  	    }
  	}
  	if ($colname=='ok_stat') {
  	    $answer = $this->devLib->anaOK_state($sqlo);
		$output = '';
		if ($answer=='o') {
			$output = '<span style="color: #008000">Working</span>';
		}
		if ($answer=='s') {
			$output = '<span style="color: #FF0000">STOP</span>';
		}
  	}
    if ($colname=='qualiStatus') {
        $result = $this->devLib->anaQualiState($sqlo);
        $output = '';
        
        if ($result['qualiok']==1) $output = 'Qualified';
        if ($result['qualiok']==2) $output = 'Quali_needed';
  	}
  	if ($colname=='qualiDate') {
  	    $result = $this->devLib->anaQualiState($sqlo, 1);
  	    $output = $result['qdate'];
  	}
  	
  	if ( $colname == "Thumbnail" ) {
  	   
  	    $relid = $this->attachLib->getThumbAttach( $sqlo, 'CHIP_READER', $idarr[0]);
  	    if ($relid) {
  	        $htmlTmp = $this->attachLib->getImgTag($relid);
  	        $output  = $htmlTmp;
  	    } else {
  	        
  	        $sqlsel = 'A_CHIP_READER_ID from CHIP_READER where CHIP_READER_ID='.$idarr[0] ;
  	        $sqlo->Quesel($sqlsel);
  	        $sqlo->ReadRow();
  	        $adev_id=$sqlo->RowData[0];
  	        if ($adev_id) {
      	        $relid  = $this->attachLib->getThumbAttach( $sqlo, 'A_CHIP_READER', $adev_id);
      	        if ($relid) {
      	            $htmlTmp = $this->attachLib->getImgTag($relid);
      	            $output  = $htmlTmp;
      	        }
  	        }
  	    }
  	}
  	
  	if ($colname=='device_dir') {
  		
  		if (is_object($this->dev_dir_lib)) {
  			$this->dev_dir_lib->set_device($idarr[0]);
  			
  			$cct_pid = $this->dev_dir_lib->get_cct_pid($sqlo);
  			if (!$cct_pid) {
  				// nothing
  			} else {
	  			
	  			$answer = $this->dev_dir_lib->get_device_dir_info($cct_pid);
	  			if ($error->Got(READONLY))  {
	  				$errLast   = $error->getLast();
					$error_txt = $errLast->text;
					$error->reset();
	  				$output = 'ERROR: '.$error_txt;
	  			} else {
	  				$output = $answer['dir_windows'];
	  			}
  			}
  		}
  	}
  	
	return ( $output );
  
  }
}
