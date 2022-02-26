<?php
/**
 * GUI definitions and methods for REA_LOG.NOTES with DEVSIC.INFOARR
 * @package o.REA_LOG.guisub.inc
 * @swreq UREQ:0000019 GLOBAL
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
require_once 'f.text.inc';
require_once 'f.text_html.inc';

require_once ('o.REA_LOG.subs.inc');
require_once 'o.DEVSIC.subs.inc';

 
class oREA_LOG_guisub {

    var $f_ACCEPT_icons = array(
    	'1' =>'i13_realog_P.png',
    	'2' =>'i13_realog_A.png',
    	'3' =>'i13_realog_S.png',
    	'4' =>'i13_realog_F.png',
    	'5' =>'i13_realog_X.png'
    );
     
    function __construct () {
    }
     
    /**
     * get icon-name of accept-flag
     */
    function getIconOfAccept($accept) {
    	return $this->f_ACCEPT_icons[$accept];
    }
 
}
 
/**
* REA_LOG:DEVSIC_ID specials
* show special form for REA_LOG:NOTES if KEY=Service
* @package o.REA_LOG.devsic_spec.inc
* @swreq   UREQ:UREQ:0001458: g > Ger�tewartung (Concept)
* @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
*/
class oREA_LOG_infoarrGui {
    
    private $infoarr;
    const XDATA_reserved_keys = array('KEY','STATUS', 'NOTES');
    
    function devsic_init(&$sqlo, $devsic_id) {
        $this->infoarr = oDEVSIC_subs::get_info_struct($sqlo, $devsic_id);
    }
    
    private function _get_infoarr_row($key) {
        $found=0;
        foreach($this->infoarr as $row) {
            if ($row['t']==$key) {
                $found=1;
                break;
            }
        }
        
        if (!$found) return;
        
        return $row;
    }
    
    /**
     * encode string to escape the COLON ":"
     * @param string $input
     * @return mixed
     */
    // 	private function decode_value($input) {
    // 		$temp_str = str_replace('&#58;', ':',$input);
    // 		return $temp_str;
    // 	}
    
    /**
     * merge abstract plan and real values
     * create $this->infoarr[] = array('t'=>$title_loop, 'type'=>'text', 'n'=>'');
     * @param string $xdata
     * @return array $now_infoarr
     */
    function structInit($xdata) {
        
        //$nowStrArr = json_decode($xdata, TRUE);
        //if (!sizeof($nowStrArr)) $nowStrArr=array();
        
        // build structure INFOARR
        $now_infoarr = oREA_LOG_subs2::get_XDATA_keyvals($xdata);
        
        // merge arrays of $now_infoarr and $this->infoarr (the definition of FORM)
        // glob_printr( $this->infoarr, $FUNCNAME.":infoarr info" );
        
        
        if ( is_array($now_infoarr) ) {
            
            if  (!is_array($this->infoarr) ) $this->infoarr=array();
            
            $found_arr=array();
            // check, if keys match
            foreach( $this->infoarr as $valarr) {
                $title_loop = $valarr['t'];
                if (is_array($now_infoarr[$title_loop])) $found_arr[$title_loop]=1; // has matched !
            }
            
            
            // create missing entries ...
            foreach( $now_infoarr as $title_loop=>$valarr) {
                if (!$found_arr[$title_loop]) {
                    $this->infoarr[] = array('t'=>$title_loop, 'type'=>'text', 'n'=>'');
                }
            }
            
        }
        return $now_infoarr;
    }
    
    /**
     * show Service plan actions : EDIT-form
     * - form-output:
     * 	_REQUEST rowx array ( index => array(
     *    KEY - title
     *    STATUS - value
     *    NOTES - notes
     *    ) )
     * @params array $now_infoarr
     */
    function editform($now_infoarr) {
        
        // show edit-array
        if ( is_array($this->infoarr) ) {
            
            $tabobj = new visufuncs();
            $headOpt = array( "title" => "Service plan actions");
            $headx  = array ("#", "Title", "Value", "Notes", "Notes of the plan", 'Other');
            
            $tabobj->table_head($headx,   $headOpt);
            
            $feldtmp = array('checkbox', 'text');
            
            foreach( $this->infoarr as $i=>$valarr) {
                
                $onerow = $valarr;
                $KEY   = $onerow['t'];
                $title = $onerow['x'];
                if ($title==NULL) $title=$KEY; // fall back
                $data_row = &$now_infoarr[$KEY];
                
                $fieldType = $onerow['type'];
                if (!in_array($fieldType, $feldtmp)) {
                    $fieldType='text';
                }
                $valueHtmlTag = NULL;
                if ($fieldType=='checkbox') {
                    if ($data_row['STATUS']>0) $tmpChecked='checked';
                    else $tmpChecked='';
                    $valueHtmlTag = '<input type=checkbox name="rowx['.$i.'][STATUS]"  value="1" '.$tmpChecked.'>';
                }
                if ($fieldType=='text') {
                    $valueHtmlTag = '<input type=text name="rowx['.$i.'][STATUS]"  value="' .$data_row['STATUS'].
                    '" size=20 maxlength=150>';
                }
                
                $dataArr = array(
                    ($i+1).'.<input type=hidden name="rowx['.$i.'][KEY]"  value="' .htmlspecialchars($KEY). '">',
                    $title,
                    $valueHtmlTag,
                    '<input type=text name="rowx['.$i.'][NOTES]"  value="' .$data_row['NOTES']. '" size=30 maxlength=70>',
                    htmlspecialchars($onerow['n'])
                );
                
                // other custom keys : not editable
                $other='';
                if(!empty($data_row)) {
                    $extra_arr=array();
                    foreach($data_row as $key=>$val) {
                        if( in_array($key, self::XDATA_reserved_keys)) continue;
                        $extra_arr[] = $key.': '.htmlspecialchars($val) .
                        '<input type=hidden name="rowx['.$i.']['.$key.']"  value="' .htmlspecialchars($val). '">';
                    }
                    if (!empty($extra_arr)) {
                        $other = implode("<br>",$extra_arr); // extra table cell
                    }
                }
                $dataArr[] = $other;
                $tabobj->table_row ($dataArr);
                
            }
            
            
            $tabobj->table_close();
            
            echo "<br />\n"; // extra break
        }
    }
    
    /**
     * show table of params, NO edit!
     *
     */
    function viewtable($sqlo, $now_infoarr) {
        //global $error;
        //$FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        
        if ( is_array($this->infoarr) ) {
            
            $tabobj  = new visufuncs();
            $headOpt = array( ); // "title" => "Actions"
            $headx   = array ("#", "Title", "Value", "Notes", "Other");
            
            $tabobj->table_head($headx,   $headOpt);
            
            // $feldtmp = array('checkbox', 'text');
            $i=0;
            foreach( $this->infoarr as $def_row) {
                
                
                $KEY   = $def_row['t']; 
                $title = $def_row['x'];
                if ($title==NULL) $title=$KEY; // fall back
                
                $data_row  = &$now_infoarr[$KEY];
                
                $notes = $data_row['NOTES'];
                if (f_text::find_db_obj($notes, 0)) {
                    $notes_out = f_text_html::notes_out($sqlo, $notes, 0, 1);
                } else {
                    $notes_out = htmlspecialchars( $data_row['NOTES'] );
                }
                
                $dataArr = array(
                    ($i+1),
                    $title,
                    $data_row['STATUS'],
                    $notes_out
                );
                
                // other custom keys : not editable
                $tmp=array();
                if(!empty($data_row)) {
                    foreach($data_row as $key=>$val) {
                        if( in_array($key, self::XDATA_reserved_keys)) continue;
                        $tmp[] = $key.': '.htmlspecialchars($val);
                    }
                }
                if (!empty($tmp)) {
                    $tmp2 = implode("<br>\n",$tmp);
                } else $tmp2='';
                $dataArr[] = $tmp2;
                
                $tabobj->table_row ($dataArr);
                $i++;
            }
            
            
            $tabobj->table_close();
            
            echo "<br />\n"; // extra break
        }
    }
    
    /**
     * show table of params
     *
     */
    function view_slim($sqlo, $xdata_str) {
        
        $now_infoarr = oREA_LOG_subs2::get_XDATA_keyvals($xdata_str);
        
        if(empty($now_infoarr)) {
            return;
        }
        $tabobj  = new visufuncs();
        $headOpt = array( "title" => "Service plan actions");
        $headx   = array ("#", "Title", "Value", "Notes", "Other");
        
        $tabobj->table_head($headx,   $headOpt);
        
        $i=0;
        foreach( $now_infoarr as $data_row) {
            
            $KEY   = $data_row['KEY'];
            
            $def_row = $this->_get_infoarr_row($KEY);
            $title   = $def_row['x'];
            if ($title==NULL) $title=$KEY; // fall back
            
            $notes = $data_row['NOTES'];
            if (f_text::find_db_obj($notes, 0)) {
                $notes_out = f_text_html::notes_out($sqlo, $notes, 0, 1);
            } else {
                $notes_out = htmlspecialchars( $data_row['NOTES'] );
            }
            
            $dataArr = array(
                ($i+1),
                $title,
                $data_row['STATUS'],
                $notes_out
            );
            
            // other custom keys : not editable
            $tmp=array();
            foreach($data_row as $key=>$val) {
                if( in_array($key, self::XDATA_reserved_keys)) continue;
                $tmp[] = $key.': '.htmlspecialchars($val);
            }
            
            if (!empty($tmp)) {
                $tmp2 = implode("<br>\n",$tmp);
            } else $tmp2='';
            $dataArr[] = $tmp2;
            
            $tabobj->table_row ($dataArr);
            $i++;
        }
        
        $tabobj->table_close();
        echo "<br />\n"; // extra break
        
    }
}
