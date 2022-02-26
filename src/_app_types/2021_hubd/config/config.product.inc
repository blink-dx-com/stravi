<?php
/**
 * config for the  for HUB-D variant
 * Initialize $s_product[] variables.
 *
 * This file should be compiled/readonly in a customers installation.
 * Do NOT make any changes in this file except for the CVS version!
 *
 * Configuration files are ALWAYS read in this order:
 * 1) "config.product.inc"
 * 2) "config.local.inc"
 *
 * @module config/config.product.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
$s_product=array();
$s_product["login.text"]	= "<b>Pionir</b> - The Web-GUI of Gozilla"; // for login header-page

$s_product["type"]	         ="2021_hubd"; 	   	 /* describes the application-type [blinkLIMS] or "2021_hubd" or other */
$s_product["company.URL"]	 ="http://blink-dx.com"; 
$s_product["product.name"]	 ="Blink HUB-D";   // Name of the Product, appears in the Website title
$s_product["product.icon"]	 ="ic.system.logo.png";  // icon of the Partisan system
$s_product["company.icon"]	 ="ic.blink.logo.png";  // icon of the Partisan system
#$s_product["product.head.extratext"]	 ='<span style="color:yellow;">&nbsp;_DEV_ </span>';   // extra text in header; e.g. DEV-SYSTEM
$s_product["project"]="HUB-D";            /* "bucha", "erlkoenig"  */
$s_product["version"]="0.1";  			  /* special arrayLIMS "1.847"  */
$s_product["date"]	 ="2021-06-05";       /* last modification date */
$s_product["notes"]	 ="For BLINK-Customers: Assay Development";  

/*$s_product["loginPostTxt"]	= "
				<tr><td  colspan=3>
				<br><br>
				
				</td></tr>";
*/


#eof
