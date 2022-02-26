<?php 
// @module o.EXTRA_CLASS.mod.inc
require_once 'insertx.inc';


class oEXTRA_CLASS_new {
    
    /**
     * create a class by OFS, insert also attributes
     * @param object $sqlo
     * @param array $ofs object.info.inc:objFeatStruct -- OFS
     *    ['vals']=> array of class features
     *    ['ass']['EXTRA_ATTRIB'] => array of class atributes
     */   
    static function create_by_OFS(object $sqlo, array $ofs) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;

        if (empty($ofs['vals'])) {
            $error->set( $FUNCNAME, 10, 'Bad Input: [vals] are missing.' );
            return;
        }
        
        if ($ofs['vals']['TABLE_NAME']==NULL or $ofs['vals']['NAME']==NULL) {
            $error->set( $FUNCNAME, 12, 'Bad Input: [vals][TABLE_NAME] or  [vals][NAME] are missing.' );
            return;
        }
        
        // check exists
        $class_id = glob_elementDataGet2($sqlo, 'EXTRA_CLASS', array('TABLE_NAME'=>$ofs['vals']['TABLE_NAME'], 'NAME'=>$ofs['vals']['NAME']), 'EXTRA_CLASS_ID');
        if ($class_id) {
            $error->set( $FUNCNAME, 15, 'Class ('.$ofs['vals']['TABLE_NAME'].':'.$ofs['vals']['NAME'].') already exists.' );
            return $class_id;
        }
        
        $class_id = insertC::new_meta($sqlo, 'EXTRA_CLASS', $ofs);
        if ($error->Got(READONLY))  {
            $error->set( $FUNCNAME, 20, 'Error on creation.' );
            return;
        }
        
        if ( !empty($ofs['ass']['EXTRA_ATTRIB']) ) {
            
            // insert attributes
            foreach($ofs['ass']['EXTRA_ATTRIB'] as $attrib_args) {  
                $attrib_args['EXTRA_CLASS_ID'] = $class_id;              
                // secure the insert
                unset( $attrib_args['EXTRA_ATTRIB_ID']);               
                insertRawC::insertR($sqlo, 'EXTRA_ATTRIB', $attrib_args, 'EXTRA_ATTRIB_ID');
            }
        }
        
        return $class_id;
    }
}

class oEXTRA_CLASS_mod {
    function __construct($id) {
        $this->id=$id;
    }
    
    /**
     * copy all attributes from SRC to this destination
     * @param object $sqlo
     * @param int $src_class_id
     */
    function copy_all_atributes($sqlo, int $src_class_id) {
        
        $class_id = $this->id;
        $sqlsel = "* from EXTRA_ATTRIB where EXTRA_CLASS_ID=".$src_class_id.' order by POS';
        $sqlo->Quesel($sqlsel);
        $attribs=array();
        while ( $sqlo->ReadArray() ) {
            $attribs[] = $sqlo->RowData;
        }
  
        // insert attributes
        foreach( $attribs as $attrib_args) {
            $attrib_args['EXTRA_CLASS_ID'] = $class_id;
            unset( $attrib_args['EXTRA_ATTRIB_ID']);
            insertRawC::insertR($sqlo, 'EXTRA_ATTRIB', $attrib_args, 'EXTRA_ATTRIB_ID');
        }
    }
}
