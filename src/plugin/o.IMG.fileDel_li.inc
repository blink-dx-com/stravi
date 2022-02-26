<?php
/**
 * delete FILE-attachment of selected images (table: IMG)
 */
require_once ('o.IMG.file.inc');
require_once ("f.visuTables.inc");

class o_IMG_fileDel_li_XPL extends gPlugin {
	var $CLASSNAME='o_IMG_fileDel_li_XPL';
	
	function __construct(&$sqlo) {
		parent::__construct($sqlo);
	}
	
	function register() {
		$tablename	= 'IMG';
		$title		= 'delete image-files of selected images';
		
		$this->_go = $_REQUEST['go'];
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title']    = $title;
		$this->infoarr['title_sh'] = 'delete files';
		$this->infoarr['form_type']= 'list'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;
		
		$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('write'), 'obj'=>array() ),
				'listChkSelect'=>1,
				'progressBar'=>1
			);
	}
	
	/**
	 * handle one image (delete the image file)
	 * @param $sqlo
	 * @param $imgid
	 * @return 0,1 deleted?
	 */
	function manageOneImg(&$sqlo, $imgid) {
		global $error;
		$FUNCNAME='manageOneImg';
		
		$exists = $this->imgFileLib->onServerExists( $imgid );
		if (!$exists) return 0;
		
		$o_rights = access_check($sqlo, 'IMG', $imgid);
		if ( !$o_rights["write"] ) {
			$error->set( $FUNCNAME, 10, 'no write permission on object' );
			return;
		}
		
		if ($this->_go>0 ) {
			$retval = $this->imgFileLib->deleteFile($sqlo, $imgid);
			if ($error->Got(READONLY))  {
				$error->set( $FUNCNAME, 11, 'delete failed' );
				return;
			}
		} else $retval = 1;
		
		return $retval;
	}
	
	function  formshow() {
		require_once ('func_form.inc');
		require_once ("func_formSp.inc");
		
		$formLib = new formSpecialc();
		
		$params= array('go'=>1, 'mod'=>$this->_mod );
		$formLib->deleteForm( 'Accept delete', 'Do you want to delete the files of selected images?',
			 $_SERVER['PHP_SELF'] , $params );
		
		
		$this->_html->htmlKeyOut('BR');
	}
	
	function startMain() {
		global $error;
		$FUNCNAME='startMain';
		
		$sqlo  = &$this->_sqlo;
		$sqlo2 = &$this->_plug_regSql2();
		
		$this->imgFileLib = new oIMG_fileC();
		
		if ( !$this->_go ) {
			$this->formshow();
		}
		
		$objcnt = $this->listLib->getObjCnt();
		
		$prgopt['objname']= 'rows';
		$prgopt['maxnum'] = $objcnt;
		$this->_html->progressInit($prgopt);
		
		$sqlAfter = $this->listLib->getSqlAfter();
		$sqlsel = "x.IMG_ID from ".$sqlAfter;
		$sqlo2->Quesel($sqlsel);
		$sum=NULL;
		$sum['error']=0;
		$sum['cnt']=0;
		$storeErr = NULL;
		$stoErrTxt= NULL;
		
		while ( $sqlo2->ReadRow() ) {
		    $imgid   = $sqlo2->RowData[0];
		    $delFlag = $this->manageOneImg($sqlo, $imgid);
			if ($error->Got(READONLY))  {
				$errLast   = $error->getLast();
				// $error_txt = $errLast->text;
				$error_id  = $errLast->id;
				$storeErr[$error_id]++;
				if ($stoErrTxt[$error_id]==NULL) $stoErrTxt[$error_id]=$errLast->text;
				
				$error->reset();
				$sum['error']++;
				
			} else {
				if ($delFlag>0) $sum['ok']++;
			}
			$sum['cnt']++;
			
			$this->_html->progressAlive($sum['cnt']);
		}
		$this->_html->progressAlive($sum['cnt'], 1);
		
		$visuLib = new fVisuTables();
		$visuLib->showSummary( $sum);
		
		if (sizeof($storeErr)>0) {
			$this->_html->htmlKeyOut('BR');
			
			$outarr=NULL;
			foreach( $storeErr as $error_id=>$errcnt) {
				$outarr[]= array($error_id,$errcnt,$stoErrTxt[$error_id]);
			}
			reset ($storeErr); 
			
			$tabobj = new visufuncs();
			$headOpt = array( "title" => "Error-IDs");
			$headx   = array ("Error-ID", 'Count', "Info");
			$this->_html->_table_out2($headx, $outarr);
			
		}
	}
}
