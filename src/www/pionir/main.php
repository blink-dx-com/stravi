<?php  
/**
 * Initiate a new session.
 * 
 * The user must supply only two mandatory variables for login: 
 *   $cctuser   -> user name
 *   $cctpwd    -> user password
 *   
 * Optional login/connection parameters:
 * either
 *   $db_index  -> index of array $database_access[]
 * or
 *   DEPRECATED ...
     *   $db        -> DBMS service name         
     *   $_SESSION['sec']['dbuser']   -> DBMS user name 
     *   $passwd    -> DBMS password
     *   [$_dbtype]   -> DBMS type
 *
 * or log in as user ($su_cctuser) via the root-authentication
 *    - if $su_cctuser: check password for "root" ($cctuser) with the root-password ($cctpwd) 
 *
 *   The connection parameters are
 *   taken from associative array $database_access[0]
 *   defined in 'config.local.inc'.
 * 
 * @package main.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param string $cctuser 
 * @param string $cctpwd 
 * @param string $db_index 
 * @param string $su_cctuser 
 * Optional:  
 * @param string $sPwdCook    = 1 : save user, index, password in cookie
 * @param string $logFromCook = 1 : take login from cookie 
 * @param string $sNoFrame    = 0|1 : do not show frame-set
 * @param string $sSurfMobile = 0|1 : mobile device output
 * @param string $tablename   - start with this object
 * @param string $id
 * @param string $forwardUrl  - encoded forward-URL
 */

session_start(); 


require_once ("f.logcookie.inc");
require_once ("f.app.checkvers.inc");

header('Content-type: text/html; charset=iso-8859-1');



class gMainC {

	var $CLASSNAME='gMainC';
    /**
     * check application-version against database model version 
     */
	function __construct($logCookieObj) {
	    $this->logCookieObj=$logCookieObj;
	    $this->db_index==NULL;
	}
	
	function set_db_index($db_index, $database_dbid_def) {
	    $this->db_index=$db_index;
	    $this->database_dbid_def=$database_dbid_def;
	}
	
	
    function checkDbVersion() {
    	global $error;
    	$FUNCNAME=$this->CLASSNAME.':checkDbVersion';
    
    	$versLib = new f_app_checkvers();
    	$versLib->checkDbVersion();
    	
    	if ($error->Got(READONLY))  {
    		$error->set( $FUNCNAME, 1, '' );
    	}
    }
    
    function back_login( $username=NULL ) {
        // do not delete the cookie, because may be the login is only temporarily blocked 
        $out='<br>';
        $db_index = $this->db_index;
        // delete cookie
        $cookieExists = $this->logCookieObj->cookieExists($db_index);

        // show back login text
        $tmpextra = "";
        if ( $username!="" )   $tmpextra .= "&cctuser=".$username;
        if ( $db_index!="" )   $tmpextra .= "&dbid=".$db_index;  // add the database ID, if there is a list of possible databases
        
        
        
        if (!$cookieExists) {
            $out  .= "<font><a href=\"index.php?".$tmpextra."\"><b>Back to LOGIN</b>";
            if ( $this->db_index and !$this->database_dbid_def) {
                $out  .= ' (Database "'.$this->db_index.'")';
            }
            $out  .= "</a></font><br>\n";
        } else {
            
            if ( $db_index!="" )   $tmpextraCookie = "&dbid=".$db_index;
            $out  .=  "<font size=+1><a href=\"index.php?".$tmpextraCookie."\"><b>Back to saved login</b> (with saved user and password)</a></font><br><br>\n";
            $out  .=  "<br><font size=+1><a href=\"index.php?".$tmpextra."&logCookieDel=1\"><b>Back to new login</b> (delete cookie, new user and password possible)</a></font><br>\n";
        }
        return $out;
    }   
    
    function set_cctuser($cctuser) {
        $this->cctuser = $cctuser;
    }
    
    function error($text_head, $text_info, $notes=NULL) {
        
        $cctuser = $this->cctuser;

        echo '<br><table width="80%" border="0">';
        
//         if ( $this->db_index and !$this->database_dbid_def) {
//             echo '<tr><td>';
//             echo 'Database name: <b>'.$this->db_index.'</b>';
//             echo '</td></tr>';
//         }
        echo '<tr><td>';
        echo '<font color="red" ><b>';
        echo $text_head, '</b></font>';
        echo ' <font color="#000000">';
        echo $text_info;
        echo '<br><br>';
        if ($notes) echo $notes."<br>";
        
        $backlogin    = $this->back_login( $cctuser );
        echo $backlogin;
        echo '<br></font>';
        echo '</td></tr></table>';
        echo '</body></html>';
        exit;
    }

}


############  Script variables  ################## 

$cctuser      = $_REQUEST['cctuser'];
$cctpwd       = $_REQUEST['cctpwd'];
$db_index     = $_REQUEST['db_index'];
$su_cctuser   = $_REQUEST['su_cctuser'];
$sPwdCook     = $_REQUEST['sPwdCook'];
$logFromCook  = $_REQUEST['logFromCook'];
$sNoFrame     = $_REQUEST['sNoFrame'];
$sSurfMobile  = $_REQUEST['sSurfMobile'];
$tablename    = $_REQUEST['tablename'];
$id           = $_REQUEST['id'];
$forwardUrl   = $_REQUEST['forwardUrl'];


$logCookieObj = new logCookieC();
// Fallback to default database defined in $database_access[0]:
$db_index_tmp=NULL;
if (isset($db_index))  $db_index_tmp = $db_index; // save this for later storing


$mainLib = new gMainC($logCookieObj);

$stop_referring = 0; // stop automatic referring after error?
$loginfo        = ""; 
$scriptPreInfo  = "";


if ( $logFromCook AND $logCookieObj->cookieExists($db_index) AND $cctuser=="" ) {  // if no external input!
    $scriptPreInfo .= "<font color=gray>... take login information from Cookie</font><br>\n";
    $logger_en =  $logCookieObj->getLogInfo($db_index);
    if ($logger_en["u"]!="" AND $logger_en["p"]!="") {
         $cctuser  = $logger_en["u"];
         $cctpwd   = $logger_en["p"];
         $sPwdCook = 1; // save it again to update the expiry time
    }
         
} 

if ( !empty($su_cctuser) ) {   // loginin as user $su_cctuser with password from root
    
    $loginfo .= "loginfromROOT,";
    $cctuser = $su_cctuser; 
    
}  

########################################################################

## Now read the configuration files:
$s_product=array();
$database_access = array();
$globals = array();
$database_dbid_def=NULL;
require("config.inc"); // get $globals, $database_access, $s_product from there !!!

require_once("func_head2.inc");

echo gHtmlHead::getHTMLDocType(); 
?>
<html>
 <head>
  <title><?php echo $s_product['product.name'];?> - Login check</title>
  <meta http-equiv="expires" content="3">
<?php

if ($sPwdCook) { 
    $logCookieObj->setCookiex($cctuser, $cctpwd, $db_index); 
}

?>
 </head>
 <body alink="#0000ff" vlink="#0000ff" bgcolor="#ffffff" marginheight=0 marginwidth=0>
 <!-- BODY:CENTER:START -->
 <div style="text-align: center;">
 

<table width="100%"  cellspacing=0 cellpadding=2 border=0 style="color: #FFFFFF; background-image: url(images/login_2018.jpg); background-repeat: no-repeat; background-size: cover;">
 <tr>
  <td nowrap align=center>
  <br>
   <b><?php echo $s_product['product.name'];?> Pionir</b> - The ExperimentNavigator</b>
   <br>
   (version: <?php echo $s_product["project"] . "/" . $s_product["version"] . ". " . $s_product["notes"]; ?>)
   <br>&nbsp;
  </td>
</tr>
</table>

<?php
echo '<br><br>';
echo '<div style="text-align: center;">';
echo '<font color="#808080" >Application loading ...</font><br><br>'."\n";
echo $scriptPreInfo;
echo '</div><br><br>';
echo "\n";
flush();

############  Read function definitions  ##################
require_once("db_access.inc");
require_once("globals.inc");
require_once ('o.USER_PREF.manage.inc');
require_once("utilities.inc");      /* goodies from piet */
require_once("logincheck.inc");
require_once("f.profilesub.inc");

$mainLib->set_cctuser($cctuser);


if ( $db_index==NULL ) {
    $mainLib->error( 'Missing login information: ', 'No Database name given.');
}
if ( !is_array($database_access[$db_index]) ) {
    $mainLib->error('Bad Login: ',  'Database name "'.$db_index.'" is unknown.');
}

$mainLib->set_db_index($db_index, $database_dbid_def);

// Check mandatory login parameters:
if (empty($cctuser)|| empty($cctpwd)) {
    
    $notes_arr=array();
    if (empty($cctuser)) $notes_arr[] = 'user name';
    if (empty($cctpwd))  $notes_arr[] = 'user password';
    $maininfo = implode("', ", $notes_arr);
    $mainLib->error( 'Missing login information: ', $maininfo );
    
}   

 
// $database_access comes from "config.inc"
$db      = $database_access[$db_index]["db"]; 
$_dbtype = $database_access[$db_index]["_dbtype"]; 
$LogName = $database_access[$db_index]["LogName"]; 
$passwd  = $database_access[$db_index]["passwd"];  



## Check optional login/connection parameters:
if ( $LogName == "" || $passwd == "" || $db == ""  ) {
    
    $notes_arr=array();
    if (empty($db))       $notes_arr[] = 'DBMS service name';
    if (empty($LogName))  $notes_arr[] = 'DBMS user name';
    if (empty($passwd))   $notes_arr[] = 'DBMS password';
    
    $maininfo = implode("', ", $notes_arr);
    
    $mainLib->error( 'Missing system information: ', $maininfo );

}

#########################################################################################
/* INIT */


$savedErrors = false;         // last occured error (needed for a smart view of errors) 
$s_history =array();          // history: [] = array( "TAB_NAME => ID )
$s_historyL=array();          // history for object-lists: [] = array( "TAB_NAME )     


require_once("main.sessvars.inc");  
  
$php_self_str    = $_SERVER['PHP_SELF'];
$script_file_str = $_SERVER['SCRIPT_FILENAME'];
$s_sessVars = array();
makeSessVars($php_self_str, $script_file_str);

if (isset($db_index_tmp)) {
    $s_sessVars["g.db_index"] = $db_index_tmp;   // save input var for relogin
}   

$_SESSION['s_sessVars']  = $s_sessVars;
$_SESSION['savedErrors'] = $savedErrors; // necessary for error display


## Attention:
## All queries send to the query object $sql instanciated here 
## CANNOT be logged since the sql_logging flag has not yet been 
## read from the database.
$error = & ErrorHandler::get();

/* -- if a partial database (missing TABLES, except DB_USER) was called: do no init_cache() !!!
	$conntype='';
	$usecache=false;
 	$sql   = logon_to_db( $LogName, $passwd, $db, $_dbtype, $conntype, $usecache );
*/


$sql   = logon_to_db( $LogName, $passwd, $db, $_dbtype, '', false ); // first login WITHOUT cache build (because cache needs SESSION vars)
if ($error->printLast()) {
    
    $mainLib->error( 'DB connect/internal login failed.', '' );
}

srand ((double) microtime() * 1000000); /* initialize random generator for rand()  */

$loggedin = -1;  // default login NOT OK 

$loginLib = new fLoginC();
$loginfo  = $loginLib->loginCheck($sql, $cctpwd, $cctuser, $su_cctuser);

if ( !is_array($loginfo) or $loginfo["logok"] < 1) {
	$errLast = $error->getLast();
	echo '<font color="#ff0000" face="Arial,Helvetica" size="+1"><b>'.$errLast->text.'</b></font><br>';
	echo "<br>\n";
	
// ... this could be a security problem
// 	$loginlog = $loginLib->getLoginLog();
// 	if ( !empty($loginlog)>0 ) {
// 		echo "Login-details: ";
// 		print_r($loginlog);
// 		echo "<br>\n";
// 	} 
	$notes    = $mainLib->back_login( $cctuser );
	echo $notes;
	exit;
}


$suflag		= $loginfo["su"];

/** OLD:
 * $_SESSION['db_user_id'] = $loginfo["userid"]; // application user DB_USER_ID
$_SESSION['LogName']=$LogName;	     // database user
$_SESSION['cctuser']=$cctuser;	     // application user nick
$_SESSION['passwd'] =$passwd;	     // database user password
$_SESSION['db']     =$db;
$_SESSION['_dbtype']=$_dbtype;
 */

// create user session vars
$_SESSION['sec']=array(); // secure session vars
$_SESSION['sec']['db_user_id'] = $loginfo["userid"]; // application user DB_USER_ID
$_SESSION['sec']['dbuser']  = $LogName;	     // database user:         OLD: "LogName"
$_SESSION['sec']['appuser'] = $cctuser;	     // application user nick; OLD: cctuser
$_SESSION['sec']['passwd']  = $passwd;	     // database user password
$_SESSION['sec']['db']      = $db;
$_SESSION['sec']['_dbtype'] = $_dbtype;
$_SESSION['sec']['dbid']    = $db_index;

$sql   = logon_to_db( $LogName, $passwd, $db, $_dbtype, '', true ); // second login WITH cache build

  
$loginLib->loginGlobals( $sql );
$loginLib->setSessionVars($s_sessVars);

$userGlob  = array();
$s_objCache=array(); // object mother cache

$s_clipboard=array();   /* session clipboard for BOs 
                             $s_clipboard[] = ( tab=>"TABLENAME", 
                                    id0=>"FIRST_ID", 
                                id1=>"SECOND_ID",
                                 id2=>"THIRD_ID"
                                ) 
						 */        
$s_funcclip=array();   /* session clipboard for functions 
                             $s_funcclip[] = ( "link_text"=>"URL" ) 
                         */
                         
$s_tree=array();    /* [$slice_cnt][$tab_key][$id]="ls".$level; 
                         " ", "l"       -leave
                         "h", "s", "e"  -show
                         LEVEL           -level
                     */
 
$s_suflag=$suflag;    /* su flag */
$s_tabSearchCond=array(); /* search condition per table
             "f"=> $fromClause  : extra string after FROM (e.g. cct_access a )
                "w"=> $whereClause : string after WHERE without joins (e.g. x.exp_id=300 )
            "x"=> $whereXtra   : extra whereclause for joined tables (e.g. a.cct_access_id=x.cct_access_id ) 
            "c"=> $CLASS_NAME  : a class has been selected [OPTIONAL]
            "info" => "user friendly where clause string" [OPTIONAL]
            "mothid" => "MOTHER_TABLE_ID" [OPTIONAL] show only elements with the ID of the mother-table
            
           */
$s_tabSearchCond['PROJ'] = array ("f"=>"", "w"=>"PRO_PROJ_ID is NULL", "x"=>"" ); /* clean conditions ??? */

$prefLib = new oUSER_PREF_manage();
$loaded = $prefLib->loadprefs($sql, $userGlob, $cctuser, $_SESSION['sec']['db_user_id']);
 
if ( !$loaded ) {
  $userGlob["setsPerView"]="20";
  $userGlob["editRemarks"]="1";
}
if (!$userGlob["g.appDesign"]) $userGlob["g.appDesign"]= 3; // green
 


/** 
 * defines "jump back"-parameters for a modal dialog in view.tmpl.php
 * Format:  array[tablename] = array( $cctgoba, $cctgobacol, $cctgosel );
 */
$s_formback = array(); 

$_SESSION['s_sessVars']  = $s_sessVars; // set SESSION-var s_sessVars again (due to setSessionVars() )
$_SESSION['s_suflag']    = $s_suflag;
$_SESSION['globals']     = $globals;	/** @var array $globals system global variables
								  *  @link file://CCT_QM_doc/89_1002_SDS_code.pdf#var:globals
								  */


$_SESSION['s_tabSearchCond']=$s_tabSearchCond;	//currenttable-search-condition-array
$_SESSION['userGlob']=$userGlob;	   // * @link file://CCT_QM_doc/89_1002_SDS_code.pdf#var:userGlob
$_SESSION['s_objCache']=$s_objCache;	// cache object mother array[table][id] => array( 't'=>mother table, 'id'=> )
$_SESSION['s_history']=$s_history;
$_SESSION['s_historyL']=$s_historyL;
$_SESSION['s_formback']=$s_formback;	// temporary array for goback operations
$_SESSION['s_clipboard']=$s_clipboard;
$_SESSION['s_tree']=$s_tree;
$_SESSION['s_product']=$s_product;
$_SESSION['s_funcclip']=$s_funcclip;
$_SESSION['s_formState']=$s_formState;	// description below

// obly root is doing this check (needs SESSION-vars)
if ($cctuser == "root") {
	$mainLib->checkDbVersion();
	if ($error->Got(READONLY))  {
		$error->printAll();
		$stop_referring=1;
	}
}
     
// usage of s_formState:
//
//      s_formState is an array, used to keep form data in wizard action in
//      memory. for example view at rogo solution II
//      first index in s_formState describes the wizard/module/context in which
//      variables are saved. for substance import it is s_formState["simport"]
//      use second index for your variables s_formState["simport"]["var1"],
//      s_formState["simport"]["var2"], s_formState["simport"]["var3], ...

if (!isset($tablename)) $tablename = "";
if (!isset($id))        $id = "";  

$dest_url = "main.fr.php?"; // forward to next page 
if ($globals['app.main.php.forward']!=NULL) {
	$dest_url = $globals['app.main.php.forward'];
}

if ( $tablename!="")  {
	$dest_url .= "&tablename=" .$tablename. "&id=" . $id ;  // jump to object
}
if ( $forwardUrl!="") $dest_url .= "&forwardUrl=".$forwardUrl;  // jump to URL
if ( $loginfo!=""  )  $dest_url .= "&info=".$loginfo; 
if ( $sNoFrame!="" )  $dest_url .= "&sNoFrame=".$sNoFrame; 

if ($sSurfMobile>0) {
	$_SESSION['s_sessVars']["g.surfMode"]='mobile';
}
                 
// original place: $logCookieObj->setCookiex()


// get history
$profileObj = new profileSubC();
$profileObj->getLastObject( $sql );

if ($_SESSION['userGlob']["g.debugLevel"]>0) { 
    echo "<br><font color=red>DEBUG-mode:</font> level:".$_SESSION['userGlob']["g.debugLevel"]. " automatic forward stopped.<br>\n";
    
    $stop_referring=1;
}

echo "<script language=\"JavaScript\" TITLE=\"Javascript deactivated!\"> <!--\n";

// now pure javascript to calculate Browser-info
// this value will be used in main.fr.php $s_sessVars["g.browserInfo"] 
// no support for browsers below Moz1.0, IE5
?>
  var tmpkomma="";
  var sumBrowserInfo="";  
  var isDOM = false;
  
  if (document.getElementById) {
       sumBrowserInfo = sumBrowserInfo + "DOM";
       isDOM = true;
       tmpkomma=",";
  }
  
  if (document.all) { 
       sumBrowserInfo = sumBrowserInfo +tmpkomma + "IE";
       tmpkomma=",";
  }
  
<?

$dest_urlFull = $dest_url."&browserInfo=";
if ($stop_referring)
{
  echo "document.write('<p>Javascript-referring stopped.<br>');";
  echo "document.write('<a href=\"".$dest_urlFull."'+ sumBrowserInfo +'\">Start now anyway</a><br>');";
  echo "//-->\n";
  echo "</Script>\n";

} else {
  echo "location.href='".$dest_urlFull."'+ sumBrowserInfo;\n";
  echo "//-->\n";
  echo "</Script>\n";
  echo "<noscript>\n";
  echo "<br>\n";
  info_out("WARNING", "If this page will not be referred, <b>Javascript</b> is deactivated!<br>"); 
  echo "Please switch Javascript on.<br><br>";                                                                     
}
?>


</div><!-- BODY:CENTER:END -->
</body>
</html>
