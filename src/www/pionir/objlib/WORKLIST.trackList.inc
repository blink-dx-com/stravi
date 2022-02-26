<?php
require_once 'o.ABS_WORKLIST.subs.inc';

/**
 * object tracking for WORKLIST
 * @package WORKLIST.trackList.inc
 * @swreq   UREQ:0001239: g > object tracking > show usage objects in worklist 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/www/pionir/objtools/WORKLIST.trackList.inc 59 2018-11-21 09:04:09Z $
 */
class oWORKLIST_listTrack {
	var $sqlCondition;  // SQL-condition after WHERE
	var $attrList=NULL; // attribute list of ABS_WORKLIST
	
	function __construct($sqlCondition, $parx) {
		$this->sqlCondition=$sqlCondition;
		$this->parx = $parx;
	}
	
	
	
	/**
	 * analyse a list of worklists
	 * @swreq UREQ:0001239:002 Worklists 
	 * @param unknown $sqlo
	 * @return void|NULL|unknown
	 */
	function _anaOne(&$sqlo) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
		
		$sqlsel = 'count(distinct(x.ABS_WORKLIST_ID)) from WORKLIST x where '.$this->sqlCondition;
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$cntAbsWorklists = $sqlo->RowData[0];
		if ($cntAbsWorklists>1) {
			// @swreq UREQ:0001239:002.a wenn unterschdl. Typen: zeige Warnung
			echo "<br>\n";
			htmlInfoBox( "Different Worklist types", "Warning: the worklists have different types.", "", "WARN" );
			echo "<br>\n";
		}
		
		if (!$cntAbsWorklists) {
			$error->set( $FUNCNAME, 1, 'track sub-objects: no worklist (abstract) on worklists found.' );
			return;
		}
		
		// get one entry
		$sqlsel = 'x.ABS_WORKLIST_ID from WORKLIST x where '.$this->sqlCondition;
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$abs_objid = $sqlo->RowData[0];
		
		
		
		
		if (!$abs_objid) {
			$error->set( $FUNCNAME, 1, 'no abstract worklist defined.' );
			return;
		}
		
		$absWorkLib = new oABS_WORKLIST_subs($abs_objid);
		
		// array[POS] = array('t'=>$table, 'title'=>$title, 'typ'=>$TYPE, 'note'=>$NOTES );
		$attrList = $absWorkLib->getNameAttribs($sqlo);
		if (!is_array($attrList))  {
			$error->set( $FUNCNAME, 2, 'no columns defined in worklist.' );
			return;
		}
		$this->attrList=$attrList;
		
		reset ($attrList);
		foreach( $attrList as $xpos=>$valarr) {
			if ($valarr['typ']=='OBJ') {
				$showArr[$xpos] = $valarr['title'];
			}
		}
		reset ($attrList); 
	
		return $showArr;
	}
	
	/**
	 * - analyse workflow columns
	 * @param $sqlo
	 */
	function showForm(&$sqlo) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		$tablename='WORKLIST';
		
		$retArr = $this->_anaOne($sqlo);
		if ($error->Got(READONLY))  {
			return;
		}
		
		if (!is_array($retArr)) return;
		
		echo '<form style=\"display:inline;\" method="post" name="form3" action="'.$_SERVER['PHP_SELF'] .'">'."\n";
		echo '<input type="hidden" name="actionx"   value="forwNorm">'."\n";
		echo '<input type="hidden" name="parx[destTab]"   value="WORKLIST_ENTRY">'."\n";
		echo '<input type="hidden" name="parx[special]"   value="1">'."\n";
		echo '<input type="hidden" name="tablename" value="'.$tablename.'">'."\n";
		echo 'Track objects in worklist entries: ';
		$seltext = formc::selectFget('parx[xcol]', $retArr, ''); 
		echo $seltext;
		echo " <input type=submit value=\"Select\">\n"; // SUBMIT
		echo '</form>'."\n";
	}
	
	/**
	 * forward to the query page
	 * @param $sqlo
	 */
	function goSearch(&$sqlo) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$retArr = $this->_anaOne($sqlo);
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 1, '' );
			return;
		}
		
		$theColx = $this->parx['xcol'];
		if (!$theColx)  {
			$error->set( $FUNCNAME, 1, 'no column selected' );
			return;
		}
		
		$attribs   = $this->attrList[$theColx];
		$destTable = $attribs['t'];
		$destTable_colName = PrimNameGet2($destTable);
		
		if ($destTable==NULL) {
			$error->set( $FUNCNAME, 2, 'column def contains no table.' );
			return;
		}
		
		$sqlCondition=$this->sqlCondition;
		
		
		// @swreq UREQ:0001239:003 filtere einfach alle entries vom selben Tabellentyp
		$newCond = 'x.'.$destTable_colName. ' in ('.
			'select x.OBJID from WORKLIST_ENTRY x where TABLE_NAME='.$sqlo->addQuotes($destTable).' and  '.$sqlCondition.
			')';
		
	
		echo '<form style=\"display:inline;\" method="post" name="form3" action="view.tmpl.php">'."\n";
		echo '<input type="hidden" name="condclean" value="1">'."\n";
		echo '<input type="hidden" name="t" value="'.$destTable.'">'."\n";
		echo '<input type="hidden" name="tableSCond" value="'.$newCond.'">'."\n";
		echo '<input type="submit" value="...">'; 
		echo '  ... auto forward. Please wait!'."<br>\n";
		echo '</form>'."\n";
		
		
		if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
		    echo "<B>INFO:</B> Stopped due to debug mode.<br>\n";
		    exit;
		}
		
		?>
		<script language="JavaScript">
			document.form3.submit();
		</script>
		<?php 
		return;
	}
}
