<?php
require_once ("obj.concrete_proto.edit.inc");
require_once ($_SESSION['s_sessVars']['AppRootDir'].'/_test/misc/unittest_onetest.inc');

/**
 * UT for o.PROTO.stepout1.inc
 * @author Steffen Kube
 *
 */
class UT_o_PROTO_stepout1_XPL extends gPlugin {
	
	
	function register() {
		
		$title		= 'GUI UnitTest for o.PROTO.stepout1.inc';	
		$this->infoarr			     = array();
		$this->infoarr['title']      = $title;
		$this->infoarr['form_type']  = 'tool'; 
		$this->infoarr2['onlyAdmin'] = 1;
		$this -> infoarr ['locrow'] = array (
		    array ('../_tests/www/test/index.php', 'Unittest home')
		    );
	}
	
	
	
	function startMain() {
		global $error;
		$FUNCNAME='startMain';
		
		$sqlo = &$this->_sqlo;
		$sqlo2= &$this->_plug_regSql2();
		
		$tablename     = "CONCRETE_PROTO";
		
		$ut_help_lib = new gUnitTestSub();
		$ut_help_lib->initClass_lazy($sqlo, $sqlo2);
		if ($error->Got(READONLY))  {
		    $error->set( $FUNCNAME, 1, 'Init failed.' );
		    return;
		}
		
		$c_proto_id    = $ut_help_lib->_getDefObj_by_vario_key($sqlo, "CONCRETE_PROTO", 'default');
		
		if ( !$c_proto_id )  {
		    $error->set( $FUNCNAME, 1, 'UT object not found.' );
		    return;
		}
		
		$this->_html->line_out('C-Proto-ID:'.$c_proto_id);
		
		
		$edFormLib = new oConcProtoEdform($sqlo, $c_proto_id);
		$editAllow=1;
		$editProtoAllow = $edFormLib->showIntro( $sqlo, $tablename, $editAllow, 'Component protocol' );
		$devopt = array(
		    "backurl"=>'p.php?mod=TST/phplib/gui/UT_o.PROTO.stepout1',
		    'cs.inp' =>'text'
		);
		$edFormLib->showform( $sqlo, $sqlo2, $editProtoAllow, $devopt );
	
	}
	
}
