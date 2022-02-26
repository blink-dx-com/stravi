<?php

require_once('f.rider.inc');
require_once ("f.head_proj.inc");

/**
 * common sub functions for compare GUI (all compare forms)
   used by: 
    obj.concrete_proto.m_comgui.php
	obj.concrete_proto.m_comp2e.php
	obj.concrete_proto.m_comp.php
	obj.exp.protocol_comp.php
 * @package obj.concrete_proto.cmpgui.inc 
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class oPROTO_cmpgui{
    
    private $sess_cache=array();

    function init(
    	$proj_id, 
    	$tablename
    	) {
    
    	$this->proj_id   = $proj_id;
    	$this->tablename = $tablename;
    	if ($proj_id AND $tablename=="") $this->tablename = "EXP";
    	$this->sqlAfter = NULL;
    }
    
    /**
     * - save variables in $_SESSION['s_formState']
    	- if exists, get them from there before
     * @param object $sql
     * @param int $step_no
     * @param int  $aprotoid
     * @param int $first_obj_id
     * @return array
     */
    function anaFormState( $step_no, $aprotoid) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        debugOut("(43) IN: step_no:$step_no, aprotoid:$aprotoid", $FUNCNAME, 1);
        debugOut('(44) session:'.print_r($_SESSION['s_formState']["oPROTOcmp"],1), $FUNCNAME, 1);
        
    	do {
    		if (!is_array($_SESSION['s_formState']["oPROTOcmp"])) {
    			break; // not exist
    		}
    		
    		$this->sess_cache = $_SESSION['s_formState']["oPROTOcmp"];
    		if ($this->sess_cache["tablename"] != $this->tablename) {
    			break; // other table type
    		}
    		
    		if (!$step_no)  $step_no  = $this->sess_cache["step_no"];
    		if (!$aprotoid) $aprotoid = $this->sess_cache["aprotoid"];
    		
    	} while (0);

    	$this->sess_cache = NULL;
    	$this->sess_cache = array ( "tablename"=>$this->tablename, "step_no"=>$step_no, "aprotoid"=>$aprotoid );
    	
    	$_SESSION['s_formState']["oPROTOcmp"] =  $this->sess_cache;
    	
    	debugOut('(64) savearr:'.print_r($this->sess_cache,1), $FUNCNAME, 1);
    	
    	return array($step_no, $aprotoid);
    }
    
    function set_cache_val($key,$val) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        $this->sess_cache[$key]=$val;
        $_SESSION['s_formState']["oPROTOcmp"] =  $this->sess_cache;
        
        debugOut('(73) sess_cache:'.print_r($this->sess_cache,1), $FUNCNAME, 1);
    }
    
    function showRider( 
    	$key // "wizard", "overview", "all", "selected"
    	) {
    	$urlparams ="tablename=".$this->tablename."&proj_id=".$this->proj_id;
    	
    	$overviewURL = "";
    	switch ($this->tablename) {
    	    case "CONCRETE_SUBST":
    	        $overviewURL = "p.php?mod=DEF/o.CONCRETE_SUBST.comp&proj_id=".$this->proj_id;
    	        break;
    		case "W_WAFER":
    			$overviewURL = "obj.w_wafer.list_pcomp.php?proj_id=".$this->proj_id;
    			break;
    		case "EXP":
    			$overviewURL = "obj.exp.protocol_comp.php?proj_id=".$this->proj_id;
    			break;
    	}
    	
    	$xmodes = NULL;
    	$xmodes["wizard"] 	=  array( "Home"		 , "obj.concrete_proto.m_comgui.php?". $urlparams );
    	if ($overviewURL) $xmodes["overview"] = array( "Overview"		 , $overviewURL );
    	$xmodes["all"] 		= array( "ALL steps"		   , "obj.concrete_proto.m_comp.php?action=all_steps&". $urlparams );
    	$xmodes["selected"] = array( "SELECTED steps"      , "obj.concrete_proto.m_comp2e.php?selsteps=1&". $urlparams );
    	$xmodes["sea"]      = array( "Search step params"  , "obj.concrete_proto.m_comp2e.php?parx[action]=protocomp2&".$urlparams );
    	
    	$addcell   = 'Other: [<a href="obj.exp.imp_sample.php?tablename='.$this->tablename.'">Import step data</a>]';
    	
    	$riderObj = new fRiderC();
    	$wizopt = array("bgcolor"=>"#CCCCFF", "addcell"=>$addcell );
    	$riderObj->riderShow( $xmodes, $key, "&nbsp;Views:", $wizopt);
    	
    	
    }
    
    function setSqlAfter($sqlAfter) {
    	$this->sqlAfter  = $sqlAfter;
    }
    
    /**
     * 
     * @param object $sql
     * @param string $title
     * @param array  $obj_arr -- can be empty ...
     * @param array  $infoarr
     * @param string $key
     * @param array $option // "norider" = 0,1
     */
    function showHeader( object &$sql, string $title, $obj_arr, $infoarr, string $key, $option=NULL ) {
    		
    	// show script header
    	$this->sqlAfter = NULL;
    	$infoarr["help_url"] = "o.PROTOCOL.compare.html"; 
    	$pageHeadListObj = new gHtmlHeadProjC();
    	
    	$answerArr = $pageHeadListObj->showHead( $sql, $this->tablename, $this->proj_id, $obj_arr, $title, $infoarr);
    	$this->sqlAfter  = $answerArr["sqlAfter"];
    	if ($option["norider"] != 1) $this->showRider($key);
    
    }
    
    function getOneObject( &$sql ) {
    	// REQUIRES: call of showHeader()
    	$sql->query("SELECT ".$this->tablename."_ID FROM ".$this->sqlAfter); 
    	$sql->ReadRow();
    	$tmpobj = $sql->RowData[0];
    	return ($tmpobj);
    }
    
    function get_step_from_proto(&$sql, $tablename, $objid, $aprotoid) {
    	// FUNCTION: get PROTO_NO from A_PROTO_ID
    	$sqls= "select e.STEP_NR from W_WAFER_STEP e, CONCRETE_PROTO c ".
    			" where e.W_WAFER_ID=".$objid." AND c.ABSTRACT_PROTO_ID=".$aprotoid. " AND e.CONCRETE_PROTO_id=c.CONCRETE_PROTO_id";
    	$sql->query("$sqls");
    	$sql->ReadRow();
    	$step_nr = $sql->RowData[0];
    	return ($step_nr);
    }

}