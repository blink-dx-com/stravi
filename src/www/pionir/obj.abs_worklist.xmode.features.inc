<?php
/**
 * abstract worklist extension 
 * used by edit.tmpl.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq UREQ:0000932 db.WORKLIST: Messauftrag organisieren
 * @version $Header: trunk/src/www/pionir/obj.abs_worklist.xedit.php 59 2018-11-21 09:04:09Z $
 */
require_once ("visufuncs.inc");
require_once ('func_form.inc');
require_once ('o.ABS_WORKLIST.subs.inc');

class oABS_WORK_gui {
	
	private $absWorkLib;
	
	function __construct($id) {
		$this->id=$id;
		
		$this->absWorkLib = new oABS_WORKLIST_subs($this->id);
		
		$tables=array(
			'CHIP_READER',
			'CONCRETE_SUBST',
			'CYTCART',
			'EXP',
			'SOCKET'
		);
		$this->types=array(
			'OBJ'  =>'OBJ',
			'NOTES'=>'NOTES',
			'VIEW' =>'VIEW (data view)'
		);
		
		$this->tablesNice=NULL;
		reset ($tables);
		foreach( $tables as $dummy=>$table) {
			$this->tablesNice[$table] = tablename_nice2($table);
		}
		reset ($tables); 
		
		
	}
	
	function oneEditRow($pos,$oneRow) {
		
		$tableSelect =  formc::selectFget( 'cell['.$pos.'][TABLE_NAME]', $this->tablesNice,
			$oneRow['TABLE_NAME'] ); 
		$typeSelect =	formc::selectFget( 'cell['.$pos.'][TYPE]', $this->types, $oneRow['TYPE']);
		$dataArr = array( 
			'<a href="edit.tmpl.php?t=ABS_WORKLIST_ENTRY&id='.$this->id.'&primasid[1]='.$oneRow['XPOS'].'">'.$oneRow['XPOS'].' =&gt;</a>',
			 $tableSelect, 
			 $typeSelect, 
			 '<input type=text name="cell['.$pos.'][KEY]" value="'.$oneRow['KEY'].'" size=6>',
			 '<textarea name="cell['.$pos.'][OPTIONS]" rows="3" cols="35">'.$oneRow['OPTIONS'].'</textarea>',
			 '<input type=text name="cell['.$pos.'][SH_NOTES]" value="'.$oneRow['SH_NOTES'].'" size=14>' 
			);
	   	return $dataArr;
	}
	
	function pluginAnalysis(&$sqlo) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;

			
		// check for Plugin-Class
		$wl_plugin_name = $this->absWorkLib->get_wl_plugin($sqlo);
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 1, 'Error on WL-Plugin-Analysis for this WLA.' );
			$error->printAll();
			$error->reset();
			return;
		}
		if ($wl_plugin_name==NULL) {
			echo '<span class="yGgray">No Plugin configured.</span>';
			return;
		}
		
		echo 'Plugin exists. Class-Name: "'.$wl_plugin_name.'"';
	}
}


class obj_edit_ext extends gObj_edit_feat_ABS {
    
    function rightWindow( &$sqlo, &$sqlo2) {
        $id = $this->id;
        $viewmode = $this->viewmode;
       
    	echo '<ul>'."\n";
    	$mainLib = new oABS_WORK_gui($id);
    	
    	echo '<span class="yGgrayBold">WL-Plugin-Analysis:</span> ';
    	$mainLib->pluginAnalysis($sqlo);
    	echo '<br>'."\n";
    	
    	echo '<span class="yGgrayBold">WL-Columns:</span> ';
    	if ($viewmode=='edit') {
    		echo '<form style="display:inline;" method="post" '.
    			 ' name="editform"  action="p.php?mod=DEF/o.ABS_WORKLIST.guimod" >'."\n";
    		echo '<input type=hidden name="act" value="update">'."\n";
    		echo '<input type=hidden name="id" value="'.$id.'">'."\n";
    	}
    	$tabobj  = new visufuncs();
    	$headOpt = array( ); // "title" => "column definitions"
    	$headx   = array ("Col", "table", "type", "key", "options", "Notes", "SQL");
    	$tabobj->table_head($headx,   $headOpt);
    	
    	$sqlsel = '* from ABS_WORKLIST_ENTRY where ABS_WORKLIST_ID='.$id.' order by XPOS';
    	$sqlo->Quesel($sqlsel);
    	while ( $sqlo->ReadArray() ) {
    		$oneRow = $sqlo->RowData;
    		$table  = $oneRow['TABLE_NAME'];
    		$html_table=NULL;
    		if ($table!=NULL) {
    			$html_table = tablename_nice2($table);
    		}
    		
    	    if ($viewmode=='edit') {
    	    	$dataArr = $mainLib->oneEditRow($oneRow['XPOS'],$oneRow);
    	    } else {
    	    	$tmplink='<a href="edit.tmpl.php?t=ABS_WORKLIST_ENTRY&id='.$id.'&primasid[1]='.$oneRow['XPOS'].'">'.$oneRow['XPOS'].' =&gt;</a>';
    	    	$sh_SQL=NULL;
    	    	if (strlen($oneRow['AUTO_COLLECT'])>0) {
    	    		$tmparr = explode(':',$oneRow['AUTO_COLLECT']);
    	    		$sh_SQL = $tmparr[0];
    	    	}
    	    	$dataArr = array( 
    	    		$tmplink, $html_table, $oneRow['TYPE'], $oneRow['KEY'], htmlspecialchars($oneRow['OPTIONS']), $oneRow['SH_NOTES'], $sh_SQL
    	    		);
    	    }
    	    
    	    $tabobj->table_row ($dataArr);
    	}
    	$tabobj->table_close();
    	if ($viewmode=='edit') {
    		echo '<input type=submit value="Update">'."\n";
    		echo '</form>'."\n";
    		echo '&nbsp;[<a href="p.php?mod=DEF/o.ABS_WORKLIST.guimod&act=new&id='.$id.'">new entry</a>]';
    	}
    	
    }

}