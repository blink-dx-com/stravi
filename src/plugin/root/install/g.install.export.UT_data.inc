<?php

require_once 'o.PROJ.addelems.inc';

/**
 * export basic UnitTest data via PAXML
 * - add UnitTest objects to a project
 * - then the project can be exported
 * @package g.install.export.UT_data.inc
 * @swreq UREQ:14602 g.UnitTest > export basic UnitTest data
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/plugin/root/install/g.install.export.UT_data.inc 59 2018-11-21 09:04:09Z $
 */
class  g_install_export_UT_data_XPL extends gPlugin {


	function register() {
		
		$title		  = 'Export basic UnitTest data via PAXML';

		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'tool';
		$this->infoarr['design']   = 'norm';
		$this->infoarr['locrow']   = array( array('rootsubs/rootFuncs.php', 'home') );

		$this->infoarr2 = array(
				'accCheck' => array('tab'=>array('read'), 'obj'=>array() ),
				'onlyAdmin'=>1
		);

	}

	/**
	 * - creates a form with one text field "Min-value"
	 * - creates a HTML-page parameter $_REQUEST['parx']['minval'] after SUBMIT
	 */
	function form1() {

		$initarr   = NULL;
		$initarr["title"]       = "Set Destination Project"; // title of form
		$initarr["submittitle"] = "Submit"; // title of submit button
		$initarr["tabwidth"]    = "AUTO";   // table-width: AUTO

		$hiddenarr = NULL; // hidden form variables
		$formobj = $this->_html->formInit($initarr, $hiddenarr, 0);

		$fieldx = array ( // form-field definition
				"title"   => "Project ID",
				"name"  => "proj_id",
				"object"=> "text",
				"val"    => 0,
				"notes" => ""
		);
		$formobj->fieldOut( $fieldx ); // output the form-field

		$formobj->close( TRUE ); // close form, sshow submit-button
	}

	function startMain() {
		global $error;

		$FUNCNAME='startMain';
		
		$UT_dir  = dirname(__FILE__).'/../../../_test/';
		$UT_file = $UT_dir.'misc/test_config.inc';
		
		if (!file_exists($UT_file)) {
			$this->htmlFoot('ERROR', 'UnitTest-file not found: '.$UT_file);
		}
		require_once($UT_file);
		
		// include unitest library ...
		require_once($UT_dir.'misc/unittest_onetest.inc');

		$parx   = $_REQUEST['parx'];
		$sqlo   = &$this->_sqlo;
		$sqlo2  = &$this->_plug_regSql2();
		
		$UT_conf_lib = new unittest_config();		
		$object_dict = $UT_conf_lib->get_objects();
		
		if (!$parx['proj_id']) {
			$this->form1();
			return;
		}
		
		echo 'PROJECT ID:'. $parx['proj_id']."<br>";
		
		$proj_mod_lib = new oProjAddElem($sqlo);
		$proj_mod_lib->setProject($sqlo, $parx['proj_id']);
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 1, 'Error on Init.' );
			return;
		}
		
		$unittest_lib = new gUnitTestSub();
		
		$config_unittest = array(); // can be empty
		$unittest_lib->initClass( $sqlo, $sqlo2, $config_unittest);
		
		foreach($object_dict as $tablename => $row) {
			
			// is a buisiness object ?
			if (cct_access_has2($tablename) ) {
				
				foreach($row as $key => $objid_ORI) {
					
					echo '... adding '. $tablename.' Key:'. $key." ";
					// get the real object ID and check, if object exists
					$objid = $unittest_lib->_getExistObjID( $tablename, $key );
					if ($error->Got(READONLY))  {
						$errLast   = $error->getLast();
						$error_txt = $errLast->text;
						$error_id  = $errLast->id;
						echo "; ERROR: $error_txt ($error_id)";
						$error->reset();
						echo "<br>\n";
						
					} else {
					
						$proj_mod_lib->addObj($sqlo, $tablename, $objid);
						if ($error->Got(READONLY))  {
							$error->set( $FUNCNAME, 1, 'Error on adding: '. $tablename.' ID:'. $objid);
							return;
						}
						echo '; add ID:'. $objid."<br>";
					}
				}
			}
		}
	}

}
