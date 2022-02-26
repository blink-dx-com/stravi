<?php
/**
 * details of container
 * extension for    edit.tmpl.php
 * @package obj.container.xmode.details.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
 
 
require_once('gui/o.SATTACH.imgshow.inc');
require_once ('o.ABS_CONTAINER.subs.inc');

/**
 * details GUI
 */
class oCONTAINER_detGui {
	
	var $args; /**
		[s]
		[r]
		[b]
		[act] = 'boxDetail'
	*/
	
    function __construct(&$sqlo, $id, $args) {
    	global $error;
    	$FUNCNAME= 'oCONTAINER_detGui';
    	
    	$this->id = $id;
    	$this->args = $args;
    	
    	$sqlsel = '* from CONTAINER where CONTAINER_ID='.$this->id;
    	$sqlo->Quesel($sqlsel);
    	$sqlo->ReadArray();
    	$this->cfeat   = $sqlo->RowData;
    		
    	if (!$this->cfeat['ABS_CONTAINER_ID']) {
    		$error->set( $FUNCNAME, 1, 'no container (abstract) defined.' );
    		return;
    	}
    	
    	$this->absContainerLib = new oABS_CONTAINER_subs();
    	$this->absContainerLib->setContainer($sqlo, $this->cfeat['ABS_CONTAINER_ID']);
    	$this->absDimension = $this->absContainerLib->getDimensions($sqlo);
    	
    	if (!$this->absDimension['BP_x'])  {
    		$error->set( $FUNCNAME, 2, 'container (abstract) '.$this->cfeat['ABS_CONTAINER_ID'].
    			' has no defined Boxes.' );
    		return;
    	}
    	
    }
    
    function _colShow($text, $varname, $val) {
    	echo '&nbsp;&nbsp;'.$text.': <input type=text name="tabArgu['.$varname.']" value="'.$val.'" size=2>';
    }
    
    /**
     * show form to select a BOX 
     * @var array tabArgu[s], tabArgu[r], tabArgu[b] : shelf, rack, box
     * @return 
     */
    function showForm() {
    	require_once ('func_form.inc');
    	
    	$tablename='CONTAINER';
    	$initarr   = NULL;
    	$initarr["action"]      = $_SERVER['PHP_SELF'];
    	$initarr["title"]       = "Show box detail";
    	$initarr["submittitle"] = "Show";
    	$initarr["tabwidth"]    = "AUTO";
    
    	$hiddenarr = NULL;
    	$hiddenarr["tablename"]  = $tablename;
    	$hiddenarr["id"]         = $this->id;
    	$hiddenarr["tabArgu[act]"] = 'boxDetail';
    
    	$formobj = new formc($initarr, $hiddenarr, 0);
    	
    	$formobj->freeRow_open();
    	$showCols=array('s'=>'Shelf', 'r'=>'Rack', 'b'=>'Box');
    	foreach( $showCols as $key=>$nice) {
    		$val=$this->args[$key];
    		$this->_colShow($nice, $key, $val);
    	}
    	reset ($showCols); 
    	
    	echo "&nbsp;&nbsp;<input type=submit value=\"".$initarr["submittitle"]."\">\n"; 
    	
    	$formobj->freeRow_close();
    	$closeOpt=array("noRow"=>1);
    	$formobj->close( TRUE, $closeOpt );
    	
    }
    
    /**
     * show a container box as HTML-table
     * 
     * - analyse x,y
     * - analyse substances
     * 
     *  1 2 3 4 5
     *  6 7 8 9 10
     *  11 12 13 14 15 
     *  ...
     * @return -
     * @param object $sqlo
     */
    function showBox(&$sqlo) {
    	global $error;
    	$FUNCNAME= 'showBox';
    	$args = $this->args;
    	
    	$pos_keyxs = array('s', 'r', 'b');
    	foreach($pos_keyxs as $key) {
    	    if (!$args[$key]) {
    	        $error->set( $FUNCNAME, 1, 'ID of shelf, rack or box not set.' );
    	        return;
    	    }
    	    if (!is_numeric($args[$key])) {
    	        $error->set( $FUNCNAME, 2, 'ID of shelf, rack or box must be a number.' );
    	        return;
    	    }
    	}
    
    	
    	$extremPos = $this->absContainerLib->getBoxPosMinMax($sqlo, $args['s'], $args['r'], $args['b']);
    	if (!$extremPos['posFirst'] or !$extremPos['posLast']) {
    		$error->set( $FUNCNAME, 3, 'No position defined for S:'.$args['s'].' R:'.$args['s'].' B:'.$args['b'] );
    		return;
    	}
    	
    	/* analyse substances
    	 * max position per box: 1000 ?
    	 */
    	$posarr=NULL;
    	$sqlsel = 'POS, CONCRETE_SUBST_ID, ALIQUOT_NO from CONT_HAS_CSUBST where CONTAINER_ID='.$this->id.
    			' and (POS>='.$extremPos['posFirst'].' and POS<='.$extremPos['posLast'].' )'.
    			' order by POS';
    	$sqlo->Quesel($sqlsel);
    	while ( $sqlo->ReadRow() ) {
    		$pos     = $sqlo->RowData[0];
    		$substid = $sqlo->RowData[1];
    		$aliquotID = $sqlo->RowData[2];
    		$posarr[$pos]=array($substid,$aliquotID);
    	}
    	
    	if ($posarr==NULL) {
    		echo 'no substance in box.';
    		return;
    	}
    	
    	// show HTML-table
    	$maxnum=array();
    	$maxnum[0] = $this->absDimension['BP_x'];
    	$maxnum[1] = $this->absDimension['BP_y'];
    	
    	echo '<style type="text/css">'."\n";
        echo '.xAliquot  { color: #606060; }'."\n";
    	echo 'td.xNewSubst { background-color: #E0E0FF; }'."\n";
        echo '</style>'."\n";
    	
        echo "<br>";
    	echo '<table cellpadding=1 cellspacing=1 style="font-size:0.8em;">'."\n";
    	
    	echo "<tr>";
    	echo "<td></td>";
    	
    	$xCnt= 1;
    	while ($xCnt<=$maxnum[0]) {	
    		echo "<td bgcolor=#D0D0D0>" .$xCnt. "</td>";
    		$xCnt++;
    	}
    	//$yCnt++;
    	echo "</tr>\n";
    	
    	$yCnt   = 1;
    	$lastID = 0;
    	$posBoxStart = $extremPos['posFirst'];
    	
    	while ($yCnt<=$maxnum[1]) {
    		$xCnt= 1;
    		echo "<tr>";
    		echo "<td bgcolor=#D0D0D0>" .$yCnt. "</td>";
    		while ($xCnt<=$maxnum[0]) {
    			
    			$pos = ($yCnt-1)*$maxnum[0] + $xCnt-1 + $posBoxStart;
    			
    			$substName   =NULL;
    			$aliquotOut  =NULL;
    			$specialClass=NULL;
    			
    			if ($posarr[$pos][0]>0) {
    				
    				$substID = $posarr[$pos][0];
    				$aliid   = $posarr[$pos][1];
    				$substName = obj_nice_name ( $sqlo, 'CONCRETE_SUBST', $substID );
    				
    				// try to insert a "break line"
    				$stripPos = strpos($substName,'-');
    				if ($stripPos>0) $stripPos = strpos($substName,'-', $stripPos+1);
    				if ($stripPos>0) {
    					$substName = substr($substName,0,$stripPos+1).'<br />'.
    						substr($substName,$stripPos+1);
    				}
    				if ($aliid>0) {
    					$aliquotOut='<br /><span class="xAliquot">('.$aliid.')</span>';
    				}
    				if($substID!=$lastID) {
    					$specialClass=' class="xNewSubst"';
    				}
    				$lastID=$substID;
    			}
    			echo "<td ".$specialClass.">" .$substName . $aliquotOut. "</td>";
    
    			$xCnt++;
    		}
    		$yCnt++;
    		echo "</tr>\n";
    	}
    	echo "</table>\n";
    	
    }

}



class oCONTAINER_xmode_details {
	
    function __construct( &$editLib ) {
    }
    
    function _showGraphics(&$sqlo, $id, $ABS_CONTAINER_ID, $entryCnt) {
    	
    	echo "<table cellpadding=1 cellspacing=1 border=0 width=100%>\n<tr valign=top>";
    	echo "<td>";
    	
    	if ($entryCnt) {
    		echo '<font color=gray>Container-Content-Graphics:</font><ul>';
    		echo '<img src="obj.container.img.php?id='.$id.'">';
    	}	
    	echo "</td><td>";
    	
    	$attachLib = new oSATTACH_imgshow();
    	$found=0;
    	do {
    		
    		$relid = $attachLib->getThumbAttach( $sqlo, 'CONTAINER', $id);
    		if ($relid) {
    			$htmlTmp = $attachLib->getImgTag($relid, 500);
    			$found=1;
    			break;
    		}
    		if ($ABS_CONTAINER_ID) {
    			$relid = $attachLib->getThumbAttach( $sqlo, 'ABS_CONTAINER', $ABS_CONTAINER_ID);
    			if ($relid) {
    				$htmlTmp = $attachLib->getImgTag($relid, 500);
    				$found=1;
    				break;
    			}
    		}
    		
    	} while (0);
    	
    	if ($found) {
    		echo '<font color=gray>Thumbnail:</font><ul>'.$htmlTmp.'</ul>'."\n";
    	} else {
    		echo '<font color=gray>No Thumbnail attached.</font>'."<br />\n";
    	}
    	echo "</td></tr></table>\n";
    }
     
    // PLEASE use this function; still optional 
    function xmode_start( &$sqlo, &$sqlo2, $id, &$arguByKey, &$x_rights, &$extraobj_o, &$tabArgu, $editAllow) {
    	global $error;
    	
    	echo '<ul>'."\n";
    	
    	$ABS_CONTAINER_ID = $arguByKey["ABS_CONTAINER_ID"]; 
    	
    	$sqlsel = 'count(1) from CONT_HAS_CSUBST where CONTAINER_ID='.$id;
    	$sqlo->Quesel($sqlsel);
    	$sqlo->ReadRow();
    	$entryCnt = $sqlo->RowData[0];
    	
    	if ($entryCnt) {
    		$boxDetailLib = new oCONTAINER_detGui($sqlo, $id, $tabArgu);
    		$boxDetailLib->showForm();
    		if ($tabArgu['act']=='boxDetail') {
    			$boxDetailLib->showBox($sqlo);
    			if ($error->Got(READONLY))  {
    				 $error->printAllEasy(7);
    			}
    		}
    		echo '<br />'."\n";
    		
    		echo '</ul>'."\n";
    	}
    	
    	// show thumbnail of ABS_CONTAINER or CONTAINER
    	$this->_showGraphics($sqlo, $id, $ABS_CONTAINER_ID, $entryCnt);
    	
    	
    	if ( gGlobal::productIsIntern() ) echo '&nbsp;[<a href="f.help.php?f=o.CONTAINER.html&base=lab" target="_help">Clondiag help</a>]';
    	echo '</ul>'."\n";
    }

}

