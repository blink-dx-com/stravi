<?php
/**
 * for ABSTRACT_SUBST
 * @author skube
 *
 */
class oABSTRACT_SUBST_xedForm  extends gui_xedForm_ABS {
    function x_cols_extra( &$sqlo, &$arguByKey ) {
        $assoc=array();
        $assoc['ABS_CHILD_SUBST'] = array('edit'=>'[<a href="p.php?mod=DEF/o.ABSTRACT_SUBST.metaAdd&id='.$_REQUEST['id'].'">Add</a>]' );
        $colopt = array('assoc'=> $assoc);
        return($colopt);
    }
}
