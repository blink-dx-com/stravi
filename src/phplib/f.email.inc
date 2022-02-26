<?php
require_once('o.DB_USER.subs.inc');

/**
 * manage email from server to a user
 * @example <pre>
 * $mailLib = new fEmailC();
 * if ( $mailLib->userWantsEmail($sqlo, $userid) ) {
 *     $mailLib->sendMessage ( $emailadd, $subject, $message, $fromModule  );
 * }
 * </pre>
 * @package f.email.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class fEmailC {
	var $CLASSNAME='fEmailC';
	var $doSend; // do sending or not 
	
	function __construct() {
		$this->doSend=1;
	}
	
	/**
	 * set SEND-flag
	 * @param int $flag: 0,1
	 */
	public function send_flag($flag) {
	    $this->doSend=$flag;
	}
	
	/**
	 * check, if user with DB_USER_ID $userid want an emails
	 * - this function supports, that a user can deny email-sending
	 * - check $_SESSION['userGlob']['g.emailOpt']
	 * @param $sqlo
	 * @param $userid
	 * @param string $fromModule optional give module name
	 * @return $wantEmail 0,1
	 */
	function userWantsEmail(&$sqlo, $userid, $fromModule=NULL) {
		$wantEmail = 1;
		
		$userGlobVal = glob_elementDataGet2( $sqlo, 'USER_PREF', 
			array('DB_USER_ID'=>$userid, 'VAR_NAME'=>'g.emailOpt'), 
			'VALUE' );
		if ($userGlobVal==NULL) return $wantEmail;
		$tempArr  = unserialize($userGlobVal);
		$wantEmailTmp = $tempArr['send'];
		if ($wantEmailTmp<0) $wantEmail = 0;
		return $wantEmail;
	}
	
/**
	 * check, if user with DB_USER_ID $userid forwards an emails
	 * - this function supports, that a user can forward email-sending
	 * - check $_SESSION['userGlob']['g.emailOpt']
	 * @param $sqlo
	 * @param $userid
	 * @param string  $fromModule optional give module name
	 * @return int user_id
	 *   0: no forward !
	 */
	function user_forward(&$sqlo, $userid, $fromModule=NULL) {
		$forward_user_id = 0;
		
		$userGlobVal = glob_elementDataGet2( $sqlo, 'USER_PREF', 
			array('DB_USER_ID'=>$userid, 'VAR_NAME'=>'g.emailOpt'), 'VALUE' );
		
		if ($userGlobVal==NULL) return $forward_user_id;
		$tempArr  = unserialize($userGlobVal);
		$forwardNickTmp = $tempArr['forward'];
		if ($forwardNickTmp < 0) return 0;
		
		$nick = $tempArr['forwardto'];
		if ($nick==NULL) return $forward_user_id;
		
		$forward_user_id = DB_userC::get_ID_by_Nick($sqlo, $nick);
		
		return $forward_user_id ;
	}
	
	/**
	 * - send one email to a user
	 * - add some Partisan-text at end of message
	 * @param string $emailadd
	 * @param string $subject : UTF-8
	 * @param string &message : UTF-8
	 * @param string $fromModule sending-module, e.g. "oCYTORD_send"
	 * @param array $opt : 
	 * 	'cc'    : array of CC-email-adresses
	 *  'reply' : optional email-address for reply-to
	 *  'subject.pure' : 0,1 : if set 1: do not add the Partisan-preface to the subject
	 *  'out.type' : [text], 'html'
	 */
	function sendMessage ( $emailadd, $subject, $message, $fromModule, $opt=NULL  ) {
		$FUNCNAME = $this->CLASSNAME.":sendMessage";
		global  $error;
		$error    = & ErrorHandler::get();
		
		if ($opt['out.type']=='html') {
		    $mess_BR='<br>'."\n";
		} else {
		    $mess_BR="\n";
		}
		
		$sendername   = $_SESSION['s_product']['product.name'];
		$headers    = 'From: '.$sendername.' <'.$_SESSION['globals']["adminEmail"].">\r\n";
		$subjectUse = '';
		if ($opt['subject.pure']!=1) $subjectUse .= $sendername. ': '; // default signature to recognize a Gozilla-email
		$subjectUse .= $subject;
		
		if ($fromModule==NULL) {
			sys_error_my( $FUNCNAME.': No module name given.' );
		}
		
		//$baseurl = "http://".$_SERVER['SERVER_NAME'];
		$moduleinfo = $_SERVER['SERVER_NAME']."/MODULE=".$fromModule;
		$messEnd = "PS: Module: ".$moduleinfo . $mess_BR.
			'PS: Switch on/off personal emails: see user-preferences: "g.emailOpt"';

		$messOut = $message . $mess_BR . $mess_BR .$messEnd;
		
		if ($opt['cc']!=NULL) {
			$headers .= 'Cc: '.implode( ',',$opt['cc'] ) . "\r\n";
		}
		if ($opt['reply']!=NULL) {
			$headers .= 'Reply-To: '.$opt['reply'] . "\r\n";
		}
		if ($opt['out.type']=='html') {
		    $headers .= "Mime-Version: 1.0\r\n";
		    $headers .= "Content-type: text/html; charset=utf-8\r\n";
		} else {
		    $headers .= 'Content-Type: text/plain; charset=utf-8'."\r\n";
		}
		
		if ( $_SESSION["userGlob"]["g.debugLevel"]>1 ) {
			echo "DEBUG: mailto: ".$emailadd.
			'<br />headers:<pre>'.htmlspecialchars($headers)."</pre>\n".
			"<br />subject: ".$subjectUse."<br>\n".
			"<br />message:<pre>".htmlspecialchars($messOut)."</pre><br>\n";
		}
		
		if ( $this->doSend ) {
		    
			$sender = $_SESSION['globals']["adminEmail"];
			ini_set('sendmail_from', $sender);
			
			// https://ncona.com/2011/06/using-utf-8-characters-on-an-e-mail-subject/
			$subject_mail = '=?utf-8?B?'.base64_encode($subjectUse).'?=';
			if (!mail($emailadd, $subject_mail, $messOut, $headers, "-f".$sender)) {
				$error->set( $FUNCNAME, 2, "Error during email sending." );
				return;
			}
			
		}
	
	}
	
	/**
	 * send email to user, identified by $user_id
	 * - manage user forward 
	 * - manage flag userWantsEmail
	 */
	function sendMessage_user ( $sqlo, $user_id, $subject, $message, $fromModule, $opt=NULL  ) {
	    global  $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    //$user_feats = glob_elemDataGet3( $sqlo, 'DB_USER', array('DB_USER_ID'=>$user_id), array('EMAIL') );
	    //$emailadd   = $user_feats['EMAIL'];
	    
	    $userdata = $this->getuserdata_by_id($sqlo, $user_id);
	    
	    
	    if ( !$this->userWantsEmail($sqlo, $user_id) ) {
	        $this->maillog .='No mail sent to user ' . $userdata["address"] . ': <br /> User has switched
						off personal emails (Option g.emailOpt).<br />';
	        return;
	    }

	    $alter_user_id = $this->user_forward($sqlo, $user_id);
	    
	    if ( $alter_user_id ) {
	        $user_id = $alter_user_id;
	        $userdata = $this->getuserdata_by_id($sqlo, $alter_user_id);
	        
	        $this->maillog .='Mail forwarded to ' . $userdata["address"] .'<br />';
	        
	        if ( !$this->userWantsEmail($sqlo, $user_id) ) {
	            $this->maillog .='No mail sent to user ' . $userdata["address"] . ': (User-option g.emailOpt).<br />';
	            return;
	        }
	    }
	    
	    $this->maillog .= 'Send: '. $userdata["address"] . '; ';
	    $this->sendMessage( $userdata["address"], $subject, $message, $fromModule, $opt);
        if ($error->Got(READONLY))  {
            $error->set( $FUNCNAME, 2,'Error during email sending to '.$userdata["address"].'.' );
            return;
        }
        

	}
	
	/**
	 * -send email to many adresses
	 * @param array $addresses: list of nicknames of retrievers
	 * @param array $mailvalues
	 * 	subject : string
	 * 	message	: string
	 * @param $sqlo sql-handle
	 * @param array $send_opt
	 */
	function sendgrpEmails($addresses, $mailvalues, &$sqlo, $send_opt=array() ) {
		$FUNCNAME = __CLASS__.":sendgrpEmails";
		global $error;
		$error    = & ErrorHandler::get();
		
		$this->maillog ="";
		foreach ($addresses as $account) {
			
		    $userdata = $this->getuserdata_by_nick($account, $sqlo);
		    $user_id = $userdata['id'];
		    if ($user_id) {
		        $this->sendMessage_user ( $sqlo, $user_id, $mailvalues['subject'], $mailvalues['message'], $FUNCNAME, $send_opt  );
		        if ($error->Got(READONLY))  {
		            $error->set( $FUNCNAME, 2,'Error during email sending to '.$userdata["address"].' user-id:'.$user_id.'.' );
		            return;
		        }
		    }
		}
		
	}
	
	function getuserdata_by_nick($nick, &$sqlo){
		//$FUNCNAME = __CLASS__.":getuserdata";
		//global $error;
		$userdata = array();
		$sqls = "DB_USER_ID, EMAIL from DB_USER where NICK =" . $sqlo->addQuotes($nick);
		$sqlo->Quesel($sqls);
		$sqlo->ReadRow();
		$userdata["id"] = $sqlo->RowData[0];
		$userdata["address"] = $sqlo->RowData[1];
		return $userdata;
	}
	
	function getuserdata_by_id(&$sqlo, $user_id) {
	    $userdata = array();
	    $sqls = "NICK, EMAIL from DB_USER where DB_USER_ID = ".$user_id;
	    $sqlo->Quesel($sqls);
	    $sqlo->ReadRow();
	    $userdata["nick"]    = $sqlo->RowData[0];
	    $userdata["address"] = $sqlo->RowData[1];
	    return $userdata;
	}
	
	function reset_log() {
	    return $this->maillog=array();
	}
	
	function getmaillog() {
		return $this->maillog;
	}
}
 