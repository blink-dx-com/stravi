<?php
require_once ('f.update.inc');
require_once 'o.CHIP_READER.logs.inc';
require_once 'o.DEVSIC.subs.inc';

/**
 * main methods of CHIP_READER
 * DB-structure: see LATEX-document
 * STATUS_MX:Status Matrix: 
		 x: service is active \\
		 q: is qualified \\
		 n: qualification needed \\
		 o: working (see also s) \\
		 s: stop (see also o) \\

 * @package o.CHIP_READER.subs.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/phplib/o.CHIP_READER.logs.inc 59 2018-11-21 09:04:09Z $
 */
class oCHIP_READER_subs {
    
    private $_features = array();

    
    function set_dev($sqlo, $dev_id) {
        $this->dev_id=$dev_id;
        
        $sqlsel = '* from CHIP_READER where CHIP_READER_ID='.$dev_id;
        $sqlo->Quesel($sqlsel);
        $sqlo->ReadArray();
        $this->_features = $sqlo->RowData;
    }

	/**
	 * get CONTAINER of device
	 * @param object $sqlo
	 * @param int $dev_id
	 * @return $value
	 */
	function get_container($sqlo) {
	    if (!$this->dev_id) {
	        throw new Exception('Input: dev_id missing.');
	    }
	    $cont_id = glob_elementDataGet( $sqlo, 'CONTAINER', 'CHIP_READER_ID', $this->dev_id, 'CONTAINER_ID');
	    return $cont_id;
	}
	
	/**
	 * get all CONTAINER of device
	 * @param object $sqlo
	 * @param int $dev_id
	 * @return $value
	 */
	function get_containers_all($sqlo) {
	    if (!$this->dev_id) {
	        throw new Exception('Input: dev_id missing.');
	    }
	    
	    $cont_arr=array();
	    $sqlsel = "CONTAINER_ID from CONTAINER where CHIP_READER_ID=".$this->dev_id.' order by NAME';
	    $sqlo->Quesel($sqlsel);
	    while ( $sqlo->ReadRow() ) {
	        $tmpid = $sqlo->RowData[0];
	        $cont_arr[]=$tmpid;
	    }
	    
	    return $cont_arr;
	}
	
	/**
	 * get WORK/OK status
	 * @param object $sqlo
	 * @throws Exception
	 * @return string 'o' (Work) or 's' (stop)
	 */
	function anaOK_state(&$sqlo) {
	    if (!$this->dev_id) {
	        throw new Exception('Input: dev_id missing.');
	    }
	    $result='';
	    $mx = $this->_features['STATUS_MX'];
	    if (strstr($mx,'s')!=NULL) $result = 's';
	    if (strstr($mx,'o')!=NULL) $result = 'o';
	    return $result;
	}
	
	/**
	 * analyse current qualification state
	 * - 1:qualified means: substatus=4 (finished)
	 * @param object $sqlo
	 * @param int $advanced 0,1 : with 'qdate' ?
	 * @return array (
	 *   'qualiok'=>0:N/A, 1:qualified, 2: expected, 
	 *   'qdate'=>qualidate (human) 
	 *  )
	 */
	function anaQualiState(&$sqlo, $advanced=0) {
	    if (!$this->dev_id) {
	        throw new Exception('Input: dev_id missing.');
	    }
	    
	    $qualiok = 0;
	    $mx = $this->_features['STATUS_MX'];
	    if (strstr($mx,'q')!=NULL) $qualiok = 1;
	    if (strstr($mx,'n')!=NULL) $qualiok = 2;
	    
	    $result=array('qualiok'=>$qualiok);
	   
	    if ($advanced) {
	       $infoarr3 = oREA_LOG_subs::getMax_DevSic($sqlo, $this->dev_id, oDEVSIC_subs::TYPE_Q, oREA_LOG_subs::ACC_FINISHED ); // finished Qualifizierung
	       $result['qdate'] = $infoarr3['XDATE2'] ;
	    }
	    return $result;
	    
	}
	
	/**
	 * analyse current active service
	 * @param object $sqlo
	 * @return array ('active'=>0,1, 'sub'=>ACCEPT value);
	 */
	function anaServiceStatus($sqlo) {
	    $mx = $this->_features['STATUS_MX'];
	    if (strstr($mx,'x')==NULL) return array('active'=>0);
	    
	    $infoarr = oREA_LOG_subs::_getMaxKey($sqlo, $this->dev_id, oREA_LOG_subs::KEY_Service);
	    return array('active'=>1, 'sub'=>$infoarr['ACCEPT']);
	}
	
	/**
	 * update related CONTAINER.LOCATION
	 * @param object $sqlo
	 * @param string $dev_loc
	 * @throws Exception
	 */
	function update_container_loc(object $sqlo, string $dev_loc) {
	    if (!$this->dev_id) {
	        throw new Exception('Input: dev_id missing.');
	    }
	    if (!glob_column_exists('CONTAINER', 'LOCATION')) return;
	    
	    $cont_arr=array();
	    $sqlsel = "CONTAINER_ID from CONTAINER where CHIP_READER_ID=".$this->dev_id;
	    $sqlo->Quesel($sqlsel);
	    while ( $sqlo->ReadRow() ) {
	        $cont_arr[]=$sqlo->RowData[0];
	    }
	    if(empty($cont_arr)) {
	        return 0;
	    }
 
	    $upd_cnt=0;
	    foreach($cont_arr as $cont_id) {
	        $cont_loc = glob_elementDataGet( $sqlo, 'CONTAINER', 'CONTAINER_ID', $cont_id, 'LOCATION');
	        if ($cont_loc!=$dev_loc) {
	            $idarr = array('CONTAINER_ID'=> $cont_id);
	            if ($dev_loc==NULL) $dev_loc=NULL; // force to NULL
	            $c_argu=array(
	                'LOCATION'=>$dev_loc
	            );
	            gObjUpdate::update_row_s($sqlo, 'CONTAINER', $c_argu, $idarr);
	            $upd_cnt++;
	        }
	    }
	    return $upd_cnt;
	}
}

