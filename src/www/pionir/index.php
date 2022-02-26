<?php
/**
 * - main GUI login form
 * - for xmlrpc-login use xmlrpc/icono_svr.php
 * 
 * - forwarded params:
 *   - $sSurfMobile
 *   - sPwdCook
 * INPUT from config.inc:
 *   $database_access_sel - 0|1 : optional - allow to select an DBID from combo-box ?
 *   $database_access[]
 *   $database_dbid_def   - optional - default DBID
 *   					   identifies the default id e.g. "0"
 *   $globals[]  (some variables)
 *   DEPRICATED: $logintype  [''], 'developer'
 * @package mainIndex
 * @swreq  UREQ:0001688: g > user login to system SRS:ADM01
 * @see 89_1002_SDS_code.pdf : section index.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param  $dbid         : preselected DB
 * @param  $logCookieDel : remove cookie
 * @param  $tablename + $id  : for forwarding
 * @param  $forwardUrl   : encode forwarded URL
 * @version $Header: trunk/src/www/pionir/index.php 66 2018-12-11 12:10:01Z $
 */
session_start(); 
require_once ("utilities.inc");   // provides function 'is_partisan_open()'
require_once ("f.logcookie.inc");  

// preset values, modified in onfig.inc
$database_access_sel = NULL;
$database_access     = NULL;
$database_dbid_def   = NULL;
$globals = NULL;
$s_product=NULL;
require 	 ("config.inc");      // Now read the configuration files:
require_once ('reqnormal.inc');

class gIndexPage {
    
    private $s_product;
    private $dbid;
    public $g;  // HTML template content variable

    function __construct(&$database_access, $s_product, $dbid, $database_dbid_def) {
	   $this->database_access = &$database_access;
	   $this->s_product = $s_product;
	   $this->dbid = $dbid;
	   $this->database_dbid_def = $database_dbid_def;
	   
	   $this->logCookieObj = new logCookieC();
	   $this->g = array(
	       'head'=>array(),
	       'body'=>array(),
	       );
	   
	   if ( !isset($this->dbid) AND $database_dbid_def!="" ) {
	       $this->dbid = $database_dbid_def; // set default dbid
	   }
	   
    }
    
    function get_dbid() {
        return $this->dbid;
    }

    function delCookie() {
    	$this->logCookieObj->delCookie($this->dbid);
    }
    
    function htmlHead() {
        $this->g['head']['title']= $this->s_product['product.name'].' / Login';
        $this->g['body']['product.name'] = $this->s_product['product.name'];
    }
    
    function set_flagAppIsOpen($flag) {
        $this->g['body']['AppIsOpen'] = $flag;
    }
    function set_login_post_text($text) {
        $this->g['body']['post_text'] = $text;
    }
    
    /**
     * original version: year 2020; Steffen (2021): changed the regexp, because, the patterns change too fast
     * @author https://www.geeksforgeeks.org/how-to-detect-a-mobile-device-using-php
     * @return int $mobile_browser 0,1
     */
    function detectMobile() {
        $user_agent = $_SERVER["HTTP_USER_AGENT"];
        return preg_match("/(applewebkit|phone|pie|tablet|up\.browser|up\.link|webos|wos)/i"
            , $user_agent );
    }
    
    
    function loginFormStart( $tablename, $id, $flagAppIsOpen, $forwardUrl  ) {
        
        $productIcon   = $this->s_product["loginicon"];
        
        $this->g['body']['form']=array();
        
        $this->g['body']['product.icon']  = $productIcon;

    	## Pass HTTP parameters which may result from an external hypertext link
    	## into the application:
    	$hiddenarr=array();
    	if (isset($tablename)  && isset($id)) { 
    	    $hiddenarr['tablename'] = $tablename;
    	    $hiddenarr['id'] = $id;
    	} else { 
    		## For some obscure reasons, at least one input tag is required:
    	    $hiddenarr['tablename'] = '';
    	}
    	if ($forwardUrl!="") {
    	    $hiddenarr['forwardUrl'] = urlencode($forwardUrl);
    	} 
    	$this->g['body']['form']['appIsOpen'] = $flagAppIsOpen;
    	$this->g['body']['form']['hidden'] = &$hiddenarr;

    }
    
    function isOpenHead($xcctuserx, $db) { 
        $this->g['body']['appuser'] = $xcctuserx;
        $this->g['body']['db_name'] = $db;
        $this->g['body']['db_user'] = $_SESSION['sec']['dbuser'];
    }
    
    function shStdLogin($database_access_sel, $login_allow) {
    
        $dbid = $this->dbid;
    	$dbAccArr = &$this->database_access;
    	
    
    	$db_index_onlyOne = TRUE;
    	if (sizeof($dbAccArr)>1) $db_index_onlyOne = FALSE;
    	
    	$this->g['body']['form']['dbid'] = $dbid;
 
    	do {	
  
    		if (isset($dbid)) {  // preselected DB
    		    
    			if ($db_index_onlyOne == FALSE) { // more than one entry
    				if ($dbAccArr[$dbid]["unshow"]!=1) {
    				    $this->g['body']['form']['row_dbid'] = 1;
    					
    					$tmpalias = $dbid;
    					if ( ($tmpalias=="") && ($dbAccArr[$dbid]["db"]!="") )  {
    						$tmpalias = $dbAccArr[$dbid]["LogName"]. "@". $dbAccArr[$dbid]["db"]; 
    					}
    					$this->g['body']['form']['db_alias_out'] = $tmpalias;
    				}
    			}
    			
    			if ( $dbAccArr[$dbid]['deny']>0 ) {				
    			    $this->g['body']['form']['deny'] = 1;
    			    $this->g['body']['form']['deny.message'] = $dbAccArr[$dbid]['deny.message'];
    				$login_allow = 0;
    			}
    			
    			break;
    		} 

    		if ( sizeof($dbAccArr) > 1 ) { 
                                                               
    			if ($database_access_sel) { // selection allowed
    				## Select database service alias name if several services are configured.
    				
			        $this->g['body']['form']['acc_sel'] = 1;

					$sel_opts = array();
					$sel_opts[]=array("", '--- Database name ---');
					
					foreach($dbAccArr as $index => $subhash) {
						if ( isset($dbAccArr[$index]["alias"]) ) { 
							$db_alias = $dbAccArr[$index]["alias"]; 
						} else {
							$db_alias = $index . ":" . $dbAccArr[$index]["db"]; 
						}
						$sel_opts[]=array($index, $db_alias);
					}
					$this->g['body']['form']['db_index_arr']  = &$sel_opts;

    			}  else { 
    			    // TBD: take dbid form cookie ???
    			    $this->g['body']['form']['login_show_db_sel'] = 1; 
				    // $this->g['body']['form']['acc_miss'] = 1;
					$login_allow = 1;
					
					
    			}
    		}  
    	} while (0);
    	
    	$this->g['body']['form']['login_allow'] = $login_allow;

    	// echo "<tr><td colspan=2><hr size=1 noshade></td></tr> \n";
    	/* Table of action buttons */
    	if ($this->g['body']['form']['login_allow']) {
    		$isMobile = $this->detectMobile();
    		$this->g['body']['form']['isMobile'] = $isMobile;
    	} 
    }
  
    function tryCookieLogin() {
        $dbidLog = $this->dbid;
    	if ( $this->logCookieObj->cookieExists($dbidLog)==True ) { 	    
    	    $logger_en =  $this->logCookieObj->getLogInfo($dbidLog);
            if ($logger_en["u"]!="" AND $logger_en["p"]!="") {  
                $this->g['body']['cookie_forward'] = 1;
            } 
        }
    }

}

// --------------------------------------------

$error = & ErrorHandler::get();

$dbid       = $_REQUEST['dbid'];
$logCookieDel = $_REQUEST['logCookieDel'];
$tablename  = $_REQUEST['tablename'];
$id         = $_REQUEST['id'];
$forwardUrl = $_REQUEST['forwardUrl'];

/**
 * comes from config.inc ...
 * $database_access_sel - 0|1 : optional - allow to select an DBID from combo-box ?
 * $database_access[]
 * $database_dbid_def  
 */
if ( !isset($database_access) ) $database_access=NULL;
$mainlib = new gIndexPage($database_access, $s_product, $dbid, $database_dbid_def);
$mainlib->htmlHead();

// --- HTML START



// if (!$dbid AND sizeof($database_access)<=1) $dbid = "0";	// only if ONE entry for $database_access
if ( $logCookieDel and $mainlib->get_dbid() ) {
    // delete login cookie     
    $mainlib->delCookie();
}


$flagAppIsOpen = is_partisan_open();

$mainlib->loginFormStart( $tablename, $id, $flagAppIsOpen, $forwardUrl );
	
// Table of action buttons  
// <LOGIN_FIELDS>  
$login_allow   = 1;

if ( $flagAppIsOpen ) {
    $mainlib->set_flagAppIsOpen(1);
    $mainlib->isOpenHead($_SESSION['sec']['appuser'], $_SESSION['sec']['dbuser'], $_SESSION['sec']['db']);
} else { #####  This is a standard login:
	$mainlib->shStdLogin( $database_access_sel, $login_allow);
}


if ( !$flagAppIsOpen AND $s_product["loginPostTxt"]!="" ) {
	// view a product-dependent login-text	
    $mainlib->set_login_post_text( $s_product["loginPostTxt"] );
}

if ( !$flagAppIsOpen ) {   // can ignore login_allow !!!
	$mainlib->tryCookieLogin();
} 

$html_obj = &gHtmlTmpl::getInstance();
$html_obj->out('index.html', $mainlib->g );