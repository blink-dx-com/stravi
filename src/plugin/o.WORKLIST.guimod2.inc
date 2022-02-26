<?php
require_once 'o.WORKLIST.subs.inc';

/**
 * - import object/ID for one column; 
 *  the destination column must be empty !!!
 *    
 * @package o.WORKLIST.guimod2.inc
 * @swreq UREQ:0001264: o.WORKLIST > Import von Daten aus Datei in Worklists 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param $_REQUEST['id'] id of CYTCART
 * @param $_REQUEST['col'] id of column
 * $parx['data']; 
 * $parx['type']: ID, NAME
 * 
 */
class o_WORKLIST_guimod2_XPL extends gPlugin {
	
	var $absEntries; // tablenames
	
	function __construct(&$sqlo) {
		parent::__construct($sqlo);
	}
	
	function register() {
		$tablename	= 'WORKLIST';
		$title		= 'Import one column';
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'obj'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_id']   = $_REQUEST['id'];
		$this->infoarr['checkid']  = 1;
		$this->infoarr['inforow']  = 'UREQ:0001264: Import von Daten aus Excel in die Worklists';
		
		
		$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('read', 'write'), 'obj'=>array('write') )
				);
					
	}
	
	
	function form0() {
		$initarr   = NULL;
		$initarr["title"]       = "Import data"; // title of form
		$initarr["submittitle"] = "Update Worklist"; // title of submit button
		$initarr["tabwidth"]    = "AUTO";   // table-width: AUTO

		$hiddenarr = NULL; // hidden form variables
		$hiddenarr["id"]     = $this->objid; // is a required input parameter
		$hiddenarr["col"]    = $this->col; // is a required input parameter

		// init form, uses class formc
		$formobj = $this->_html->formInit($initarr, $hiddenarr, 0);

		$fieldx = array ( // form-field definition
			"title"   =>  "object names/IDs", 
			"name"    =>  "data",
			"object"  =>  "textarea",
			"val"     =>  "", 
			"notes"   =>  "new line separated names/IDs; Example: <br> exp42342342<br>exp32132132<br>wrtr94342342",
			"inits"   => array('rows'=>20, 'cols'=>40)
			 );
		$formobj->fieldOut( $fieldx ); // output the form-field
		
		$fieldx = array ( 
			"title" => "Type", 
			"name"  => "type",
			"object"=> "select",
			"val"   => 'NAME', 
			"inits" => array('NAME'=>'Name', 'ID'=>'ID'),
			"notes" => "type of object data"
			 );
		$formobj->fieldOut( $fieldx );

		$formobj->close( TRUE ); // close form, sshow submit-button
	}
	
	/**
	 * check IDs, Names
	 * - build array
	 * - forward to o.WORKLIST.guimod.inc: act='update.plan'    - update all planned cells: cell[y][x] = ID of object or other values
	 */
	function action1(&$sqlo, $parx) {
		$dataarr = explode ("\n",$parx['data']);
		if (!sizeof($dataarr)) {
			echo 'no data.';
			return;
		}
		
		
		$url='p.php?mod=DEF/o.WORKLIST.guimod&id='.$this->objid.'&act=update.plan&parx[input]='.$parx['type'];
		echo "<form style=\"display:inline;\" method=\"post\" ".
		 " name=\"editform\"  action=\"".$url."\" >\n";
		echo "<input type=hidden name='go' value='1'>\n";

		reset ($dataarr);
		$row = 1;
		$col = $this->col;
		foreach( $dataarr as $dummy=>$val) {
			$formval = str_replace('"','\"', trim($val));
			echo '<input type=hidden name="cell['.$row.']['.$col.']" value="'.$formval.'">'."\n";
			$row++;
		}
		
		echo "</form>";
		
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
		
		$parx  = $_REQUEST['parx'];
		$col   = $_REQUEST['col'];
		if (!$col) {
			$this->htmlFoot('ERROR', 'Param col missing.');
		}
		
		$this->col=$col;
		
		$worklistLib = new oWORKLIST_subs($this->objid);
		$hasData = $worklistLib->colHasData($sqlo, $col, 'OBJID');
		if ($hasData) {
			$this->htmlFoot('ERROR', 'Die Worklist hat Daten in Spalte '.$col.
				'. Um Daten in diese Spalte zu importieren, muss diese Spalte leer sein.');
		}
		
		if (!$go) {
			$this->form0();
			return;
		}
		
		if ($parx['data']==NULL) {
			$this->htmlFoot('ERROR', 'No data given.');
		}
		if ($parx['type']!='NAME' and $parx['type']!='ID') {
			$this->htmlFoot('ERROR', 'No valid type.');
		}
		
		if ($go==1) {
			$outarr = $this->action1($sqlo, $parx);
			if ($error->got(READONLY))  {
				return;
			}
			if ($outarr==NULL) return;
			
		}
		$this->_html->line_out('o.k.');
		
	}
	
}
