<?php
require_once ('func_form.inc');

/**
 * for edit.tmpl.php => the feature TAB abstract class
 * @package f.FormEditTab.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
abstract class FormEditTab {
    
    protected $editAllow;
    protected $tablename;
    protected $id;
    protected $editFormLib;
    
    function __construct($table, $id) {
        
        $this->table = $table;
        $this->id = $id;
        $this->editFormLib = new fFormEditC();
        $this->editFormLib->setObject($table, $id);
    }
    
    function openTable($hasAdvCols) {
        $headopt=array( "colModeSplit"=>1, 'HasAdvCols'=>$hasAdvCols );
        $this->editFormLib->formHead($headopt);
    }
    
    
    function _oneRow($key, $text, $notes=NULL) {
        $colInfos=NULL;
        $this->editFormLib->oneRow($key, $text, $notes, $colInfos);
    }
    
    function close() {
        
        if ($this->editAllow) {
            if ($this->cntRows) {
                // if attributes exist
                $this->editFormLib->formSubmitClose('update',NULL);
            } else {
                $this->editFormLib->formTableClose();
            }
            echo "</form>\n";
        } else {
            $this->editFormLib->formTableClose();
        }
        
    }
    
    function table_close() {
       $this->editFormLib->formTableClose();
    }
}