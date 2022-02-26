<?php
require_once 'o.S_VARIO.subs.inc';
require_once 'o.S_OBJLINK.subs.inc';
require_once ('object.info.inc');
require_once $_SESSION['s_sessVars']['AppLabLibDir'].'/lablib/ASB/o.SUC.PB.sub.inc';

/**
 * helper methods to handle SUA class "ASR Lib" and "BLDK" (Blink Library Dev Kit)
 * 
 *  Codes: 
 *  - ASRLib
 *  - BLDK
 * data format in database:
 *   TABLE: SUA_BOM
 *     POS : Species ID
 *     COMPO_SUA_ID : reagent ID
 *     DATAX: contains JSON-string of rest of structure
 *        'chan_def'=>array()
 *        'bead_type_name' => string
   
    see also in WIki : BeadLibDataSpec
   @var array ASBLib_dict_STRUCT
        'chan_def'=>$chan, 
        'bead_type'=>$bt, 
        'version'=>$this->format_version,
        'origin'=>array('roid'=>$this->asr_id) 
     

   @var array CHAN_dictionary_STRUCT
      of array(
     *  'chan_id' => 1,2,3
     *  'name' => e.g. 'Cy5'
     *  'func' => 'segmentation'
     *  )
   
   @var bead_type_STRUCT array of array( index == Position
        'bsid'=> int,
        'bead_type_name'=> xxx
        'channels': [
            array ( 'func'=>, 'params' : .. )
         ]
        'reag_link' => array ( 'bead_reag_id' => id, 'lib_reag_id'=> $asr_sua_id, 'lib_reag_name'=>)
    )
   
   
 * @package o.AS_ASR.subs.inc
 */
class oAS_ASRLib_subs{
    
    const FORMAT_VERSION=1.02;
    const DEAFULT_CHAN_DICT= array(
        array("chan_id"=>"1","name"=>"blue",    "func"=>array(), "coding"=>array( "al", "pl") ),
        array("chan_id"=>"2","name"=>"green",   "func"=>array(), "coding"=>array( "al", "pl") ),
        array("chan_id"=>"3","name"=>"red",     "func"=>array(), "coding"=>array( "al", "pl") ),
        array("chan_id"=>"4","name"=>"darkred", "func"=>array(), "coding"=>array( "al", "pl") )
    );
  
    private static $class_name='ASR';
    /**
     * 
     * @var array $CHAN_dictionary_STRUCT 
     */
    private $CHAN_dictionary;
    private $MAX_CHANNNELS=3;
    private $format_version=NULL;    // format version, please keep in sync with class oSUC_PB_crea_prod !!!
    private $asr_id;
    
    function __construct(&$sqlo, $asr_id=NULL) {
        global $varcol, $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $varcol = & Varcols::get();

       
        $this->class_id	= $varcol->class_name_to_id( 'ABSTRACT_SUBST', self::$class_name );
        if (!$this->class_id) {
            $error->set( $FUNCNAME, 1, 'Class '.self::$class_name.' unknown' );
            return;
        }
        
        if ($asr_id) {
            $this->set_asr($sqlo, $asr_id);
        }

    }
    
    // <static> some  methods ...
    
    /**
     * SUA is ASR ?
     */
    static function is_asr($sqlo, $sua_id) {
       
        $answer = 0;
        $objInfoC_lib = new objInfoC();
        $class_info   = $objInfoC_lib->class_params( $sqlo, 'ABSTRACT_SUBST', $sua_id );
        $sua_class = $class_info['class_name'];
        
        if ($sua_class==self::$class_name) {
            $answer = 1;
        }
        
        return $answer;
    }
    
    static function get_class_id() {
        global $varcol, $error;
        $class_id	= $varcol->class_name_to_id( 'ABSTRACT_SUBST', self::$class_name );
        return $class_id;
    }

    
    /**
     * get ASR-ID linked by mother ($src_table, $src_id)
     * @param object $sqlo
     * @param string $src_table e.g. ABSTRACT_SUBST
     * @param int $src_id  e.g. 43
     * @return number
     */
    static function get_asrid_by_obj($sqlo, $src_table, $src_id) {
        
        $obj_link_lib = new oS_OBJLINK_subs();
        $asr_ids = $obj_link_lib->getChiObjByChiTab($sqlo, $src_table, $src_id, 'AS_ASR');
        if (!sizeof($asr_ids)) {
            return 0;
        }
        $asr_id = current($asr_ids);
        
        return $asr_id;
        
    }
    
    /**
     * convert v1.01 => v1.02
     * @param array $datax_struct
     */
    private static function chan_convert1to2(&$datax_struct) {
        $ind_arr = array_keys($datax_struct['chan_def']);
           
        foreach($ind_arr as $index) {
            $old_func = $datax_struct['chan_def'][$index]['func'];
            $new_func = array();
            if (trim($old_func)!=NULL) {
                $new_func_x = explode(",",$old_func);
                $new_func = array_map('trim', $new_func_x);
            }
            $coding=array();
            $tmp = $datax_struct['chan_def'][$index]['al_coding'];
            if ($tmp) $coding[]='al';
            $tmp = $datax_struct['chan_def'][$index]['pl_coding'];
            if ($tmp) $coding[]='pl';
            
            $datax_struct['chan_def'][$index]['func']   = $new_func;
            $datax_struct['chan_def'][$index]['coding'] = $coding;
            
        }
    }
    
    // </static> End of static 
    
    /**
     * get ABSTRACT compos
     * @param object $sqlo
     * @return NULL[]
     */
    function get_compos_by_pos(&$sqlo) {
        
        if ( !$this->asr_id )	sys_error_my( 'class not initialized.' );
        
        $sua_bom = array();
        $sqlsel = "LIB_REAG_ID, BSID from AS_ASR_H_SP where AS_ASR_ID=".$this->asr_id . ' order by BSID';
        $sqlo->Quesel($sqlsel);
        while ( $sqlo->ReadArray() ) {
            $pos = $sqlo->RowData['BSID'];
            $sua_bom[$pos] = $sqlo->RowData;
        }
        
        return $sua_bom;
    }
    
    /**
     * get CONCRETE compos
     * @param object $sqlo
     * @return array 
     *    BSID, LIB_REAG_ID, BEAD_REAG_ID,  DATAX
     */
    function get_c_compos_by_pos(&$sqlo) {
        
        if ( !$this->asr_id )	sys_error_my( 'class not initialized.' );
        
        $suc_bom = array();
        $sqlsel = "BSID, LIB_REAG_ID, BEAD_REAG_ID,  DATAX from AS_ASR_H_CSP where AS_ASR_ID=".$this->asr_id . ' order by BSID';
        $sqlo->Quesel($sqlsel);
        while ( $sqlo->ReadArray() ) {
            $pos = $sqlo->RowData['BSID'];
            $suc_bom[$pos] = $sqlo->RowData;
        }
        
        return $suc_bom;
    }
    
    function set_asr($sqlo, $asr_id) {
        //global $error,$varcol;
        //$FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $this->asr_id = $asr_id;

        $this->asr_feats = glob_elemDataGet3( $sqlo, 'AS_ASR', array('AS_ASR_ID'=>$asr_id), array('DATAX') );
        
        $temp_str = $this->asr_feats['DATAX'];
        $this->CHAN_dictionary = array();
        if ($temp_str!='') {
            $datax_struct = json_decode($temp_str, True);
            //TBD: repair chan_def for v1.02 ???
            if ( $datax_struct['chan_def'][0]['func']!==NULL ) {
                if (!is_array($datax_struct['chan_def'][0]['func'])) {
                    self::chan_convert1to2($datax_struct);
                    $datax_struct['version']=1.02;
                }
            }
            $this->CHAN_dictionary = $datax_struct['chan_def'];
            $this->format_version  = $datax_struct['version'];
            if ($this->format_version==NULL) $this->format_version=1.01;
        } else {
            // new chan def: set DEFAULT format  ...
            $this->format_version=self::FORMAT_VERSION;
        }
  
    }
    
    /**
     * @todo recode !
     * @param object $sqlo
     * @throws Exception
     * @return array[]|string
     */
    private function get_a_BT_defs($sqlo) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $ASBLib_dict=array();

        $sqlsel = '* from AS_ASR_H_SP where AS_ASR_ID='.$this->asr_id . ' order by BSID' ;
        $sqlo->Quesel($sqlsel);
        
        // add Library Load Reag to this
        while ( $sqlo->ReadArray() ) {
            
            $loop_entry = array();
            $pos        = $sqlo->RowData['BSID'];
            $asr_sua_id = $sqlo->RowData['LIB_REAG_ID'];
            $bea_sua_id = $sqlo->RowData['BEAD_REAG_ID'];
            
            $loop_entry['bsid'] = $pos;
            
            if ($sqlo->RowData['DATAX']!=NULL) {
                $channel_one = json_decode($sqlo->RowData['DATAX'], TRUE);
                if (!is_array($channel_one)) {
                    throw new Exception('BSD_dictionary: channel_one (BSID:'.$pos.') does not contain the right format.');
                }
                $loop_entry = array_merge($loop_entry, $channel_one);
            } else {
                $loop_entry['channels']=NULL;
            }
            
            
            $loop_entry['reag_link']['lib_reag_id']  = $asr_sua_id;
            $loop_entry['reag_link']['bead_reag_id'] = $bea_sua_id;
            
            $ASBLib_dict[$pos] = $loop_entry;
        }
        
        // add names
        foreach($ASBLib_dict as $pos => $row) {
            $sua_name  = obj_nice_name ( $sqlo, 'ABSTRACT_SUBST', $row['reag_link']['lib_reag_id'] );
            $ASBLib_dict[$pos]['reag_link']['lib_reag_name'] = $sua_name;
        }
        
        debugOut("BSD_dictionary: <pre>".print_r($ASBLib_dict,1)."</pre>", $FUNCNAME, 1);
        
        return $ASBLib_dict;
    }
    
    private function set_format_vers($version) {
        $this->format_version=$version;
    }
    
    /**
     * get CONCRETE BT defs
     * - BEAD_REAG_ID is optional, could be EMPTY after a PAXML-Import ...
     * @param object $sqlo
     * @throws Exception
     * @return mixed[]
     */
    private function get_c_BT_defs($sqlo) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $bead_type_struct=array();
        $pb_lib = new oSUC_PB_sub(0);
        
        $compos = $this->get_c_compos_by_pos($sqlo);
        
        $min_version = self::FORMAT_VERSION;
        
        $arr_ind0=0;
        foreach($compos as $row) {
            
            $loop_entry = array();
            $pos        = $row['BSID'];
            $asr_suc_id = $row['LIB_REAG_ID'];
            $bea_suc_id = $row['BEAD_REAG_ID'];
            $datax_loop = $row['DATAX'];
            $loop_entry['bsid'] = $pos;
            
            if ($datax_loop==NULL) {
                throw new Exception('Bead_type-dictionary: ASB (BSID:'.$pos.') DATAX is empty.');
            }
            $chan_one_x = json_decode($datax_loop, TRUE);
            if (!is_array($chan_one_x)) {
                throw new Exception('Bead_type-dictionary: ASB (BSID:'.$pos.') does not contain the right format.');
            }
            
            if (!$bea_suc_id and empty($chan_one_x['channels'])) {
                throw new Exception('(5) (BSID:'.$pos.'): no channel-info from Precursor-BEAD or stored data in ASBLib.');
            }

            
            if ($bea_suc_id) {
                // WARNING: this object is optional, could be EMPTY after a PAXML-Import ...
                $pb_lib->set_obj($bea_suc_id);
                $pb_def = $pb_lib->get_PB_def($sqlo); // def of existing SUC
                if (!is_array($pb_def))  throw new Exception('(10) PB-Bead: (BSID:'.$pos.') Bead has no info.');
                if (!is_array($pb_def['bead_type']))  throw new Exception('(20) PB-Bead: (BSID:'.$pos.') Bead has no bead_type info.');
                
                $loop_entry['channels'] = $pb_def['bead_type']['channels'];
                $bead_format_version = $pb_def['version'];
                $min_version = min($min_version, $bead_format_version);
            
            } else{
                // is EMPTY but $chan_one_x is set
                $loop_entry['channels'] = $chan_one_x['channels']; // preload
            }

            
            // first: merge ['channels'][i]['params'] : array('al'=>, 'cl'=>)
            $chans_ids = array_keys($loop_entry['channels']);
            foreach($chans_ids as $i) {
                $loop_entry['channels'][$i]['params']['al'] = floatval($chan_one_x['channels'][$i]['params']['al']);
                unset($chan_one_x['channels'][$i]['params']); // was merged before ...
                
                $loop_entry['channels'][$i] = array_merge($loop_entry['channels'][$i], $chan_one_x['channels'][$i]);
            }
            unset($chan_one_x['channels']); // was merged before ...
            
            
            $loop_entry = array_merge($loop_entry, $chan_one_x);
            debugOut("loop_entry2: ".print_r($loop_entry,1), $FUNCNAME, 2);

            $loop_entry['reag_link']['lib_reag_MAC_id']  = $asr_suc_id;
            $loop_entry['reag_link']['bead_reag_MAC_id'] = $bea_suc_id;
            
            if ($asr_suc_id) {
                $suc_feats  = glob_elemDataGet3 ( $sqlo, 'CONCRETE_SUBST', array('CONCRETE_SUBST_ID'=>$asr_suc_id),
                    array('NAME', 'ABSTRACT_SUBST_ID') );
            
                $sua_name = obj_nice_name ( $sqlo,  'ABSTRACT_SUBST', $suc_feats['ABSTRACT_SUBST_ID'] );
                $loop_entry['reag_link']['lib_reag_name']     = $sua_name;
                $loop_entry['reag_link']['lib_reag_MAC_name'] = $suc_feats['NAME'];
            }
            
            
            $bead_type_struct[$arr_ind0] = $loop_entry;
            $arr_ind0++;
        }
        
        if ($min_version=='1.01' or $min_version==NULL) $min_version='1.02'; // this is allowed for v1.02 !
        $this->set_format_vers($min_version);
        
        
        debugOut("BSD_dictionary: <pre>".print_r($bead_type_struct,1)."</pre>", $FUNCNAME, 2);
        
        return $bead_type_struct;
        
    }
  
    
    /**
     * get bead_type_dictionary
     * @param object $sqlo
     * @return array bead_type_STRUCT (see BeadLibDataSpec)
     */
    public function get_bead_type_defs($sqlo) {
        
        
        if ( !$this->asr_id )	sys_error_my( 'class not initialized.' );
        
        $sqlsel = 'count(1) from AS_ASR_H_CSP where AS_ASR_ID='.$this->asr_id;
        $sqlo->Quesel($sqlsel);
        $sqlo->ReadRow();
        if ($sqlo->RowData[0]) {
            return $this->get_c_BT_defs($sqlo);
        }
        
        $sqlsel = 'count(1) from AS_ASR_H_SP where AS_ASR_ID='.$this->asr_id;
        $sqlo->Quesel($sqlsel);
        $sqlo->ReadRow();
        if($sqlo->RowData[0]) {
            return $this->get_a_BT_defs($sqlo);
        }
    }
    
    /**
     * get CHAN_dictionary
     * @param object $sqlo
     */
    public function get_chan_defs() {
        return $this->CHAN_dictionary;  
    }
    
    
    /**
     * get full data of BeadLibDataSpec
     * @param object $sqlo
     * @return array ASBLib_dict_STRUCT
     */
    public function get_BeadLibDataSpec(&$sqlo) {
        $chan = $this->get_chan_defs() ;
        $bt   = $this->get_bead_type_defs($sqlo);
        
        $result = array(
            'chan_def'=>$chan, 
            'bead_type'=>$bt, 
            'version'=>$this->format_version,
            'origin'=>array('roid'=>$this->asr_id)
            
        );
        return $result;
    }
    
    function get_MAX_CHANNNELS() {
        return $this->MAX_CHANNNELS;
    }
    function get_version() {
        return $this->format_version;
    }
    
    /**
     * get allowed keys
     * @return string[]
     */
    function get_chan_keys() {
        /**
         *
         * @var array $result
         *  'type'=>
         *    'string',
         *    'tupel',
         *    'number'
         */
        $result = array(
            'params' => array('need'=>1, 'type'=>'tupel'),
            
        );
        return $result;
    }
    
    function get_empty_form_dict() {
        $result = array(
            'params' => array(),
        );
        return $result;
    }
    
}
