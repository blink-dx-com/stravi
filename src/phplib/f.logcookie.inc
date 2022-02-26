<?php

/**
 * manage login cookie
 * manage one cookie per DB-index
 * @package f.logcookie.inc 
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class logCookieC {
    
    var $text_arr;

function getCookieName($db_index) {
	// each DB_ID has its own cookie
	return ("gozilla_s_memlog_".$db_index);
}

function set_text($text) {
    $this->text_arr[]=$text;
}

function setCookiex ($cctuser, $cctpwd, $db_index) {  
    // set cookie
	// !!! must be called inside <html><head> !!!
    $logger =  base64_encode (  serialize( (array("u" => $cctuser, "p" => $cctpwd ) ) )  );
    $verfalls_datum =  time()+ 60*60*24*60; // gilt xxx Tage
    setcookie ( $this->getCookieName($db_index), $logger, $verfalls_datum); // ACHTUNG: erscheint im global VAR space
    $this->set_text("<font color=gray>... save password in cookie </font><br>");
} 

function cookieExists ($db_index=NULL) {
	$cookieName = $this->getCookieName($db_index);
	if (isset($_COOKIE[$cookieName]) ) return true;
	else return false;
}

function getLogInfo( $db_index=NULL ) {
	$cookieName = $this->getCookieName($db_index);
	$logger_en =  unserialize( base64_decode (  $_COOKIE[$cookieName] ) );
	return ($logger_en);
}

function delCookie($db_index=NULL) {
	$cookieName = $this->getCookieName($db_index);
	setcookie ( $cookieName, NULL);
    $_COOKIE[$cookieName] = NULL;
}
}


