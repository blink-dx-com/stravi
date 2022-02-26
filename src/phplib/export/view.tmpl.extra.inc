<?php

/**
 * extras for view.tmpl.php : e.g. extra COLUMNS
 * @package view.tmpl.extra.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class view_tmpl_extras {
    
    /**
     * 
     * @var array
     * KEY = >array('Folder paths', 'Object linked in folders', GLOBAL_FUNC)
     */
    private $xcols;
    
    function __construct($tablename) {
        
        $this->tablename = $tablename;
        $this->access_id_has = cct_access_has2($tablename);
        $this->xcols = array();
        
        $this->_tab_cols_pref=array();
        if ($_SESSION['userGlob']['o.'.$tablename.'.viewcols']!=NULL) {
            $this->_tab_cols_pref = unserialize($_SESSION['userGlob']['o.'.$tablename.'.viewcols']);
        }
        
        if ($this->access_id_has) {
            
            require_once 'o.PROJ.paths.inc';
            
            // only for business objects
            $this->xcols = array(
                'proj_show'  => array('Folder paths', 'Object linked in folders', 1),
                'vario_show' => array('Vario values', 'Show all vario values', 1)
            );
  
        }
        
    }
    
//     function get_xcols_user_set() { 
//         if (!sizeof($this->xcols)) return array();
//         $xcols1=array();
//         foreach($this->xcols as $col_code=>$row) {
//             $viewcol_name = "_xcol." .$col_code;
//             if ( $this->_tab_cols_pref[$viewcol_name] > "0" ) {
//                 $col_text = $row[0];
//                 $xcols1[$col_code] = $col_text;
//             }
//         }
//         return $xcols1; 
//     }
    
    function get_xcol_def() {
        return $this->xcols;
    }
}
