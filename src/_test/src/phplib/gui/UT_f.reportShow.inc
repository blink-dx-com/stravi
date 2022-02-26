<?php
/**
 * testing: f.reportShow.inc
 * @version $Header: trunk/src/www/_tests/phplib/gui/UT_f.reportShow.inc 59 2018-11-21 09:04:09Z $
 */

require_once 'object.info.inc';
require_once ("f.objview.inc");
require_once ("gui/f.reportShow.inc"); // include here, otherwise unittest throws error on class-definition oSUA_UT_test ...

class UT_f_reportShow_inc extends gUnitTestSub {
	
function __construct() {
	$this->GUI_test_flag=1;
}

// return: 0 : not passed, 1: passed
function dotest( &$sqlo, $options ) {

	$sqlo2 = $this->_getSqlo2();

	$testLib = new oSUA_UT_test();
	
	$testLib->init2($sqlo);
	$testLib->show($sqlo, $sqlo2);
	
	
	$retval = 1;
	
	return ($retval);
}

}

/**
 * the reporting class
 * @author steffen
 *
 */
class oSUA_UT_test extends fReportShow_abs {

	function init2(&$sqlo) {

		$this->objInfoLib = new objInfoC ();
		$this->objLinkLib = new fObjViewC();
		
		$sqlAfter = 'ABSTRACT_SUBST x where x.ABSTRACT_SUBST_ID<1000';
		
		$sqlsel = 'count(1) from '.$sqlAfter;
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$objCnt = $sqlo->RowData[0];
		
		$url='p.php?mod=DEF/xxxxx'; // your plugin-URL

		$infoarr = array(
				'title'  =>      'QM-Analysis',
				'entries'=>      $objCnt,
				'entryPerPage'=> 10,
		 	'sqlCmd'     => "x.ABSTRACT_SUBST_ID, x.CCT_ACCESS_ID from ". $sqlAfter,
		 	'toolUrl'    => $url,
		 	'tableHeader'=> array(
		 			'SUA-ID',
		 			'NAME',
		 			'Creator',
		 			'Usage-Type'
		 	)
		);
		$this->init($infoarr);
	}


	/**
	 *
	 * @see phplib/gui/fReportShow_abs::getOneRow()
	 */
	function getOneRow(&$sqlo, $rowData) {

		$objid = $rowData[0];
		if (!$objid) return array(); // bizarre !!!!

		$user_name= NULL;
		$sapmnr   = NULL;
		$H_USAGE_name = NULL;

		$objAttribs = $this->objInfoLib->getParamsRaw ($sqlo , 'ABSTRACT_SUBST', $objid );

		$objUrl = $this->objLinkLib->bo_IDLink('ABSTRACT_SUBST', $objid);

		if ($objAttribs['access']['DB_USER_ID']) {
			$user_name = obj_nice_name ( $sqlo, 'DB_USER', $objAttribs['access']['DB_USER_ID'] );
		}
		
		if ($objAttribs['vals']['H_USAGE_ID']) {
			$H_USAGE_name = obj_nice_name ( $sqlo, 'H_USAGE', $objAttribs['vals']['H_USAGE_ID'] );
		}


		$outarr = array(
				$objUrl,
				$objAttribs['vals']['NAME'],
				$user_name,
				$H_USAGE_name
		);
		return $outarr;
	}
}
