<?php
// $Header: trunk/src/phplib/f.xmlrpc_cli.inc 59 2018-11-21 09:04:09Z $
require_once 'ErrorHandler.inc';

/**
 * XMLRPC client functions for the PARTISAN-xmlrpc interface
 * - can use the simple module: ErrorHandler_simu.inc
 * @package f.xmlrpc_cli.inc
 * @swreq   SREQ:0001806: g > XML-RPC-Client: provide an xml-rpc-client in PHP 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @global 
 *   $_SESSION['userGlob']["g.debugLevel"] : just for debugging (optional)
 *   $error  error-library
 * @unittest exists!
 * @example
 *  $global $error;
  	$obj1 = new fXmlrpcCli($connect_params);
	$obj1->connect();   // can set $error
	$answer = $obj1->funcCall("get_version", $params); // can set $error
	if ($error->Got(READONLY))  {
		$errLast   = $error->getLast();
		return;
	}
 */
class fXmlrpcCli{

var $connpar; /* connect-parameters array(
	"cct_user"    => 'steffen', 
	"cct_password"=> 'xxxx', 
	"dbid"        => 'alere' 
	"hostname"    => 'magasin.clondiag.com'
	"srv_url"     => '/pionir/xmlrpc/icono_svr.php'
	"protcol"     => OPTIONAL: ['http'], 'ssl'
	)
	
*/
var $sess_id; // session_id
var $debugx;
var $CLASSNAME = "fXmlrpcCli";

/**
 * init class
 * @param array $connectParams : see $this->connpar
 */
function __construct( $connectParams  ) {
	
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	$error = & ErrorHandler::get();
	
	$this->senderEncoding = 'ISO-8859-1';
		
	$this->connpar = $connectParams;
	$this->sess_id = NULL;
	$this->debugx  = 0;
	if ($_SESSION['userGlob']["g.debugLevel"]>1) {
		$this->debugx = $_SESSION['userGlob']["g.debugLevel"];
	}
	
	
}

function getConnectionParams() {
	return $this->connpar;
}

function setConnectionParams($connectParams) {
	$this->connpar = $connectParams;
	
	if ($this->debugx>2) {
		echo "DEBUG: fXmlrpcCli:setConnectionParams: connparams: ".print_r($this->connpar,1)."<br>";
	}
}

function getSenderEnc() {
	return $this->senderEncoding;
}

public function getSessionID() {
	return $this->sess_id;
}

function strip_h($text){
	
	// analyse XML-part
	 $xml_start = strpos($text, "<?xml");
	 $xml_part  = strpos($text, "?>", $xml_start);
	 if ($xml_part>0) {
	 	$xml_str = substr($text,$xml_start,$xml_part-$xml_start);
	 	if ( stristr( $xml_str, 'UTF-8') === FALSE ) {
	 		// nothing
	 	} else {
	 		$this->senderEncoding = 'UTF-8';
	 	}
	 }
	 
     return strstr($text,"<?xml");
}

/**
 * - new easy send; 
 * - old method send_OLD() did not work any more (2017-08-03, Steffen)
 * @param object $request
 * @return string
 */
function send($request) {
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	$context = stream_context_create(array('http' => array(
			'method' => "POST",
			'header' => "Content-Type: text/xml",
			'content' => $request
	)));
	
	$hostname        = $this->connpar["hostname"];
	$hostname_new='http://'.$hostname;
	if ($this->connpar["protocol"]=='ssl') {
		$hostname_new = 'https://'.$hostname;
		$port=443;
	}
	$url= $hostname_new.$this->connpar["srv_url"];
	
	if ($this->debugx>1) echo $FUNCNAME.":DEBUG:URL:".$url."<br>";
	
	$file = file_get_contents($url, false, $context);
	return ($file);
}

/*
function send_OLD(
	$message
	){
	global $error;
	$FUNCNAME = $this->CLASSNAME.":send()";
	// FUNCTION: open socket to server
	
	$errno ="";
	$errstr="";
	$retstr="";
	
	$port=80;
	$hostname        = $this->connpar["hostname"];
	$xml_rpc_srv_str = $this->connpar["srv_url"];
	
	if ($this->connpar["protocol"]=='ssl') {
		$hostname = 'tls://'.$hostname; // 'ssl:'
		$port=443;
		
	}
	
	$sk = fsockopen($hostname,$port,$errno,$errstr,1000);
	
	if(!$sk){
		$error->set($FUNCNAME, 1, "open host '$hostname' failed. Error:".$errstr);
		return;
	}
	else{
		$str="";
		$str.="POST ".$xml_rpc_srv_str." HTTP/1.0 \r\n";
		$str.="User-Agent: xmlrpc-php-test/0.2 (PHP) \r\n";
		$str.="Content-type: text/xml \r\n";
		$str.="Content-length: ".strlen($message)."\r\n\r\n";
		$str.=$message;

		fputs($sk,$str);

		while(!feof($sk)){
			$resp=fgets($sk,80);
			$retstr.=$resp;
			// if($display) echo $resp;
		}

		fclose($sk);
		return $retstr;
	}
}
*/

// set session-ID
function setSessionID($sess_id) {
	$this->sess_id=$sess_id;
	
}

/**
 * call a XML-RPC function
 * @param $funcname e.g."batch_qa_set"
 * @param array $params one_value or array of params
 * @param array $opt <pre>
 * 
 * </pre>
 * @return array $my_data1
 */
function funcCall(
	$funcname, 
	&$params,
	$opt=NULL  
	) {
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	$tmparr = NULL;
	$tmparr[] = $this->sess_id;
	
	
	if (!is_array($params)) $tmparr[] = $params;
	else {
		foreach( $params as $key=>$val) {
			$tmparr[] = $val;
		}
		reset ($params); 
	}
	
	$encOpt       = array('encoding'=>'UTF-8','escaping'=>'markup');
	$my_request_1 = xmlrpc_encode_request( $funcname, $tmparr, $encOpt );
	if ($this->debugx>1) echo $FUNCNAME.":DEBUG:xmlrpc_encode_request:<pre>" .htmlspecialchars($my_request_1). "</pre>";
	
	$my_ans_1 = $this->send($my_request_1);
    
	if ($this->debugx>1) echo $FUNCNAME.":DEBUG:Answer:<pre>" .htmlspecialchars($my_ans_1). "</pre>";
	
    $my_p_ans_1 = $this->strip_h($my_ans_1);
   
    //Interpret it
    $no_need  = NULL;
    $my_data1 = xmlrpc_decode_request( $my_p_ans_1,$no_need );
	
	return ($my_data1); 
}

/**
 *  connect to server
 * GLOBAL-RETURN: $this->sess_id
 * @param int $loginMethod
 *   1 - new method
 *   2 - old method
 * @param int $conn2 [DEPRECATED] - connection method
 * @return void
 */
function connect($dummy=0, $loginMethod=1) {
	global $error;
	$FUNCNAME = $this->CLASSNAME.":connect()";
	
	// only support "connect2" method; old "connect" is deprecated
		
	$pwB64     = base64_encode( $this->connpar["cct_password"] );
	$connparams = array( 
		0=> $this->connpar["cct_user"], 
		1=> $pwB64, 
		2=> $this->connpar["dbid"]
		);
	$connFunc = "connect2";
	
	if ($loginMethod==2) {
	    throw new Exception('This method is not supported anymore.');
	}
	
	
	$my_request = xmlrpc_encode_request( $connFunc, $connparams);
	
	//Send Request
	$my_ans   = $this->send($my_request);
	$my_p_ans = $this->strip_h($my_ans);
	$no_need  = NULL;
	//Interpret results
	$xml_answer = xmlrpc_decode_request($my_p_ans, $no_need );
	
	if ($this->debugx>2) {
		echo "DEBUG: ".$FUNCNAME.':'.$connFunc.":<br>";
		echo "-request:<pre>".htmlspecialchars($my_request)."</pre><br>";
		// echo "-connparams: ".print_r($this->connpar,1)."<br>";
		echo "-SESSION:".$this->sess_id."<br>";
		echo "-RAW_answer: <pre>".htmlspecialchars($my_ans)."</pre><br>";
		echo "-Stripped_answer: <pre>".$my_p_ans."</pre><br>";
	}
	
	if ($xml_answer[0]>0) {
		$error->set( $FUNCNAME, 1, "connect-error:".$xml_answer[0].": ".$xml_answer[1]);
		return;
	}
	$my_sess_id = $xml_answer[1];
	$this->sess_id = $my_sess_id;
	
	
	
	if (!$my_sess_id) {
		$error->set( $FUNCNAME, 1, "got no session_id");
		return;
	}
	
}



}
