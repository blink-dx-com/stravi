<?php 
/**
 * config for Unittest
 * @swreq UREQ:30; FS-ID:FS-REG02-t2 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
$config_unittest = array(
	
	    // this user must hhave an active account in the AD
	    // 'pw_b64': is the password encoded with base64
		'logincheck.inc'=>array('user'=>'ldap-user', 'pw_b64'=>''),
	    'jsonrpc'=>array(
	        'url'=>'http://jenblipart02/pionir/api/jsonrpc', // /jsonrpc.php
	        'user'=>'test', 
	        'pw'=>'1234AbCd',
	        'dbid'=>'blk'
	        
	    )

);