<?php
require_once 'o.PROJ.paths.inc';

/**
 * func_form level2 : create config arrays for class formc()
 * @package f.func_form.lev2.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class formc_lev2 {
    
    /**
     * 
     * @param object $sqlo
     * @param int $proj_id
     * @param string $varname
     * @param int $form_pos
     * @param array $opts
     * @return array
     */
    static function get_PROJ_selector(object $sqlo, $proj_id, string $varname, int $form_pos, $opts=array() ) {
        if ($proj_id) {
            $projPathObj = new oPROJpathC();
            $proj_name   = $projPathObj->getPathText($sqlo, $proj_id, 0, 4  );
        }
        
        $fieldx = array (
            "title" => "Folder",
            "name"  => $varname,
            "object"=> "dblink",
            "namex" => TRUE,
            "val"   => $proj_id,
            "inits" => array(
                "table"  => "PROJ",
                "objname"=> $proj_name,
                "pos"    => $form_pos,
                "projlink"=> 1,
                'noshDel' =>1
            ),
            "notes" => ""
        );
        return $fieldx;
    }
}