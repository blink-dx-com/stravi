<?php
require_once 'o.PROTO.steps.inc';

/**
 * layout things
 * @package o.ABSTRACT_PROTO.layout.inc
 * @author  Steffen Kube (steffen@blink-dx.com)

 */
class o_ABSTRACT_PROTO_layout {
    
    private static $keys = array(
        'AN', //A:NAME
        'AS', //A:SUA
        'AQ',
        'AU', //A:Units
        'AO', // A:NOTES
        'Q',
        'S',
        'D', // device
        'N',  
    );
    
    /**
     * 
     * @var array $layout
     * 'head'=>
     * 'rows'
     */
    private $layout;
    
    function __construct($sqlo, $pra_id) {
        
        $this->pra_id = $pra_id;
        $protLib = new gProtoOrg( );
        $orgopt=array("noStepScan"=>1);
        $protLib->setProto( $sqlo, $pra_id, 0, $orgopt);  
        $this->pra_steps = $protLib->get_pra_all_steps($sqlo, $pra_id);
    }
    
    function load_layout($sqlo) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $sqlo->Quesel('* FROM ABSTRACT_PROTO WHERE ABSTRACT_PROTO_ID='.$this->pra_id);
        $sqlo->ReadArray();
        $ap_feat = $sqlo->RowData;

        if ($ap_feat['LAYOUT']==NULL) {
            $error->set( $FUNCNAME, 1, 'No LAYOUT found.' );
            return;
        }
        
        $layout = json_decode($ap_feat['LAYOUT'],TRUE);
        if (empty($layout['head'])){
            $error->set( $FUNCNAME, 2, 'Bad LAYOUT format..' );
            return;
        }
        
        return $layout;
    }
    
    function get_steps_raw() {
        return $this->pra_steps;
    }
    
    public function check_cell($cell) {
        
        $cell=trim($cell);
        if ($cell==NULL) {
            return array();
        }
        $cellarr=explode(':',$cell);
        
        $step = $cellarr[0];
        $code = $cellarr[1];
        if (!is_numeric($step)) {
            throw new Exception('1: Step "'.$step.'" must be a number');
        }
        
        if (!isset($this->pra_steps[$step])) {
            throw new Exception('3: Step "'.$step.'" not exists in protocol.');
        }
        
        if (!in_array($code, self::$keys )) {
            throw new Exception('2: Code "'.$code.'" is not valid.');
        }
        
        return array('st'=>$step, 'co'=>$code);
    }
}
