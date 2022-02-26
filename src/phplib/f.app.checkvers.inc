<?php
/**
 * application version checker
 * $Header: trunk/src/phplib/f.app.checkvers.inc 59 2018-11-21 09:04:09Z $
 * @package f.app.checkvers.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
 
class f_app_checkvers {
	
function __construct() {
	
	/* array minimum-DBVersion , current pionir_version */
	$this->versarr = array(
		array( 'db'=>'1.0.4.4', 'kern'=>2.006 ),
		array( 'db'=>'1.0.4.7', 'kern'=>2.007 ),
		array( 'db'=>'1.0.4.9', 'kern'=>2.014 )
		);
}
	
/**
 * check application-version against database model version 
 * - problem, if system has an OLD DBVersion 
 */
function checkDbVersion() {
	global  $error;
	$FUNCNAME='checkDbVersion';

	$db_tmp_pos1 = strpos ( $_SESSION['globals']["DBVersion"], "-" );
	if ( $db_tmp_pos1<=1 ) {
		$error->set( $FUNCNAME, 1, 'Invalid DBVersion [' . $_SESSION['globals']["DBVersion"] . ']!'.
			       ' Please contact your administrator.' );
		return;
	} else {
		$db_tmp_version_num  = substr( $_SESSION['globals']["DBVersion"], 0, 7 );
		$kernelVersString    = $_SESSION['s_product']['pionir_version'];
		$kernVersArr = explode(" ",$kernelVersString); // format: "FLOAT DATE"
		$kernNow = floatval($kernVersArr[0]); // get FLOAT val
		

		
		foreach($this->versarr as $valarr) {
		
			$db_require = $valarr['db'];
			$keX = $valarr['kern'];
			if ($kernNow>=$keX) {
				if ( $db_tmp_version_num < $db_require ) { 
					$error->set( $FUNCNAME, 1, '<b>WARNING!</B> '.
					 "This Database version ($db_tmp_version_num) is too old for the ".
					 " Application-Kernel-version (" .$kernNow.").<br>" .
					 'Database version ('.$db_require.') is required.<br>' .
					 'You should not use this database and ask the admin.' ); 
					return;
				}
			}
		}

		
	}
	return;
}

}
