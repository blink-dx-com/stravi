<?php 
/**

 * show project categories as tree
 * @package o.PROJ.category_gui.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class oPROJ_category_gui {
    
    function __construct($sqlo) {
        $proj_id_ORI = 206;
        $wiidLib = new fWiidRoidC();
        $this->proj_id = $wiidLib->getObjID ($sqlo, 'PROJ', 39, $proj_id_ORI);
        
        $this->icon = "images/icon.PROJ.cat.gif";
    }
    
    private function _get_cnt_suas($sqlo, $proj_id) {
        $cnt = cProjSubs::getTableCnt( $sqlo, $proj_id, 'ABSTRACT_SUBST' );
        return $cnt;
    }
    
    /**
     * descr: prints the pure project tree to the screen
     warning! recursive function
     input: proj_id   ... proy id of the subtree to print
     proj_name ... name of the top-proj of the sub-tree
     proj_arr  ... array containing whole project tree
     
     * @param object $sql
     * @param object $sql2
     * @param int $proj_id
     * @param string $proj_name
     * @param array $proj_arr
     */
    private function print_tree_pure(&$sqlo,  $proj_id, $proj_name,  &$proj_arr  ) {
        
        $cnt_sua = $this->_get_cnt_suas($sqlo, $proj_id);
        
        echo '<a href="edit.tmpl.php?t=PROJ&id='.$proj_id.'"><img src="'.$this->icon.'" border="0"> '.$proj_name.'</a> '; // <li>
        
        if ($cnt_sua) echo '('.$cnt_sua.')';
        echo "\n";
        
        
        $this->projcnt++;
        
        // if (empty($proj_arr[$proj_id])) return;
        
        echo '<ul class=xul>'."\n";
        
        foreach( $proj_arr[$proj_id] as $proj_id_sub=>$projname) { // show subprojects
            $this->print_tree_pure($sqlo, $proj_id_sub, $projname, $proj_arr);
            $this->projcnt++;
        }
        
        echo '</ul>'."\n"; // </li>
    }
    
    function show($sqlo, $sqlo2) {
        
        $proj_id = $this->proj_id;
        if (!$proj_id) {
            echo 'No ROOT-project given. Stop.<br>';
            return;
        }
        
        $projname = obj_nice_name ( $sqlo, 'PROJ', $proj_id );
        
        $proj_arr  = &oPROJ_tree::tree2array($sqlo, $proj_id);
        
        echo '<ul class=xul>';
        
        $this->print_tree_pure($sqlo, $proj_id, $projname, $proj_arr);
        
        echo '</ul>';
    }
    
}
