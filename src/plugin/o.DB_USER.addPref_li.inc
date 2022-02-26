<?php
/**
 * - add a preference key/val-pair to a list of users
 *    
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq SREQ:0002257: o.DB_USER > set preferences for a list of users 
 * @param $_REQUEST['go'] 0,1,2
 * @param $_REQUEST['parx[key]'] the key
 * @param $_REQUEST['parx[key]'] the value
 *   
 */
require_once ('insertx.inc');
require_once ('f.update.inc');

/**
 * @author steffen
 *
 */
class o_DB_USER_addPref_li_XPL extends gPlugin {
	
	function __construct(&$sqlo) {
		parent::__construct($sqlo);
	}
	
	function register() {
		$tablename	= 'DB_USER';
		$title		= 'Set a preference for a list of users';
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title']    = $title;
		$this->infoarr['title_sh'] = 'Set preference';
		$this->infoarr['form_type']= 'list'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;
		
		
		$this->infoarr2 = array( 
			'accCheck' => array('tab'=>array('read','insert'), 'obj'=>array() ),
			'listChkSelect' => 1,
			'onlyAdmin' => 1
		);
		
				
		
	}
	
	function _form0() {
		
		$parx=$this->parx;
		$initarr   = NULL;
		$initarr["title"]       = "Set pref val";
		$initarr["submittitle"] = "Next &gt;";
	
		$hiddenarr = NULL;
		
		$formobj = $this->_html->formInit($initarr, $hiddenarr);
		
		$fieldArr=NULL;
		$fieldArr[] = array (
			"title" => "Key", 
			"name"  => "key", 
			"val"   => $parx["key"], 
			"notes" => "preference key", 
			"object"=> "text" 
			);
		$fieldArr[] = array (
			"title" => "Value", 
			"name"  => "val", 
			"val"   => $parx["val"], 
			"notes" => "preference value", 
			"object"=> "text" 
			);
		
		$formobj->fieldArrOut( $fieldArr ) ;
		$formobj->close( TRUE );
	}
	
	function _form1(&$sqlo, &$sqlo2, $key) {
		
		$parx=$this->parx;
		$initarr   = NULL;
		$initarr["title"]       = "Set pref val";
		$initarr["submittitle"] = "Set";
	
		$hiddenarr = NULL;
		
		$hiddenarr['parx[key]']=$parx["key"];
		$hiddenarr['parx[val]']=$parx["val"];
		$formobj = $this->_html->formInit($initarr, $hiddenarr,1);
		$formobj->close( TRUE );
		
		$headx=array('user', 'val', 'info');
		$headOpt=array();
		$this->_html->_table3_init($headx, $headOpt);
		
		// show table
		$sqlAfter = $this->listLib->getSqlAfter();
		$sqlsel = "x.DB_USER_ID, x.NICK from ". $sqlAfter;
		$sqlo2->Quesel($sqlsel);
		while ( $sqlo2->ReadRow() ) {
		    $userid  = $sqlo2->RowData[0];
		    $nick    = $sqlo2->RowData[1];
			
		    $sqlsel  = 'VALUE from USER_PREF where DB_USER_ID='.$userid.' and VAR_NAME='.$sqlo->addQuotes($key);
			$sqlo->Quesel($sqlsel);
			$sqlo->ReadRow();
			$valLoop = $sqlo->RowData[0];
			
			$info='different';
			if ($valLoop==$parx['val']) {
				$info='<spac style="color:green;">equal!</span>';
			}
			
			$dataArr=array($nick, htmlentities($valLoop), $info);
			$this->_html->_table3_row($dataArr);
		}
		$this->_html->_table3_close();
	}
	
	/**
	 * set one key-val-pair
	 * @return 
	 * @param object $sqlo
	 * @param object $userid
	 */
	function _setPrefOne( &$sqlo, $userid) {
		$varname= $this->parx['key'];
		$sqlsel = 'VALUE from USER_PREF where DB_USER_ID='.$userid.' and VAR_NAME='.$sqlo->addQuotes($varname);
		$sqlo->Quesel($sqlsel);
		if ( $sqlo->ReadRow() ) {
			$argu = array('VALUE'=>$this->parx['val']);
			$idarr= array('DB_USER_ID'=>$userid, 'VAR_NAME'=>$varname );
			gObjUpdate::update_row_s( $sqlo, 'USER_PREF',$argu, $idarr);  
		} else {
			$argu = array(
				'DB_USER_ID'=>$userid, 
				'VAR_NAME'=>$varname, 
				'VALUE'=>$this->parx['val']
				);
			$this->insertLib->insertR( $sqlo, 'USER_PREF',$argu, 'DB_USER_ID');
		}
		
	}
	
	function setKeyValues(&$sqlo, &$sqlo2) {
		global $error;
		$FUNCNAME='setKeyValues';
		
		$this->_html->line_out('... update now!');
		
		$this->insertLib = new insertRawC();
		$sqlAfter = $this->listLib->getSqlAfter();
		$sqlsel = "x.DB_USER_ID, x.NICK  from ". $sqlAfter;
		$sqlo2->Quesel($sqlsel);
		while ( $sqlo2->ReadRow() ) {
		    $userid = $sqlo2->RowData[0];
		    $nick = $sqlo2->RowData[1];
			$answer = $this->_setPrefOne($sqlo, $userid);
			echo '- '.$nick.' <br />'."\n";
			if ($error->Got(READONLY))  {
				$error->set( $FUNCNAME, 1, 'Error for userid:'.$userid );
				return;
			}
		}

	}
	

	function startMain() {
		global $error;
		
		$FUNCNAME='startMain';
		
		$this->go   = $_REQUEST['go'];
		$this->parx = $_REQUEST['parx'];
		$sqlo = &$this->_sqlo;
		$sqlo2 = &$this->_plug_regSql2();
		
		$go = $this->go;
		
		if ( !$go ) {
			$this->_form0();
			return;
		}
		
		if ($this->parx['key']==NULL ) {
			$error->set( $FUNCNAME, 1, 'Key not set' );
			return;
		}
		
		if ( $go==1 ) {
			$this->_form1($sqlo, $sqlo2, $this->parx['key']);
			return;
		}
		
		
		if ($go==2 ) {
			$this->setKeyValues($sqlo, $sqlo2);
		}
	}
	
}