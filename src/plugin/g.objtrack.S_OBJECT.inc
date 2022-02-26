<?php
/**
 * - object tracking for list of objects in S_OBJLINK
 * @namespace g.objtrack.S_OBJECT.inc 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq   UREQ:0001044: g > object tracking : support S_OBJLINK 
 */

/**
 * help class
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class oS_OBJLINK_trackLi {
    
	function __construct($table, $sqlSel, $option) {
		$this->option = $option;
		$this->typex  = $option['type'];
		$this->sqlSel = $sqlSel;

		$this->tablename   = $table;
		$this->moTableArr  = NULL;
		$this->showLevel   = 9;
		
		$this->usedtabs  =NULL;
		$this->usedtabs_i=NULL;
		$this->obj_num_all = 0;
		
		$this->header_text = "<br />\n<font color=\"#999999\">Linked as child (meta) by objects:</font>\n<br><br>\n";
		
	}
	
	/**
	 * start object link analysis
	 * @param $sqlo
	 * @return number of tables
	 */
	function start(&$sqlo) {
		$tablename = $this->tablename;
		
		$moTableArr=NULL;
		if ($this->typex=='chi') {
			$sqlsel = "distinct(MO_TABLE) from S_OBJLINK where CH_TABLE=".$sqlo->addquotes($tablename).
				' and CH_ID in (select '.$this->sqlSel.') order by MO_TABLE';
		} else {
			$sqlsel = "distinct(CH_TABLE) from S_OBJLINK where MO_TABLE=".$sqlo->addquotes($tablename).
				' and MO_ID in (select '.$this->sqlSel.') order by CH_TABLE';
		}
		$sqlo->Quesel($sqlsel);
		
		while ($sqlo->ReadRow() ) {
			$moTable = $sqlo->RowData[0];
			$moTableArr[] = $moTable;
		}
		$this->moTableArr=$moTableArr;
		
		return $this->moTableArr;
	}
	
	// @return 0,1
	function getTables() {
		return $this->moTableArr;
	}
	
	/**
	 * count/show usage of ONE parent table ( from PARAMS )
	   RETURN: 0,1 entry exists ?
				
	 * @param $sql
	 * @param $parent
	 * @param $parPkName
	 * @return int
	 */
	function oneTable( &$sqlo, $parent ) {
		$tablename = $this->tablename;
		
		$retval=1; // entry exists ?	
		$parPkName = PrimNameGet2($parent);

		if ($this->typex=='chi') {
			$sqlAfter = "S_OBJLINK where CH_TABLE=" .$sqlo->addquotes($tablename). ' and CH_ID in (select '.$this->sqlSel.')'.
				" and MO_TABLE=".$sqlo->addquotes($parent);
			$sqlCondEncoded = urlencode($parPkName. " in (select MO_ID from ".$sqlAfter.")");
		} else {
			$sqlAfter = "S_OBJLINK where MO_TABLE=" .$sqlo->addquotes($tablename). ' and MO_ID in (select '.$this->sqlSel.')'.
				" and CH_TABLE=".$sqlo->addquotes($parent);
			$sqlCondEncoded = urlencode($parPkName. " in (select CH_ID from ".$sqlAfter.")");
		}
		$sqlsel   = "count(1) from ".$sqlAfter;
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$found_obj_num  = $sqlo->RowData[0];
		

		$parentNice = tablename_nice2($parent);
		
		if ($found_obj_num > 0) {  
			if ( !$this->obj_num_all ) {
				echo $this->header_text;
			}
			
			echo " <a href=\"view.tmpl.php?condclean=1&t=".$parent."&tableSCond=". $sqlCondEncoded ."\">";
			$icon = htmlObjIcon($parent, 1);
			$this->iconCache[$parent] = $icon;
			echo "<img src=\"$icon\" border=0> ";
			
			
			echo $parentNice."</a>: ";
			echo "<B>$found_obj_num</B>";
			
			
			echo "<br>\n";
							
			
		} else {
			if ($this->showLevel > 1) echo $parentNice.": none<br>\n";
		}
			
		
		$this->obj_num_all = $this->obj_num_all + $found_obj_num;
		
		return ( $retval );
	}
}


/**
 * - object tracking for S_OBJLINK
 *    
 * @package g.objtrack.S_OBJECT.inc 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param $_REQUEST['t'] destination table name
 * @param $_REQUEST['type'] 
 * 	   'mo' : is mother, 
 *     'chi' : is child
 *   
 */
class g_objtrack_S_OBJECT_XPL extends gPlugin {
	
	function __construct(&$sqlo) {
		parent::__construct($sqlo);
	}
	
	function register() {
		$tablename	= $_REQUEST['t'];
		$title		= 'Object tracking in object link';
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['help_base']= '';
		$this->infoarr['title']    = $title;
		$this->infoarr['title_sh'] = 'Object tracking';
		$this->infoarr['form_type']= 'list'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $_REQUEST['t'];
		$this->infoarr['obj_cnt'] = 1;

		
		
		$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('read'), 'obj'=>array() )
				);
				
	}
	
	
	

	function startMain() {
		global $error;
		
		//$FUNCNAME='startMain';
		
		$this->table   = $_REQUEST['t'];
		$typex = $_REQUEST['type'];
		$sqlo = &$this->_sqlo;
		//$sqlo2 = &$this->_plug_regSql2();
		
		$tablename = $this->table;
		
		if ( !cct_access_has2($tablename) ) {
			$this->htmlfoot('ERROR', 'Only Business objects allowed.');
		}
		
		if (!table_exists2($tablename)) {
			$this->htmlfoot('ERROR', 'Please update the system to run this tool.');
		}
		
		switch ($typex) {
			case "chi":
				$this->_html->line_out('Track for parents');
				break;
			case "mo":
				$this->_html->line_out('Track for children');
				break;
			default:
				$this->htmlFoot('Need a valid TYPE.');
		}
		
		$sqlAfter = $this->listLib->getSqlAfterNoOrder();
		$pkname   = PrimNameGet2($tablename);
		// show ID and NAME of selected objects as simple list
		$sqlsel = $pkname." from " . $sqlAfter;
		$option = array('type'=> $typex);
		
		$trackLib = new oS_OBJLINK_trackLi($tablename, $sqlsel, $option);
		$trackLib->start($sqlo);
		$objLinkTabs = $trackLib->getTables();
		
		if (!empty($objLinkTabs)) {
			foreach( $objLinkTabs as $moTable) {
				$trackLib->oneTable($sqlo, $moTable);
			}
			
		}

	}
	
}
