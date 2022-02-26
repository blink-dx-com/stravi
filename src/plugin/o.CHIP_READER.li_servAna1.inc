<?php
require_once ('date_funcs.inc');
require_once ("visufuncs.inc");
require_once ("o.REA_LOG.servAna.inc");
require_once ("o.CHIP_READER.logs.inc");

/**
 * LIST of devices > analyse service actions
 * 
 * calendar:
 *  - BLUE:  service to to
 *  - GREEN: done
 * 
 * @package o.CHIP_READER.li_servAna1.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq UREQ:0001635: o.DEV > service > List of devices> zeige ersten Service-Termin 
 * @param $go: 0,1
 * @param $parx[KEY]
 * 
 */
class o_CHIP_READER_li_servAna1_XPL extends gPlugin {
	
	
	function register() {
		$tablename	= 'CHIP_READER';
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title']    = 'ServiceAnalyse1 - Suche im ServiceLog';
		$this->infoarr['title_sh'] = 'ServiceAnalyse1';
		$this->infoarr['form_type']= 'list'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_cnt']  = 1;
		
		
		$this->infoarr2 = array( 
			'accCheck' => array('tab'=>array('read'), 'obj'=>array() ),
			'listChkSelect'=>1
		);	
	}
	
	function anaInit(&$sqlo) {
		
		$this->devLogLib = new oREA_LOG_subs();
		
		$key        = $this->parx['KEY'];
		$param_name = $this->parx['NAME'];
		
		$this->sqlpart = array();
		
		$this->sqlpart['KEY'] = NULL;
		if ($key!=NULL)
			$this->sqlpart['KEY'] = " and KEY=".$sqlo->addQuotes($key);
		
		$this->sqlpart['NAME'] = NULL;

		if ($param_name!=NULL) {
			$param_name = strtoupper($param_name);
			$this->sqlpart['NAME'] = " and UPPER(NAME) like '%".$param_name."%'";
		}
	}
	
	function _getInfoPerDev(&$sqlo, $devid) {

		$loginfo = $this->devLogLib->ana_OKState($sqlo, $devid);
		
		$sqlsel = 'POS, KEY, NAME, XDATE from REA_LOG where CHIP_READER_ID='.$devid.
			$this->sqlpart['KEY'] . $this->sqlpart['NAME'] . ' order by XDATE DESC';
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadArray();
		$infoarr = $sqlo->RowData;
		
		return array('p'=>$infoarr, 'ok'=>$loginfo );
	}
	
	
	
	/**
	 * show the list
	 */
	function showList(&$sqlo, &$sqlo2) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
		$tabobj  = new visufuncs();
		$headOpt = array( "title" => "Service Liste; " );
		$headx   = array ("#", "DEV-ID", "Device", "Status", "Key", "Short description", "Date", "Pos");
		$tabobj->table_head($headx,   $headOpt);
	
		// $realogGuiLib = new oREA_LOG_guisub();
		
		$sqlsel = 'x.CHIP_READER_ID, x.NAME from '.$this->sqlAfter;
		
		$sqlo->Quesel($sqlsel);
		$cnt=0;
		while ( $sqlo->ReadRow() ) {
		
    		$loopDevid = $sqlo->RowData[0];
    		$loopName  = $sqlo->RowData[1];
    		
			$infoarr  = $this-> _getInfoPerDev($sqlo2, $loopDevid);
			
			if ($error->Got(READONLY))  {
				$error->set( $FUNCNAME, 1, 'error on device '.$loopDevid );
				return;
			}
			
			$nameOut = $infoarr['p']['NAME'];
			if (strlen($nameOut)>20) {
				$nameOut = substr($nameOut,0,20).'...';
			}
			$status= $infoarr['ok'];
			if ($status=='Working') {
				$status = '<span style="color:green">'.$status.'</span>';
			}
				
			$dataArr = array(
				$cnt+1,
				'<a href="edit.tmpl.php?t=CHIP_READER&id='.$loopDevid.'">'.$loopDevid.'</a>',
				$loopName, 
				$status,
				$infoarr['p']['KEY'],
				$nameOut,
				$infoarr['p']['XDATE'],
				$infoarr['p']['POS']
				);
			
			$tabobj->table_row ($dataArr);
			$cnt++;
		}

		
		$tabobj->table_close();
	}
	
	/**
	 * 
	 */
	function form1() {
		
		$initarr   = NULL;
		$initarr["title"]       = "Suchparameter"; // title of form
		$initarr["submittitle"] = "Suche"; // title of submit button
		$initarr["tabwidth"]    = "AUTO";   // table-width: AUTO

		$hiddenarr = NULL; // hidden form variables


		// init form, uses class formc
		$formobj = $this->_html->formInit($initarr, $hiddenarr, 0);

		$fieldx = array ( // form-field definition
			"title"   => "Key", 
			"name"  => "KEY",
			"object"=> "text",
			"val"    => $this->parx['KEY'], 
			"notes" => "Suche nach Key: Working, Service, Qualifizierung"
			 );
		$formobj->fieldOut( $fieldx ); // output the form-field
		
		$fieldx = array ( // form-field definition
			"title"   => "Short description", 
			"name"  => "NAME",
			"object"=> "text",
			"val"    => $this->parx['NAME'], 
			"notes" => 'Suche nach Zeichenkette in "Short description" '
			 );
		$formobj->fieldOut( $fieldx ); // output the form-field

		$formobj->close( TRUE ); // close form, sshow submit-button
	}
	
	

	function startMain() {
		global $error;
		
		$FUNCNAME='startMain';
		
		$this->parx   = $_REQUEST['parx'];
		$go    = $_REQUEST['go'];
		$sqlo  = &$this->_sqlo;
		$sqlo2 = &$this->_plug_regSql2();
		
		
		$this->_html->line_out('Zeigt alle ausgew&auml;hlte Ger&auml;te und den ersten Treffer der Suche an.');
		
		
		$sqlAfterNoOrd = $this->listLib->getSqlAfterNoOrder();
		$this->sqlAfter = $sqlAfterNoOrd. ' order by x.CHIP_READER_ID';
		
		$ObjCnt=$this->listLib->getObjCnt();
		if ($ObjCnt>3000) {
			$this->htmlFoot('ERROR', 'Too many devices selected. Max 3000 allowed.');
		}
		
		$this->anaInit($sqlo);
		
		$this->form1();
		
		if ($go) {
			$this->showList($sqlo, $sqlo2);
		}
		
	}
	
}