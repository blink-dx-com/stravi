<?php
require_once( $_SESSION['s_sessVars']['loginPATH'].'/api/lib/metacall.inc' );

/**
 * @package o.EXP.create.inc
 * @author skube
 *
 */
class oEXP_create_UT extends create_UT_lib {
    
    
    private function cr_toolbox(&$sqlo) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        if ( $_SESSION['s_product']['type']!="blinkLIMS") {
            return;
        }
        
        $test_data_dir = $this->_getTestDir_path();
        $json_file = $test_data_dir.'/www/lab_blk/api/mods/UT_oEXP_create.json';
        if (!file_exists($json_file)) {
            throw new Exception('Test-file "'.$json_file.'" not exists.');
        }
        
        $this->infoout('JSON-file', $json_file);
        
        $assay_id = 'unittest-'. rand(10000,1000000000);
        $assay_content = file_get_contents($json_file);
        $assay_dict    = json_decode($assay_content, TRUE);
        $assay_dict['run']["start"] = date_unix2datestr(time(),1); // create current data
        
        $args_use=array(
            'args'=>array(
                'vals'=>array(
                    'NOTES'=>'UnitTest '.$FUNCNAME,
                    'EXT_ID'=>$assay_id
                )
            ),
            'json'=>$assay_dict,
            'assay' => array('data'=> $assay_content)
        );
        $metalib = new metacall('LAB/oEXP_create', $args_use);
        $answer = $metalib->run();
        
        $objid = $answer['data']['objid'];
        
        if (!$objid) {
            throw new Exception('No UT-experiment created. err:'.print_r($answer['error'],1) );
        }
        
        $this->obj_update_UT($sqlo, $objid);
        
        return $objid;
    }
    
    function create(&$sqlo) {

        
        switch ($this->synonym) {
            
            case 'default':
                $args = array(
                'vals'=> array(
                'NOTES'=> 'default UT experiment'
                    )
                );
                $obj_id = $this->_create_UT_obj_RAW($sqlo, $args, $this->synonym, 'EXP' );
                break;
                
            case 'toolbox':
                $this->infoout('Tool!');
                $obj_id = $this->cr_toolbox($sqlo);
                break;
        }
        return $obj_id;
    }
}