<?php
require_once ("f.objview.inc");	
require_once ("f.sql_query.inc");

/**
 * o.CONCRETE_SUBST > show list of substances as DATE-Tree 
 * - this tool analyses selected substances in a stream (one major SQL-query)
 * - this tool does NOT cache any data of substances, so in principle, 
 *   it can handle an unlimited amount of substances ! (cool!)
 * @swreq UREQ:0002553: o.CONCRETE_SUBST > show list of substances as DATE-Tree 
 * @package o.SUC.datetree.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com) 
 * @param $parx[]
 * 		'y' analyse the Year; e.g. 2012 => show months
 * 		'm' analyse the month; e.g. 07  => show days
 * 		'd' analyse the day; e.g. 20    => show substances
 * @param $go => 1: activate selection of $cx
 * @param $cx[ID] = 1 : IDs of selected substances
 */
class o_SUC_datetree_XPL extends gPlugin {
	
	function __construct(&$sqlo) {
		parent::__construct($sqlo);
	}
	
	function register() {
		$tablename	= 'CONCRETE_SUBST';
		$title		= 'DATE-tree (sort by by some-date)';
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title']    = $title;
		$this->infoarr['title_sh'] = 'DATE-tree';
		// $this->infoarr['help_url'] = 'o.CONCRETE_SUBST.html';
		$this->infoarr['form_type']= 'list';  // must be set for a list
		$this->infoarr['design']   = 'norm';  // normal design
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_cnt']  = 1;  // count current selection
		
		$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('read'), 'obj'=>array() ),
				'listChkSelect' =>1
				);	
		
	}
	
	function _date2Arr($datex) {
		if ($datex==NULL) return array();
		
		$dateArr['y'] = substr($datex,0,4);
		$dateArr['m'] = substr($datex,5,2);
		$dateArr['d'] = substr($datex,8,2);
		return $dateArr;
	}
	
	function _getProjIcon() {
		return '<img src="images/icon.PROJ.gif" border=0> ';
	}
	
	function _show_Y($year) {
		echo ' <a href="p.php?mod='.$this->_mod.'&parx[y]='.$year.'">'.$this->_getProjIcon().$year."</a><br>\n";
	}
	function _show_M($month) {
		echo ' &nbsp;&nbsp;&nbsp;&nbsp;<a href="p.php?mod='.$this->_mod.
			 '&parx[y]='.$this->parx['y'].'&parx[m]='.$month.'">'.$this->_getProjIcon().$month."</a><br>\n";
	}
	
	function _show_D($day) {
		echo ' &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href="p.php?mod='.$this->_mod.
			 '&parx[y]='.$this->parx['y'].'&parx[m]='.$this->parx['m'].'&parx[d]='.$day.'">'.
			 $this->_getProjIcon().$day."</a><br>\n";
	}
	
	function _show_Sub($objid, $name, $datex) {
		$htmlTmp = $this->objLinkLib->bo_IDLink('CONCRETE_SUBST', $objid, $name).' [SUC-ID:'.$objid.']';
		
		echo ' &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;'.
			'<input type=checkbox name="cx['.$objid.']" value=1> '.
			'<img src="images/icon.CONCRETE_SUBST.gif" border=0> '.
			$htmlTmp.' ('.$datex.')'."<br>\n";
	}
	
	function _openForm() {
		echo '<form style="display:inline;\" method="post" '.
		 ' name="editform"  action="p.php?mod='.$this->_mod.'" >'."\n";
		echo "<input type=hidden name='go' value='1'>\n";
		
	}
	
	function _closeForm() {
		echo "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
		echo "<input type=submit value=\"Select\">\n"; // SUBMIT
		echo "</form><br />\n";
	}
	
	/**
	 * show tree
	 * - sort substances by x.SOME_DATE 
	 * - start from LATESTS
	 * - analyse Y,M,D
	 * @param $sqlo
	 */
	function showTree(&$sqlo, &$sqlo2) {
		$parx = $this->parx;
		$this->objLinkLib = new fObjViewC();
	
		
		$ana_month = $parx['y'];
		$ana_day   = $parx['m'];
		$the_day   = $parx['d'];
		
		$sqlAfter = $this->listLib->getSqlAfterNoOrder(). ' order by x.SOME_DATE ASC';
		
		// show ID and NAME of selected objects as simple list
		$sqlsel = "x.CONCRETE_SUBST_ID, x.NAME, x.SOME_DATE from " . $sqlAfter;
		$sqlo2->Quesel($sqlsel);
		
		$dateLast = array(); // 'y', 'm', 'd'
		$badDataCnt=0;
		$day_subst_cnt=0;  // count of day substances in loop
		$day_subst_active=0;
		
		while ( $sqlo2->ReadRow() ) {
		    $objid  = $sqlo2->RowData[0];
		    $name   = $sqlo2->RowData[1];
		    $datex  = $sqlo2->RowData[2];
		    
		    $dayArr = $this->_date2Arr($datex); 
		    if (!sizeof($dayArr)) {
		    	$badDataCnt++;
		    } else {
		    
		    	if ($day_subst_active) {
		    		// check, if substance list is finished ... next day ?
		    		if ( $dateLast['y'].$dateLast['m'].$dateLast['d'] == 
		    			 $dayArr['y'].$dayArr['m'].$dayArr['d']) {
		    			// nothing special ...
		    		} else {
		    			$day_subst_active=0;
		    			$this->_closeForm();
		    		}
		    	}
		    	
			    if ($dateLast['y']!=$dayArr['y']) {
			    	$this->_show_Y($dayArr['y']);
			    	$dateLast['y'] = $dayArr['y'];
			    	$dateLast['m'] = 0;
			    	$dateLast['d'] = 0;
			    }
			    
			    // analyse this year
			    if ($ana_month and $dayArr['y']==$ana_month) {
					if ($dateLast['y'].$dateLast['m']!= $dayArr['y'].$dayArr['m']) {
				    	$this->_show_M($dayArr['m']);
				    	$dateLast['m'] = $dayArr['m'];
			    		$dateLast['d'] = 0;
				    }
				    
				    if ($ana_day and $dayArr['m']==$ana_day) {
						if ( $dateLast['y'].$dateLast['m'].$dateLast['d'] !=
							 $dayArr['y'].$dayArr['m'].$dayArr['d']) {
							$this->_show_D($dayArr['d']);
				    		$dateLast['d'] = $dayArr['d'];
				    	}
				    	
				    	
				    	
				    	if ($dayArr['d']==$the_day) {
				    		
					    	if (!$day_subst_cnt) {
					    		$this->_openForm();
					    		$day_subst_active=1;
					    	}
				    		$this->_show_Sub($objid, $name, $datex);
				    		
				    		$day_subst_cnt++;
				    	}
				    	
				    }
			    }
		    }
		}
		
		// if the selected day was last leaf in the tree  ...
		if ($day_subst_active) {
			$this->_closeForm();
		}
		
		if ($badDataCnt) 
			echo '<br \>! Substances without Prod-Date: '.$badDataCnt."<br>\n";
	}
	

	function startMain() {
		global $error;
		
		$FUNCNAME='startMain';
		
		$sqlo  = &$this->_sqlo;
		$sqlo2 = &$this->_plug_regSql2();
		
		$this->parx = $_REQUEST['parx'];
		$go = $_REQUEST['go'];
		
		if ($go) {
			$cx = $_REQUEST['cx'];
			$this->_html->line_out('Select substances ...');
			
			if (!sizeof($cx)) {
				$this->htmlFoot('INFO', 'no substances selected.');
			}
			
			// activate selection and go to list view
			$buildSqlLib = new fSqlQueryC ('CONCRETE_SUBST'); // init class
			$buildSqlLib -> cleanCond(); // clear old condition
			
			$cx_substid = array_keys($cx);
			$newCond    = 'x.CONCRETE_SUBST_ID in ('.implode(',',$cx_substid).')'; // the new condition
			
			$infostr ='selected substances'; // a user info text
			$buildSqlLib->addCond ( $newCond , "AND", $infostr ); // add condition
			$buildSqlLib->queryRelase(); // release as ActiveTableCondition
			
			$this->_html->pageForward('view.tmpl.php?t=CONCRETE_SUBST' ,'list view');
			$this->htmlFoot();
		}
		
		$this->showTree($sqlo, $sqlo2);
		
	}
	
}
