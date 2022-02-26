<?php
/**
 * Alternative Protocol Editor
 * - image can be located in lab/images/TB.xxxx
 * CONFIGs:
 *   CONCRETE_PROTO:VARIO:AlterEditor: array : contains protocol data
 *   ABSTRACT_PROTO:VARIO:EditorPostPlugin: string : name of PostAction Plugin
 * @package 
 * @swreq UREQ:6576 o.CONCRETE_PROTO > Protokoll-Editor > Alternative
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/phplib/gui/o.PRC.altereditor1.inc 59 2018-11-21 09:04:09Z $
 */
require_once('o.SATTACH.subs.inc');
require_once('o.S_VARIO.mod.inc');

class oPRC_altereditor1 {
	
	private $htmlTemplate;
	private $prc_id;
	private $errorInfo=NULL;
	private $lineno;
	private $oldvals; // array(index =>array('v'=>value, 'u'=>userid) )
	private $username_cache;
	
	/**
	 *	0: only view values
	 * 	1: allow edit
	 *  2: show form with edit fields, no SUBMIT button; for the ABSTRACT-VIEW
	 **/
	private $editAllow;
	private $pra_id;
	
	function __construct(&$sqlo) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$this->prc_id  = 0;
		$this->oldvals = NULL;
		$this->_sqlo   = &$sqlo;
		

	}
	
	/**
	 * set abstract PROTO
	 * @param unknown $sqlo
	 * @param unknown $pra_id
	 */
	function set_PRA(&$sqlo, $pra_id) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
		$this->pra_id  = $pra_id;
		$this->oldvals = NULL;
	
		$ABSTRACT_PROTO_ID = $this->pra_id;
	
		// get attachment
		$key = 'AlterEditor';
		$attachlib = new cSattachSubs();
		$relid = $attachlib->get_LateRelIDByKey($sqlo, 'ABSTRACT_PROTO', $ABSTRACT_PROTO_ID, $key);
	
		if (!$relid) {
			$error->set( $FUNCNAME, 1, 'Attachment missing! Protocol (abstract) needs HTML-attachment with key "'.$key.'"' );
			return;
		}
	
		$pathx = $attachlib->getDocumentPath('ABSTRACT_PROTO', $ABSTRACT_PROTO_ID, $relid);
	
		if (!file_exists($pathx)) {
			$error->set( $FUNCNAME, 2, 'File missing! Protocol (abstract) needs HTML-attachment with key "'.$key.'"' );
			return;
		}
	
		$this->htmlTemplate = $pathx;
	
	
	
	}
	
	function set_PRC(&$sqlo, $prc_id) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
		$this->prc_id  = $prc_id;
		$this->oldvals = NULL;
	
		$features = glob_elemDataGet3( $sqlo, 'CONCRETE_PROTO', array('CONCRETE_PROTO_ID'=>$prc_id), array('ABSTRACT_PROTO_ID') );
		$ABSTRACT_PROTO_ID = $features['ABSTRACT_PROTO_ID'];
	
		$this->set_PRA($sqlo, $features['ABSTRACT_PROTO_ID']); 
	
		// get data
		$this->_getDB_data($sqlo);
	
	
	}
	
	private function _getDB_data(&$sqlo) {
		$variolib = new oS_VARIO_sub('CONCRETE_PROTO');
		$valstr = $variolib->getValByKey($sqlo, $this->prc_id, 'AlterEditor');
		if ($valstr!=NULL) {
			$this->oldvals = unserialize($valstr);
		}
	}
	
	private function setError($text) {
		$this->errorInfo[]=array('l'=>$this->lineno, 't'=>$text);
	}
	
	private function _htmlSelect($selectArr, $tempVal) { 
		$output ='<select name="tabArgu['.$this->valueID.']">'."\n";
		$output.='<option value="">---</option>';
		foreach($selectArr as $val) {
			$selTmp=NULL;
			if ($tempVal==$val) $selTmp=' selected ';
			$output.='<option'.$selTmp.'>'.$val.'</option>';
		}
		$output.='</select>'."\n";
		return $output;
	}
	
	private function getUser() {
		$userid = $this->oldvals[$this->valueID]['u'];
		if (!$userid) return '';
		
		if ( isset($this->username_cache[$userid])) {
			return $this->username_cache[$userid];
		}
		
		$answer = glob_elemDataGet3( $this->_sqlo, 'DB_USER', array('DB_USER_ID'=>$userid), array('FULL_NAME', 'NICK') );
		$name = $answer['NICK'];
		$this->username_cache[$userid]=$name;
		return $name;
	}
	
	/**
	 * transform one pattern DATA:
	 * 		SEL:ksks|jsjsj
	 * 		TEXT
	 * 		TEXTAREA
	 * @param string $pattern
	 */
	private function _transOnePattern($pattern) {
		$errflag = 0;
		$errtext = NULL;
		$newtext = '?';
		
		$sep=':';
		$tempArr = explode($sep, $pattern);
		
		if ($this->editAllow>0) {
			// EDIT
			switch ($tempArr[0]) {
				case 'DATA':
					// next separator
					$this->valueID++; // next value, must be incremented BEFORE, because after this comes [USER]-key
					
					$oldval  = $this->oldvals[$this->valueID]['v'];
					if ($oldval!=='' and $oldval!==NULL) {
						if ($tempArr[1]=='CHECK') {
							if ($oldval>0)
								$newtext = '<span style="background-color:#DDDD00;">x</span>';
							else
								$newtext = '<span style="background-color:#DDDD00;">_</span>';
							break;
						} else {
							$newtext = '<span style="background-color:#DDDD00;">'.htmlentities($oldval).'</span>'; // show the value
						}
					} else {
					
						switch ($tempArr[1]) {
							case 'TEXT':
								$newtext = '<input type=text name="tabArgu['.$this->valueID.']" value="'.$oldval.'" size=20>';
								break;
							case 'SEL':
								$selectionString = $tempArr[2];
								$selArr = explode('|', $selectionString);
								$newtext =$this->_htmlSelect($selArr, $oldval);
								break;
							case 'CHECK':
								$tmpchecked = NULL;
								$newtext = '<input type=checkbox name="tabArgu['.$this->valueID.']" value="1" '.$tmpchecked.'>';
								break;
							case 'TEXTAREA':
								$newtext = '<textarea rows=3 cols=40 name="tabArgu['.$this->valueID.']">'.$oldval.'</textarea>';
								break;
						}
					}
					
					
					break;
				case 'USER':
					$newtext = '<span style="background-color:#E0E0B0;">u:'.$this->getUser().'</span>';
					break;
				default:
					$errflag = 1;
					$this->setError('KEY '.$tempArr[0]. ' unknown');
			}
		} else {
			switch ($tempArr[0]) {
				case 'DATA':
					// next separator
					$this->valueID++; // next value, must be incremented BEFORE, because after this comes [USER]-key
					
					$oldval  = $this->oldvals[$this->valueID]['v'];
					if ($oldval!=='' and $oldval!==NULL) {
						
						switch ($tempArr[1]) {
							
							case 'CHECK':
								if ($oldval>0)
									$newtext = '<span style="background-color:#DDDD00;">x</span>';
								else 
									$newtext = '<span style="background-color:#DDDD00;">_</span>';
								break;
							default:
								$newtext = '<span style="background-color:#DDDD00;">'.htmlentities($oldval).'</span>'; // show the value
								break;
						}
					} else {
						$newtext = '<span style="background-color:#DDDDDD;">&nbsp;</span>';		
					}
							
					break;
				case 'USER':
					$newtext = '<span style="background-color:#E0E0B0;">u:'.$this->getUser().'</span>';
					break;
				default:
					$errflag = 1;
					$this->setError('KEY '.$tempArr[0]. ' unknown');
			}
		}
		
		return $newtext;
	}
	
	/**
	 * transform one line
	 * @param unknown $line
	 */
	private function _transOneLine($line) {
		
		$output = NULL;
		$offset = 0;
		
		$key    ='[';
		while ( ($foundpos=strpos($line, $key,$offset) )!==FALSE ) {
			// echo "DEBUG: XXX:$foundpos<br>";
			
			if ( substr($line,$foundpos,5)!='[DATA' and substr($line,$foundpos,5)!='[USER' )  {
				$offset++; // inc offset
				continue; // wrong pattern
			}
			
			// look for end
			$endpos = strpos($line, ']',$foundpos);
			
			
			$pattern = substr( $line, $foundpos+1,$endpos-$foundpos-1); // Pattern without BRACKETs
			$new_string = $this->_transOnePattern($pattern);
			
			$output .= substr($line,$offset,$foundpos-$offset); // part between last PATTERN and NEW PATTERN
			$output .= $new_string;
			
			$offset = $endpos+1;
		}
		
		// last part
		$output .= substr($line,$offset,strlen($line)-$offset); 
		
		return $output;
	}
	
	/**
	 * transform for EDIT
	 * @return string $newData
	 */
	private function transformfile() {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$datafile = $this->htmlTemplate;
		$newData  = NULL;
		
		$LINE_LENGTH = 32000;
		$FH = fopen($datafile, 'r');
		if ( !$FH ) {
			$error->set( $FUNCNAME, 2, 'Can not open file '.$datafile );
			return;
		}
		
		$this->lineno  =  1;
		$this->valueID = -1; // must start with -1, because of INC before use
		while( !feof ( $FH ) ) {
			$line = fgets($FH, $LINE_LENGTH);
			$newData .= $this->_transOneLine($line);
			$this->lineno++;
		}
		fclose ( $FH);
		
		return $newData;
	}
	
	private function saveData($sqlo) {
		// print_r($_REQUEST['tabArgu']);
		
		$userid = $_SESSION['sec']['db_user_id'];
		$newvals = $this->oldvals;
		foreach($_REQUEST['tabArgu'] as $loopind=>$loopval ) {
			if ($loopval!=='' and $loopval!==NULL) {
				$newvals[$loopind]=array('v'=>$loopval, 'u'=>$userid);
			}
		}
		
		
		// save data ....
		$varioMlib = new oS_VARIO_mod();
		$varioMlib->setObject($sqlo, 'CONCRETE_PROTO', $this->prc_id);
		$dataSerial = serialize($newvals);
		if (strlen($dataSerial)>4000) {
			$this->setError('too much data; Please inform the Admin!');
			echo "<b>UPDATE FAILED!</b><br>\n";
			return;
		}
		
		$varioMlib->updateKeyVal($sqlo, 'AlterEditor', $dataSerial);
		
		$this->_getDB_data($sqlo);
		
		echo "UPDATE o.k.<br>\n";
	}
	
	private function checkPostActions(&$sqlo) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$variolib = new oS_VARIO_sub('ABSTRACT_PROTO');
		$pluginname = $variolib->getValByKey($sqlo, $this->pra_id, 'EditorPostPlugin');
		if ($pluginname==NULL) return;
		
		echo "INFO: PostAction activated ...<br>\n";
		
		$fullurl = $_SESSION['s_sessVars']['AppLabLibDir'].'/obj/ABSTRACT_PROTO/plug.'.$pluginname.'.inc';
		
		if ( !file_exists($fullurl) ) {
			$error->set( $FUNCNAME, 1, 'Plugin '.$pluginname.' for ABSTRACT_PROTO not found. Please inform Admin. [ERR:'.$fullurl.']' );
			return;
		}
		
		require_once($fullurl);
		
		$classname = $pluginname.'_PRAPLUG1';
		$postlib = new $classname();
		$answer = $postlib->startx($sqlo, $this->prc_id);
		
		$reloadPage = $postlib->get_reloadObjectFlag();
		
		return array('reload'=>$reloadPage);
	}
	
	/**
	 * show the form
	 * - load file
	 * - parse for PATTERNS "[DATA]", "[USER]"
	 * - transform PATTERNS
	 * - show form
	 * @param unknown $sqlo
	 * @param unknown $sqlo2
	 * @param int $editAllow 
	 * 		0: only view values
	 * 		1: allow edit
	 *      2: show form with edit fields, no SUBMIT button; for the ABSTRACT-VIEW
	 */
	function showform( &$sqlo, &$sqlo2, $editAllow ) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		echo "<ul>\n";
		$this->editAllow = $editAllow;
		
		if ($_REQUEST['go']) {
			$this->saveData($sqlo);
			if ($error->Got(READONLY))  {
				$error->set( $FUNCNAME, 1, 'error on data saving.' );
				return;
			}
			$answer = $this->checkPostActions($sqlo);
			if ($error->Got(READONLY))  {
				$error->set( $FUNCNAME, 2, 'PostAction failed.' );
				return;
			}
			
			if ($answer['reload']>0) {
				require_once ( "javascript.inc" );
				$url = 'edit.tmpl.php?t=CONCRETE_PROTO&id='.$this->prc_id;
				js__location_replace( $url );
				return;	
			}
		}
		
		if ($editAllow==1) {
			$newData = '<form style="display:inline;" method="post" name="AlterEditor"  action="'. $_SERVER['PHP_SELF'] .'" >'."\n";
			$newData .= $this->transformfile();
			$newData .= '<input type=hidden name="go" value="1">'."\n";
			$newData .= '<input type=hidden name="t" value="CONCRETE_PROTO">'."\n";
			$newData .= '<input type=hidden name="id" value="'.$this->prc_id.'">'."\n";
			$newData .= '<ul><div style="background-color:#6699FF; width: 600px;padding: 5px; border: 0px; margin: 5px;"><input type=submit value="Submit"></div></ul>'."\n"; // SUBMIT
			$newData .= '</form>'."\n";
		} else {
			$newData .= $this->transformfile();
		}
		
		echo $newData; // show the form
		
		if (is_array($this->errorInfo)) {
			echo '<br><B>Editor-Parse-Errors:</b><br>';
			foreach($this->errorInfo as $oneError) {
				echo '- '.$oneError['t'].' (line:'.$oneError['l'].")<br>";
			}
			
			echo "<br>\n";
		}
		echo "</ul>\n";
	}
}
