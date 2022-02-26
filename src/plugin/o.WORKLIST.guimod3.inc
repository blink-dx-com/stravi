<?php
require_once 'o.WORKLIST.subs.inc';
require_once 'f.sql_query.inc';
require_once ("func_form.inc");

/**
 * - import object/ID for one column from object selection 
 *  the destination column must be empty !!!
 *    
 * @package o.WORKLIST.guimod3.inc
 * @swreq UREQ:0001769: o.WORKLIST > Import von Objeckt-Links (aus Selektion) in Worklist  
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param $_REQUEST['id'] id of CYTCART
 * @param $_REQUEST['col'] selected id of column
 * 
 */
class o_WORKLIST_guimod3_XPL extends gPlugin {
	
	var $absEntries; // tablenames
	
	function __construct(&$sqlo) {
		parent::__construct($sqlo);
	}
	
	function register() {
		$tablename	= 'WORKLIST';
		$title		= 'Import selected Objects';
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'obj'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_id']   = $_REQUEST['id'];
		$this->infoarr['checkid']  = 1;
		$this->infoarr['inforow']  = 'UREQ:0001769: Import von Object-Links (aus Selektion) in Worklist';
		
		
		$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('read', 'write'), 'obj'=>array('write') )
				);
					
	}
	
	/**
	 * select table
	 */
	function form0(&$sqlo) {
		$initarr   = NULL;
		$initarr["title"]       = "Select table type"; // title of form
		$initarr["submittitle"] = "Select"; // title of submit button
		$initarr["tabwidth"]    = "AUTO";   // table-width: AUTO

		$hiddenarr = NULL; // hidden form variables
		$hiddenarr["id"]     = $this->objid; // is a required input parameter
		
		$tableArr=NULL;
		
		$attribs = $this->worklistLib->getPlannedAttribs($sqlo);
		
		if (!sizeof($attribs)) return;
		
		reset ($attribs);
		foreach( $attribs as $colx=>$valarr) {
			if ($valarr['typ']=='OBJ' and $valarr['t']!=NULL) {
				$tabNice = tablename_nice2($valarr['t']);
				$tableArr[$colx]=$tabNice;
			}
		}
		
		if (!sizeof($tableArr)) {
			$this->_html->line_out('no type OBJECT in worklist columns expected.');
			return;
		}
		

		// init form, uses class formc
		$formobj = $this->_html->formInit($initarr, $hiddenarr, 0);
		
		$fieldx = array ( 
			"title" => "Table", 
			"name"  => "col",
			"object"=> "select",
			"namex" => TRUE,
			"val"   => NULL, 
			"inits" => $tableArr,
			"notes" => "import objects for this column"
			 );
		$formobj->fieldOut( $fieldx );

		$formobj->close( TRUE ); // close form, sshow submit-button
	}
	
	function _getObjSelSql( &$sqlo ) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$MAXOBJ_NUM = 200;
		
		$selCol  =$this->selCol;
		$seltable=$this->seltable;
		
		$this->_html->line_out( 'Objekte vom Typ '.tablename_nice2($seltable).' ...');
		
		$utilLib  = new fSqlQueryC($seltable); 
		$sqlAfter = $utilLib->get_sql_after( );
		$sqlsel   = 'count(1) from '. $sqlAfter;
		$sqlo->Quesel($sqlsel);
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 1, 'select-object query is broken.' );
			return;
		}
		$sqlo->ReadRow();
		$numobj = $sqlo->RowData[0];
		
		if ($numobj<=0) {
			$error->set( $FUNCNAME, 2, 'No objects selected.' );
			return;
		}
		
		if ( $numobj> $MAXOBJ_NUM ) {
			$error->set( $FUNCNAME, 3, 'Too many objects ('.$numobj.') selected. MAX:'.$MAXOBJ_NUM );
			return;
		}
		
		$optget =array("order"=>1);
		$sqlAfterSort = $utilLib->get_sql_after( $optget );
		
		return array('sqlAfterSort'=>$sqlAfterSort, 'numobj'=>$numobj);
	}
	
	/**
	 * preselect objects
	 */
	function action1(  &$sqlo  ) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$selCol  =$this->selCol;
		$seltable=$this->seltable;
		
		$selectInfo = $this->_getObjSelSql( $sqlo );
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 1, 'Object-Selection failed.' );
			return;
		}
		
		$this->_html->line_out( '<b>'.$selectInfo['numobj'].'</b> Objekte vom Typ '.tablename_nice2($seltable).' wurden ausgewaehlt.');
		$this->_html->line_out('');
		
		$initarr   = NULL;
		$initarr["title"]       = "Update Worklist with ".$selectInfo['numobj']." objects"; // title of form
		$initarr["submittitle"] = "Update with objects"; // title of submit button
		$initarr["tabwidth"]    = "AUTO";   // table-width: AUTO

		$hiddenarr = NULL; // hidden form variables
		$hiddenarr["id"]     = $this->objid; // is a required input parameter
		$hiddenarr["col"]    = $selCol;
		$formobj = $this->_html->formInit($initarr, $hiddenarr, 1);
		
		$formobj->close( TRUE ); // close form, sshow submit-button
	}
	
	/**
	 * update worklist with preselected objects
	 */
	function action2(  &$sqlo  ) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$selCol  =$this->selCol;
		$seltable=$this->seltable;
		
		$selectInfo = $this->_getObjSelSql( $sqlo );
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 101, 'Object-Selection failed.' );
			return;
		}
		
		$this->_html->line_out( '<b>'.$selectInfo['numobj'].'</b> Objekte vom Typ '.tablename_nice2($seltable).' wurden ausgewaehlt.');
		
		
		$url='p.php?mod=DEF/o.WORKLIST.guimod&id='.$this->objid.'&act=update.plan&parx[input]=ID';
		echo "<form style=\"display:inline;\" method=\"post\" ".
		 " name=\"editform\"  action=\"".$url."\" >\n";
		echo "<input type=hidden name='go' value='1'>\n";
		
		$sqlAfterSort = $selectInfo['sqlAfterSort'];
		
		$pkname = PrimNameGet2($seltable);
		$sqlsel   = 'x.'.$pkname.' from '. $sqlAfterSort;
		$sqlo->Quesel($sqlsel);
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 102, 'Object-Selection failed.' );
			return;
		}

		$row=1;
		while ( $sqlo->ReadRow() ) {
		    $tmpid = $sqlo->RowData[0];
		   
		    // update cell
		    echo '<input type=hidden name="cell['.$row.']['.$selCol.']" value="'.$tmpid.'">'."\n";
		    
		    $row++;
		}
		
		echo "</form>";
		
		if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
	    	echo "<B>INFO:</B> DEBUG-mode: Script stopped.<br>\n";
		}
		
		echo 'Object data will be forwarded; please wait ...'."<br>\n";
		// forward script
		?>
		<script language="JavaScript">
		document.editform.submit();
		</script>
		<?php 
		
		
	}
	
	

	function startMain() {
		global $error;
		$FUNCNAME='startMain';
		
		$this->objid   = $_REQUEST['id'];
		$go   = $_REQUEST['go'];
		$sqlo = &$this->_sqlo;
		$sqlo2 = &$this->_plug_regSql2();
		
		$goArray   = array( "0"=>"Select table", 1=>"Review Selection", 2=>"Update Object cells" );
		$this->_html->GoInfo($goArray, '&id='.$this->objid);
		 
		
		
		$this->worklistLib = new oWORKLIST_subs($this->objid);
		$this->worklistLib->init($sqlo);
		
		if (!$go) {
			$this->form0($sqlo);
			return;
		}
		
		$selCol = $_REQUEST['col'];
		if ($selCol<=0) {
			$this->htmlFoot('ERROR', 'Input-Parameter col missing.');
		}
		
		$attribs   = $this->worklistLib->getPlannedAttribs($sqlo);
		$attribCol = $attribs[$selCol];
		if ($attribCol['typ']!='OBJ' or $attribCol['t']==NULL) {
			$this->htmlFoot('ERROR', 'No data-type OBJ defined for column '.$selCol.'.');
		}
		$seltable = $attribCol['t'];
		
		$hasData = $this->worklistLib->colHasData($sqlo, $selCol, 'OBJID');
		if ($hasData) {
			$this->htmlFoot('ERROR', 'Die Worklist hat Daten in Spalte '.$selCol.
				'. Um Daten in diese Spalte zu importieren, muss diese Spalte leer sein.');
		}
		
		$this->selCol=$selCol;
		$this->seltable=$seltable;
		
		if ($go==1) {
			$this->action1($sqlo );
			return;
		}
		
		if ($go==2) {
			$this->action2($sqlo );
			return;
		}
		
		
		$this->_html->line_out('o.k.');
		
	}
	
}
