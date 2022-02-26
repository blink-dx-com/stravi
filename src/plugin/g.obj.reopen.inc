<?php
/**
 * REOPEN object
 * @package g.obj.reopen.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   string $t tablename
 * @param   int $id object id
 $parx      : the audit-log parameters  (acclock_parx_STRUCT)
 
 'notes'   : notes
 'repPolIgn' : 0,1 ignore release policy ???
 'qm_val'  : QM-value: 0.5 , 1
 'signpw'  : signin password; needed if signing is required
 'mo.CERT_FLAG' : (optional) [1],2,3,4 Life cycle flag of MOTHER object
 
 
 */ 

require_once ('access_lock.inc');
require_once ('o.CCT_ACCLOG.gui.inc');
require_once ('f.signature.sub.inc');
require_once ('lev1/o.CCT_ACCESS.reopen.inc');

class gObjaccReopenGui {
    
    var $cct_access_id; // access_id of object
    var $action; /**
    ['lock'], : lock object + add action
    'add'     : only add an action, no LOCK-operation : NOT allowed for 'released'
    'reopen'  : reopen
    */
    var $ACT_name; // name of FORM parameter of H_ALOG_ACT_ID
    
    function __construct(&$sqlo, $tablename, $id, $_mod, $parx, $ignore, $backurl) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $this->_mod=$_mod;
        
       
        $this->parx    = $parx;
        $this->tablename=$tablename;
        $this->id      = $id;
        $this->ignore  = $ignore;
        $this->backurl = $backurl;
        
        $this->userHasAccLogRights = 0;
        $pk = PrimNameGet2($tablename);
        
        $this->accLogLib    = new oAccLogC();
        
        $this->infoIsWarning = 0;
        $this->ACT_name = 'reopen';

        $this->cct_access_id = glob_elementDataGet( $sqlo, $tablename, $pk, $id, 'CCT_ACCESS_ID');
        $this->QM_role_right = 'g.QM_expert';
        $this->isQM_admin    = role_check_f ( $sqlo, $this->QM_role_right );
        $this->adminErrTxt   = 'You have not the role-right "'.$this->QM_role_right.'" .';
        
        $this->is_table_admin = role_admin_check ( $sqlo, $tablename );
    }
    
    function userHasAccLogRights() {
        return 0; // @todo recode later
    }
    
    function isTableAdmin() {
        return $this->is_table_admin;
    }
    
    /**
     * check special STATUS-policies of the company
     * @param $sqlo
     */
    function checkStatusPol(&$sqlo) {
        
        $statusname = $this->ACT_name;
        $polOpt = array( 'mo.CERT_FLAG'=> $this->parx['mo.CERT_FLAG'] ); // @todo do we need that ?
        
        $this->accLogLib->setObject( $sqlo, $this->tablename, $this->id);
        $answer = $this->accLogLib->checkReleasePolicy($sqlo, $statusname, $polOpt);
        $this->qcPolAnswer = $answer;
        if ($answer==NULL) return; // no policy
        if ($answer['ok']<0) {
            htmlInfoBox( "Status Warning", $answer['txt'], "", "WARN" );
            echo '<br />';
        }
        
    }
    
    /*
     * Builds the form used to reopen a locked audit-trail object.
     */
    function formReopen(&$sqlo, $sqlo2, $parx) {
        $accLogGuigLib = new oAccLogGuiC();
        $accLogGuigLib->setObject( $sqlo, $this->tablename, $this->id );
        
        $initarr   = NULL;
        $initarr["action"]      = $_SERVER['PHP_SELF'];
        $initarr["title"]       = "Reopen and add reason";
        $initarr["submittitle"] = "Reopen and add reason";
        $initarr["tabwidth"]    = "AUTO";
        
        $hiddenarr = NULL;
        $hiddenarr["t"] = $this->tablename;
        $hiddenarr["id"]= $this->id;
        $hiddenarr["mod"]     = $this->_mod;
        $hiddenarr["backurl"]    = $this->backurl;

        $formobj = new formc($initarr, $hiddenarr, 0);
        
        $fieldx = array (
            "title" => "Reopen reason:",
            "name"  => "notes",
            "req"   => 1,
            "object"=> "textarea",
            "colspan"=>2,
            "inits" => array("cols"=>60),
            "val"   => $parx["notes"]
        );
        
        $formobj->fieldOut( $fieldx );
        
        if ($parx['repPolIgn']>0) {
            $fieldx= array (
                "title" => "Ignore Policy", "name"  => "repPolIgn", "val"   => 0,
                "object"=> "checkbox",  "notes" => "Ignore problems of the release policy?" );
            $formobj->fieldOut( $fieldx );
        }
        
        $formobj->close( TRUE );
    }
    
    /**
     * - reopen object
     * - entail-right was checked before ...
     * @global
     * 		$this->cct_access_id
     * 		$this->tablename
     * 		$this->id
     */
    function doReopen(&$sqlo, $parx) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        echo  "... reopen object.<br>\n";
        
        $tablename     = $this->tablename;
        $cct_access_id = $this->cct_access_id;
        
        $reopenLib = new gObjAccReopen($sqlo);
        $reopenLib->doReopen($sqlo, $cct_access_id, $tablename, $this->id, $parx);
        
        $this->accLogInfo    = $reopenLib->getInfo();
        $this->infoIsWarning = $reopenLib->getInfoxWarnFlag();
    }
    
    /**
     * analyse warning; show warning; return 0,1
     * @return int warning exists
     * 	0: nothing
     *  1: info exists
     *  2: warning, errro
     */
    function showWarning() {
        return oAccLogGuiC::show_Warning($this->accLogInfo, $this->infoIsWarning );
    }

}


class g_obj_reopen_XPL extends gPlugin {
	
	
	function register() {
		
		$title		= 'Reopen an object';
		
		$this->infoarr			 = NULL;
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'obj'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $_REQUEST['t'];
		$this->infoarr['obj_id']   = $_REQUEST['id'];
		$this->infoarr['checkid']  = 1;
		$this->infoarr["help_url"] = 'glob.obj.acclock.html';
		
		$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('read'), 'obj'=>array('read') )
			);
	}
	

	
	function startMain() {
	    
		global $error;
		$FUNCNAME='startMain';
		
		$sqlo = &$this->_sqlo;
		$sqlo2= &$this->_plug_regSql2();
		
		$tablename = $_REQUEST['t'];
		$id        = $_REQUEST['id'];
		$go        = $_REQUEST['go'];
		$parx      = $_REQUEST['parx'];
		$backurl   = $_REQUEST['backurl'];
		$ignore= 0; // @todo later

		$i_tableNiceName=tablename_nice2($tablename);
		$isbo = cct_access_has2($tablename);
		if ( !$isbo ) {
		    $this->htmlFoot("INFO", "This function only works for business objects!");
		}
		
		$mainLib = new gObjaccReopenGui($sqlo, $tablename, $id, $this->_mod, $parx, $ignore, $backurl);
		
		$o_rights = access_check($sqlo, $tablename, $id);
		if ( !$o_rights["entail"] and !$mainLib->isTableAdmin() ) { //TBD: !!!
		    $this->htmlFoot("ERROR", "you need entail permissions on ".$i_tableNiceName." or you must be a table admin!");
		}
		
		
	    if (!$go) {
	        $mainLib->checkStatusPol($sqlo);
	        $mainLib->formReopen($sqlo, $sqlo2, $parx);
	        return;
	    }
	    
	    $mainLib->doReopen($sqlo, $parx);
	    
	    if ($error->Got(CCT_ERROR_READONLY))  {
	        
	        if ( $error->Got( READONLY,'addLogSub',22 ) ) { // check release-policy error
	            
	            $error->set($FUNCNAME, 100, 'object '.$tablename.':'.$id); // add the object identification to error-log
	            $error->printAll();
	            echo '<br />';
	            $qmusers = $mainLib->getQM_users($sqlo);
	            $infoTxt = 'Only a QM-manager (role-right "'.$mainLib->QM_role_right.
	               '") can reopen this object! He can ignore the policy.<br />'.
	               " Following users can help: ".$qmusers;
	            
	            
	            htmlInfoBox( "Info", $infoTxt, "", "INFO" );
	            echo '<br />';
	            if ( $mainLib->isQM_admin ) {
	                $parx['repPolIgn']=1;
	                $mainLib->formReopen($sqlo, $sqlo2, $parx);
	            }
	            return;
	        }
	    }
	    
	    $hasWarning = $mainLib->showWarning();
	    if ($hasWarning>1) {
	        return;
	    }
	    
	    $forwardDelay=0;
	    if ($hasWarning==1) {
	        $forwardDelay = 1000; // delay in milliseconds
	    }
	    
	    if ($error->Got(READONLY))  {
	        return;
	    }
	    
	    if ($backurl!=NULL) {
	        $backurl = urldecode($backurl);
	    } else {
	        $backurl = "edit.tmpl.php?t=".$tablename."&id=".$id;
	    }
	    
	    $this->_html->pageForward($backurl, "Back to object", $forwardDelay);

	}
}
