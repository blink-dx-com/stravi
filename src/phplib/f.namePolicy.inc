<?php
require_once ('o.S_VARIO.subs.inc');
require_once ('o.S_VARIO.mod.inc');
require_once 'glob.obj.conabs.inc';
require_once 'o.GLOBALS.mod.inc';

/**
 * manage name policy strings, generate names
 * Name_Policy:

    	string{KEYWORDS}
    	
    	keyword-format: {KEYWORD,FORMAT}
    	example: {DATE,2}{NUM,03}
    	Keywords:
    	
    	DATE  - generate a DATE: 
    		format: 
    			"1" [DEFAULT]: 20090517
    			"2" 090517
    			"3" 2009-05-17
    	NUM   - running AUTO-number from ABSTRACT_SUBST:VARIO:fNamePolicy.nextnum (if not set: 0)
    		format: 
    			NUMBER: number of leading ZEROS; e.g. 3
        GNUM   - running GLOBAL (system-wide) AUTO-number store in GLOBALS table
    		syntax: {GNUM,FORMAT,GENERATOR_ID}
    			FORMAT: number of leading ZEROS; e.g. 3
    			GENERATOR_ID: a number of the global generator, saved in GLOBALS table: KEY: app.fNamePolicy.G{GENERATOR_ID}
    			
    	ID    - ID of object
    	BATCH - the BATCH string (e.g. external CHARGE num)
    	MORDER.name - name of MORDER
	
	@var paramarr_STRUCT
	  	'NUM' : NUM for key NUM
	    'ID'  : ID of object
	    'DATE':   [optional] date as UNIX-stamp
	    'BATCH':  [optional] BATCH string
	    
	    'MORDER.name' : [optional] 
 *  @package f.namePolicy.inc
 * 
 */
class fNamePolicy {
    
	var $CLASSNAME='fNamePolicy';
	const ABS_VARIO_KEY='fNamePolicy.nextnum';
	const GNUM_GEN_KEY='app.fNamePolicy.G';
	
	/**
	 * the SUA where the name policy is based on
	 * @param string        $main_table e.g. CONCRETE_SUBST
	 * @param int $abs_id   e.g. ABSTRACT_SUBST_ID : 345
	 */
	function __construct($main_table, $abs_id) {
	    $this->main_table= $main_table;
	    $this->abs_id    = $abs_id;
	    $conabs_lib      = new gConcAbsC();
	    $this->abs_table = $conabs_lib->getTemplate($main_table);
	}
	
	private function _entry2txt($entry) {
	    $text = $entry['key'];
	    if ($entry['for']) $text .= ','.$entry['for'];
	    $text = '{'.$text.'}';
	    return $text;
	}
	
	/**
	 * analalyse policy string, get the policy parts
	 * KEYWORDS: 'DATE', 'NUM', 'ID', 'BATCH', 
	 * @param $policyStr : examples: 
	 *    CER{NUM}_X{DATE,2}
	 * @return $policyArray : array(format_string)
	 *   format_string: array: 'txt' or 'key' must be given
	 *    'txt' : simple text
	 *    'for' : format string
	 *    'key' : the keyword
	 *    'par' : extra parameter
	 *   example: array(array('txt'=>'CER'), array('key'=>'NUM'))
	 */
	function analysePolicyString($policyStr) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$policyStr = trim($policyStr); // remove white spaces ..
		$allowKeys=array('DATE', 'NUM', 'GNUM',  'ID', 'BATCH',  'MORDER.name');
		$pos=0;
		$policyStrLen = strlen($policyStr);
		$policyArray=array();
		$ok=1;
		$errtxt = NULL;
		$oneposTxt=NULL;
		while ($pos<$policyStrLen) {
			
			$onechar = substr($policyStr, $pos, 1);
			if ($onechar=='{') {
				// analyse keyword
				if ($oneposTxt!=NULL) {
					$policyArray[]=array('txt'=>$oneposTxt);
				}
				$oneposTxt=NULL;
				$onepos  = array();
				$keyPos  = $pos+1;
				$keyPosEnd  = strpos($policyStr, "}",$keyPos);
				if ($keyPosEnd===FALSE) {
					$errtxt = 'ending } not found.';
					break;
				}
				$keyInfoLen = $keyPosEnd-$keyPos;
				$subKeystring = substr($policyStr, $keyPos, $keyInfoLen );
				$pos = $keyPosEnd;
				
				// contains komma ?
				$pol1arr = explode(",",$subKeystring);
				$keyStr  = $pol1arr[0];
				if (sizeof($pol1arr)>0) {
				    $onepos['for']= $pol1arr[1];
				}
				if (sizeof($pol1arr)>1) {
				    $onepos['par']= $pol1arr[2];
				}
				
// 				$keyEndPos = strlen($subKeystring)-1;
// 				$kommaPos = strpos($subKeystring, ",");
// 				if ($kommaPos>0) {
// 					$keyEndPos = $kommaPos-1;
// 					$onepos['for']= substr($subKeystring,$kommaPos+1);
// 				}
// 				$keyStr = substr($subKeystring,0,$keyEndPos+1);
				if (!in_array($keyStr, $allowKeys )) {
					$errtxt = 'key: "'.$keyStr.'" not allowed here.';
					break;
				}
				$onepos['key'] = $keyStr;
				$policyArray[]=$onepos;
				
			} else {
				$oneposTxt .= $onechar;
				
			}
			$pos=$pos+1;
		} 
		
		if ($oneposTxt!=NULL) {
			$policyArray[]=array('txt'=>$oneposTxt);
		}
		if ($errtxt!=NULL)  {
			$error->set( $FUNCNAME, 1, 'error during analysis: '.$errtxt);
			return;
		}
		return $policyArray;
	}
	
	/**
	 * analyse policyArray and generate name for PRE/POST create
	 * @param array $policyArray policyArray_STRUC 
	 * @param array $paramarr: paramarr_STRUCT
	 * @return string
	 */
	function generate_name_ABS($sqlo, &$policyArray, array &$paramarr, string $pre_post_flag) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		$nameOut = NULL;
		
		
		if (!is_array($policyArray)) {
			$error->set( $FUNCNAME, 2, 'policy array missing');
			return;
		}
		if ($pre_post_flag=='PRE' and $pre_post_flag=='POST' ) {
		    $error->set( $FUNCNAME, 4, 'bad input: pre_post_flag.');
		    return;
		}
		
		
		foreach( $policyArray as $onearr) {
		    
			if ($onearr['txt']!=NULL) {
			    
				$nameOut .= $onearr['txt'];
			} else {
			    
				// 'key'
				$key    = $onearr['key'];
				$format = $onearr['for'];
				switch ($key) {
					case 'DATE':
						$dateDefs = array(1=>'Ymd', 2=>'ymd', 3=>'Y-m-d');
						if ($format==NULL) $format=1;
						$dateFormat = $dateDefs[$format];
						if ($dateFormat==NULL) $dateFormat  = $dateDefs[1];
						if ($paramarr['DATE']>0) $timeStamp = $paramarr['DATE'];
						else $timeStamp=time();
						$atomx = date($dateFormat, $timeStamp);
						break;
						
					case 'NUM':	
					    
					    if ( !$this->abs_id ) {
					        $error->set( $FUNCNAME, 10, 'INPUT missing: abs_id.');
					        return;
					    }
					    // get NUM form ABSTACT object:VARIO:self::ABS_VARIO_KEY
					    // OLD:  $onenum = $paramarr['NUM'];
					    $vario_lib = new oS_VARIO_sub($this->abs_table);
					    $onenum    = $vario_lib->getValByKey( $sqlo, $this->abs_id, self::ABS_VARIO_KEY );
					    if(!$onenum) $onenum = 1;
					    
						if (!is_numeric($onenum)) {
							$errtxt = 'input param NUM must be numeric.';
							break;
						}
						if ($format==NULL) $atomx = $onenum;
						else {
							$atomx = sprintf('%'.$format.'d', $onenum);
						}
						
						if ($pre_post_flag=='POST') {
						    // increment vario val
						    $onenum=$onenum+1;
						    debugOut('update SUA:'.$this->abs_id.' VARIO:'.self::ABS_VARIO_KEY.':'.$onenum, $FUNCNAME, 1);
						    $this->set_ABS_next_val($sqlo, $onenum);
						}
						
						break;
						
					case 'GNUM':
					    
					    $genid = intval($onearr['par']);
					    if ( !$genid ) {
					        $error->set( $FUNCNAME, 20, 'INPUT missing: parameter: GENERATOR_ID missing.');
					        return;
					    }
  
					    $genkey=self::GNUM_GEN_KEY . $genid;
					    $onenum    = oGLOBALS_mod::getval($sqlo, $genkey);
					    if(!$onenum) $onenum = 1;
					    
					    if (!is_numeric($onenum)) {
					        $errtxt = 'input param NUM must be numeric.';
					        break;
					    }
					    if ($format==NULL) $atomx = $onenum;
					    else {
					        $atomx = sprintf('%'.$format.'d', $onenum);
					    }
					    
					    if ($pre_post_flag=='POST') {
					        // increment vario val
					        $onenum=$onenum+1;
					        debugOut('Update GNUM: '.$genkey.': '.$onenum, $FUNCNAME, 1);
					        oGLOBALS_mod::update($sqlo, $genkey, $onenum);
					    }
					    
					    break;
					case 'ID':	
					    
					    if ($pre_post_flag=='PRE') {
					        $atomx = 'XXXX';
					    } else {
    						$onenum = $paramarr['ID'];
    						if ($onenum==NULL) {
    							$errtxt = 'input param ID must be given.';
    							break;
    						}
    						$atomx = $onenum;
					    }
						break;
						
					case 'BATCH':	
						$batch = $paramarr['BATCH'];
						if ($batch===NULL or $batch==='') {
							$errtxt = 'input param BATCH not set.';
							break;
						}
						$atomx = $batch;
						
						break;
					case 'CHARGE':	
						$batch = $paramarr['CHARGE'];
						if ($batch===NULL) {
							$errtxt = 'input param CHARGE not set.';
							break;
						}
						$atomx = $batch;
						break;
					case 'MORDER.name':
					    $tmp = $paramarr[$key];
					    if ($tmp===NULL) {
					        $errtxt = 'input param '.$key.' not set.';
					        break;
					    }
					    $atomx = $tmp;
					    break;
				}
				$nameOut .= $atomx;
			}
		}
		
		if ($errtxt!=NULL)  {
			$error->set( $FUNCNAME, 3, 'error during generation: '.$errtxt);
			return;
		} 
		
		return $nameOut;
	}
	
	/**
	 * create UNIQUE NAME
	 * @param object $sqlo
	 * @param string $in_name
	 * @param number $obj_id
	 * @return string
	 */
	function create_unique_name($sqlo, $in_name, $obj_id=0) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    if ( !$this->abs_id ) {
	        $error->set( $FUNCNAME, 1, 'INPUT missing: abs_id.');
	        return;
	    }
	    
	    $new_name  = $in_name;
	    $tablename = $this->main_table;
	    $pk_name   = PrimNameGet2($this->abs_table);
	    
	    if ( fileC::redundancyTest2( $sqlo, $tablename, array('NAME'=>$in_name,  $pk_name=>$this->abs_id), $obj_id) ) {
	        // check, if object with name already exists
	        // create alternative name ...
	        // try 50 times ...
	        $cnt=0;
	        $MAX_CNT=50;
	        do {
	            if ($cnt>$MAX_CNT) {
	                $error->set( $FUNCNAME, 2, 'object with name "'.$new_name.'" already exists. stop after '. $cnt . ' alternative names.');
	                return;
	            }
	            $loop_name = $new_name.' ('.($cnt+2).')';
	            if ( !fileC::redundancyTest2( $sqlo, $tablename, array('NAME'=>$loop_name,  $pk_name=>$this->abs_id), $obj_id) ) {
	                $new_name = $loop_name;
	                break;
	            }
	            $cnt++;
	        } while (1);
	    }
	    return $new_name;
	}
	
	/**
	 * analyse policyArray and generate name PRE create
	 * @param array $policyArray policyArray_STRUC
	 * @param array $paramarr: paramarr_STRUCT
	 * @return string
	 */
	function generate_name_PRE($sqlo, &$policyArray, &$paramarr) {
	    return $this->generate_name_ABS($sqlo, $policyArray, $paramarr, 'PRE');
	}
	
	/**
	 * analyse policyArray and generate name POST create
	 * @param array $policyArray policyArray_STRUC
	 * @return string
	 */
	function generate_name_POST($sqlo, &$policyArray, &$paramarr) {
	    return $this->generate_name_ABS($sqlo, $policyArray, $paramarr, 'POST');
	}
	
	/**
	 * set NEXT VAL for VARIO of ABSTRACT object
	 * @param object $sqlo
	 * @param int $next_val
	 */
	function set_ABS_next_val($sqlo, int $next_val) {
	    $vario_mod_lib = new oS_VARIO_mod();
	    $vario_option=array('noAccCheck'=>1);
	    $vario_mod_lib->setObject( $sqlo, $this->abs_table, $this->abs_id, $vario_option);
	    $vario_mod_lib->set_touched(1); // no touch ...
	    $vario_mod_lib->updateKeyVal($sqlo, self::ABS_VARIO_KEY , $next_val);
	}
}