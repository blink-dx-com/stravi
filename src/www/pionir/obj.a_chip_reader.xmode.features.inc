<?php
/**
 * extension for single object view
 * @package obj.a_chip_reader.xedit.php
 * @swreq UREQ:0001493: o.A_CHIP_READER > extension obj.a_chip_reader.xedit.php 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param   string INPUT 
 */

require_once ("visufuncs.inc");
require_once ("f.objview.inc");	
require_once ('func_form.inc');
require_once 'gui/f.mothChildTree.inc';

class AbsDevHelp {
	function __construct($id, $viewmode, $o_rights, $optmod) {
		$this->id=$id;
		$this->viewmode=$viewmode;
		$this->optmod = $optmod;
		$this->tabArgu = $_REQUEST['tabArgu'];
		
		if ($this->optmod==NULL) $this->optmod='addService';
	}
	
	function showHead($text) {
	    echo "<b>".$text."</b> \n";
	}
	
	
	function showAddSic(&$sqlo) {
		if (!glob_table_exists('ADEV_HAS_SIC')) return;
		
		$adevid = $this->id;
		if ($this->viewmode!='edit') return;
		if ($this->optmod!='addService') return;
			
		//echo '<b>Service plans</b> &nbsp;&nbsp;&nbsp;[<a href="edit.insert.php?tablename=ADEV_HAS_SIC&argu_xtra[A_CHIP_READER_ID]='.$adevid.'">'.
		//		'Add Service plan</a>]'."<br />\n";
		
		$tablename='ADEV_HAS_SIC';
		
		$initarr   = NULL;
		$initarr["action"]      = 'edit.insert.php';
		$initarr["title"]       = "Add Service plan";
		$initarr["submittitle"] = "Add";
		$initarr["tabwidth"]    = "400";
		$initarr["tabnowrap"]   = 1;
		$initarr["dblink"]      = 1;
	
		$hiddenarr = NULL;
		$hiddenarr["tablename"]     = $tablename;
		$hiddenarr["argu[A_CHIP_READER_ID]"]  = $adevid;
		$hiddenarr["go"]  = 1;
		$hiddenarr["options[backurl]"]  = urlencode('edit.tmpl.php?t=A_CHIP_READER&id='.$adevid);
	
		$formobj = new formc($initarr, $hiddenarr, 0);
	
		$fieldx = array ( 
			"title" => "Service plan", 
			"name"  => "argu[DEVSIC_ID]",
			"object"=> "dblink",
			"val"   => "", 
			"namex" => TRUE,
			"inits" => array( "table"=>"DEVSIC", "objname"=>'', "pos" =>"0", "projlink"=> 1),
			"notes" => ""
			 );
		$formobj->fieldOut( $fieldx );
	
		$formobj->close( TRUE );
		
		echo "<br>\n";
	}
	
	function _showServPlan(&$sqlo) {
		
		
		if (!glob_table_exists('ADEV_HAS_SIC')) return;
		
		$adevid = $this->id;
		
		
		$planids=NULL;
		$sqlsel = "DEVSIC_ID from ADEV_HAS_SIC where A_CHIP_READER_ID=".$adevid. ' order by DEVSIC_ID';
		$sqlo->Quesel($sqlsel);
		while ( $sqlo->ReadRow() ) {
		    $tmpid = $sqlo->RowData[0];
		    $planids[]=$tmpid;
		}
		
		if (!is_array($planids)) return;
		
		$tabobj = new visufuncs();
		$headOpt = array( "title" => "Service plan list");
		$headx   = array ("Plan", "Type", "Period");
		$tabobj->table_head($headx,   $headOpt);
		
		$objLinkLib = new fObjViewC();
		foreach( $planids as $dummy=>$devsicid) {
			$sqlsel = 'NAME, TYPE, PERIOD from DEVSIC where DEVSIC_ID='.$devsicid;
			$sqlo->Quesel($sqlsel);
			$sqlo->ReadArray();
			$featureLoop = $sqlo->RowData;
			
			$linkTmp= $objLinkLib->bo_display( $sqlo, 'DEVSIC', $devsicid );
			
			$dataArr = array( $linkTmp, $featureLoop['TYPE'], $featureLoop['PERIOD']);
			
			$tabobj->table_row ($dataArr);
		}
		$tabobj->table_close();
		
	}
	
	function _getNumDev(&$sqlo,$aDevID) {
		$sqlsel = 'count(1) from CHIP_READER where A_CHIP_READER_ID='.$aDevID;
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$num = $sqlo->RowData[0];
		return $num;
	}
	
	/**
	 * show mother, parent
	 * @param $sqlo
	 */
	function showMotherChild(&$sqlo, &$sqlo2) {
		

		
	    $this->showHead( "Device-Hierarchy");
	    
		if ($this->viewmode=='edit') {
			
			if ($this->optmod=='addmother') {
				echo "<br><br>\n";
				$this->_addMotherForm($sqlo);

			} else {
				$baseurl='edit.tmpl.php?t=A_CHIP_READER&id='.$this->id;
				echo '&nbsp;&nbsp;&nbsp;[<a href="'.$baseurl.'&tabargu[optmod]=addmother">Add mother device (abstract)</a>]';
			}
		}
		
		
		$tablename = 'A_CHIP_READER';
		//$useTab    = "S_OBJLINK";
		$id = $this->id;
		$objLinkLib = new fObjViewC();
		
		$children=NULL;
		$mothers =NULL;
		
		// mothers
		$sqlsel = "MO_ID, KEY from S_OBJLINK where ".
			' CH_ID='.$id. ' and MO_TABLE= '.$sqlo->addquotes($tablename).' and CH_TABLE='.$sqlo->addquotes($tablename).' order by POS';
		$sqlo->Quesel($sqlsel);
		$cnt=0;
		while ($sqlo->ReadRow()) {
			$textLoop = "";
			$moID    = $sqlo->RowData[0];
			$key     = $sqlo->RowData[1];
			$mothers[$moID]=$textLoop;
			$cnt++;
		}
		
		//children
		$sqlsel = "CH_ID, KEY from S_OBJLINK where ".
			' MO_ID='.$id. ' and MO_TABLE= '.$sqlo->addquotes($tablename).' and CH_TABLE='.$sqlo->addquotes($tablename).' order by POS';
		$sqlo->Quesel($sqlsel);
		$cnt=0;
		while ($sqlo->ReadRow()) {
			$textLoop  = NULL;
			$childID    = $sqlo->RowData[0];
			$key        = $sqlo->RowData[1];
			if ($childID) {
			    $textLoop = '('. $this->_getNumDev($sqlo2,$childID) .')';
			}
			$children[$childID]=$textLoop;
			$cnt++;
		}
		
		if (empty($mothers) and empty($children)) {
		    echo ' <span class="yGgray">None</span> <br>'."\n";
		    return;
		}
		
		echo "<br /><br />\n";
		
		$helpLib = new gMotherChildGui();
		$shOpt=array('table'=>'A_CHIP_READER');
		
		$textLoop=NULL;
		// count only numbers, if this object has parents !!!
		// if this has children, it should not have objects
		if (empty($children)) $textLoop = '('. $this->_getNumDev($sqlo2,$id) . ')';
		$refarr = array('id'=>$id, 'txt'=>$textLoop);
		$helpLib->showMotherChild($sqlo, $mothers, $children, $refarr, $shOpt);
		
	}
	
	function _addMotherForm() {
		require_once ('func_form.inc');
		
		$tablename='A_CHIP_READER';
		
		$initarr   = NULL;
		$initarr["action"]      = 'p.php?mod=DEF/o.S_OBJLINK.addMother';
		$initarr["title"]       = "Neue Verbindung Mutter-Kind";
		$initarr["submittitle"] = "Verbinden";
		$initarr["tabwidth"]    = "AUTO";
		$initarr["dblink"]      = 1;
		$initarr["goNext"]      = 2;
	
		$hiddenarr = NULL;
		$hiddenarr["t"]     = $tablename;
		$hiddenarr["id"]    = $this->id;
		$hiddenarr["parx[childkey]"] = 'devHierarchy';
		$hiddenarr["parx[action]"]   = 'add';
		$hiddenarr["backurl"]   = urlencode('edit.tmpl.php?t='.$tablename.'&id='.$this->id);
	
		$formobj = new formc($initarr, $hiddenarr, 1);
	
		$fieldx = array ( 
			"title" => "Mutter-Ger&auml;teklasse", 
			"name"  => "moid",
			"object"=> "dblink",
			"val"   => 0, 
			"notes" => 'du musst "insert"-Recht auf Kind-Ger&auml;t haben',
			"inits" => array( 'table'=>$tablename, 'objname'=>'', 'pos' =>'0', 'projlink'=> 1)
			 );
		$formobj->fieldOut( $fieldx );
	
		$closeOpt = array("noBackButton"=>1);
		$formobj->close( TRUE, $closeOpt );
	}
	
	function show_conc_devs($sqlo) {
	    
	    $tablename='CHIP_READER';
	    $nice_tab = tablename_nice2($tablename);
	    $this->showHead( "Info for derived ". tablename_nice2($nice_tab)  );
	    echo '<ul>'."\n";
	    
	    $conc_cnt = $this->_getNumDev($sqlo,$this->id);
	    
	 
	    $objLinkLib = new fObjViewC();
	    $icon  = $objLinkLib->_getIcon($tablename);
	    
	    echo ''.$nice_tab.': <a href="view.tmpl.php?t=CHIP_READER&condclean=1&searchCol=A_CHIP_READER_ID&searchtxt='. $this->id .
	    '"><img src="'.$icon.'"> '.$conc_cnt.' objects</a><br>'."\n";
	    echo '</ul>'."\n";
	}


}

class obj_edit_ext extends gObj_edit_feat_ABS { 
    function rightWindow( &$sqlo, &$sqlo2) {
        $id = $this->id;
        
        echo "<ul>\n";
    	
    	$tablename ='A_CHIP_READER';
    	$optmod = $_REQUEST['tabargu']['optmod'];
    	
    	$helpLib = new AbsDevHelp($id, $this->viewmode, $this->o_rights, $optmod);
    	echo '<table><tr valign=top><td>'."\n";
    	$helpLib->showAddSic($sqlo);
    	$helpLib->_showServPlan($sqlo);
    	
    	
    	// show thumbnail
    	require_once('gui/o.SATTACH.imgshow.inc');
    	$attachLib = new oSATTACH_imgshow();
    	$relid = $attachLib->getThumbAttach( $sqlo, $tablename, $id);
    	if ($relid) {
    		$htmlTmp = $attachLib->getImgTag($relid);
    		echo '<span style="color:gray;">Thumbnail:</span><ul>'.$htmlTmp.'</ul>'."\n";
    	}
    	
    	
    	echo '</td><td>&nbsp;&nbsp;&nbsp;&nbsp;</td><td>'."\n".'<td>';
    	$helpLib->showMotherChild($sqlo, $sqlo2);
    	echo "<br>\n";
    	$helpLib->show_conc_devs($sqlo);
    	echo '</td></tr></table>'."\n";
    	echo "</ul>\n";
    }


}

