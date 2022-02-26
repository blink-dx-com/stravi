<?php
/**
 * create missing objects
 * @package xml_rpc_client2.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   
 */
require_once ('func_form.inc');
require_once ("visufuncs.inc");
require_once ("reqnormal.inc"); // includes all normal *.inc files
require_once ("f.xmlrpc_cli.inc");
require_once (dirname(__FILE__)."/../f.xmlrpc.test.inc");
require_once (dirname(__FILE__)."/../../../config/xml_rpc_tests.inc");

class fXmlrpcCliGui {
    
    var $testLib; // the XML-RPC library
    
    function __construct($parx) {
        $this->parx = $parx;
        $dummy=NULL;
        
        if ( $parx["shparams"]>0 ) {
            $debug = $parx["shparams"];
        }
        $this->testLib = new fXmlrcpFuTest($dummy, $parx, $debug);
        
        $this->connArr = xml_rpc_testLogins();
        
        $this->targetArr=array();
        foreach( $this->connArr as $key=>$valarr) {
            $this->targetArr[$key]=$valarr["cct_user"].'@'.$valarr["dbid"].' http:'.$valarr["hostname"];
        }
        reset ($this->connArr);
        
        
    }
    
    function setClient(&$obj1) {
        $this->testLib->setClient($obj1);
    }
    
    function getConnArr() {
        global $error;
        $FUNCNAME= 'getConnArr';
        $target = $this->parx["target"];
        $connParams = $this->connArr[$target];
        if ($connParams==NULL) {
            $error->set( $FUNCNAME, 1, 'no connection params found for target '.$target );
            return;
        }
        return ($connParams);
    }
    
    function form1() {
        
        
        
        $funclist = $this->testLib->getFuncList();
        $funcnames=NULL;
        foreach( $funclist as $key=>$val) {
            $funcnames[$key] = $key;
        }
        reset ($funclist);
        
        $targnames = $this->targetArr;
        
        $initarr   = NULL;
        $initarr["action"]      = $_SERVER['PHP_SELF'];
        $initarr["title"]       = "Select function";
        $initarr["submittitle"] = "Submit";
        $initarr["tabwidth"]    = "AUTO";
        
        $hiddenarr = NULL;
        
        $formobj = new formc($initarr, $hiddenarr, 0);
        
        $fieldx = array (
            "title" => "Target",
            "name"  => "target",
            "object"=> "select",
            "req"	=> 1,
            "val"   => $this->parx["target"],
            "inits" => $targnames,
            "notes" => "the function"
        );
        $formobj->fieldOut( $fieldx );
        
        $fieldx = array (
            "title" => "Function",
            "name"  => "funcname",
            "object"=> "select",
            "req"	=> 1,
            "val"   => $this->parx["funcname"],
            "inits" => $funcnames,
            "notes" => "the function"
        );
        $formobj->fieldOut( $fieldx );
        
        $fieldx = array (
            "title" => "Subtest",
            "name"  => "subtest",
            "object"=> "text",
            "val"   => $this->parx["subtest"],
            "notes" => "subtest string"
        );
        $formobj->fieldOut( $fieldx );
        
        $fieldx = array (
            "title" => "Parameters",
            "name"  => "params",
            "object"=> "text",
            "val"   => $this->parx["params"],
            "notes" => "Optional parameters, e.g. first PARAMETER of method"
        );
        $formobj->fieldOut( $fieldx );
        
        
        $fieldx = array (
            "title" => "Show params",
            "name"  => "shparams",
            "object"=> "checkbox",
            "val"   => $this->parx["shparams"],
            "notes" => "show input parameters"
        );
        $formobj->fieldOut( $fieldx );
        
        $formobj->close( TRUE );
    }
    
    function doAction( &$sqlo ) {
        return ( $this->testLib->doAction( $sqlo, $this->parx["funcname"], $this->parx["subtest"] ) );
    }
    
}


class xml_rpc_client2_XPL extends gPlugin {
	
	
	function register() {
		
		$title		= 'UnitTest for XML_RPC';
		
		$this->infoarr			     = NULL;
		$this->infoarr['title']      = $title;
		$this->infoarr['form_type']  = 'tool'; 
		$this->infoarr2['onlyAdmin'] = 1;
		
		$this -> infoarr ['locrow'] = array (
		    array ('www/_tests/index.php', 'Unittest home')
		    );
	}
	
	
	
	function startMain() {
		global $error;
		$FUNCNAME='startMain';
		
		$sqlo = &$this->_sqlo;
		$sqlo2= &$this->_plug_regSql2();

		$error = & ErrorHandler::get();
		
		$go    = $_REQUEST["go"];
		$parx  = $_REQUEST["parx"];
		
		if ($go) {
		    echo "[<a href=\"".$_SERVER['PHP_SELF']."\">Start again</a>]";
		}
		echo "<ul>";
		
		
		/*
		 $connect_params = array(
		 "dbid"	  =>"",
		 "hostname"=>"robur",
		 "srv_url" =>"/pionir/xmlrpc/icono_svr.php",
		 "cct_user"=>"root",
		 "cct_password"=>""
		 );
		 */
		
		
		
		$mainlib = new fXmlrpcCliGui($parx);
		
		
		$mainlib->form1();
		
		if ( !$go ) {
		    $this->htmlFoot("<hr>");
		}
		
		if ($parx['funcname'] == "" ) {
		    $this->htmlFoot("Error", "Please select a function");
		}
		if ($parx['target'] == "" ) {
		    $this->htmlFoot("Error", "Please select a target");
		}
		
		$connect_params = $mainlib->getConnArr();
		if ($error->Got(READONLY))  {
		    return;
		}
		
		$tabobj = new visufuncs();
		$dataArr= NULL;
		
		$pw_nice='NOT SET!';
		if ($connect_params["cct_password"]!=NULL) {
		    $pw_nice='***';
		}
		$dataArr[] = array( 'Target-System:','<B>'.$parx['target'].'</B>');
		$dataArr[] = array( 'User:','<B>'.$connect_params["cct_user"].'</B>');
		$dataArr[] = array( 'PW:','<B>'.$pw_nice.'</B>');
		$dataArr[] = array( 'DBID:','<B>'.$connect_params["dbid"].'</B>');
		$dataArr[] = array( 'Hostname:','<B>'.$connect_params["hostname"].'</B>');
		$dataArr[] = array( 'Protocol:','<B>'.$connect_params["protocol"].'</B>');
		$dataArr[] = array( 'Source:','<B>'.$connect_params["srv_url"].'</B>');
		$headOpt = array( "title" => "INFO", "headNoShow" =>1);
		$headx   = array ("Key", "Val");
		$tabobj->table_out2($headx, $dataArr, $headOpt);
		
		echo "<br>\n";
		
		$obj1 = new fXmlrpcCli( $connect_params );
		echo "- start connection ...<br>";
		
		
		$connectmethod= 1;
		if ($parx["funcname"]=='connect') {
		    $connectmethod= 2; // old login
		    echo "<b>WARN</b>: use old method: CONNECT<br>";
		}
		
		if ($parx["funcname"]!='connect2') {
		    // no connect on connect2
		    $obj1->connect(0,$connectmethod);   // can set $error
		    
		    if ( $error->Got(READONLY) ) {
		        $error->set( 'xml_rpc_client2', 2, 'Error on connect.' );
		        $error->printAll();
		        htmlfoot();
		    }
		}
		
		$mainlib->setClient($obj1);
		
		//Start time measurement for action:
		$sTime = strtok(microtime(), " ") + strtok(" ");
		
		$resultArray = $mainlib->doAction( $sqlo );
		$answer   = $resultArray[0];
		$testinfo = $resultArray[1];
		
		//end of time measurement:
		$eTime =  strtok(microtime(), " ") + strtok(" ");
		$oTime =  number_format($eTime - $sTime, 4);
		
		
		if ( $answer == NULL and !$error->Got(READONLY) ) {
		    // throw if no other error occured
		    $error->set( $FUNCNAME, 1, 'no answer from XML_RPC method.' );
		}
		if ( $error->printAll() ) return;
		
		if ($testinfo['nice']!=NULL) {
		    echo "<b>TEST-nicename:</b> ".$testinfo['nice']."<br />";
		}
		if ($testinfo['info']!=NULL) {
		    echo "<b>TEST-info:</b> ".$testinfo['info']."<br />";
		}
		
		if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
		    echo "ANSWER: ReceiveEncoding: ".$obj1->getSenderEnc();
		}
		
		echo "<br><pre>";
		print_r($answer);
		echo "</pre>";
		echo "Time taken: ".$oTime." seconds";
		echo "<hr>";
		
		
	}
	
}
