<?php
require_once 'o.EXP.RESULT.inc';
require_once 'o.SATTACH.mod.inc';
require_once 'lev1/f.exportDataFile.inc';
require_once 'f.compress.inc';

/**
 * o.EXP > EXP_RAW_SPOT_RESULT > extract results as xlsxml-Attachment, optional delete DB-values
 * @package o.EXP.raw_save_att.inc
 * @swreq UREQ:4720 EXP_RAW_SPOT_RESULT > extract results as xlsxml-Attachment, optional delete DB-values
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/phplib/export/o.EXP.raw_save_att.inc 59 2018-11-21 09:04:09Z $
 */
class oEXP_raw_save_att {
	
	private $raw_cols; // array (MAPCOL => NICECOL)
	
	public function __construct($workdir) {
		$this->workdir    = $workdir; // TBD
	}
	
	/**
	 * export all RAW-RESULTs as XML
	 * EXP_ID, Spot_id, Channel_id, ....
	 * @param unknown $sqlo
	 * @param unknown $expid
	 */
	public function initExp(&$sqlo, $exp_id) {
		$this->exp_id = $exp_id;
		$this->RAW_DESC_ID= 0;
		$this->datatable  = 'EXP_RAW_SPOT_RESULT';
		$this->raw_cols   = array();
		$this->sql_select = NULL;
		$this->temp_file  = NULL;
		$this->fileNameZip_Pure = NULL; // only the file name
		
		
		$this->getColumns($sqlo);
		
	}
	
	private function getColumns(&$sqlo) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$exp_id = $this->exp_id;
	
		$sqls = "H_EXP_RAW_DESC_ID from EXP where EXP_ID=".$exp_id;
		$sqlo->Quesel($sqls);
		$sqlo->ReadRow();
		$this->RAW_DESC_ID = $sqlo->RowData[0];
		
		if (!$this->RAW_DESC_ID) {
			$error->set($FUNCNAME,1,"no H_EXP_RAW_DESC_ID for EXP-ID:".$exp_id);
			return;
		}
	
		$sqlsel = gSpotResC::sqlsel_allCols($this->RAW_DESC_ID, 'EXP_RAW_RESULT');
		$sqlo->Quesel($sqlsel);
		while ($sqlo->Readrow()) {
			$this->raw_cols[$sqlo->RowData[0]] = $sqlo->RowData[1];
		}
		
		unset($this->raw_cols['SPOT_ID']);
		
		
		if (!sizeof($this->raw_cols)) {
			$error->set($FUNCNAME,2,"no columns defined for H_EXP_RAW_DESC_ID:".$this->RAW_DESC_ID);
			return;
		}
			
		// build sql-string
		$map_cols   = array_keys($this->raw_cols);
		$sqls       = 'CHANNEL_ID, SPOT_ID, ABSTRACT_SUBST_ID, ';
		$rawcolsstr = implode(', ',$map_cols);
		
		$this->sql_select = $sqls . $rawcolsstr . ' from '.$this->datatable.' where EXP_ID='.$exp_id.' order by CHANNEL_ID, SPOT_ID';
	}
	
	/**
	 * create the file
	 * - experiment MUST have raw-data, otherwise => error
	 * @param  $sqlo
	 * @param  $sqlo2
	 * @return void|string
	 */
	public function createFile(&$sqlo, &$sqlo2) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$scriptName = __CLASS__;
		$this->down_text = NULL;
		$exp_id = $this->exp_id;
		
		$filename          = $exp_id.'.raw.xml.xls';
		$this->fileNameZip_Pure = $filename . '.gzip';
		$raw_niceCols = array_values($this->raw_cols);
		
		// creates a work-dir, inits the temporary file
		$export_opt = array( 'encoding'=>'UTF-8', 'autoFileExt'=>-1, 'workDir'=>$this->workdir );
		$fileExpLib = new f_exportDataFile_C('xlsxml', $scriptName, $filename, $export_opt);
		
		$columnnames = array('channel-id', 'spot-id', 'probe');
		$columnnames = array_merge($columnnames, $raw_niceCols);
		
		
		$headerarr=NULL; // the file header, pre information
		$headerarr[]=array('#UREQ:4720: EXP_RAW_SPOT_RESULT > extract results as xlsxml-Attachment');
		$headerarr[]=array('#EXP-ID: '.$exp_id.' Export-Datum:'. date('Y-m-d H:i:s') );
		$headerarr[]=$columnnames;
		
		$fileExpLib->outputStart($headerarr);
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 1, 'export start failed.' );
			return;
		}
		
		$map_cols   = array_keys($this->raw_cols);
		
		$sqls = $this->sql_select;
		$sqlo->Quesel($sqls);
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 2, 'error on SQL-command:'.$sqls );
			return;
		}
		
		$rows = 0;
		while ($sqlo->ReadArray()) {
				
			// for all spots ...
				
			$channelid= $sqlo->RowData['CHANNEL_ID'];
			$spot_id  = $sqlo->RowData['SPOT_ID'];
			$probe_id = $sqlo->RowData['ABSTRACT_SUBST_ID'];
			$prob_name = NULL;
			
			if ($probe_id) {
				$prob_name = obj_nice_name ( $sqlo2, 'ABSTRACT_SUBST', $probe_id );
			}
	
			$outarr = array($channelid, $spot_id, $prob_name); // start of line ...
	
			// output all cols
			reset ($map_cols);
			foreach( $map_cols as $dummy=>$map_col) {
				$outarr[] = $sqlo->RowData[$map_col];
			}
	
			$fileExpLib->oneRow($outarr);  // write one line to file
			$rows++;
				
		}
		
		$fileExpLib->close();
		
		if (!$rows)  {
			$error->set( $FUNCNAME, 3, 'experiment has no raw-data!' );
			return;
		}
		
		$this->down_text = $fileExpLib->getDownloadText();
		$temp_file1 = $fileExpLib->getFileNameFull();
		
		$compressLib = new compressC();
		$this->temp_file = $this->workdir .'/'.$this->fileNameZip_Pure;
		
		
		$answer  = $compressLib->zip($temp_file1, $this->temp_file);
		if ($answer==FALSE) {
			$error->set( $FUNCNAME, 5, 'error on ZIP!' );
			return;
		}
		
		
		
		
		// get filename
		return $fileExpLib->getFileNameFull();
	}
	
	/**
	 * save XL-file as experiment-attachment
	 * @param unknown $sqlo
	 */
	public function saveAttachment(&$sqlo) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$exp_id = $this->exp_id;
		if ($this->temp_file==NULL) {
			$error->set( $FUNCNAME, 3, 'input param fileNameZip missing.' );
			return;
		}
		
		
		$attachLib = new oSattachMod();
		$attachLib->setObj('EXP', $exp_id);
		
		$attachName     = $this->fileNameZip_Pure;
		
		$exists = $attachLib->checkNameExists($sqlo, $attachName);
		
		if ($exists) {
			$error->set( $FUNCNAME, 1, 'attachment already exists.' );
			return;
		}
		
		$argin = array(
				'NAME'=>$attachName,
				'KEY'=>'DB.EXP_RAW_RESULT'
				);
		$rel_id = $attachLib->insertAtt( $sqlo, $argin );
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 2, 'insert attachment failed.' );
			return;
		}
		
		$attachLib->copyFile($rel_id, $this->temp_file );
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 3, 'attachment upload failed.' );
			return;
		}
		
	}
	
	public function getDownloadText() {
		return $this->down_text;
	}
	
}

 