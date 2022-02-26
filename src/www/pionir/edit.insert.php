<?php
/**
 *  - show GUI to CREATE one object (BO or ASSOC) ($go=0)
 *  - do the insert ($go=1)
 *  - GUI is allways in editmode !!!
 *  
 * @todo show input-argus again ON ERROR !!!   
 * @package edit.insert.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq   SREQ:0001494: g > edit.insert.php > GUI to insert a new object (BO or ASSOC)
 * @param 
 * 		$tablename ( UPPER CASE ) 
 * 		$go  control the main action
 * 			0: prepare: show GUI
 * 			1: do the insert
     optional:
        $argu     array[COL_NAME] = val ( arguments ready for insert/update  )
        $xargu    - extra arguments despite this TABLE, 
        			[CLASS] = extra_class_id 
        $arguobj  ( arguments for EXTRA OBJECT )
	        	  info: CCT_ACCESS_ID will not be passed at the edit form, even for root !!!   
        $argu_xtra  array[COL_NAME] = val ( preselected data for insert-form; on $go=0 )
        $remotepos ( this edit window was called by OPENER and sends back the PRIMARY KEY to valueInputRemote )
                   - variable must be saved in the form to inherit it to the INSERT PAGE (input hidden name=remotepos ...)
        $insertToProjId ( >0 : if action=="insert" and element is BO, add it to project with ID $insertToProjId
        $prepAutoInc   - name of column, which will be auto-incremented before showing insert-form
        			   - e.g. 	$tablename='A_PROBE_ON_ARRAY',  $prepAutoInc=POS
        $options : array of options
        		['presel'] (array of columns) = ABSTRACT_SUBST_ID
        		['backurl'] urlencoded URL to jump back after insert
        $newpx		    - extra parameters
	       'worflow_id' : ID of workflow: overrules all other workflow settings (see objCreaWiz_act::_post_crea_actions)
 */

session_start(); 


require_once ('reqnormal.inc');
require_once ('gui/edit.insert.inc');

class fEditInsPage {
	
	var $CLASSNAME='fEditInsPage';

    function showHead(&$sql, $tablename, $insertToProjId) {
    	
    	$this->tablename = $tablename;
    	$objformXlib = new fObjFormSub();
    	$tablename_nice = tablename_nice2($tablename);
    	
    	if ($insertToProjId>0) {
    		$backurl = "edit.tmpl.php?t=PROJ&id=".$insertToProjId;
    		$backtxt = "project";
    	}
    	else {
    		$backurl = "view.tmpl.php?t=".$tablename;
    		$backtxt = "list";
    	}
    	$title = "Create a new ".$tablename_nice;
    	$infoarr			 = NULL;
    	$infoarr["scriptID"] = "";
    	$infoarr["title"]    = $title;
    	$infoarr["form_type"]= "tool";
    	$infoarr["obj_name"] = $tablename;
    	$infoarr['help_url'] = "o.".$tablename.".html";
    	$infoarr["locrow"]   = array( array($backurl, $backtxt) );
    	$infoarr["css"]      = $objformXlib->datatab_css();
    	$pagelib = new gHtmlHead();
    	$headarr = $pagelib->startPage($sql, $infoarr);
    	
    	?>
    	<script language="JavaScript">
    	    <!--
    	    function submitCheckModal( remotepos, valuex, valtext) {        
    	        if (window.opener!=null) { 
    	            window.opener.inputRemote( remotepos, valuex, valtext); 
    	            window.close();
    	        }                
    	    }
    	    
    	    //-->
    	    </script> 
    	<?
    	js_formAll(); 
    
    }
    
    /**
     * - do autoIncrement of column $column for an ASSOC-table
     * - column should have PRIMARY_KEY>0
     * @param $sql
     * @param array $argu as reference, will be modified
     * @param string $column the column name
     * @param int $id ID of mother
     * @return $argu as input reference
     */
    function prepareAutoInc( &$sqlo, &$argu, $column ) {
    	global $error;
    	$FUNCNAME= $this->CLASSNAME.':prepareAutoInc';
    
    	$tablename = $this->tablename;
    	$pkname    = PrimNameGet2($tablename);
    	
    	$id = $argu[$pkname];
    	if ( !$id ) {
    		$error->set( $FUNCNAME, 1, 'need mother-id for this option.' );
    		return;
    	}	
    	
    	$feats = colFeaturesGet( $sqlo,  $tablename, $column );
    	if ($feats['PRIMARY_KEY']<=1) {
    		$error->set( $FUNCNAME, 2, 'auto-inc column '.$column.' must be of definition PRIMARY_KEY>1' );
    		return;
    	}
    	
    	// get last value of $column
    	
    	$sqlsel = 'max('.$column.') from '.$tablename.' where '.$pkname.'='.$id;
    	$sqlo->Quesel($sqlsel);
    	$sqlo->ReadRow();
    	$lastval = $sqlo->RowData[0];
    	if (!$lastval) $lastval=0;
    	$newVal = $lastval+1;
    	
    	$argu[$column]=$newVal;
    }
    
    function pend() {
    	?>
    	<br><br>
    	<hr noshade size=1>
    	<?
    	htmlFoot();
    
    }

}

// -------------------------------------

$argu      = $_REQUEST['argu'];
$argu_xtra = $_REQUEST['argu_xtra'];
$arguobj   = $_REQUEST['arguobj'];
$go        = $_REQUEST['go'];
$insertToProjId = $_REQUEST['insertToProjId'];
$options   = $_REQUEST['options'];
$prepAutoInc    = $_REQUEST['prepAutoInc'];
$remotepos = $_REQUEST['remotepos'];
$tablename = $_REQUEST['tablename'];
$xargu     = $_REQUEST['xargu'];
$newpx     = $_REQUEST['newpx'];

if ($tablename=="PROJ") { // TBD: check this ???
	include ("obj.proj.xsubst.inc");
	return 0;
}


$varcol= & Varcols::get();
$error = & ErrorHandler::get();
$sql   = logon2();
$sql2  = logon2();


$pageLib2 = new fEditInsPage();
$pageLib2->showHead( $sql, $tablename, $insertToProjId);

if ($prepAutoInc!=NULL) {
	$pageLib2->prepareAutoInc($sql, $argu_xtra, $prepAutoInc);
}
if ( !$go ) $argu = $argu_xtra;

$mainlib = new fEditInsLibC($tablename, $insertToProjId);
$mainlib->setArgus( $arguobj, $xargu, $argu, NULL, $newpx );
$mainlib->initChecks($sql);
if ( $error->printAll() ) {
	htmlFoot();
}


 /* do INSERT if $argu has values */
if ( $go and $mainlib->hasArgus() ) {              
    $mainlib->doInsert( $sql, $sql2); 
	if ( $error->printAllEasy() ) {
		// show form again
	} else {    
		$tmpurl=NULL;
		if ($options['backurl']!=NULL ) $tmpurl = urldecode($options['backurl']);
		$mainlib->pageForward( $sql, $remotepos, $tmpurl );
		exit;
	}
}  

$shOpt = NULL;
if ($remotepos!=NULL) $shOpt['remotepos'] = $remotepos;
$shOpt['extraHidden'] = array('go'=> 1 );
if ($newpx['worflow_id']) {
    $shOpt['extraHidden']['newpx[worflow_id]'] = $newpx['worflow_id'];
}

if ( $options['presel']!=NULL ) {
	$shOpt['presel'] = $options['presel'];
}

$mainlib->showForm( $sql, $sql2, $shOpt);

$pageLib2->pend();
