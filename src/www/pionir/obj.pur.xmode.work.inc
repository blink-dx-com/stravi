<?php
require_once ('func_form.inc');
require_once ("f.objview.inc");
require_once 'role.inc';
require_once 'o.CCT_ACCLOG.subs.inc';

/**
 * show PUR
 * @package    edit.tmpl.php
 */
class oPUR_xmode_work {
  
    /**
     *
     * @return
     * @param object $sql
     * @param object $sql2
     * @param object $id
     * @param object $arguByKey
     * @param object $x_rights
     * @param object $extraobj_o
     * @param object $tabArgu
     * @param object $editAllow
     * @global array $_REQUEST['parx']
     * @global int $_REQUEST['go']
     */
    function xmode_start( &$sqlo, &$sqlo2, $id, &$arguByKey, &$x_rights, &$extraobj_o, &$tabArgu, $editAllow, $editmode) {
        global $error;
        
        $this->id = $id;

        echo "<ul>\n";

        echo "<br>\n";
        
    }
}