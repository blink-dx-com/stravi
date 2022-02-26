<?php

require_once 'o.AS_ASR.subs.inc';
require_once ("f.assocUpdate.inc");
require_once ("glob.obj.update.inc");
require_once 'o.S_VARIO.mod.inc';

/**
 * MODIFY data for ASR: class ASR and BLDK
 * AS_ASR:
 *   'DATAX'=>array('chan_def'=>$BLDK_chan_def_str, ''=>)
 * AS_ASR_H_CSP:
 *   DATAX =>  {"channels":[{"params":{"al": ....}
 *   
 * Definitions:
 *   CHAN: "chan_def" dict
 *   BSD : Bead -- "bead_type" dictionary 
 * @author Steffen
 * @package o.AS_ASR.mod.inc
 */
class o_AS_ASR_up {
    
    private $CHAN_dictionary;
    
    /**
     * 
     * @var array $update_args
     *   [$bsid]
     *      ['toJSON'] = array()
     *      ['bead_reag_MAC_id'] = int
     */
    private $update_args = array();
    private $err_stack   = array(); 
    private $ASR_type; // see constants ASR_TYPE_...
    
    const ASR_TYPE_ABS =1;
    const ASR_TYPE_CONC=2;
    
    function __construct($sqlo, $objid) {
       
        
        $this->CHAN_dictionary=NULL;
        $this->ASR_type=NULL;
        
        $this->update_args = array();
        $this->err_stack   = array(); 
        $this->assoctable='AS_ASR_H_SP';
        $this->objid   = $objid;
        $this->asr_lib = new oAS_ASRLib_subs($sqlo, $this->objid);
        
    }
    
    function _add_err_stack($pos, $text) {
        $this->err_stack[$pos][] = $text;
    }
    
    
    function check_CHAN($sqlo, $CHAN_dictionary) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $this->CHAN_dictionary = $CHAN_dictionary;
        
        if (empty($CHAN_dictionary)) {
            $error->set( $FUNCNAME, 2, 'No channels defined in CHAN_dictionary.' );
            return;
        }
    }
    
    
    public function set_ASR_type($asr_type) {

        $this->ASR_type = $asr_type;
        if ($asr_type==self::ASR_TYPE_CONC) {
            $this->assoctable='AS_ASR_H_CSP';
        }
        if ($asr_type==self::ASR_TYPE_ABS) {
            $this->assoctable='AS_ASR_H_SP';
        }
    }
    
    /**
     * create $this->update_args
     * @param object $sqlo
     * @param array $BSD_dict_IN
     * @param int $ASR_TYPE
     * @param array $bsd_opt=array(
            'bead_reag_MAC_id.ignore'=>1
        );
     */
    function check_BSD(object $sqlo, array &$BSD_dict_IN, int $ASR_TYPE, $bsd_opt=array() ) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $this->set_ASR_type( $ASR_TYPE );
        
        debugOut("(89) BSD_dict_IN:".print_r($BSD_dict_IN,1), $FUNCNAME, 2);
        
        if (!is_array($BSD_dict_IN)) {
            $error->set( $FUNCNAME, 1, 'No BSD dict given.' );
            return;
        }
        
        foreach( $BSD_dict_IN as $row) {
            
            $bsid = $row['bsid'];
            if (!$bsid) {
                $this->_add_err_stack($bsid, '(53) bsid must be >0.');
                return;
            }
            $BSD_dict_one = array();
            $channels_arr = array();
            
            $reag_IN_arr = &$row['reag_link'];

            // 'lib_reag_id',	'bead_reag_id',	'lib_reag_name','bead_reag_MAC_id', 'lib_reag_MAC_id' 
            if ( $ASR_TYPE==self::ASR_TYPE_CONC ) {

                
                $keyx='bead_reag_MAC_id';
                if ($reag_IN_arr[$keyx]) {
                    $this->update_args[$bsid][$keyx] = $reag_IN_arr[$keyx];
                } else {
                    if ($bsd_opt['bead_reag_MAC_id.ignore']) {
                        // ignore this missing MAC (usefule for ASBLib update in a customer database)
                    } else {
                        $this->_add_err_stack($bsid, 'bead_reag_MAC_id missing!');
                    }
                }
                
                // no error, if not exists ...
                $keyx='lib_reag_MAC_id';
                if ($reag_IN_arr[$keyx]) {
                    $this->update_args[$bsid][$keyx] = $reag_IN_arr[$keyx];
                }
                
            } else {
                
                $keyx='bead_reag_id';
                if ($reag_IN_arr[$keyx]) {
                    $this->update_args[$bsid][$keyx] = $reag_IN_arr[$keyx];
                }else {
                    $this->_add_err_stack($bsid, 'bead_reag_id missing!');
                }
                
                // no error, if not exists ...
                $keyx='lib_reag_id';
                if ($reag_IN_arr[$keyx]) {
                    $this->update_args[$bsid][$keyx] = $reag_IN_arr[$keyx];
                }
                
            }

            $chan_in = &$row['channels'];
            
            if (!is_array($chan_in)) {
                $this->_add_err_stack($bsid, 'No channel features found!');
            } else {
                
                $chan_id = 1;
                foreach($chan_in as $chan_dict) {
                    
                    if (!is_array($chan_dict)) {
                        $this->_add_err_stack($bsid, 'Channel '.$chan_id.' no features found.');
                    }
                    $channels_arr[] = $chan_dict;
                    
                    $chan_id++;
                    
                }
            }
            $BSD_dict_one = $channels_arr;
            
            if (!is_array($this->err_stack[$bsid])) {
                $this->update_args[$bsid]['toJSON'] = array(
                    'channels'      => $BSD_dict_one,
                    "bead_type_name"=> $row['bead_type_name']
                );
            }
            
            
        }
        
        debugOut("(166) update_args:".print_r($this->update_args,1), $FUNCNAME, 2);
        
        if (sizeof($this->err_stack)) {
            $error->set( $FUNCNAME, 1, 'Errors:'.print_r($this->err_stack,1) );
            return;
        }
    }
    
    private function get_max_bsid(&$old_compos) {
        if (empty($old_compos)) return 0;
        
        $max_bsid=0;
        foreach($old_compos as $row) {
            $max_bsid = max($max_bsid, $row['BSID']);
        }
        return $max_bsid;
    }
    
    
    
    
    /**
     * check input data CHAN + BSD
     * @param object $sqlo
     * @param array $BSD_dict_IN
     *   POS => array( reag_id, bead_id, channels )
     */
    function check($sqlo, $BSD_dict_IN, $CHAN_dictionary) {
        //global $error;
        //$FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $this->err_stack=array();  
        $this->update_args = array();

        $this->check_CHAN($sqlo, $CHAN_dictionary);
        $this->check_BSD($sqlo, $BSD_dict_IN);
    }
    
    /**
     * insert one BEAD row
     * @param object $sqlo
     * @param array $dict_row
     *    'bsid'           -- optional
     *     lib_reag_MAC_id'
     *     bead_reag_MAC_id  -- REQUIRED
     *     'toJSON'          -- REQUIRED
     *
     */
    function insert_bead_row(object $sqlo, array $dict_row, int $ASR_type ) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $bsid = $dict_row['bsid'];
        
        if ($ASR_type==self::ASR_TYPE_CONC) {
            
            $this->assoctable='AS_ASR_H_CSP';
            if( !$dict_row['bead_reag_MAC_id'] ) {
                $error->set( $FUNCNAME, 1, 'Input "bead_reag_MAC_id" missing.' );
                return;
            }
            if( !is_array($dict_row['toJSON']) ) {
                $error->set( $FUNCNAME, 2, 'Input "toJSON" missing.' );
                return;
            }
            $old_compos = $this->asr_lib->get_c_compos_by_pos($sqlo);
            $last_bsid  = $this->get_max_bsid($old_compos);
            
        } else {
            
            $this->assoctable='AS_ASR_H_SP';
            if( !$dict_row['bead_reag_id'] ) {
                $error->set( $FUNCNAME, 1, 'Input "bead_reag_id" missing.' );
                return;
            }
            $old_compos = $this->asr_lib->get_compos_by_pos($sqlo);
            $last_bsid  = $this->get_max_bsid($old_compos);
        }
        
        if (!$bsid) {
            $bsid = $last_bsid + 1;
        }
        
        $assoclib = new  fAssocUpdate();
        $assoclib->setObj( $sqlo, $this->assoctable, $this->objid );
        if ($error->Got(READONLY))  {
            $error->set( $FUNCNAME, 1, 'Error on Object mod-init.' );
            return;
        }
        
        $argu = array('BSID'=>$bsid);
        
        if ($ASR_type==self::ASR_TYPE_CONC) {
            $argu['LIB_REAG_ID'] = $dict_row['lib_reag_MAC_id'];
            $argu['BEAD_REAG_ID']= $dict_row['bead_reag_MAC_id'];
        } else {
            $argu['LIB_REAG_ID'] = $dict_row['lib_reag_id'];
            $argu['BEAD_REAG_ID']= $dict_row['bead_reag_id'];
        }
        
        if (is_array($dict_row['toJSON'])) {
            $argu['DATAX']       = json_encode($dict_row['toJSON']);
        }
        
        debugOut("(296) dict_row:".print_r($dict_row,1), $FUNCNAME, 2);
        debugOut("(297) ASR_type:$ASR_type argu:".print_r($argu,1), $FUNCNAME, 2);
        $assoclib->insert( $sqlo, $argu);
    }
    
    function get_update_args() {
        return  $this->update_args;
    }
    
    
    function update_CHAN( $sqlo ) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        if (empty($this->CHAN_dictionary) ) {
            $error->set( $FUNCNAME, 1, 'CHANNEL-data missing.' );
            return;
        }
        
        $asr_base_data = array(
            'chan_def'=>$this->CHAN_dictionary,
            'version' =>$this->asr_lib->get_version()
        );
        $BLDK_chan_def_str = json_encode( $asr_base_data );
        $arg_meta=array( 
            'vals'=> array(
                'DATAX'=>$BLDK_chan_def_str
            )
            
        );
        debugOut("(221) arg_meta: ".print_r($arg_meta,1), $FUNCNAME, 3);
        
        $UpdateLib = new globObjUpdate();
        $UpdateLib->update_meta( $sqlo, 'AS_ASR', $this->objid , $arg_meta );
        
        if ($error->Got(READONLY))  {
            $error->set( $FUNCNAME, 2, 'Error on updating AS_ASR.' );
            return;
        }
        
    }
    
   
   
    
    /**
     * update BSD ...
     * update of BEAD_REAG_ID : NOT_ALLOWED !!!
     * @param object $sqlo
     */
    function update_BSD( $sqlo ) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $update_args = $this->update_args;
        
        debugOut("(236) update_args:".print_r($this->update_args,1), $FUNCNAME, 2);

        if ($this->ASR_type==self::ASR_TYPE_CONC) {
            $this->assoctable='AS_ASR_H_CSP';
            $old_c_compos = $this->asr_lib->get_c_compos_by_pos($sqlo);
        }
        
        $assoclib = new  fAssocUpdate();
        $assoclib->setObj( $sqlo, $this->assoctable, $this->objid );
        if ($error->Got(READONLY))  {
            $error->set( $FUNCNAME, 1, 'Error on Object mod-init.' );
            return;
        }
    
        foreach( $update_args as $bsid=> $row) {
            
            if (!$bsid) {
                $error->set( $FUNCNAME, 241, 'BSID missing.' );
                return;
            }
            $idarr= array('BSID'=>$bsid);
            $argu = array();
            if ($this->ASR_type==self::ASR_TYPE_CONC) {
                $argu['LIB_REAG_ID'] = $row['lib_reag_MAC_id'];
                // $argu['BEAD_REAG_ID']= $row['bead_reag_MAC_id'];
            } else {
                $argu['LIB_REAG_ID'] = $row['lib_reag_id'];
                //$argu['BEAD_REAG_ID']= $row['bead_id'];
            }
            
            $argu['DATAX']       = json_encode($row['toJSON']);
            
            //echo "DDX_JS: ".$argu['DATAX']  ."<br>";
            
            if ( is_array($old_c_compos[$bsid]) ) {
                
                // update
                
                // $old_bead_row = &$old_c_compos[$bsid];
                //                 if ($old_bead_row['BEAD_REAG_ID']!=$argu['BEAD_REAG_ID']) {
                //                     $error->set( $FUNCNAME, 264, 'You cannot change the bead_reag_MAC_id (POS:'.$bsid. ' old:'.$old_bead_row['BEAD_REAG_ID'].
                //                         ' new:'.$argu['BEAD_REAG_ID'] . ')' );
                //                     return;
                                   
                //                 }
                debugOut("(362) update: bsid:$bsid argu:".print_r($argu,1), $FUNCNAME, 2);
                $assoclib->update( $sqlo, $argu, $idarr);
                
            } else {
                
                // insert 
                if ($argu['BEAD_REAG_ID']) {
                    $argu['BSID']    = $bsid;
                    debugOut("(370) insert: bsid:$bsid argu:".print_r($argu,1), $FUNCNAME, 2);
                    $assoclib->insert( $sqlo, $argu);
                }
            }
            
            if ($error->Got(READONLY))  {
                $error->set( $FUNCNAME, 1, 'Error on POS:'.$bsid );
                return;
            }
        }
    }
    
    /**
     * Cupdate from $this->update_args
     * @param object $sqlo
   
     */
    function update( $sqlo ) {
        global $error;

        // channel_def
        $this->update_CHAN($sqlo);
        if ($error->Got(READONLY))  {
            return;
        }
        
        $this->update_BSD($sqlo);
        if ($error->Got(READONLY))  {
            return;
        }
        
    }
    
    /**
     * delete all AS_ASR_H_SP data
     * @param object $sqlo
     */
    function delete_all( $sqlo ) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        if (!$this->ASR_type) {
            throw new Exception('ASR_type not set.');
        } 
        
        if ($this->ASR_type==self::ASR_TYPE_CONC) {
            $old_compos = $this->asr_lib->get_c_compos_by_pos($sqlo);
        } else {
            $old_compos = $this->asr_lib->get_compos_by_pos($sqlo);
        }

        $assoclib = new  fAssocUpdate();
        $assoclib->setObj( $sqlo, $this->assoctable, $this->objid );

        foreach($old_compos as $pos => $data) {
            $idarr= array('BSID'=>$pos);
            $assoclib->delOneRow($sqlo, $idarr);
        }
        
        // delete chan_def
        $arg_meta=array(
            'vals'=> array(
                'DATAX'=>NULL
            )
        );
        $UpdateLib = new globObjUpdate();
        $UpdateLib->update_meta( $sqlo, 'AS_ASR', $this->objid , $arg_meta );
        
        if ($error->Got(READONLY))  {
            $error->set( $FUNCNAME, 2, 'Error on reseting AS_ASR.' );
            return;
        }
    }
    
    /**
     * $this->ASR_type must be set ...
     * @param object $sqlo
     * @param int $bsid
     * @throws Exception
     */
    function delete_row($sqlo, int $bsid) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        if (!$this->ASR_type) {
            throw new Exception('ASR_type not set.');
        } 
        debugOut("(448) Delete row: BSID:".$bsid , $FUNCNAME, 2);
        
        $assoclib = new  fAssocUpdate();
        $assoclib->setObj( $sqlo, $this->assoctable, $this->objid );

        $idarr= array('BSID'=>$bsid);
        $assoclib->delOneRow($sqlo, $idarr);
        
    }
    
}

