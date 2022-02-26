<?php
/**
 * - lock a SET of objects (remove all access rights)
   - if has ACCESS_RIGHTS: only access, if entail right !
   - if NO ACCESS_RIGHTS: can add CCT_ACCLOG, if role  "o.".$tablename.".acclog" !!!
   - analyse last log, if not same key, add it
 *   
 * @package glob.objtab.acclock.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq   UREQ:0000704: f.glob.obj.acclock.php > lcok object and add audit status 
 * @param  
 * $t       : tablename
 * $go		: 0,1,2
   $parx    : see params for class oAccLogC()
      'action' : H_ALOG_ACT_ID
   	  'notes'  : notes
   	  'repPolIgn' : 0,1 ignore release policy ???
   	  'qm_val' : 0.5 , 1
   	  'signpw' : optional password
   $options : options
     'shExtraFields' : 0,1
 */
session_start(); 


require_once ("reqnormal.inc"); // includes all normal *.inc files
require_once ('access_lock.inc');
require_once ('o.CCT_ACCLOG.gui.inc');
require_once ("f.objview.inc");	
require_once ("f.visu_list.inc"); 
require_once ('f.signature.sub.inc');

/**
 * manage one object
 * @author skube
 *
 */
class gObjaccLock_One {
    
    /**
     * 
     * @var $info
     *   'inf' array of info  text
     */
    private $info;
    
    /**
     * 
     * @param string $tablename
     * @param int $hasManiRole 0,1
     * @param int $action or 0
     */
    function __construct(object $sqlo, string $tablename, int $hasManiRole, $parx) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $this->tablename = $tablename;
        $this->hasManiRole = $hasManiRole;
        $this->parx = $parx;
        
        
        
        
        $this->CCT_ACCLOG_nice = tablename_nice2("CCT_ACCLOG");
        
        $this->pkname = PrimNameGet2($tablename);
        
        $this->lockObj      = new gObjaccLock();
        $this->accLogLib    = new oAccLogC();
        
        $this->ACT_name = oH_ALOG_ACT_subs::statusID2Name( $sqlo, $this->parx['action'] );
        
        debugOut('(60) parx:'.print_r($parx,1).' ACT_name:'.$this->ACT_name , $FUNCNAME, 1);
    }
    
    function setObj($objid) {
        $this->objid = $objid;
        $this->info = array();
        $this->info["inf"]=array();
    }
    
    function get_info() {
        return  $this->info;
    }
    
    /**
     * $this->qcPolAnswer
     * @param object $sqlo
     */
    function check_policy($sqlo) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        

        $polOpt = array();
        $this->accLogLib->setObject( $sqlo, $this->tablename, $this->objid);
        $answer = $this->accLogLib->checkReleasePolicy($sqlo, $this->ACT_name, $polOpt);
        
        // debugOut('(95) answer:'.print_r($answer,1) , $FUNCNAME, 1);
        
        $this->qcPolAnswer = $answer;
        if ($answer==NULL) return;
        if ($answer['ok']<0) {
            $error->set( $FUNCNAME, 1, 'Policy: '.$answer['txt']);
            return;
        } 
    }
    
    /**
     * information about one object with ID: $this->objid
     */
    function accessInfo( &$sql ) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $id        = $this->objid;
        $tablename = $this->tablename;
        $action    = $this->parx['action'];
        
        $cct_access_id = glob_elementDataGet( $sql, $tablename, $this->pkname, $id, "CCT_ACCESS_ID");
        
        $hasMani = access_hasManiRights( $sql, $cct_access_id );
        $this->info["hasMani"] = $hasMani;
        
        
        $o_rights = access_check($sql, $this->tablename, $id);
        if ( !$o_rights["entail"] ) {
            $this->info["noentail"] = 1;
        }
        
        if ( $this->info["hasMani"]>0 AND $this->info["noentail"] ) {
            // can not manipulate
            $error->set( $FUNCNAME, 1, "no 'entail'-right on object." );
            return;
        }
        
        $this->accLogLib->setObject( $sql, $tablename,  $id, $cct_access_id );
        $lastAccKey = $this->accLogLib->getLastLog($sql);
        if ($lastAccKey) $this->info["lastLog"] = $lastAccKey;
        
        
        if ( !$this->info["hasMani"] ) {
            // want add action
            // need ROLE
            if ( !$this->hasManiRole ){
                $this->info["addLogErr"] = 1;
                return;
            }
            // get last CCT_ACCLOG
            
            if ( $lastAccKey!="" AND $action!=NULL AND $lastAccKey==$action ) {
                $error->set( $FUNCNAME, 2, $this->CCT_ACCLOG_nice." entry exists" );
                return;
            }
        }
        
        
    }
    
    /**
     * LOCK + Add audit entry
     * OUTPUT: $this->info
     * @param object $sql
     */
    function accessLockOne( &$sql ) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $id   = $this->objid;
        $tablename = $this->tablename;
        $parx = $this->parx;

        
        if ( !$this->info["hasMani"] AND  $parx['action']!=NULL ) {
            // want add action
            // need ROLE
            if ( !$this->hasManiRole ){
                $error->set( $FUNCNAME, 1, "need special ROLE-right to add ACCLOG to object." );
                return;
            }
        }
        
        $sql->SetAutoCommit(false);
        
        if ( $this->info["hasMani"] ) {
            $this->lockObj->accessLockOne( $sql, $tablename, $id);
            $this->info["inf"][]="locked";
        }
        if ($parx["action"]>0 ) {
            $this->accLogLib->addLogObj( $sql, $tablename, $id, $parx );
            $this->info["inf"][]="added log";
        }
        
        if ($error->Got(READONLY))  { // on error roolback !
            $sql->Rollback();
        } else {
            $sql->Commit();
        }
        $sql->SetAutoCommit (true);
        
        $accLogInfo    = $this->accLogLib->getInfo();
        if (!empty($accLogInfo)) {
            $this->info["inf"] = array_merge($this->info["inf"],$accLogInfo);
        }
        $this->info['isWarning'] = $this->accLogLib->getInfoxWarnFlag();
        
    }
}

class gObjaccLockLiGui {
	
	private $options;
	
	/**
	 * 
	 * @param object $sqlo
	 * @param string $tablename
	 * @param int $go
	 * @param array $parx
	 * @param array $options
	 * 	'shExtraFields' : 0,1
	 */
    function __construct( &$sqlo, $tablename, $go, $parx, $options) {
    	global $error;
    	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
    	
    	$this->tablename=$tablename;
    	$this->go = $go; // "lock"
    	$this->parx = $parx;
    	$this->table_flag=0;
    	$this->options = $options;
    	
    	
    	
    	$this->QM_role_right = 'g.QM_expert';
    	$this->isQM_admin    = role_check_f ( $sqlo, $this->QM_role_right );
    	
    	$sqlopt=array();
    	$sqlopt["order"] = 1;
    	$this->sqlAfter  = get_selection_as_sql( $tablename, $sqlopt);
    	
    	$this->pkname = PrimNameGet2($tablename);
    	$this->CCT_ACCLOG_nice = tablename_nice2("CCT_ACCLOG");

    
    	if ($this->parx['action']>0) {
    	    $this->ACT_name = oH_ALOG_ACT_subs::statusID2Name( $sqlo, $this->parx['action'] );
    	} 
    	
    	$accLogGuigLib = new oAccLogGuiC($this->tablename);
    	$toolallow = $accLogGuigLib->checkToolAllowed($sqlo, 'glob.objtab.acclock.php', $this->parx['action']);
    	if ($toolallow['ok']<0) {
    		$error->set( $FUNCNAME, 1, 'Status "'.$this->ACT_name.'" nicht erlaubt: '. $toolallow['info']);
    		return;
    	}
    	
    	$this->hasManiRole = 0;
    	$hasManiRole = oAccLogStaticC::checkManiRole($sqlo, $tablename);
    	if ( $hasManiRole[0]=="execute" ) $this->hasManiRole = 1;
    	
    	if ($this->options['shExtraFields']>0) {
    		if ($this->isQM_admin!='execute') {
    			$this->options['shExtraFields'] = 0; // deny options !
    			htmlErrorBox('WARN', 'Options not allowed for you', 'Only  a user with role-right "'.$this->QM_role_right.'" can use options.');
    			echo "<br>\n";
    		}
    	}
    	
    	$goArray   = array( "0"=>"Select ".$this->CCT_ACCLOG_nice, 
    						1=>"Test", 2=>"Lock and add log" );
    	$extratext = "[<a href=\"".$_SERVER['PHP_SELF']."?t=".$this->tablename."\">Start again</a>]";
    	
    	$formPageLib = new FormPageC();
    	$formPageLib->init( $goArray, $extratext );
    	$formPageLib->goInfo( $go );
    	
    }
    

    
   
    
   
    
    function formAsk(&$sql) {
    	
    	
    	
    	$parx = $this->parx;
    	$accLogGuigLib = new oAccLogGuiC($this->tablename); // @swreq UREQ:0000704:SUBREQ:05
    	
    	$initarr   = NULL;
    	$initarr["action"]      = $_SERVER['PHP_SELF'];
    	$initarr["title"]       = "Give status info";
    	$initarr["submittitle"] = "Next &gt;&gt;";
    	$initarr["tabwidth"]    = "AUTO";
    
    	$hiddenarr = NULL;
    	$hiddenarr["t"] = $this->tablename;
    	
    	
    	$formobj = new formc($initarr, $hiddenarr, 0);
    	
    	$actionReq = 0;
    	$fields = $accLogGuigLib->formParams( $sql, $parx, $actionReq);
    	
    	if ( $this->options['shExtraFields'] ) {
    		$fields[] = $accLogGuigLib->form_getRowByType('repPolIgn');
    		$fields[] = $accLogGuigLib->form_getRowByType('qm_val');
    	}
    	
    	foreach( $fields as $fieldx) {
    		$formobj->fieldOut( $fieldx );
    	}
    	 
    
    	$closeopt=array();
    	if ( !$this->options['shExtraFields'] ) {
    		$closeopt["addObjects"]='&nbsp;&nbsp;&nbsp;&nbsp;<a href="'.$_SERVER['PHP_SELF'].'?t='.$this->tablename.'&options[shExtraFields]=1">Options</a>';
    	}
    	$formobj->close( TRUE, $closeopt );
    }
    
    function form2(&$sql) {
    	
    	
    	
    	$parx = $this->parx;
    	
    	$initarr   = NULL;
    	$initarr["action"]      = $_SERVER['PHP_SELF'];
    	$initarr["title"]       = "Lock objects and add log";
    	$initarr["submittitle"] = "Do it!";
    	$initarr["tabwidth"]    = "AUTO";
    
    	$hiddenarr = NULL;
    	$hiddenarr["t"] = $this->tablename;
    	$formobj = new formc($initarr, $hiddenarr, 1);
    	$formobj->addHiddenParx( $parx );
    	
    	$formobj->close( TRUE );
    }
    
    function _lineinfo( 
    	&$sqlo, 
        $objid, 
    	$statusflag, // 0 : ok, 1:error, 2:warn
      	$manirights, 
      	$errtxt, 
        $info_txt,
    	$lastAccLog  // ID
    	 ) {
    	
    	if ( !$this->table_flag) {
    		$this->table_flag=1;
    		$this->tabobj = new visufuncs();
    		$headOpt = array( "title" => "Object info");
    		$headx  = array ("Object", "Lock", "Last Status", "Flag",  "Error", "Info");
    		$this->tabobj->table_head($headx,  $headOpt);
    	}
    	
    	$exptxt = fObjViewC::bo_display( $sqlo, $this->tablename, $objid );
    	$statusimg="";
    	if ($statusflag==1) $statusimg="<img src=\"images/i13_err.gif\"";
    	if ($statusflag==2) $statusimg="<img src=\"images/i13_warning.gif\"";
    	$manirightsOut="";
    	if ( $manirights>0 ) {
    		$manirightsOut = "<img src=\"images/but.lock.un.gif\">";
    	} else {
    		$manirightsOut = "<img src=\"images/but.lock.in.gif\">";
    	}
    	$lastAccLogTxt="";
    	if ($lastAccLog) {
    		$lastAccLogTxt = obj_nice_name( $sqlo, "H_ALOG_ACT", $lastAccLog );
    	}
    	$dataArr = array( $exptxt, $manirightsOut, $lastAccLogTxt,  $statusimg,  $errtxt, $info_txt );
    	$this->tabobj->table_row ($dataArr);
    	
    }
    
    /**
     * lock/add status for the set of objects now
     */
    function doAll( &$sqlo, &$sqlo2 ) {
    	// do all
    	global $error;
    	$FUNCNAME= "doAll";		
    	$loopError = 0;
    	
    	$one_obj_lib = new gObjaccLock_One( $sqlo, $this->tablename, $this->hasManiRole, $this->parx );

    	$sqlsLoop = "SELECT x.".$this->pkname." FROM ".$this->sqlAfter;
    	$sqlo2->query($sqlsLoop);
    	
    	while ( $sqlo2->ReadRow() ) {
    		
    		$loopError=0;
    		$error_txt="";
    		$objid   = $sqlo2->RowData[0];
    		
    		$one_obj_lib->setObj($objid); 
    		$one_obj_lib->accessInfo( $sqlo );
 
    		if ( !$error->Got(READONLY) ) {
    		    
    		    if  ($this->go>1) $one_obj_lib->accessLockOne( $sqlo );
    		    else {
    		        $one_obj_lib->check_policy($sqlo);
    		    }
    		}
    		$info_loop = $one_obj_lib->get_info();
    		
    		if ($error->Got(READONLY))  {
         		$errLast   = $error->getLast();
         		$error_txt = $errLast->text;
    			$error->reset();
    			//$errflag = 1;
    			$loopError=1;
    		} else {
    			if ($info_loop["addLogErr"]>0) {
    				$loopError = 2;
    				$error_txt = "need special ROLE to add an ".$this->CCT_ACCLOG_nice.".";
    			}
    		}
    		
    		$this->_lineinfo( $sqlo, 
    			$objid, $loopError, $info_loop["hasMani"], $error_txt, 
       			implode('; ',$info_loop["inf"]), $info_loop["lastLog"]);
    	}
    	
    	if ( $this->table_flag) $this->tabobj->table_close();
    	
    	if ($loopError) {
    		 $error->set( $FUNCNAME, 1, "Errors occured." );
    	}
    }
    
    function paramInfo(&$sqlo) {
    	$parx = $this->parx;
    	
    	if ($parx["action"]!="") {
    		$actname = obj_nice_name( $sqlo, "H_ALOG_ACT", $parx["action"] );
    	}
    	
    	$tabobj = new visufuncs();
    	$dataArr= NULL;
    	$dataArr[] = array($this->CCT_ACCLOG_nice." entry" , $actname);
    	
    	$cssopt = array( 1=>"style=\"font-weight:bold;\"");
    	$headOpt = array( "title" => "Parameters", "headNoShow" =>1, "colopt"=>$cssopt );
    	$headx   = array ("Key", "Val");
    	$tabobj->table_out2($headx, $dataArr,  $headOpt);
    	echo "<br>";
    }
    
    /**
     * check initial parameters
     * - throws errors
     */
    function paramCheck(&$sqlo) {
    	global $error;
    	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
    	$parx=$this->parx;
    	
    	if (!$parx['action']) {
    		$error->set( $FUNCNAME, 3, 'Need a status.' );
    		return;
    	}
    	
    	// @swreq UREQ:0000704:add audit status;SUBREQ: 11.200(a)(1)(i)
    	$fSignLib = new fSignatureSub();
    	$needpw   = $fSignLib->checkSignNeed($sqlo, $this->tablename);
    	if ($needpw or $parx['signpw']!=NULL) {
    		if ($parx['signpw']==NULL) {
    			$error->set( $FUNCNAME, 4, 'Need a password.' );
    		 	return;
    		}
    		$pwOK = $fSignLib->checkPW($sqlo, $parx['signpw']); // password o.k. ?
    		if ($pwOK<=0) {
    			$error->set( $FUNCNAME, 5, 'Wrong password.' );
    		 	return;
    		}
    	}
    }

}

// --------------------------------------------------- 
global $error;

$error = & ErrorHandler::get();
$sqlo   = logon2( $_SERVER['PHP_SELF'] ); // URL-link for the first db-login; e.g. for object ($_SERVER['PHP_SELF']."?id=".$id)
$sqlo2  = logon2( );
if ($error->printLast()) htmlFoot();
//$varcol = & Varcols::get();

$t    = $_REQUEST["t"];
$parx = $_REQUEST["parx"];
$go   = $_REQUEST["go"];
$options = $_REQUEST["options"];

$tablename			 = $t;
//$i_tableNiceName 	 = tablename_nice2($tablename);


$title       		 = "[LockObjects] Lock a set of objects and add status";
$title_sh       	  = "LockObjects";
$infoarr = NULL; 
$infoarr["title"] = $title;
$infoarr["title_sh"] = $title_sh;
$infoarr["scriptID"] = "glob.objtab.acclock";
$infoarr["form_type"]= "list";

$infoarr["obj_name"] = $tablename;
$infoarr["obj_cnt"]  = 1;
$infoarr["icon"]     = "images/but.lock.in.gif";
$infoarr["locrow"]   =  array( array("glob.objtab.access.php?t=".$tablename, "AccessInfo") );

$pagelib = new gHtmlHead();
$headarr = $pagelib->startPage($sqlo, $infoarr);
if (!$go) {
	echo "<i>Info: This tool removes all access-rights from this SELECTION of OBJECTS and sets a new audit-status to the audit-trail.</i><br>";
	echo 'Options: A user with role-right "g.QM_expert" can use "Options" to ignore the release-policy.<br>';
}
echo "<ul>\n";

$isbo = cct_access_has2($tablename);
if ( !$isbo ) {
	 htmlFoot("INFO", "This function only works for business objects!");
}


$copt = array ("elemNum" => $headarr["obj_cnt"] );
$listVisuObj = new visu_listC();
$listVisuObj->exitByTest(  $sqlo, $tablename, $copt );

$mainLib = new gObjaccLockLiGui( $sqlo, $tablename, $go, $parx, $options);
$pagelib->chkErrStop();
echo "<br>";

if (!$go) {
	$mainLib->formAsk($sqlo, $sqlo2);
	echo "</ul>\n";
	htmlFoot('<hr>');
}

$mainLib->paramInfo($sqlo);

$mainLib->paramCheck($sqlo);
if ($error->Got(READONLY))  {
	$error->printAll();
	return;
}

if ($go==1) {
	$mainLib->form2($sqlo);
}

$mainLib->doAll($sqlo, $sqlo2);
echo "<br>";
$error->printAll();
echo "</ul>\n";
htmlFoot('<hr>');
