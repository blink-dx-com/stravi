<?php
/**
 * login data for XML_RPC tests
 * @package xml_rpc_client2.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
function xml_rpc_testLogins() {
	return array (

			"test01.alere2" => array (
					"dbid" => "alere2",
					"hostname" => "192.168.2.117",
					"srv_url"  => "/pionir/xmlrpc/icono_svr.php",
					"cct_user" => "test01",
					"cct_password" => "", 
					"protocol" => "ssl"
			),
	
	);
}