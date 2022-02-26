<?php
/**
 * @namespace core::xmlrpc
 */


/**
 * reflect version of interface
 *
 * - 1.2.1 new: meta call possible
 * - 0.2.2 changed: connect_xml: different usage of  $f_param[3...5]
 * - 0.2.1 new function get_subst_coher()
 * -       changed: function get_a_probe_on_an_array_subst() returns ALIAS_NAME, if available
 * - 0.2.0 initial
 *
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @return string version (e.g. "1.2.0")
 * 
 */
function get_version_xml($f_name, $f_param)
{   
	return array(0, array("1.2.1"));
}
