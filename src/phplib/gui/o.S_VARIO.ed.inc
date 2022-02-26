<?php
require_once ('o.S_VARIO.subs.inc');
require_once 'gui/f.FormEditTab.inc';

/**
 * single object S_VARIO tag editor
 * @package glob.obj.S_VARIO.php
 * @subpackage glob.obj.S_VARIO.ed.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class globObj_S_VARIO_ed extends FormEditTab {
	
	
function __construct($table, $id) {
    
    parent::__construct($table, $id);
	$this->VarioLib = new oS_VARIO_sub();

}

/**
 * init form
 * @param int $editAllow
 * @param string $url
 * @param array $hiddenarr
 * @return -
 */
function initForm($editAllow, $url, $hiddenarr) {
	
	$table= $this->table;
	$id   = $this->id;
	$this->editAllow=$editAllow;
	
	$this->VarioLib->setTable( $table );
	
	$hiddenarr['t'] =$table;
	$hiddenarr['id']=$id;
	
	if ($this->editAllow) {
		echo '<form style="display:inline;" method="post" '.
		' name="editform"  action="'.$url.'" >'."\n";
	
		if (sizeof($hiddenarr)) { 
	        foreach( $hiddenarr as $idx=>$valx) {  
	               echo "<input type=hidden name=\"". $idx ."\" value=\"".$valx."\">\n";
	        }
	        
	        echo "\n";
	     }
	}
	
	
}



/**
 * show possible new fields
 *  - which are not already set
 *  - which are allowed to edit ($colFeatures['EDITABLE'])
 * @param object $sqlo
 * @param array $notShowKeys
 * @return int number of keys
 */
function _showNewFields(&$sqlo, $notShowKeys) {
	$keyarr  = $this->VarioLib->getAllKeysNice( $sqlo );
    
    if (empty($keyarr)) {
    	$this->_oneRow('---','no defined keys!');
		return;
    } else {
    	
    	// post prep $keyarr, 
    	// show only keys, which were NOT in $valArr
    	if (!empty($notShowKeys)) {
	    	foreach( $notShowKeys as $dummy=>$key) {
	    		unset($keyarr[$key]);
			}
			
    	}
    	
    	if (!empty($keyarr) and !glob_isAdmin() ) {
    		// unset keys, where EDIT not allowed for a normal user
	    	foreach( $keyarr as $dummy=>$key) {
	    		$colFeatures    = $this->VarioLib->getColFetaures( $sqlo, $key );
	    		$vario_EDITABLE = $colFeatures['EDITABLE'];
	    		if ($vario_EDITABLE<1) unset($keyarr[$key]);
			}
			
    	}
	
    	if (!empty($keyarr)>0) {
	    	$val='';
	    	$keysel  = formc::selectFget( 'parx[newkey]', $keyarr, $val); 	
			$seltext  = '<input type=text  name="parx[newval]" value="" >';
			$this->_oneRow($keysel, $seltext);	
    	}
    	
    }
    return sizeof($keyarr);
}

/**
 * get all data of S_VARIO for object
 */
function getData(&$sqlo, $objid) {
	
	$sqlo->Quesel("KEY, VALUE FROM S_VARIO ".
			"WHERE TABLE_NAME='".$this->table."' AND OBJ_ID=".$objid.
			" order by KEY");
	$valArr=NULL;
	while ($sqlo->ReadRow() ) {
		$key   = $sqlo->RowData[0];
		$value = $sqlo->RowData[1];
		$valArr[$key]=$value;
	}
	return $valArr;
}

/**
 * - show a form of VARIO fields
 * - variables: 
 *   keyval[key] =val, 
 *   pmult[key]=vals 'multiadd' selector
 *   parx[newkey] - new key
 *   parx[newval] - new val
 * @return 
 * @param object $sqlo
 * @param array $valArr  array(key) of values
 * @param array $prefArr [key] = selection array
 * @param array $options[optional] array[key]= array(
 * 		 'type'   => ('select', 'text')
 * 		 'multiadd'=>(0,1)
 * 		 'selsrc' => ('variopref', 'docid') where comes the select from ?
 * 					
 */
function showFields( &$sqlo, $valArr, $prefArr, $options=NULL ) {
	
    $cntRows = 0;
    $this->cntRows = 0;
    
    if (!empty($valArr)) {
	    foreach( $valArr as $key=>$val) {
	    	
	    	$optCol = $options[$key];
	    	$colFeatures = $this->VarioLib->getColFetaures( $sqlo, $key );
	    	$nice        = $colFeatures['NICE'];
	    	$columnNotes = $colFeatures['NOTES'];
	    	//$columnNotes = '<a href="f.help.php?f='.$key.'&type=vario&t='.
	    	//	$this->table.'"><img src="images/help.but.gif"></a> '.$columnNotes;
			
	    	$varname  = 'keyval['.$key.']';
	    	$vario_EDITABLE = $colFeatures['EDITABLE'];
	    	if (glob_isAdmin()) $vario_EDITABLE = 1;
	    	if ($colFeatures['EDITABLE']<1) $columnNotes .= ' (editable by Admin)';
	    	
	    	$allowColumnEdit = 0;
	    	if ($this->editAllow and $vario_EDITABLE>0) $allowColumnEdit=1;
	    	
	    	if ( $allowColumnEdit ) {
		    	if ($optCol['type']=='select') {
					$usefield = &$prefArr[$key];
			    	$seltext  = formc::selectFget( $varname, $usefield, $val); 
				}
				
				//if ($optCol['type']=='text') {
				$seltext  = '<input type=text  name="'.$varname.'" value="'.htmlspecialchars($val).'" >'; //NEW:
			    //}
	    	} else {
	    		 $seltext  = htmlspecialchars($val);
	    	}
			
	    	$this->_oneRow($nice, $seltext, $columnNotes);
	    	$cntRows++;
		}
		reset ($valArr); 
    }
    
    if ( $this->editAllow ) {
    	$notShowKeys = NULL;
    	if (!empty($valArr)) $notShowKeys = array_keys($valArr);
    	$numNewFields = $this->_showNewFields($sqlo, $notShowKeys);
    	$cntRows = $cntRows + $numNewFields;
    } else {
    	$keyarr  = $this->VarioLib->getAllKeysNice( $sqlo );
    	if (empty($keyarr)) {
    		$this->_oneRow('---','no defined keys!');
    	} else {
    		$cntRows = $cntRows + sizeof($keyarr);
    	}
    }
    
    $this->cntRows = $cntRows;
}



}
