<?php
/**
 * search attachments in object list
 * @package g.objtab.attach_v.inc 
 * @swreq   UREQ:0002446: o.SATTACH > analyse attachments in list of objects 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param   $go
 * @param   $t tablename
 * @param   array $parx
 *    ['KEY']  - attachment key
 *    'DOCNAME' - document-name (pattern with *)
	  'ONLYHITS' : 0,1 show only hits ?
 */

require_once ("f.objview.inc");	
require_once ("o.SATTACH.subs.inc");

/**
 * manage one attachment
 * Enter description here ...
 * @author steffen
 *
 */
class t_oneObj_attach {
	
	var $parx; /*
		optional parameters
		'KEY' : search only attachments with specified key
	*/
	
	function __construct( &$_html, $tablename ) {
		$this->_html = &$_html; 
		$this->table = $tablename;
		$this->objLinkLib = new fObjViewC();
	}
	
	function setSearchCond($parx) {
		$this->parx = $parx;
		$this->parx['KEY'] = trim($this->parx['KEY']);
		$this->parx['DOCNAME'] = trim($this->parx['DOCNAME']);
		
	}
	
	function _getAttachlist( &$sqlo, $objid) {
		$parx = $this->parx;
		
		$attachList = array();
		
		$tablename= $this->table;
		$attachtab= 'SATTACH';
		$satObj = new cSattachSubs();

		$sqlo->Quesel("count(1) FROM ".$attachtab." WHERE TABLE_NAME='".$tablename."' AND OBJ_ID=".$objid);
		$sqlo->ReadRow();
		$numattach = $sqlo->RowData[0];
		
		if (!$numattach) {
			return;
		}
		
		$searchExtend=NULL;
		if ($parx['KEY']!=NULL) {
			$searchExtend=' and KEY like '.$sqlo->addQuotes($parx['KEY']);
		}
		if ($parx['DOCNAME']!=NULL) {
			$doc_temp = str_replace('*', '%', $parx['DOCNAME']);
			$searchExtend=' and NAME like '.$sqlo->addQuotes($doc_temp);
		}
		
		// MIME_TYPE
		$sqlo->Quesel("* FROM ".$attachtab." WHERE TABLE_NAME='".$tablename."' AND OBJ_ID=".$objid. $searchExtend . " ORDER BY REL_ID");
		while ($sqlo->ReadArray() ) {
		
			$arch_html= NULL;
			$exists = 0;
			$relid = $sqlo->RowData['REL_ID'];
			$tname = $sqlo->RowData['NAME'];
			$aKey  = $sqlo->RowData['KEY'];
			
			// new keys
			$xdate = substr($sqlo->RowData['XDATE'],0,10);
			
			$filesizex  = "";
			$docfile    = $satObj->getDocumentPath($tablename, $objid, $relid);
			$tmpAttName = htmlspecialchars($tname);
			if (file_exists( $docfile )) {
				$exists = 1;
			}
			
			$oneAttachInfo = array('relid'=>$relid, 'name'=>$tname, 'ex'=>$exists, 'key'=>$aKey);
			$attachList[]  = $oneAttachInfo;
		}
		
		return $attachList;
		
	}
	
	function oneObjectOut(&$sqlo, $objid, $objname) {
		$tablename   = $this->table;
		$attach_html = NULL;
		$html_obj    = $this->objLinkLib->bo_display( $sqlo, $tablename, $objid );
		
		$attachlist = $this->_getAttachlist($sqlo, $objid);

		
		if (!empty($attachlist)) {

				
				$tmpKomma=NULL;
				$useID = $objid;
				
				foreach( $attachlist as $dummy=>$valarr) {
					
					$tname = $valarr['name'];
					$relid = $valarr['relid'];
					$key   = $valarr['key'];
	
					$oneAttach = "<a href=\"obj.sattach.down.php?t=$tablename&id=$objid&rel_id=$relid\">".htmlspecialchars($tname)."</a>";
					
					$dataArr = array( $useID, $html_obj, $oneAttach, $key);
					$this->_html->_table3_row ( $dataArr );
					
					// show only on first attachment
					$html_obj =NULL;
					$useID   = NULL;
				}
		} else {
			if ( $this->parx['ONLYHITS']>0 )  {
				// nothing 
			} else {
				$dataArr = array( $objid, $html_obj, $attach_html, NULL);
				$this->_html->_table3_row ( $dataArr );
			}
		}
		
	}
}


class g_objtab_attach_v_XPL extends gPlugin {
	
	
	function register() {
		$tablename	= $_REQUEST['t'];
		$title		= 'list of objects: search attachments';
		
		$this->infoarr			   = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title']    = $title;
		$this->infoarr['title_sh'] = 'Search attachments';
		$this->infoarr['help_url'] = 'o.SATTACH.html';
		$this->infoarr['form_type']= 'list';  // must be set for a list
		$this->infoarr['design']   = 'norm';  // normal design
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_cnt']  = 1;  // count current selection
		
		$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('read'), 'obj'=>array() ),
				'listChkSelect' => 1
				);	
		
	}
	
	
	
	function doLoop(&$sqlo, &$sqlo2, $sqlAfter) {
		
		$parx = $this->parx;
		// show a table
		$headx = array ( 'ID', 'Object', 'Attachment', 'Key');
		$headOpt = array ("title"=>'Attachment list');
		$this ->_html -> _table3_init ( $headx , $headOpt );
		
		$this->oneObjAttLib->setSearchCond($parx);
		
		
		$pkname = PrimNameGet2($this->table);
		// show ID and NAME of selected objects as simple list
		$sqlsel = $pkname. ", NAME from " . $sqlAfter;
		$sqlo2->Quesel($sqlsel);
		
		while ( $sqlo2->ReadRow() ) {
		    $objid   = $sqlo2->RowData[0];
		    $objname = $sqlo2->RowData[1];
		    $this->oneObjAttLib->oneObjectOut($sqlo, $objid, $objname);
		}
		$this ->_html->_table3_close();
		
	}
	
	/**
	* - creates a form with one text field "Min - value "
	* - creates a HTML - page parameter $_REQUEST ['parx '][ ' minval '] after SUBMIT
	*/
	function form1 () {
		
		$initarr = NULL ;
		$initarr ["title"] = "Search Parameters"; 
		$initarr ["submittitle"] = "Anzeigen"; // title of submit button
		$initarr ["tabwidth"] = "AUTO"; // table - width : AUTO
		$hiddenarr = NULL ; // hidden form variables
		$hiddenarr['t'] = $this->table;
		$formobj = $this ->_html->formInit ( $initarr , $hiddenarr , 0);
		
		
		$fieldx = array ( // form - field definition
			"title" => "Key",
			"name"  => "KEY",
			"object"=> "text",
			"val"   => $this->parx['KEY'],
			"notes" => "show only attachment with specified key, e.g. RMS"
			);
		$formobj->fieldOut($fieldx ); // output the form - field
		
		$fieldx = array ( // form - field definition
			"title" => "Document name",
			"name"  => "DOCNAME",
			"object"=> "text",
			"val"   => $this->parx['DOCNAME'],
			"notes" => 'search attachment-name, use * as wildcart; e.g. *.docx'
		);
		$formobj->fieldOut($fieldx ); // output the form - field
		
		if ( is_array($this->parx) ) {
				if ( !array_key_exists($this->parx,'ONLYHITS') ) $this->parx['ONLYHITS']=1;
	    } else $this->parx['ONLYHITS']=1;
	    
		$fieldx = array ( // form - field definition
				"title" => "Show only hits?",
				"name"  => "ONLYHITS",
				"object"=> "checkbox",
				"val"   => $this->parx['ONLYHITS'],
				"notes" => ""
		);
		$formobj->fieldOut($fieldx ); // output the form - field
		
		$formobj -> close ( TRUE ); // close form , sshow submit - button
	}

	function startMain() {
		global $error;
		
		$FUNCNAME='startMain';
		
		$sqlo  = &$this->_sqlo;
		$sqlo2 = &$this->_plug_regSql2();
		
		$objcnt = $this->listLib->getObjCnt();
		
		$MAX_CNT=50000;
		if ($objcnt> $MAX_CNT) {
			$this->_html->line_out('Too many objects selected. Max '.$MAX_CNT.' objects allowed for this tool.');
			return;
		}
		
		$this->table = $_REQUEST['t'];
		$go   = $_REQUEST['go'];
		$this->parx = $_REQUEST['parx'];
		
		$this->form1();
		
		if (!$go) {
			return;
		}
		
		$sqlAfter = $this->listLib->getSqlAfter();
		
		$this->oneObjAttLib = new t_oneObj_attach($this->_html, $this->table) ;
		
		echo "<br>\n";
		$this->doLoop($sqlo, $sqlo2, $sqlAfter);
	}
	
}
