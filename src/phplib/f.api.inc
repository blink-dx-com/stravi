<?php
/**
 * API methods for user input checks
 * @package f.api.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class f_api_C {
    
    /**
     * parameter and object check
     * - supports also ASSOC-tables: check existance of mother object
     */
    static function objectCheck(&$sqlo, $tablename, $obj_id) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        if ( !table_exists($sqlo, $tablename) ) {
            $error->set( $FUNCNAME, 1, 'table "'.$tablename.'" unknown.' );
            return;
        }
        
        $t_rights = tableAccessCheck( $sqlo, $tablename );
        if ( $t_rights['read'] != 1 ) {
            $answer = getTableAccessMsg( $tablename, 'read' );
            $error->set( $FUNCNAME, 2, $answer );
            return;
        }
        
        //id is not set
        if (!$obj_id) {
            $error->set( $FUNCNAME, 8, 'id (table "'.$tablename.'") is not set.' );
            return;
        }
        
        //id is not numeric
        if (!is_numeric($obj_id) ) {
            $error->set( $FUNCNAME, 4, 'id (table "'.$tablename.'") is not numeric.' );
            return;
        }
        
        $objCheckTable = $tablename;
        
        $numPks = countPrimaryKeys($tablename);
        if ($numPks>1) {
            $mother = mothertable_get2($tablename);
            if ($mother==NULL) {
                $error->set( $FUNCNAME, 5, 'ASSOC-table '.$tablename.' has no mother-table.' );
                return;
            }
            $objCheckTable = $mother;
        }
        
        // object doesn't exist
        if ( !gObject_exists ($sqlo, $objCheckTable, $obj_id) ) {
            $error->set( $FUNCNAME, 6, 'object [T:'.$objCheckTable.', ID:'.$obj_id.'] not found in DB' );
            return;
        }
        
        
    }
    
}