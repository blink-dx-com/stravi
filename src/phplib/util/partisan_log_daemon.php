<?php

/**
 * g.PARTISAN_LOG > daemon
 * commandline:
 * #partisan_log_daemon.php 
 *   --noemail (OPTIONAL: do not send emails, just show debug text on console)
 *   --run  (start the analysis)
 *   --startDate DATE_VAL (OPTIONAL: Format DATE_VAL:Y-m-d; e.g. 2014-07-13)
 *   --verbose (OPTIONAL, show details of errors)
 *   
 * call it from: /opt/partisan# php phplib/util/partisan_log_daemon.php
 * - includes config/partisan_log_daemon.inc
 * - configure special TEXT-PATTERNS there
 * 
 * @package partisan_log_daemon.php
 * @swreq UREQ:4912 g.PARTISAN_LOG > daemon (FS-ID:FS-QA03a)
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param   string INPUT 
 * @version $Header: trunk/src/phplib/util/partisan_log_daemon.php 59 2018-11-21 09:04:09Z $
 * 
 * PARTISAN_LOG: FORMAT:
{Pure message text}
LOGIN: magasin_serial:1 login_error:1 :Partisan user name "Melanie" 
or password invalid!

{common error in GUI-module}
<head func="printAllEasy" phpfile="/pionir/obj.proj.edname.php" >
<info>oPROJ_newProjGui:preCheck:3:An other project with this name already 
exists in mother-project. 
Please give an other name.</info>
</head>

{common error in XMLRPC-module}
<head func="getAsTextAndLog" phpfile="exp_create_macro_xml" >
<info>_checkInput:103:Socket (NAME: 0123456789) not found.[/ERR]
create:2:Input check failed.[/ERR]
exp_create_macro_xml:1:creation of experiment failed.
</info></head>

{unexpected error somewhere in the code-stack}
CDB_OCI8:db_access OnExecute ('INSERT INTO SPOT_POS_IN_IMG 
(SPOT_ID, X_POS, Y_POS, IMG_ID)  
VALUES ('0', '216.0', '206.0', '1858186')'): ORA-1: 
ORA-00001: unique constraint 
(CCT.PK_SPOT_POS_IN_IMG) violated:
<qustack file:f.assocUpdate.inc func:query line:391>
<qustack file:f.assocUpdate.inc func:_insert_row line:185>
<qustack file:insert_ref_points2_xml.inc func:insert line:67>
<qustack file: func:insert_ref_points2_xml line:>
  
 */

/**
 * sort method
 * @param unknown $a
 * @param unknown $b
 * @return number
 */
function sort_by_cnt($a, $b) {
	if ($a['cnt']==$b['cnt']) return 0;

	if ($a['cnt']<$b['cnt']) return -1;
	else return 1;
}


/**
 * main worker class
 * @author steffen
 *
 */
class partisan_log_daemon {
	private $configFilePath='config/config.local.inc';
	private $daemonConfig  ='config/partisan_log_daemon.inc';
	private $logFilePath;
	private $ANALINES   = 300;
	private $NOTIFY_CNT_LIMIT=10;
	private $adminEmail = ""; // partisan@clondiag.com";
	private $sentEmailAddresses;
	private $emailCache; // array(EMAILADD=>array(text))
	private $dateStart_HUM; // start analysis date "Y-m-d"
	private $cnt_analysedLines=0;
	private $critical_out=array(); // store IDs of critical messages
	
	/* 
	 * all messages: array:
	 * ID => array( 
	 *   'txt'=>$use_infotxt,  all text within "<head ... >" or "<qustack"; e.g. <head func="getAsTextAndLog" phpfile="metacall_xml" >
	 *   'cnt'=>1, 
	 *   'firstLine'=>$firstline 
	 * )
	 * 
	 */
	private $textarray; 
	
	/**
	 * 
	 * @var array $options
	 *  'emailsend' : -1, 0, [1] send emails ?
	 *  'debug'   : 0,1
	 */
	private $options;
	
	private $_critical_message_DEFS;
	
	function __construct($options) {
		
		$this->options = $options;
		if ($this->options['emailsend']!=-1) $this->options['emailsend'] = 1;
		
		
		require_once($this->configFilePath);
		
		if (!file_exists($globals["app.logfile"]) ) {
			$this->exitError('logfile "'.$globals["app.logfile"].'" not found');
		}
		$this->logFilePath = $globals["app.logfile"];
		
		if (!file_exists($this->daemonConfig) ) {
			$this->exitError('daemon-config "'.$this->daemonConfig.'" not found');
		}
		require_once($this->daemonConfig);
		
		$this->config = $config; // from file $this->daemonConfig
		$this->_critical_message_DEFS = $config['critical_messages'];
		
		$this->adminEmail = $this->config['emailadd.sender'];
		
		$this->sentEmailAddresses = array();
		$this->emailCache = array();
		$this->cnt_analysedLines=0;
		$this->dateStart_HUM = NULL;
		
		if ($this->options['emailsend']<0) $this->print_on_console("INFO: Argument: EMAILS:NO");
	}
	
	/**
	 * print error text and exit
	 * @param string $text
	 */
	function exitError($text) {
		echo "ERROR: ".$text."\n";
		exit;
	}
	
	private function print_on_console($text) {
		echo $text."\n";
	}
	
	/**
	 * analyse text and $err_type
	 * - analyse only err_type: ERROR, INTERROR
	 * @param unknown $line
	 * @param unknown $oneLineArr
	 * @return number : number of found events of same type
	 */
	private function searchText($line) {
		
		$critical_types = array('ERROR', 'INTERROR');
		
		$oneLineArr= explode("\t",$line);
		$infotxt   = $oneLineArr[6];
		$err_type  = $oneLineArr[4];
		
		// search only ERRORs ...
		if (!in_array($err_type, $critical_types)) {
			return 0;
		}
		
		// analyse $infotxt
		$use_infotxt = $infotxt; // this is the chunk, used for detail analysis
		
		do {
			
			$keyx='<head';
			if (substr($infotxt,0,strlen($keyx)) == $keyx) {
				// starts with "<head", analyse just string between "<head" and ">"
				$endpos      = strpos($infotxt, '>');
				$use_infotxt = substr($infotxt, 0, $endpos+1);
				break;
				
			} else {
				// look for "<qustack"
				
				if (strstr($infotxt, '<qustack')!=NULL) {
					// just take string after first '<qustack'
					$startpos    = strpos($infotxt, '<qustack');
					$use_infotxt = substr($infotxt, $startpos);
					break;
				} else {
					// simple error, use original $infotxt
				}
				
				
			}
		} while (0);
		
		
		// analyse occurence of $use_infotxt
		$countout = 0;
		$found=0;
		foreach($this->textarray as $id => $oneEntry) {
			if ($oneEntry['txt']== $use_infotxt) {
				$this->textarray[$id]['cnt'] = $this->textarray[$id]['cnt'] + 1;
				$countout = $this->textarray[$id]['cnt'];
				$found=1;
				break;
			}
		}
		
		if (!$found) {
			
			// first occurence
			$countout = 1;
			// unset($oneLineArr[6]);
			$firstline = implode("; ",$oneLineArr);
			$this->textarray[]=array('txt'=>$use_infotxt, 'cnt'=>1, 'firstLine'=>$firstline);
		}
		return $countout;
		
	}
	
	private function addTextToAddress($emailAddress, $messOut) {
		if (!is_array($this->emailCache[$emailAddress])) $this->emailCache[$emailAddress] = array();
		$this->emailCache[$emailAddress][] = $messOut;
	}
	
	/**
	 * analyse one message
	 * @param array $message_arr 
	 * 	array('txt'=>main message, 'cnt'=>1, 'firstLine'=>$firstline)
	 * return boolean $isCritical
	 */
	private function message_is_critical( $message_arr) {
		if (!sizeof($this->_critical_message_DEFS)) return 0;
		
		$return_val = 0;
		
		foreach($this->_critical_message_DEFS as $critical_def) {
			if ($critical_def['txt']!=NULL) {
				if ( strstr($message_arr['txt'], $critical_def['txt'] ) !=NULL ) {
					$return_val=1;
					break;
					
				}
			}
			if ($critical_def['firstLine']!=NULL) {
				if ( strstr($message_arr['firstLine'], $critical_def['firstLine'] ) !=NULL ) {
					if ($critical_def['alarm_min_num']>0) {
						// only alarm after alarm_min_num occurences
						if ($message_arr['cnt']>=$critical_def['alarm_min_num']) {
							$return_val=1;
							break;
						}
					} else {
						$return_val=1;
						break;
					}	
					break;
					
				}
			}
		}
		return $return_val;
		
	}
	
	/**
	 * - prepare email sending
	 * - check, if more than $NOTIFY_CNT_LIMIT messages;
	 */
	private function prepareEmails() {
		
		if (!is_array($this->textarray)) return 0;
		
		
		foreach($this->textarray as $id => $oneEntry) {
			
			$isCritical = 0;
			do {
				if ($oneEntry['cnt']>=$this->NOTIFY_CNT_LIMIT) {
					$isCritical = 1;
					break;
				}
				
				if ( $this->message_is_critical($oneEntry) )  {
					$isCritical = 1;
					break;
				}
			
			} while (0);
			
			if (!$isCritical) continue;
			else {
				$this->critical_out[] = $id;
			}
			
			$emailAddress = $this->config['emailadd.sendto'];
			
			$messOut = NULL;
			
			$messOut .= 'Following error occurrs '.$oneEntry['cnt'].' times in the last time in the PARTISAN_LOG:'."\n";
			
			$messOut .= $oneEntry['txt'];
			$messOut .= "\n--- Other Log info of first occurence: ---\n";
			$messOut .= $oneEntry['firstLine'];
			
			
			
			$moreTextStandard = NULL;
			
			// send notification
			if (strstr($oneEntry['txt'], 'metacall_xml')  or  strstr($oneEntry['txt'], 'exp_create_macro_xml')   ) {
				$otherAddress = $this->config['emailadd.metacall_xml'];
				$this->addTextToAddress($otherAddress, $messOut);
				
				$moreTextStandard = "\n!!!!! Text also sent to: ".$otherAddress;
			}
			
			$this->addTextToAddress($emailAddress, $messOut . $moreTextStandard);
			
			
		}
		
		
		
	}
	
	/**
	 * send one email for SENDTO-address
	 * @return void|number
	 */
	private function email_send() {
		
		if (!sizeof($this->emailCache)) return 0;
		
		foreach ($this->emailCache as $emailAddress => $textarray) {
			
			$subjectUse   = 'Partisan: too many errors in PARTISAN_LOG';
			$headers      = 'From: Partisan <'.$this->adminEmail.">\r\n";
			$sender       =  $this->adminEmail;
			
			$messOut = NULL;
			$messOut .= 'Hi folks, '."\n";
			$messOut .= 'Following errors occurred too often in the last time in the PARTISAN_LOG.'."\n";
			$messOut .= 'First analysed Log-Timestamp: '.$this->dateStart_HUM .
					   '; Analysed LOG-lines: '.$this->cnt_analysedLines."\n";
			$messOut .= "\n";
			
			foreach ($textarray as $oneText) {
				$messOut .= $oneText;
				$messOut .= "\n------------------\n";
				
			}
			
			$messOut .= "\n\n";
			$messOut .= "-------------------------------------\n";
			$messOut .= 'MODULE-INFO: Message by partisan_log_daemon.php; UREQ:4912 g.PARTISAN_LOG > daemon'."\n";
			
			if ($this->options['emailsend']>0) {
				ini_set('sendmail_from', $sender);
				if (!mail($emailAddress, $subjectUse, $messOut, $headers, "-f".$sender)) {
					$this->exitError("Error during email sending. address:".$emailAddress );
					return;
				}
			} else {
				echo "NO-EMAIL-INFO: $emailAddress: TEXT:".$messOut."\n";
			}
				
			$this->sentEmailAddresses[] = $emailAddress;
		}
		
	}

	
	/**
	 * analyse last ANALINES lines of file
	 */
	function checkfile() {
		
		$this->print_on_console("INFO: logfile:".$this->logFilePath);
		
		$LINE_LENGTH = 32000;
		$dateToday_UNX = time();
		$this->dateStart_HUM = date('Y-m-d', $dateToday_UNX);
		$this->textarray = array(); // array of ID => array('txt'=>, 'cnt')
		
		if ($this->options['startDate']!=NULL) {
			// from command line
			$this->dateStart_HUM = $this->options['startDate'];
		}
		
		
		$FH = fopen($this->logFilePath, 'r');
		if ( !$FH ) {
			$this->exitError('logfile "'.$this->logFilePath.'" : could not open.');
			return;
		}
		
		$linecnt=1;
		while( !feof ( $FH ) ) {
			fgets($FH, $LINE_LENGTH);
			$linecnt++;
		}
		$lineMax = $linecnt;
		rewind($FH);
		
		$lineStart = $lineMax - $this->ANALINES;
		
		//
		// go to the start of ANALYSIS
		//
		$linecnt=1;
		$start_found=0;
		while( !feof ( $FH ) ) {
			$line = fgets($FH, $LINE_LENGTH);
			
			if ( $linecnt > $lineStart ) {
				// start date discover
				$oneLineArr = explode("\t",$line);
				$dateLoop   = $oneLineArr[0];
				if ($dateLoop >= $this->dateStart_HUM) {
					$start_found=1;
					break;
				}
			}
			
			$linecnt++;
		}
		
		if ($start_found)  {

			if ($this->options['debug']>0) {
				$this->print_on_console("INFO: FIRST_LINE: LINE_NO:$linecnt: ".$line);
			}
			
			// analyse first found line
			$cnt = $this->searchText($line);
			$this->cnt_analysedLines++;
			
			/**
			 * start analysis
			 */
			$this->cnt_analysedLines=0;
			
			while( !feof ( $FH ) ) {
				
				$line = fgets($FH, $LINE_LENGTH);
				
				//echo "DEBUG: $linecnt: ".$infotxt;
				
				$cnt = $this->searchText($line);
				
				//echo " CNT:".$cnt."\n";
						
				$linecnt++;
				$this->cnt_analysedLines++;
			}
			
			// analyse found array
			uasort ( $this->textarray, sort_by_cnt );
			
			$this->prepareEmails();
			if (sizeof($this->emailCache)) {
				$this->email_send();
			}
		}
		
		$this->print_on_console('INFO: First analysed Log-Timestamp: '.$this->dateStart_HUM .
			'; Analysed LOG-lines: '.$this->cnt_analysedLines);
		
		
		
		if (sizeof($this->sentEmailAddresses)) {
			$this->print_on_console("INFO: sent emails to: ".implode(', ', $this->sentEmailAddresses));
		}
		
		if ($this->options['debug']>0) {
			$this->print_on_console( "INFO: Critical messages: ".sizeof($this->critical_out) );
			
			if (sizeof($this->critical_out)) {
				$this->print_on_console('INFO: DEBUG: CRITICAL messgaes');
				foreach ($this->critical_out as $loopid) {
					$this->print_on_console('- '.$loopid.': CNT:'.$this->textarray[$loopid]['cnt'].'  FIRSTLINE:'.$this->textarray[$loopid]['firstLine']);
				}
			}
		}
		
	}
}

// check args
$runflag=0;

$options    = array();
$isArgVal   = 0;
$arg_value  = NULL;

foreach ($argv as $oneArg) {
	
	if ($isArgVal) {
		$arg_value = $oneArg;
		$oneArg    = $argName;
	}
	switch ($oneArg) {
		case "--noemail":
			$options['emailsend'] = -1;
			
			break;
		case "--run":
			$runflag=1;
			break;
		case "--startDate":
			if ($isArgVal) {
				$isArgVal=0;
				$options['startDate'] = $arg_value;
			} else {
				$isArgVal=1;
				$argName = $oneArg;
			}
			break;
		case "--verbose":
			$options['debug'] = 1;
					
			break;
	}
}

if (!$runflag) {
	echo "#php partisan_log_daemon.php --run --noemail --startDate --verbose Y-m-d\n";
	echo " STOP \n";
	exit;
}


$mainlib = new partisan_log_daemon($options);
$mainlib->checkfile();
