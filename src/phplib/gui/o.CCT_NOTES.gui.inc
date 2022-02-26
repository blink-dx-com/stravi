<?php
require_once ('func_form.inc');
require_once 'o.DB_USER.subs.inc';

/**
 * GUI for CCT_NOTES
 * @author skube
 *
 * @example CCT_NOTES_GUI
 * 
 *  $notes_lib = new oCCT_NOTES_gui($sqlo, 'BCBATCH', $id);
			
	$parx=array();
	$backurl = 'edit.tmpl.php?t=BCBATCH&id='.$id;
	$form_opt = array('title'=>'Add Batch notes');
	$notes_lib->show_form($sqlo, $parx, $backurl, 'add', $form_opt);
	
	echo "<br><br>";
	echo '<h2>Batch Notes ...</h2>'."\n";
	
	echo '<style type="text/css">'."\n";
	echo $notes_lib->get_css_def();
	echo '</style>'."\n";
	
	$notes_lib->show_list($sqlo, $sqlo2);
		
 */
class oCCT_NOTES_gui {
    
    function __construct(&$sqlo, $tablename, $objid) {
        $this->tablename = $tablename;
        $this->objid = $objid;
        
        $pk_col = PrimNameGet2($tablename);
        // get cct_access
        $this->CCT_ACCESS_ID = glob_elementDataGet( $sqlo, $tablename, $pk_col, $objid, 'CCT_ACCESS_ID');
    }
    
    function get_css_def() {
        return '
        .xBlog h3 {padding-bottom:1px; margin-bottom:6px; }
        .xBlog h5 {margin-top:1px; margin-bottom:8px; font-weight:normal; color:#808080;}
        .xBlog hr {border-top: 1px dotted #C0C0C0; width: 500px; margin: 0px;}
          ';
    }
    
    /**
     * 
     * @param object $sqlo
     * @param array $parx
     * @param string $backurl
     * @param string $entry_action
     * @param array $options
     */
    function show_form(&$sqlo, $parx, $backurl, $entry_action, $options=array() ) {
        
        $title = "Add object notes";
        if ($options['title']) $title = $options['title'];
        
        $initarr   = NULL;
        $initarr["action"]      = 'p.php?mod=DEF/o.CCT_NOTES.mod';
        $initarr["title"]       = $title;
        $initarr["submittitle"] = "Submit";
        $initarr["tabwidth"]    = "500";
        
        $hiddenarr = NULL;
        $hiddenarr["t"] = $this->tablename;
        $hiddenarr["id"]     = $this->objid;
        $hiddenarr["action"]    = $entry_action;
        $hiddenarr["backurl"]   = urlencode($backurl);
        
        $formobj = new formc($initarr, $hiddenarr, 0);
        
        $fieldx = array (
            "title" => "Subject",
            "name"  => "NAME",
            "object"=> "text",
            "fsize" => 100,
            "val"   => $parx["NAME"],  
            "req"   => 1
        );
        $formobj->fieldOut( $fieldx );
        $fieldx = array (
            "title" => "Notes",
            "name"  => "NOTES",
            "object"=> "textarea",
            "val"   => $parx["NOTES"],
            "inits" => array("rows"=>5, "cols" => 80 ),
            "req"   => 1
        );
        $formobj->fieldOut( $fieldx );
        
        $formobj->close( TRUE );
    }
    
    private function _one_entry_out($argu) {
        
        $notes_out = nl2br(htmlspecialchars($argu['NOTES']));
        
        echo '
          <hr>
          <h3>'.$argu['NAME'].'</h3>
          <h5>'. $argu['user.name'].', '.$argu['MOD_DATE'].'</h5>
          <p>'. $notes_out .
          '</p>'."\n";
    }
    
    /**
     * show all entries, newest first
     * @param object $sqlo
     */
    function show_list(&$sqlo, &$sqlo2) {
        
        if (!$this->CCT_ACCESS_ID) {
            throw new Exception('Class not initialized.');
        }
        
        echo '<div class=xBlog>'."\n";
        
        $sqlsel = "* from CCT_NOTES where CCT_ACCESS_ID=".$this->CCT_ACCESS_ID . ' order by POS DESC';
        $sqlo2->Quesel($sqlsel);
        while ( $sqlo2->ReadArray() ) {
            $entry_feats = $sqlo2->RowData;
            
            $user_id = $sqlo2->RowData['DB_USER_ID'];
            $entry_feats['user.name'] = DB_userC::getFullname($sqlo, $user_id);
            $this->_one_entry_out($entry_feats);
        }
        
        echo '</div>'."\n";
        
    }
}