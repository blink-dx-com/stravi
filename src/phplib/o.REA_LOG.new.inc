<?php
require_once ("date_funcs.inc");
require_once ("f.assocUpdate.inc");
require_once ("glob.obj.update.inc");
require_once ("o.REA_LOG.KEY_Booking.inc");
require_once 'o.REA_LOG.subs.inc';
require_once 'o.DEVSIC.subs.inc';
require_once 'o.CHIP_READER.logs.inc';


/**
 * create new entry for REA_LOG
 * 
 * @package o.REA_LOG.new.inc
 * @swreq UREQ:0000921: o.CHIP_READER > REA_LOG : service log abbilden 
 * subreqs:
 * - if a service > type QUALIFICATION failed (Status: finished_NIO) then set a new STOP-entry
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)

 */
class oREA_LOG_new {
	
    function __construct() {
    	$this->assoclib    = new  fAssocUpdate();
    	$this->validateLib = new fValidate();
    	$this->key_Booking_lib = new oREA_LOG_KEY_Booking();
    }
    	
    function setDevice($deviceid) {
    	$this->deviceid=$deviceid;
    }
    
    function checkAccess(&$sqlo) {
    	global $error;
    	$FUNCNAME= 'addEntry';
    	$o_rights = access_check($sqlo, 'CHIP_READER', $this->deviceid);
    	if ( !$o_rights["insert"]) {
    		$error->set( $FUNCNAME, 1,"You do not have write permission on this device!");
    		return;
    	}
    	
    }
    
    /**
     * update an XDATA entry by key with new data
     * @param string $key
     * @param array $data
     */
    public static function DATAX_update_entry(&$xdata, string $key, $data) {
        
        $found=0;
        $data['KEY'] = $key;
        
        foreach($xdata as $i=>$row) {
            $key_loop=$row['KEY'];
            if ($key_loop==$key) {
                $xdata[$i]=$data;
                $found=1;
                break;
            }
        }
        if (!$found) $xdata[]=$data; // append
        return;
    }
    
    // edit allowed by ACCEPT-flag? return: 0 or 1
    public static function edit_allowed($accept) {
        $allowed=1;
        if ( $accept==oREA_LOG_subs::ACC_FINISHED or $accept==oREA_LOG_subs::ACC_FINISHED_NIO or $accept==oREA_LOG_subs::ACC_NOTDONE) {
            $allowed=0;
        }
        return $allowed;
    }
    
    /**
     * check parameters for plausibility
     *  - KEY=Service: XDATE must not be in future, if ACCEPT>3
     * @param int $pos can be NULL (for new entries)
     * @return NULL : o.k.
              array(errflag, $errtext) ERROR
     */
    function paramCheck(&$sqlo, $argu, $pos=0) {
    	$now = time();
    	$refTimeStr = date_unix2datestr( $now + date_unixDay2Sec(1), 3 );
    	
    	if ($argu['NAME']==NULL ) {
    		return array(10, 'Short description missing.' );
    	}
    	
    	if (strlen($argu['NAME'])>100 ) {
    		return array(11, 'Short description: too long; max: 100 Zeichen.' );
    	}
    	
    	if ($argu['KEY']=='Service' or $argu['KEY']=='Qualifizierung') {
    		if ($argu['ACCEPT']<=0) {
    			return array(20, 'substatus must be set.');
    		}
    		if ($argu['ACCEPT']>oREA_LOG_subs::ACC_ACCEPTED) { // DATE must not be in the future (+1 day)
    			if ($argu['XDATE']>$refTimeStr) {
    				return array(21, '"Start date" is only allowed in FUTURE, if substatus="planned".');
    			}
    		}
    		if ($argu['ACCEPT']==oREA_LOG_subs::ACC_FINISHED or $argu['ACCEPT']==oREA_LOG_subs::ACC_FINISHED_NIO) { // DATE must not be in the future (+1 day)
    			if ($argu['XDATE2']==NULL) {
    				return array(22, 'need "end date",if substatus="finished".');
    			}
    		}
    		
    	}
    	
    	if ($argu['KEY']=='Booking') {
    		if ($argu['XDATE2']==NULL) {
    			return array(25, 'Booking: Start-Date and End-Date must be set!');
    		}
    	}
    	
    	
    	if ($argu['XDATE']==NULL) {
    		return array(30, '"start date": value missing.');
    	}
    	
    	if ($argu['XDATE']!=NULL) {
    		// check date
    		$info = $this->validateLib->check_type($argu['XDATE'], 'DATE2');
    		if ($info['ok']<1) {
    			return array(35, 'XDATE: Format-Error: '.$info['info']);
    		}
    	}
    	if ($argu['XDATE2']!=NULL) {
    		$info = $this->validateLib->check_type($argu['XDATE2'], 'DATE2');
    		if ($info['ok']<1) {
    			return array(40, 'XDATE2: Format-Error: '.$info['info']);
    		}
    		if ($argu['XDATE2'] < $argu['XDATE']) {
    			return array(42, 'XDATE2: "End time" must be older than "Start time"!');
    		}
    	}
    	
    	if ( $this->deviceid ) {
    		
    		if ($argu['KEY']=='Booking') {
    		
    			$this->key_Booking_lib->initDev($sqlo, $this->deviceid);
    			$answer = $this->key_Booking_lib->checkBlock($sqlo, $argu['XDATE'], $argu['XDATE2'], $pos);
    			if ($answer[0]>0) {
    				return array(50, 'Booking: device already booked within this period!');
    			}
    		}
    	}
    	
    }
    
    /**
     * manage KEY = Service, Qualifizierung
     * @param object $sqlo
     * @param array $argu
     */
    private function _STATUS_MX_actions($sqlo) {
        
        $main_table='CHIP_READER';
        if (!glob_column_exists($main_table, 'STATUS_MX')) return;

        $old_val = glob_elementDataGet( $sqlo, $main_table, 'CHIP_READER_ID', $this->deviceid, 'STATUS_MX') ;
        
        $devLogLib =  new oREA_LOG_subs();
        $new_val = $devLogLib->calc_STATUS_MX($sqlo, $this->deviceid);

        if ($new_val!=$old_val) {
            
            // update silently
            
            $UpdateLib = new globObjUpdate();
            $args  = array('vals'=>  array('STATUS_MX'=>$new_val) );
            $idarr =array('CHIP_READER_ID'=>$this->deviceid );
            $UpdateLib->update_raw($sqlo, $main_table, $args, $idarr);
            
        }
    }
 
    /**
     * do POST modification actions
     * - check, if a new STOP-entry should be set 
     * - calculate the STATUS_MX field
     * @param object $sqlo
     * @param int $pos
     */
    private function _update_post_act(object $sqlo, int $pos) {
        $devid = $this->deviceid;
        
        // analyse, if qualificaion produced NIO => set to NOT-working
        
        $sqlsel="* from REA_LOG where CHIP_READER_ID=".$devid . ' and POS='.$pos;
        $sqlo->Quesel($sqlsel);
        $sqlo->ReadArray();
        $old_args = $sqlo->RowData;
        
        $ACCEPT   = $old_args['ACCEPT'];
        $devsic_id= $old_args['DEVSIC_ID'];
        if ($devsic_id) {
            $devsic_fea = oDEVSIC_subs::features($sqlo, $devsic_id);
            if ($devsic_fea['TYPE']==oDEVSIC_subs::TYPE_Q) {
                // is a qualification entry ...
                if ( $ACCEPT==oREA_LOG_subs::ACC_FINISHED_NIO) {
                    
                    $last_status = oREA_LOG_subs::ana_OKState($sqlo, $devid);
                    if ($last_status!=oREA_LOG_subs::KEY_STOP) {
                        // add STOP-entry
                        $args_new = array (
                            'NAME' => 'Auto-STOP',
                            'XDATE'=> date_unix2datestr( time(), 1),
                            'KEY'  => oREA_LOG_subs::KEY_STOP,
                            'NOTES'=>'Stop auto set by a failed Qualification-Service (Pos:'.$pos.'): Status: Finished_NIO'
                        );
                        $this->addEntry( $sqlo, $args_new);
                    }
                }
            }
        }
        
        $this->_STATUS_MX_actions($sqlo);
    }
    
    /**
     * insert e new entry of REA_LOG
     * @param $sqlo
     * @param $parx : "NAME", "KEY", "NOTES", [XDATE]
     * 	  keys are not allowed: POS, DB_USER_ID
     * @param array $options
     *   'noAccCheck'=>0,1
     * @return $newpos of entry
     */
    function addEntry( &$sqlo, $parx, $options=array()) {
    	global $error;
    	$FUNCNAME= 'addEntry';
    	
    	$id = $this->deviceid;
    	if (!$id) {
    		$error->set( $FUNCNAME, 1, 'no device set.' );
    		return;
    	}
    	
    	$allowKeys = array ('NAME','XDATE','XDATE2','KEY','PRIO','ACCEPT','NOTES','DEVSIC_ID', 'XDATA');
    	
    	// get last entry
    	$sqls="select max(POS) from REA_LOG where CHIP_READER_ID=".$id;
    	$sqlo->query($sqls);
    	$sqlo->ReadRow();
    	$lastpos = $sqlo->RowData[0];
    	if ($lastpos=="" OR $lastpos<=0) $lastpos = 0;
    	$newpos = $lastpos + 1;
    	
    	if ($parx['XDATE']!=NULL) {
    		$nowdateStr = $parx['XDATE'];
    	} else {
    		$nowdateStr = date_unix2datestr( time(), 1);
    	}
    	
    	// copy allowed parameters 
    	$argu=NULL;
    	
    	foreach( $allowKeys as $key) {
    		if ($parx[$key]!==NULL) $argu[$key] = $parx[$key];
    	}
    	
    	
    	$argu["POS"] 	= $newpos;
    	$argu["DB_USER_ID"] = $_SESSION['sec']['db_user_id'];
    	$argu["XDATE"] 	= $nowdateStr;
    	
    	$inopt = array(); // array("types" => array("XDATE" => "DATE1") );
    	
    	$set_opts = array();
    	if ($options['noAccCheck']>0) {
    		$set_opts['noAccCheck']=1;
    	}
    	$this->assoclib->setObj( $sqlo, "REA_LOG", $id, $set_opts );
    	if ($error->Got(READONLY)) {
    		$error->set( $FUNCNAME, 1, 'error on object set' );
    		return;
    	}
    	
    	$retval = $this->assoclib->insert( $sqlo, $argu, $inopt );
    	if ($retval<=0) {
    		$error->set( $FUNCNAME, 1, 'SQL insert failed!' );
    		return;
    	}
    	$this->_update_post_act($sqlo, $newpos);
    	
    	
    	return $newpos;
    }
    
    /**
     * update one entry
     * - update of KEY not allowed !
     * @param array $options
     *   'noAccCheck'=>0,1
     */
    function updateEntry(&$sqlo, $pos, $parx, $options=array() ) {
    	global $error;
    	$FUNCNAME= __CLASS__.':updateEntry';
    	
    	$id = $this->deviceid;
    	if (!$id) {
    	    $error->set( $FUNCNAME, 1, 'no device set.' );
    	    return;
    	}
    	if (!$pos) {
    	    $error->set( $FUNCNAME, 2, 'POS not set.' );
    	    return;
    	}
    	
    	$sqlsel="* from REA_LOG where CHIP_READER_ID=".$id . ' and POS='.$pos;
    	$sqlo->Quesel($sqlsel);
    	$sqlo->ReadArray();
    	$old_args = $sqlo->RowData;
    	
    	if (array_key_exists('KEY', $parx)) { 
    	    if ($old_args['KEY']!=$parx['KEY']) {
    	       $error->set( $FUNCNAME, 3, '"KEY" can not be modified.' );
    	       return;
    	    }
    	}
    	
    	$set_opts = array();
    	if ($options['noAccCheck']>0) {
    		$set_opts['noAccCheck']=1;
    	}
    	
    	$this->assoclib = new  fAssocUpdate();
    	$this->assoclib->setObj( $sqlo, "REA_LOG", $id, $set_opts );
    	if ($error->Got(READONLY)) {
    		$error->set( $FUNCNAME, 4, 'error on object set' );
    		return;
    	}
    	
    	// plaus check; check date ???
    	
    	
    	$idarr = array('POS'=>$pos);
    	$parx["DB_USER_ID"] = $_SESSION['sec']['db_user_id']; // update user
    	$parx["MOD_DATE"]   = date_unix2datestr(time(),1);
    	$this->assoclib->update( $sqlo, $parx, $idarr );
    	if ($error->Got(READONLY)) {
    		$error->set( $FUNCNAME, 5, 'SQL update failed!' );
    		return;
    	}
    	
    	$this->_update_post_act($sqlo, $pos);
    }
    
    /**
     * get POS of last entry of KEY
     * @param object $sqlo
     * @param string $key
     */
    function get_last_entry_of_key($sqlo, $key) {
    	global $error;
    	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
    	
    	$id = $this->deviceid;
    	if (!$id) {
    		$error->set( $FUNCNAME, 1, 'no device set.' );
    		return;
    	}
    	
    	$sqlsel = 'POS, XDATE from REA_LOG where CHIP_READER_ID='.$id.' and KEY='. $sqlo->addQuotes($key).
    		' order by XDATE DESC';
    	$sqlo->Quesel($sqlsel);
    	$sqlo->ReadRow();
    	$pos   = $sqlo->RowData[0];
    	
    	return $pos;
    	
    }
    
    

}

 