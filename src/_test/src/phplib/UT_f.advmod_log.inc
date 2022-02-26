<?php
// test the library
require_once 'date_funcs.inc';

class UT_f_advmod_log_inc  extends gUnitTestSub {
	
function __construct() {

}

function _keepOpen(&$testLib) {
	$filename = $testLib->getFileName();
	
	$fp = fopen($filename, 'a');
	$cct_access_id = 111;
	$date_str = date_unix2datestr( time(), 4);
	$dictDummy= NULL;
	
	for ($i=0; $i<5; $i++) {
		$unxstamp = time();
		$date_str = date_unix2datestr( $unxstamp, 4);
		$content = $cct_access_id ."\t". $date_str . "\t" . $dictDummy ."\n";
		//$this->_infoNow( 'KeepOpenTest:'.$i, $unxstamp);
		
		$retVal = fputs( $fp, $content ); /* write data */
		usleep(1500000); 
	}
	
	
	fclose( $fp );
}

/* return: 0 : not passed, 1: passed
 * 'subTest' = 'keepOpen' : keep file handle open for 5 seconds
 */
function dotest( &$sqlo, $options ) {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	$retval = 1;
	$testLib = new f_advmod_log();
	
	// LONG change
	$entryDict = array(
	    'old'=>array(
	        'fea' => array('NAME'=>'checky', 'NOTES'=>'short text'),
	        'ass' => array(
	            'CONCRETE_PROTO_STEP'=>array(
	                array('st'=>12, 'su'=>178273, 'q'=>23.3, 'dv'=>12456, 'n'=>'oleole')
	            )
	        )
	    ),
	    'new'=> array(
	        'fea' => array('NAME'=>'checky 234', 
	            'NOTES'=>'0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789
0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789
0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789
ABCDEF'),
	        'ass' => array(
	            'CONCRETE_PROTO_STEP'=>array(
	                array('st'=>12, 'su'=>88888, 'q'=>23.378, 'dv'=>1249, 'n'=>'oleole ola')
	            )
	        )
	    )
	);
	
	$LEN_LIMIT=300;
	$info = 'ORI-length:'.strlen( $entryDict['new']['fea']['NOTES']);
	f_advmod_log::_reduce_dict_REC($entryDict);
	$result=0;
	$info .= ' NEW-length:'.strlen( $entryDict['new']['fea']['NOTES']);
	if ( strlen( $entryDict['new']['fea']['NOTES'])<=$LEN_LIMIT ) {
	    $result=1;
	} 
	$this->_infoNow( 'TEST01:OUTPUT', print_r($entryDict,1) );
	$this->_saveTestResult('TEST01', $result, $info);
	
	
	
	if ( $options['subTest']=='keepOpen') {
		$this->_infoNow( 'KeepOpenTest', 'Start' );
		$this->_keepOpen($testLib);
		$this->_infoNow( 'KeepOpenTest', 'End' );
		return (1);
	}

	$cct_access_id = 1287238;
	$date_str  = '2013-01-17T13:34:84';
	$entryDict = array(
		'old'=>array(
			'fea' => array('NAME'=>'checky'),
			'ass' => array( 
	    		'CONCRETE_PROTO_STEP'=>array(
			    	array('st'=>12, 'su'=>178273, 'q'=>23.3, 'dv'=>12456, 'n'=>'oleole')
			       ) 
			   )
		     ),
		'new'=> array(
			'fea' => array('NAME'=>'checky 234'),
			'ass' => array( 
	    		'CONCRETE_PROTO_STEP'=>array(
			    	array('st'=>12, 'su'=>88888, 'q'=>23.378, 'dv'=>1249, 'n'=>'oleole ola')
			    ) 
			 )
		)		
	);
	
	$testLib->writeLine($cct_access_id, $date_str, $entryDict);
	
	$this->_infoNow( 'Input', print_r($entryDict,1) );
	
	if ($error->Got(READONLY))  {
		return;
	}
	
	// check file
	$filename = $testLib->getFileName();
	$exists=1;
	if ( !file_exists($filename) ) {
		$exists=0;
		$retval=0;
	}
	$this->_infoNow( 'FileName:', $filename.' exists? '.$exists );
	
	$infoarr = $testLib->searchLine($cct_access_id, $date_str);
	
	if (!$infoarr[0]) {
		$error->set( $FUNCNAME, 1, 'Entry could not read out.' );
		return;
	}
	
	$entryOut = $infoarr[1];

	$this->_infoNow( 'ReadBack_data', print_r($entryOut,1) );
	
	return ($retval);
}

}
