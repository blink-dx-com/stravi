<?php
require_once 'o.PROJ.subs.inc';


/**
 * - print label X to printer Y via TCP/IP
 * - host class gZebraOneLabel
 *    
 * @package f.zebraPrint.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq SREQ:0002346: g > labels > zebra printer driver 
 * @version $Header: trunk/src/phplib/f.zebraPrint.inc 59 2018-11-21 09:04:09Z $  
 */
class gZebraOneLabel2 {
	
	var $ZPL_chars = array(
		'STX'=>0x02,
		'ETX'=>0x03,
		'EOT'=>0x04
	    );
	    
	var $doSend;
	/**
	 * 
	 * Enter description here ...
	 * @param $printer
	 * @param $label e.g. WE2_MYSA.ZPL or 
	 * 		"FREETEXT"
	 */
	function gZebraOneLabel2($printer, $label) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$this->label=$label;
		$this->sock = NULL;
		$this->doSend = 1;
		
		// $file = 'test2.txt';
		$this->remote_file = 'test2.txt';
		
		
		$isConnected=0;
		$sockPort   =9100;
		
		$printerIP = gethostbyname( $printer  );
		if ($printerIP==$printer) {
			$error->set( $FUNCNAME, 1, 'Kann IP-Adresse nicht aufloesen. host: '.$printer );
			return;
		}
		
		if ( //If
	        function_exists('socket_create') AND //socket_create exists
	        $this->sock = socket_create(AF_INET, SOCK_STREAM, SOL_TCP) //Can create the socket 
    		) {  //Then
			
			//reduce connect timeout
			socket_set_option($this->sock, SOL_SOCKET, SO_SNDTIMEO, array('sec' => 4.0, 'usec' => 0.0));
			// Can connect to the socket
			// suppress PHP-Warnings with @-Symbol
			if ($sock_data  = @socket_connect($this->sock, $printerIP, $sockPort)) {
    			$isConnected=1;
			} else {
				socket_close ( $this->sock );
			}
    	}
		
		if (!$isConnected) {
			$error->set( $FUNCNAME, 2, 'Keine Verbindung zum Printer '.$printer );
			return;
		}
		
		// Login mit Benutzername und Passwort
		
	}
	
	
	
	function _oneLine( &$tempPtr, $line) {
		// fwrite($tempPtr, $line ."\n");
		$tempPtr .= $line ."\n";
	}
	
	/**
	 * print $num labels of labeltype ($this->label)
	 * @param array $parx
	 *   KEY, VAL
	 *     - if $this->label=='FREETEXT' : KEY='TEXT'
	 * @param int $num - number of labels
	 */
	function printOne($parx, $num) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		// $tempPtr = tmpfile();
		$tempPtr=NULL;
		
		$this->_oneLine($tempPtr, "^XA"); // format start
		
		if ($this->label=='FREETEXT') {
			
			$escapedText = $parx['TEXT'];
			$escapedText = str_replace("\\", "\\\\", $escapedText);
			$escapedText = str_replace("^", " ", $escapedText); // remove CARET
			
			
			/* put text direct to text block
			 * ^CI28 - UTF-8 Code Page
			 */
			$textpre ='^FS';
			$textpre .='~JS^LT0^MNW^MTD^PON^PMN^LH0,0^JMA^PR6,6^MD15^LRN^CI28^MMT^PW799^LL0376^LS0^FS';
			$this->_oneLine($tempPtr, $textpre );
			// ^TBN,1,1 - Text Block
			// ^FB0,5,,
			$textline = "^FT15,350^A0N,40,40^FH\\^FB799,5,20,^FD". $escapedText.'^FS';
			$this->_oneLine($tempPtr, $textline );
		} else {
		
			$this->_oneLine($tempPtr, "^XFE:".$this->label); // ZPL-tutorial: ^DF and ^XF
		
			
			foreach ( $parx as $key => $val ) {
				$this->_oneLine($tempPtr, "^".$key."^FD".$val."^FS");
			}
			
		}
		
		$this->_oneLine($tempPtr, "^XZ");
		
		if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
			// fseek($tempPtr, 0);
			$buffer=NULL;
			/*
			while (!feof($tempPtr)) {
				$buffer .= "<br>".fgets($tempPtr, 16096);
			}
			
			*/
			print ("DEBUG_Data:");
			echo htmlentities($tempPtr);
			echo "<br />";
			
		}
		
		if ($this->doSend) {
			$sock_data = socket_write($this->sock, $tempPtr, strlen($tempPtr)); //Send data
	       
			if (!$sock_data) {
				$error->set( $FUNCNAME, 1, 'Problem beim Senden der Daten.' );
				return;
			}
		}
		
	}
	
	/**
	 * write a command to printer
	 * @param $text
	 */
	function writeCommand($text) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$sock_data = socket_write($this->sock, $text, strlen($text)); // Send data
		if (!$sock_data) {
			$error->set( $FUNCNAME, 1, 'Problem beim Senden der Daten.' );
			return;
		}
	}
	
	/**
	 * read text from printer
	 * @return $text
	 */
	function readText($maxlen=5000000) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$string=NULL;
		$startText=0;
		$loopCnt=0;
		
		while(TRUE){
			
            if ( ($chars = @socket_read($this->sock, 1024, PHP_BINARY_READ) ) === false) {
	        	$err_text = socket_last_error($this->sock);
	        	$errormsg = socket_strerror($err_text);
	        	$error->set( $FUNCNAME, 1, 'readError: '. $err_text. ':'.$errormsg);
				return $string;
	        	
            } else {
            	
            	$startpos = strpos($chars, $this->ZPL_chars['STX']);
            	$endpos   = strpos($chars, $this->ZPL_chars['ETX']);
            	
            	if ($startpos===FALSE) $startpos=0;
            	else $startpos=$startpos+1;

            	if ($endpos===FALSE) {
            		$string.= substr($chars, $startpos);
            	} else {
            		$lenStore = $endpos-1-$startpos;
            		$string.= substr($chars,$startpos,$lenStore);
            		break;
            	}

	        }
        }
        
		return $string;
	}

	function closex() {
		// Verbindung schlie�en
		 socket_close($this->sock); //Close socket
	}
	
}

/**
 * printer management
 * @author steffen
 *
 */
class gZebraPrinterMng {
	/**
	 * get printer-names
	 * @param $sqlo
	 */
	function getPrinters(&$sqlo) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$printerarr=NULL;
		
		$globVar='g.ZebraPrinterProjID';
		$projid = glob_elementDataGet( $sqlo, 'GLOBALS', 'NAME', $globVar, 'VALUE');
		if (!$projid or !is_numeric($projid) ) {
			$error->set( $FUNCNAME, 1, 'Device-Project-ID in GLOBALS:"'.$globVar.'" nicht gesetzt.' );
			return;
		}
		$this->printerProjectID = $projid;
		
		$projLib  = new cProjSubs();
		$sqlafter = $projLib->getTableSQLInSort( $projid, 'CHIP_READER' );
		$sqlsel   = "x.NAME, x.LOCATION from ".$sqlafter;
		$sqlo->Quesel($sqlsel);
		while ( $sqlo->ReadRow() ) {
		    $name = $sqlo->RowData[0];
		    $loc  = $sqlo->RowData[1];
		    $printerarr[$name]=$name.' ('.$loc.')';
		}
		
		if (!sizeof($printerarr) ) {
			$error->set( $FUNCNAME, 2, 'Keine Printer-devices im Project '.$projid.'.' );
			return;
		}
		
		return $printerarr;
	}
	
	/**
	 * check, if printing is allowed for this user
	 * @param $sqlo
	 * @param $printer
	 * @return array($useaAllow, $errortext)
	 * 	$useaAllow: 0,1
	 *  $errortext: text of error
	 */
	function printerAllowed(&$sqlo, $printer) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$devid = glob_elementDataGet( $sqlo, 'CHIP_READER', 'NAME', $printer, 'CHIP_READER_ID');
		if (!$devid) {
			return array(-1, 'Kein device gefunden mit Namen "'.$printer.'"');
		}
		$optyx = array("useReadFlag"=>1);
		$o_rights_tmp = access_check($sqlo, "CHIP_READER", $devid, 0, $optyx);
		
		if ($o_rights_tmp["read"]) {
			$useaAllow = 1;
		} else {
			return array(-2,  'Dieser Printer '.$printer.' ist leider nicht fuer Dich freigegeben. '.
				' Lass dir READ-Rechte fuer das Device DEV-ID:'.$devid.' geben.' );
		}
		return array($useaAllow,'');
	}
	
}



