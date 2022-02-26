<?php
/**
 * attachment forms
 * @package o.SATTACH.ed.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
require_once ("visufuncs.inc");
require_once ("o.SATTACH.subs.inc");
require_once 'gui/f.FormEditTab.inc';

class o_SATTACH_ed_sub {
    
    function show_form($sqlo, $tablename, $id, $editallow) {
        // echo "[<a href=\"\"></a>] ";
        $attachtab = "SATTACH";
        
        echo "<ul>\n";
        
        if ( $editallow) {
            echo "[<a href=\"obj.sattach.edit.php?tx=$tablename&id=$id&action=insert\">Add attachment</a>] <br><br>\n";
        }
        else {
            // echo "&nbsp;<font color=gray>Check edit mode and your rights to add new attachments to this object.</font>";
        }
        
        
        
        $satObj = new cSattachSubs();
        
        $sqlo->Quesel("count(1) FROM ".$attachtab." WHERE TABLE_NAME='".$tablename."' AND OBJ_ID=".$id);
        $sqlo->ReadRow();
        $numattach = $sqlo->RowData[0];
        
        if (!$numattach) {
            echo '<span class="yGgray">No attachments.</span>';
            return;
        }
        
        $tabobj = new visufuncs();
        $headOpt = array( "borderColor"=>"#E0E0E0" );
        $headx   = array("", "", "ID", "Name", "Key", "Date", "Prot", "Size [bytes]");
        
        echo "<form method=\"post\"  name=\"editform\"  action=\"obj.sattach.del.php\" >\n";
        echo "<input type=hidden name='t' value='".$tablename."'>\n";
        echo "<input type=hidden name='id' value='".$id."'>\n";
        echo "<input type=hidden name='action' value='delete'>\n";
        
        $tabobj->table_head($headx,   $headOpt);
        
        // MIME_TYPE
        $sqlo->Quesel("* FROM ".$attachtab." WHERE TABLE_NAME='".$tablename."' AND OBJ_ID=".$id. " ORDER BY REL_ID");
        while ($sqlo->ReadArray() ) {
            
            $arch_html= NULL;
            $relid = $sqlo->RowData['REL_ID'];
            $tname = $sqlo->RowData['NAME'];
            $aKey  = $sqlo->RowData['KEY'];
            
            // new keys
            $xdate = substr($sqlo->RowData['XDATE'],0,10);
            $archFlag  = $sqlo->RowData['ARCHIVE'];
            
            $filesizex  = "";
            $docfile    = $satObj->getDocumentPath($tablename, $id, $relid);
            $tmpAttName = htmlspecialchars($tname);
            if (file_exists( $docfile )) {
                $filesizex = filesize( $docfile );
                $tmpAttName = "<a href=\"obj.sattach.down.php?t=$tablename&id=$id&rel_id=$relid\">".htmlspecialchars($tname)."</a>";
            }
            
            $loopCheckBox="<input type=checkbox name=\"sel[$relid]\" value=1>";
            if ($archFlag>0) {
                $loopCheckBox=''; // not allowed
                $arch_html='<img src="images/ic.lock.gif" title="protected">';
            }
            
            //$tsize = "";
            $dataArr = array(
                "<a href=\"obj.sattach.edit.php?tx=$tablename&id=$id&rel_id=$relid\"><img src=\"images/arrow.but.gif\" border=0></a>",
                $loopCheckBox,
                $relid,
                "&nbsp;<b>".$tmpAttName."</b>&nbsp;",
                htmlSpecialChars($aKey),
                $xdate,
                $arch_html,
                $filesizex
            );
            $tabobj->table_row ($dataArr);
        }
        
        
        $tabobj->table_close();
        echo "<br>";
        if ($editallow) echo "<input type=submit value=\"Delete selected\">\n";
        
        echo "</form>\n";
    }
}

/**
 * single object SATTACH editor
 */
class o_SATTACH_ed extends FormEditTab {

    function __construct($table, $id) {
        parent::__construct($table, $id);
    }
    
    function show_form($sqlo, $tablename, $id, $editallow) {
        $attach_lib = new o_SATTACH_ed_sub();
        $attach_lib->show_form($sqlo, $tablename, $id, $editallow);
    }


}
