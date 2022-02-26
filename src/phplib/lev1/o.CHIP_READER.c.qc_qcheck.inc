<?php
require_once("access_check.inc");
require_once("date_funcs.inc");
require_once("f.email.inc");
require_once 'o.CCT_ACCLOG.subs.inc';
require_once("lev1/o.CHIP_READER.serviceInfoRule.inc");


/**
 * analyze all devices for VERIFICATION-date and MAINTENANCE-date and send emails to users
 * will be called by a cronjob-script;
 * @package o.CHIP_READER.c.qc_qcheck.inc 
 * @swreq   UREQ:77 o.CHIP_READER > QC_Check > next maintainace
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * 
 */
class o_CHIP_READER_qc_Mess {
	
    
    var $refDateUnx; // reference date (UNIX-stamp)
    private $send_email_flag=1;
    private $expired_dev_users; // array[$user_id] = $expDevices
    
    function __construct(&$sqlo) {
    	$this->qcDevSQLLib = new oCHIP_READER_servicInfoRule($sqlo);
    	$this->emailLib    = new fEmailC();
    	$this->expired_devices_ADM = array();
    	$this->expired_dev_users   = array();
    }
    
    function set_email_flag($flag) {
        $this->send_email_flag=$flag;
    }
    
    /**
     * send one email to a user with ALL relevant devices
     * @param array &$sqlo
     * @param array &$expDevices
     * @param bool $isAdmin 1: is an email to the admin (add responsible contact info)
     * @return String $message
     */
    function getMessage( &$sqlo, &$expDevices, $isAdmin=0 ) {
//         $FUNCNAME= __CLASS__.':'.__FUNCTION__;
//     	global $error;
//     	$error    = & ErrorHandler::get();
    
    	if ( $isAdmin ) {
    		$messageHead = 'Regulated-Equipment-AdministatorIn';
    	} else {
    		$messageHead = 'Regulated-Equipmentianerin';
    	}
    	
    	foreach( $expDevices as $objid=>$expireDate) {
    		$sqlsel = 'NAME, RES_USER_ID, SERIAL, RES_GROUP_ID from CHIP_READER where CHIP_READER_ID='.$objid;
    		$sqlo->Quesel($sqlsel);
    		$sqlo->ReadRow();
    		$qcname    = $sqlo->RowData[0];
    		$user_id   = $sqlo->RowData[1];
    		$serial    = $sqlo->RowData[2];
    		$grp_id    = $sqlo->RowData[3];
    		
    		if (strlen($qcname)>32) $qcname = substr($qcname,0,29)."...";
    		if (strlen($serial)>13) $serial = substr($serial,0,10)."...";
    		$moreinfo .= sprintf("| %-7.7s| %-32.32s| %-13.13s| %-12.12s| %-4.4s|", $objid, $qcname, $serial, $expireDate["exdate"], $expireDate["typ"]);
    		if ($isAdmin) {
    			if ($user_id) {
    			    $contname = obj_nice_name ( $sqlo, 'DB_USER', $user_id ); 
    				$moreinfo .= sprintf(" %-20.20s|",$contname);
    			} else {
    				$moreinfo .= sprintf(" %-20.20s|","!MISSING!");
    			}
    			if ($grp_id) {
    			    $contname = obj_nice_name ( $sqlo, 'USER_GROUP', $grp_id );
    			    $moreinfo .= sprintf(" %-20.20s|",$contname);
    			} else {
    			    $moreinfo .= sprintf(" %-20.20s|","!MISSING!");
    			}
    		} 
    		$moreinfo .="\n";
    	}
    	
    		
    	// $baseurl = "http://".$_SERVER['SERVER_NAME'];
    
    	$message = "Liebe ".$messageHead.",\n".
    	"dies ist eine liebe Post von der Gozilla-QC-Kontrolleurin."."\n".
    	"Folgende Geraete sind als"."\n".
    	"verifizierungs-/wartungswuerdig gekennzeichnet."."\n".
    	"D.h. die Termine fuer die naechste Verifizierung(Typ V) oder"."\n".
    	"naechste Wartung(Typ W) sind in spaetestens 28 Tagen faellig."."\n".
    	"Du bist fuer diese Geraete verantwortlich (so steht es zumindest in der Datenbank).\n\n".
    	"Typ:\n".
    	"  V = Verifizierung\n".
    	"  W = Wartung\n\n\n";
    	$sep = "---------------------------------"; #max length 33
    	$linesep = sprintf("+%-8.8s+%-33.33s+%-14.14s+%-13.13s+%-5.5s+", $sep, $sep, $sep, $sep, $sep );
    	if ($isAdmin) {
    	    $linesep .= sprintf("%-21.21s+", $sep);
    	    $linesep .= sprintf("%-21.21s+", $sep);
    	}
    	$message .= $linesep."\n";
    	$message .= sprintf("| %-7.7s| %-32.32s| %-13.13s| %-12.12s| %-4.4s|", "DEV-ID", "Device-Name", "Serial", "Expiry-Date", "Typ");
    	if ($isAdmin) {
    	    $message .= sprintf(" %-20.20s|", "Responsible User");
    	    $message .= sprintf(" %-20.20s|", "Responsible Group");
    	}
    	$message .= "\n".$linesep."\n";
    	$message .= $moreinfo.$linesep."\n\n".
    		"PS: Version: 2020-10-20 ; Hilfe: TBD:\n";
    	return $message;
    }
    
    /**
     * send one email to a user with ALL relevant devices
     * @param string &message
     * @param string $emailadd
     * @param bool $doSend 1: send mail or not
     * @return null
     */
    function sendMessage ( $message, $emailadd, $doSend=1 ) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
    	global $error;
    	$error    = & ErrorHandler::get();
    
    	if ( $_SESSION["userGlob"]["g.debugLevel"]>1 ) {
    		echo "DEBUG: mailto: ".$emailadd." message:<pre>".htmlspecialchars($message)."</pre><br>\n";
    	}
    	$refDateUnx = $this->refDateUnx;
    	$refDateHuman = date_unix2datestr($refDateUnx,3);
    
    	if ( $doSend ) {
    		$this->emailLib->sendMessage ( $emailadd, "Reg_Devices: ZielDatum: $refDateHuman",
    			$message, $FUNCNAME  );
    		if ($error->Got(READONLY))  {
    			$error->set( $FUNCNAME, 2, 'Error during email sending to '.$emailadd.'.' );
    			return;
    		}
    	}
    }
    
    /**
     * send one email to a user with ALL relevant devices
     * @param array &$sqlo
     * @param string $emailadd
     * @param array &$expDevices
     * @param bool  $isAdmin 1: is an email to the admin (add responsible contact info)
     * @return null
     */
    function __Mail2User( &$sqlo, $emailadd, &$expDevices, $isAdmin=0 ) {
    	$message = $this->getMessage( $sqlo, $expDevices, $isAdmin );
    	$this->sendMessage( $message, $emailadd, $this->send_email_flag );
    }
    
    /**
     * - send email to QC-Admin
     * - send one email to admin: contains ALL devices, which need maintenance
     * @param array &$sqlo
     * @return null
     */
    function _sendQc_Admin(&$sqlo) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
    	global $error;
    	$error    = & ErrorHandler::get();
    
    	$refDateUnx = $this->refDateUnx;
    	$globColVal = "lab.CHIP_READER.qc_device.email";
    	$emailadd = glob_elementDataGet( $sqlo, "GLOBALS", "NAME", $globColVal, "VALUE");
    	if ( $emailadd=="" ) {
    		$error->set( $FUNCNAME, 1, "no email-address set in GLOBALS[".$globColVal."]." );
    		return;
    	}
    
    	$expDevices = $this->qcDevSQLLib->getUrgentDevices($sqlo, $refDateUnx );
    	$this->expired_devices_ADM = &$expDevices;
    	if (empty($expDevices )) {
    		// nothing to do
    		return ;
    	}
    	
    	$this->__Mail2User( $sqlo, $emailadd, $expDevices, 1 );
    }
    
    /**
     * send one email to the DB_USER $user_id
     * @param  array &$sqlo
     * @param  int $contactID
     * @param  int $refDateUnx
     * @return null
     */
    private function _oneContactMail(&$sqlo, $user_id, $refDateUnx) {
 
    	// get email-address
    	$emailadd =	glob_elementDataGet( $sqlo, "DB_USER", "DB_USER_ID", $user_id, "EMAIL");
    	if ($emailadd=="") {
    		$this->savedErr[]='USER-ID:'.$user_id.' has no emailaddress.';
    		return;
    	}
    	
    	$expDevices = $this->qcDevSQLLib->getDevOneContact($sqlo, $user_id, $refDateUnx );
    	$this->expired_dev_users[$user_id] = $expDevices;
    	if (empty($expDevices )) {
    		// nothing to do
    		return;
    	}
    	$this->__Mail2User( $sqlo, $emailadd, $expDevices );
    }
    
    /**
     * send email to ALL responsible users
     * @param  array &$sqlo
     * @return void
     */
    function _sendQC_Users(&$sqlo) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
    	global $error;
    	$error    = & ErrorHandler::get();
    
    	$refDateUnx = $this->refDateUnx;
    	$users      = $this->qcDevSQLLib->getUrgDevPerUser($sqlo, $refDateUnx );
    	if ( empty($users) ) return; 
    	$errstore = array();
    
    	foreach( $users as $user_ID) {

    	    $this->_oneContactMail($sqlo, $user_ID, $refDateUnx);
    		if ($error->Got(READONLY))  {
    			$tmp=$error->getLast();
    			$errstore[]='user ID:'.$user_ID.' '.$tmp->text;
     			$error->reset();
    		}
    
    	}
    
    	if(!empty($errstore)) {
    	    $error->set( $FUNCNAME, 10, 'bad emails: '.implode('; ',$errstore) );
    	}
    }
    
    /**
     * sets the class variable refDateUnx
     * @param  string $refDateUnx
     * @return null
     */
    function _initRefDate( $refDateUnx ) {
    	$this->refDateUnx = $refDateUnx;
    }
    
    /**
     * entry for the "Test QC_DEVICE maintenance" modul
     * @param array &$sqlo
     * @return String $os (or null)
     */
    function getAdminInfo(&$sqlo) {
        
    	
    	$daysInSec = 3600 * 24;
    	//$refDateUnx = time() + 7 * $daysInSec; // next week
    	$refDateUnx = time() + 28 * $daysInSec; //next month
    	$this->_initRefDate( $refDateUnx );
    	
    	$expDevices = $this->qcDevSQLLib->getUrgentDevices($sqlo, $refDateUnx );
    	if (empty($expDevices )) {
    		// nothing to do
    		return 'noting found.';
    	}
    	
    	$os = $this->getMessage( $sqlo, $expDevices, 1 );
    	
    	return $os;
    }
    
    /**
     * send email to QC-Admin and ALL responsible users
     * @param array &$sqlo
     * @return null
     */
    function sendEmails( &$sqlo ) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
    	global $error;
    	$error    = & ErrorHandler::get();
    
    	$errstore = NULL;
    
    	$daysInSec = 3600 * 24;
    	//$refDateUnx = time() + 7 * $daysInSec; // next week
    	$refDateUnx = time() + 28 * $daysInSec; // next month
    	$this->_initRefDate( $refDateUnx );
    	
    	$this->_sendQc_Admin($sqlo);
    	if ($error->Got(READONLY))  {
    		$errstore = $error->getLast();
    	}
    
    	$this->_sendQC_Users($sqlo, $refDateUnx);
    	
    	if ($errstore!= NULL) {
    		$error->set( $FUNCNAME, 2, "_sendQc_Admin: ".$errstore->text );
    	} 
    	
    	if ( !empty($this->savedErr) ) {
    		$error->logxMeta('WARN',4,$FUNCNAME,$_SERVER['PHP_SELF'],implode(',', $this->savedErr));
    	}
    }

}	
