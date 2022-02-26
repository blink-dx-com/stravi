<?php
class UT_f_email_inc extends gUnitTestSub {

	function dotest( &$sqlo, $options ) {
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    $testlib = new fEmailC();
	    
	    $real_user_id = $this->_getDefObj_by_vario_key($sqlo, 'DB_USER', 'real_user');
	    $nick1 = glob_elementDataGet( $sqlo, 'DB_USER', 'DB_USER_ID', $real_user_id, 'NICK');
	    
	    $emailadd='steffen@blink-dx.com';
	    $subject ='UnitTest email üäµm sendMessage()';
	    $message ='This is a UnitTest. ÄÖÜäöü µm'."\n".'next line <a href="http://www.heise.de">heise</a> ok';
	    $send_opt=array('out.type'=>'html');
	    
	    $testlib->sendMessage($emailadd, $subject, $message, $FUNCNAME, $send_opt);
	    $logarr = $testlib->getmaillog();
	    $this->_infoNow( 'Info','01: sendMessage to '.$emailadd. ' Log:'.print_r($logarr,1));
	    
	    $send_opt = array('out.type'=>'html');
	    $subject ='UnitTest email via sendMessage_user()';
	    $testlib->sendMessage_user($sqlo, $real_user_id, $subject, $message, $FUNCNAME);
	    $logarr = $testlib->getmaillog();
	    $this->_infoNow( 'Info','02: sendMessage_user to user_id: '.$real_user_id. ' Log:'.print_r($logarr,1) );
	    
	    
	    $send_opt = array('out.type'=>'html');
	    $subject ='UnitTest email via sendgrpEmails()';
	    
	    $nickarr = array($nick1);
	    
	    $mailvalues = array(
	        'subject'=>$subject,
	        'message'=>$message
	    );
	    $testlib->sendgrpEmails($nickarr, $mailvalues, $sqlo);
	    $logarr = $testlib->getmaillog();
	    $this->_infoNow( 'Info','03: sendgrpEmails to user_id: '.$real_user_id. ' Log:'.print_r($logarr,1) );
		
		return 1;
	}

}
