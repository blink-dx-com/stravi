<?php
/**
 * add/delete substance to/from container
 * TBD: waht about plugin/o.CONTAINER.modone.inc ???
 * @package obj.container.addsub.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq UREQ:0000686: DB.CONCRETE_SUBST > aliquot management 
 * @swreq UREQ:0001107: o.CONTAINER > add one substance to container (MOTHER-Issue)
 * @param $parx
    [CONTAINER_ID]  CONTAINER_ID  [REQUIRED]
	[substid]  : ID of substance
	[POS]      : AUTO or number
	[POS_ALIAS]  : OPTIONAL string
	[QUANTITY] : AUTO or number; quantity of substance
	[action]  [REQUIRED]
		"ADD",: add subst
		"DEL" : remove subst from container
		"DEL_MANY": remove many aliquots of same SUC
		
	['addForce'] = 0,1 : add, also if substance already added
	['withAli']  = 0,1 : start aliquot management, start the form to ask for more info
	['aliNum']   = INT : number of aliquots
	["newBox"]   = [0],1
	[backurl]
	['alias_arr'] array of alias positions for Aliquots [OPTIONAL]
	['alias_start'] string of start alias for aliquots [OPTIONAL]
	'accept' -- needed for action "REOPEN_SUC"
	@param $go : 
		0,
		1
		2 : add aliquots
    @param $pos array [POS] = 1 // for action=DEL_MANY
 */


session_start(); 


require_once ("reqnormal.inc"); // includes all normal *.inc files
require_once ("javascript.inc" );
require_once ('func_form.inc');
require_once ('insert.inc');	
require_once ("f.objview.inc");	
require_once ("class.history.inc");
require_once ('f.msgboxes.inc'); 
require_once ("o.proj.profile.inc"); 

require_once ("o.CONTAINER.mod.inc");
require_once 'o.CONTAINER.subs.inc';
require_once 'o.ABS_CONTAINER.subs.inc';
require_once 'o.S_VARIO.subs.inc';
require_once ('o.PROJ.addelems.inc');
require_once ('o.PROJ.subs.inc');
require_once 'o.MORDER.subs.inc';
require_once 'o.CCT_ACCLOG.subs.inc';
require_once ('o.H_ALOG_ACT.subs.inc');
require_once ('o.CONCRETE_SUBST.proto.inc');




require_once ("lev1/o.CCT_ACCESS.reopen.inc");


/**
 * helper class
 * @author skube
 *
 */
class oCONT_MOD_helper {
    
    function __construct($container_id, $suc_id) {
        $this->container_id = $container_id;
        $this->suc_id = $suc_id;
    }
    
    /**
     * @swreq USN-004 o.CONTAINER > erlaube hinzufügen einer MAC, wenn MORDER: Status = abgeschlossen
     * @param object $sqlo
     * @param string $por_status
     * @param string $keyx
     */
    private function _check_MORDER($sqlo, $por_status, $keyx) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        debugOut('(87) @swreq USN-004 o.CONTAINER > erlaube hinzufügen einer MAC, wenn MORDER: Status = abgeschlossen', $FUNCNAME, 1);
        $text1 = tablename_nice2('CONTAINER').'-Configuration';
        
        $status_id_x =oH_ALOG_ACT_subs::getH_ALOG_ACT_ID( $sqlo, 'MORDER', $por_status );
        if (!$status_id_x) {
            cMsgbox::showBox("warning", tablename_nice2('CONTAINER'). "-misconfiguration: VARIO:".$keyx.
                ': bad status-name: "'.$por_status.'"');
            echo "<br>";
        }
        
        $id = $this->suc_id;
        // analyse production workflow ...
        $morder_id = oMORDER_subs::SUC_has_order($sqlo, $id);
        if (!$morder_id) {
            $error->set( $FUNCNAME, 1, $text1.': MAC must be part of a '.tablename_nice2('MORDER').'!' );
            return;
        }
        
        // check status
        
        $out=0;
        $accLogLib   = new oAccLogC();
        $accLogLib->setObject( $sqlo, 'MORDER', $morder_id );
        $now_status_id = $accLogLib->getLastLog($sqlo);
        
        if ($status_id_x!=$now_status_id) {
            $error->set( $FUNCNAME, 2, $text1.': the '.tablename_nice2('MORDER').' of the MAC must have status "'.
                $por_status.'" to add the MAC to this container!' );
            return;
        }
    }
    
    private function _ana_PRC($sqlo, $pra_id, $status_id_x, $prc_status, $text1) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
      
        $id = $this->suc_id;
        $pra_table='ABSTRACT_PROTO';
        $pra_nice = obj_nice_name ( $sqlo, $pra_table, $pra_id );

        $suc_prc_lib = new oCONCRETE_SUBST_proto($id);
        $prc_id      = $suc_prc_lib->get_c_proto_by_pra($sqlo, $pra_id);
        if (!$prc_id) {
            $error->set( $FUNCNAME, 8, 'No '.tablename_nice2('CONCRETE_PROTO').' found for '.tablename_nice2($pra_table).
                ', ID:'.$pra_id.' Name: '.$pra_nice . '<br>(Check defined by '.$text1.')');
            return;
        }
        $accLogLib   = new oAccLogC();
        $accLogLib->setObject( $sqlo, 'CONCRETE_PROTO', $prc_id );
        $now_status_id = $accLogLib->getLastLog($sqlo);
        
        if ($status_id_x!=$now_status_id) {
            $error->set( $FUNCNAME, 20, 'The '.tablename_nice2('CONCRETE_PROTO').' (Type:'.$pra_nice.') of the MAC must have status "'.
                $prc_status.'" to add the MAC to this container!<br>(Check defined by '.$text1.')' );
            return;
        }
    }
    
    /**
     * @swreq P3-USN-001: o.CONTAINER > erlaube hinzufügen einer MAC, wenn MAC:Protocol-Status = XXX
     * @param object $sqlo
     * @param string $por_status
     * @param string $keyx
     */
    private function _check_PROTO_status($sqlo, $mac_protos, $keyx_in) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        debugOut('(156) @swreq P3-USN-001: o.CONTAINER > '.
            'erlaube hinzufuegen einer MAC, wenn MAC:Protocol-Status = XXX', $FUNCNAME, 1);
        
        $text1 = tablename_nice2('CONTAINER').'-Configuration (VARIO:'.$keyx_in.')';
        
        $mac_protos=trim($mac_protos);
        $mac_prot_arr_RAW = explode(',',$mac_protos);
        
        $pra_table='ABSTRACT_PROTO';
        
        // analyse PRAs
        $mac_prot_arr=array();
        foreach($mac_prot_arr_RAW as $pra_id_RAW) {
            $pra_id = intval($pra_id_RAW);
            if ($pra_id<=0) {
                $error->set( $FUNCNAME, 2, $text1.': the value "'.$pra_id_RAW.'" '.tablename_nice2($pra_table).' is not valid or cannot be found!' );
                return;
            }
            if ( !gObject_exists ($sqlo, $pra_table, $pra_id) ) {
                $error->set( $FUNCNAME, 4, $text1.': the '.tablename_nice2($pra_table).' ID:'.$pra_id.' cannot be found!' );
                return;
            }
            $mac_prot_arr[]=$pra_id;
        }
        
        
        $keyx = 'MAC.insert.Proto.status';
        $prc_status = oS_VARIO_sub::getValByTabKey($sqlo, 'CONTAINER', $this->container_id , $keyx);
        $status_id_x= oH_ALOG_ACT_subs::getH_ALOG_ACT_ID( $sqlo, 'CONCRETE_PROTO', $prc_status );
        if (!$status_id_x) {
            $error->set( $FUNCNAME, 6, $text1.": VARIO:".$keyx.': bad status-name: "'.$prc_status.'"');
            return;
        }
        
        // search at least ONE valid PRC
        $ok=0;
        $pras_nice=array();
        foreach( $mac_prot_arr as $pra_id ) {
            
            $this->_ana_PRC($sqlo, $pra_id, $status_id_x, $prc_status, $text1);
            $pra_nice = obj_nice_name ( $sqlo, $pra_table, $pra_id );
            $pras_nice[]= '"'.$pra_nice.'"';
            if ($error->Got(READONLY))  {
                $errLast   = $error->getLast();
                $error_txt = $errLast->text;
                $error->reset();
            } else {
                $ok=1;
            }
        }
        if (!$ok) {
            $error->set( $FUNCNAME, 20, 'To add the MAC to this container, the MAC needs: '.tablename_nice2('CONCRETE_PROTO').
                ' (Type:'.implode(' or ',$pras_nice).') with status "<b>'.
                $prc_status.'</b>"!<br>(Check defined by '.$text1.')' );
            return;
        }
        
        
        
    }
    
    /**
     * check, if POS or QUANTITY are valid
     * @return array
     *   'add_single_ask_form' : 0,1
     *   'force_add' : 0,1
     */
    function addCheck_more(&$sqlo) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;

        
        //@swreq USN-004 o.CONTAINER > erlaube hinzufügen einer MAC, wenn MORDER: Status = abgeschlossen
        $vario_lib = new oS_VARIO_sub('CONTAINER');
        $keyx = 'MAC.insert.MORDER.status';
        $por_status = $vario_lib->getValByKey($sqlo, $this->container_id , $keyx);
        if ($por_status!=NULL) {
            $this->_check_MORDER($sqlo, $por_status, $keyx);
        }
        
        $keyx = 'MAC.insert.Protos';
        $mac_protos = $vario_lib->getValByKey($sqlo, $this->container_id , $keyx);
        if ($mac_protos!=NULL) {
            $this->_check_PROTO_status($sqlo, $mac_protos, $keyx);
        }
        
        
    }
}

class oContainerModiGuiC {
    
    private $container_id;
    private $suc_isOnPos;
    private $contSubLib;
    private $_stop_forward=0;
    var $pos_method;
    
    /**
     * 
     * @var array $parx
     *   'POS' : is set automatic in init()
     */
    var $parx;
    
    function __construct() {
        $this->_stop_forward=0;
    }

    function init(&$sqlo, $parx, $go) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $this->abscont_lib = NULL;
        $this->go = $go;
        $this->suc_is_on_SAME_pos=0;
        $this->ALI_NUM_MAX = 100;
        
        $parx['QUANTITY'] = trim($parx['QUANTITY']);
        if ($parx['QUANTITY']=='AUTO') {
            $parx['QUANTITY'] = NULL;
        }
        
        $this->parx = $parx;
        $contid = $this->parx['CONTAINER_ID'];
        $this->suc_table_nice =  tablename_nice2('CONCRETE_SUBST') ;
        $suc_id = $this->parx['substid'];
        $this->suc_id = $suc_id;
        $this->container_id = $contid;
        
        
        
        $this->contSubLib = new  oCONTAINER_SubsC ($this->parx);
        $this->contSubLib->setContainer($sqlo, $contid);
    
      
        $tmparr =  $this->contSubLib->getFeature( $sqlo, array('vals'=>array('ABS_CONTAINER_ID') ) );
        $this->ABS_CONTAINER_ID = $tmparr['ABS_CONTAINER_ID'];
        $tmparr = $this->contSubLib->getFeature( $sqlo, array( 'ABS_CONTAINER'=>array('poscnt') )  );
        $this->pos_method = $this->contSubLib->getFeature( $sqlo, 'pos_method'  );
        $this->ABS_CONTAINER_poscnt  = $tmparr['poscnt'];
        
        $this->suc_isOnPos = $this->contSubLib->substInCont($sqlo, $suc_id);
        
        $pos_alias = $this->parx['POS_ALIAS'];
        
        if ($this->ABS_CONTAINER_ID and $this->ABS_CONTAINER_poscnt) {
            
            if ( $this->parx['POS']=='AUTO') {
                $this->parx['POS']=NULL;
            }
            
            $this->abscont_lib = new oABS_CONTAINER_subs();
            $this->abscont_lib->setContainer($sqlo, $this->ABS_CONTAINER_ID);
            
            
            if ($this->suc_isOnPos) {
                $this->suc_isOnPos_alias = $this->abscont_lib->get_alias_by_pos($sqlo, $this->suc_isOnPos);
            }
            
            if ($pos_alias!=NULL) {
                
                if ($this->pos_method=='ALIAS.PART') {
                    $pos = $this->contSubLib->get_pos_by_alias_part($sqlo, $pos_alias, 0);
                } else {
                    $pos = $this->abscont_lib->get_pos_by_alias($sqlo, $pos_alias);
                }
                if (!$pos) {
                    $error->set( $FUNCNAME, 1, 'No POS for Position alias "'.$pos_alias.'" found on this container!');
                    return;
                }
                $this->parx['POS'] = $pos;
            }
           
            
            
        }
        
        //echo "DDD: ABS_CONTAINER_ID:".$this->ABS_CONTAINER_ID." ABS_CONTAINER_poscnt:".$this->ABS_CONTAINER_poscnt." POS: ".$this->parx['POS'].
        //" alias:".$pos_alias. " parx:".print_r( $this->parx,1). "<br>";
        
        $this->contModLib = new  oContainerModC ($this->parx);
        $this->contModLib->setContainer($sqlo, $this->parx["CONTAINER_ID"] );
        
        $cont_parx = array( "POS"=>$this->parx['POS'], "substid"=>$suc_id );
        $this->contModLib->initSubst($cont_parx);
        
        debugOut('INIT-PARX: '.print_r($this->parx,1), $FUNCNAME, 1);
        debugOut('INIT-Vals: suc_isOnPos:'.$this->suc_isOnPos, $FUNCNAME, 1);
        if ($this->ABS_CONTAINER_poscnt) {
            echo "Info: The container has defined storage positions.<br>";
        }
        
        $this->alias_preselect = $this->contSubLib->get_alias_select_vals($sqlo);
        debugOut('INIT: alias_preselect: '.print_r($this->alias_preselect,1), $FUNCNAME, 1);
        echo "Info: Position-Calc-Method: ".$this->pos_method.".<br>";
        
    }
    
    function stop_forward() {
        return  $this->_stop_forward;
    }
    
    function modiCheck(&$sqlo) {
    	// generate error on problem
    	
    	$this->contModLib->modiCheck($sqlo);
    	
    	
    }
    
    function _showSubst(&$sqlo) {
    	// get edit-link of BO + NAME + icon (object)
    	$opts=NULL;
    	$htmltxt = fObjViewC::bo_display( $sqlo,  "CONCRETE_SUBST",  $this->parx["substid"], $opts );
    	echo "substance: ".$htmltxt."<br>";
    	
    }
    
    
    /**
     * normal form to add SUC
     */
    function add_single_AskForm() {
    	require_once ('func_form.inc');
    	
    	
    	// echo "<b>The ".$this->suc_table_nice." already exists in container!<br>";
    	if ($this->ABS_CONTAINER_poscnt) {
    	    echo "The container has defined storage positions.<br>";
    	}
    	echo "<br>";
    	
    	
    	$initarr   = NULL;
    	$initarr["action"]      = $_SERVER['PHP_SELF'];
    	$initarr["title"]       = "Give Alias Position";
    	$initarr["submittitle"] = "Add";
    	$initarr["tabwidth"]    = "AUTO";
    
    	$hiddenarr = NULL;
    	$keys = array('CONTAINER_ID', 'POS', 'substid', 'action', 'QUANTITY', 'backurl' );
    	foreach($keys as $key) {
    		$hiddenarr["parx[".$key."]"]=$this->parx[$key];
    	}
    	$hiddenarr["parx[addForce]"]=1;
    	
    	$formobj = new formc($initarr, $hiddenarr, 0);
    	
    	if ($this->ABS_CONTAINER_poscnt)  {
    	    
    	    $use_alias = $this->parx["POS_ALIAS"];
    	    if ($this->parx["POS_ALIAS"]==NULL and $this->suc_isOnPos_alias!=NULL) {
    	        $use_alias = $this->suc_isOnPos_alias;
    	    }
    	    
    	    if (!empty($this->alias_preselect)) {
    	        
    	        $alias_arr=array();
    	        foreach($this->alias_preselect as $alias_loop) {
    	            $alias_loop=trim($alias_loop);
    	            $alias_arr[$alias_loop] = $alias_loop;
    	        }
    	        
    	        $fieldx = array (
    	            "title" => "Position (alias)",
    	            "name"  => "POS_ALIAS",
    	            "object"=> "select",
    	            "val"   => $use_alias,
    	            "inits" => $alias_arr,
    	            "notes" => "e.g. B03"
    	        );
    	        $formobj->fieldOut( $fieldx );
    	        
    	    } else {
    	    
            	$fieldx = array (
            	    "title" => "Position (alias)",
            	    "name"  => "POS_ALIAS",
            	    "object"=> "text",
            	    "val"   => $use_alias,
            	    "notes" => "e.g. S1R01B04"
            	);
            	$formobj->fieldOut( $fieldx );
    	    }
        }
    
    	/*
    	$fieldx = array ( 
    		"title" => "add anyway", 
    		"name"  => "addForce",
    		"object"=> "checkbox",
    		"val"   => 1, 
    		"notes" => "add substance anyway"
    		 );
    	$formobj->fieldOut( $fieldx );
    	*/
    	
    	/*
    	$fieldx = array (
    	    "title" => "new box?",
    	    "name"  => "newBox",
    	    "object"=> "checkbox",
    	    "val"   => $parx["newBox"],
    	    "notes" => "put to a new box in the container? Default: just add the quantity ..."
    	);
    	$formobj->fieldOut( $fieldx );
    	*/
    
    	$formobj->close( TRUE );
    }	
    
    function addAliqForm(&$sqlo) {
    	
    	$parx = $this->parx;
    	$initarr   = NULL;
    	$initarr["action"]      = $_SERVER['PHP_SELF'];
    	$initarr["title"]       = "Add aliquots";
    	$initarr["submittitle"] = "Add";
    	$initarr["tabwidth"]    = "AUTO";
    	
    	if ($this->pos_method!='AUTO') {
    	    $initarr["title"]       = "Set aliquot parameters";
    	    $initarr["submittitle"] = "Next";
    	}
    
    	$hiddenarr = NULL;
    	foreach( $this->parx as $key=>$val) {
    		$hiddenarr["parx[".$key."]"]=$val;
    	}
    	
    	
    	$formobj = new formc($initarr, $hiddenarr, 1);
    
    	$fieldx = array ( 
    		"title" => "number of aliquots", 
    		"name"  => "aliNum",
    		"object"=> "text",
    		"val"   => $parx["aliNum"], 
    		"notes" => "e.g. 5",
    	    'req'   => 1
    		 );
    	$formobj->fieldOut( $fieldx );
    	
    	if ($this->pos_method=='ALIAS.PART') {
    	    
    	    $first_alias = '?';
    	    if (!empty($this->alias_preselect)) {
    	        $first_alias = current($this->alias_preselect);
    	    }
    	    
    	    $fieldx = array (
    	        "title" => "Alias position for ALL aliquots",
    	        "name"  => "alias_start",
    	        "object"=> "text",
    	        "val"   => $parx["alias_start"],
    	        "notes" => "e.g. ".$first_alias,
    	        
    	    );
    	    
    	    if (!empty($this->alias_preselect)) {
    	        
    	        $alias_arr=array();
    	        foreach($this->alias_preselect as $alias_loop) {
    	            $alias_loop=trim($alias_loop);
    	            $alias_arr[$alias_loop] = $alias_loop;
    	        }
    	        
    	        $fieldx["object"] = "select";
    	        $fieldx[ "inits"] = $alias_arr;  
    	    } 
    	    
    	    $formobj->fieldOut( $fieldx );
    	}
    
    	$formobj->close( TRUE );
    }
    
    function addAliqForm_aliases(&$sqlo) {
        
        
        $initarr   = NULL;
        $initarr["action"]      = $_SERVER['PHP_SELF'];
        $initarr["title"]       = "Set aliquot storage positions";
        $initarr["submittitle"] = "Add";
        $initarr["tabwidth"]    = "AUTO";
        
        $hiddenarr = NULL;
        foreach( $this->parx as $key=>$val) {
            $hiddenarr["parx[".$key."]"]=$val;
        }
        
        $formobj = new formc($initarr, $hiddenarr, 1);
        
        for($ali_id=1; $ali_id<=$this->parx["aliNum"]; $ali_id++ ) {
            
            $val='';
            if ($this->parx["alias_start"]!=NULL) {
                $val = $this->parx["alias_start"];
            }
            
            $fieldx = array (
                "namex"=>TRUE,
                "title" => "Alias ".($ali_id),
                "name"  => "alias_arr[".$ali_id."]",
                "object"=> "text",
                "val"   => $val,
                "notes" => "e.g. S1R2"
            );
            $formobj->fieldOut( $fieldx );
        }
        
        
        
        $formobj->close( TRUE );
    }
    
   
    
   
    
    /**
     * check, if POS or QUANTITY are valid
     * @return array
     *   'add_single_ask_form' : 0,1
     *   'force_add' : 0,1
     */
    function addCheck1(&$sqlo) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $retarr=array();
        
        $go   = $this->go;
        $parx = $this->parx;
        
        do {
            if ($parx['QUANTITY']==NULL) {
                $error->set( $FUNCNAME, 1, 'Please give a QUANTITY!' );
                return;
            }
            if (!is_numeric($parx['QUANTITY'])) {
                $error->set( $FUNCNAME, 2, 'QUANTITY must be a number!' );
                return;
            }
            
            
            if ($this->ABS_CONTAINER_poscnt and $this->pos_method!='AUTO') {
                // YES: with POS_DEF
                //NO: NO_POSDEF
                
                if (!$go) $retarr['add_single_ask_form']=1;
                else {
                    $retarr['add_single_ask_form']=0;
                }
                $retarr['force_add']=0;
                
                
            } else {
                
                //NO: NO_POSDEF
                $retarr['add_single_ask_form']=0;
                $retarr['force_add']=1;
            }
            
            
        } while (0);
        
        $help_lib = new oCONT_MOD_helper($this->container_id, $this->suc_id);
        $help_lib->addCheck_more($sqlo);
        
        debugOut('(371) pos_method:'.$this->pos_method.' retarr:'.print_r($retarr,1), $FUNCNAME, 1);
        
        return $retarr;
    }
    
    /**
     * ALIAS exists on storage ?
     * @param object $sqlo
     * @param string $alias_start
     */
    function alias_check_exists($sqlo, string $alias_start) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $pos_start=0;
        $pos = $this->contSubLib->get_pos_by_alias_part($sqlo, $alias_start, $pos_start);
        if ($error->Got(READONLY))  return;
        if (!$pos) {
            $error->set( $FUNCNAME, 1, 'No POS found for Alias "'.$alias_start.'".' );
            return;
        }
    }
    
    /**
     * can be added ?
     * Enter description here ...
     * @param $sqlo
     * @return int
     * 
     */
    function addCheck2(&$sqlo, $go) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        if ( !$this->parx["aliNum"] ) {
            
        
            if ($this->ABS_CONTAINER_poscnt) {
                
                
                // POSDEF
                $pos = $this->parx["POS"];
                if (!$pos) {
                    $error->set( $FUNCNAME, 1, 'No POS given.' );
                    return;
                }
                
                if ($this->suc_isOnPos) {
                    
                    $this->suc_on_newpos = $this->contSubLib->get_SUC_of_Pos($sqlo, $this->parx['POS']);
                    
                    $suc_on_newpos = $this->suc_on_newpos;
                    if ($suc_on_newpos and $suc_on_newpos!=$this->parx["substid"]) {
                        $error->set( $FUNCNAME, 2, 'There is an other SUC (ID:'.$suc_on_newpos.') on POS:'.$pos );
                        return;
                    }
        
                    if ( $this->suc_isOnPos and $this->suc_on_newpos and ($this->suc_on_newpos==$this->suc_id) ) {
                        $this->suc_is_on_SAME_pos=1;
                    }
                    
                    debugOut('SUC_ID: '.$this->suc_id , $FUNCNAME, 1);
                    debugOut('Material is on the same position (0,1):'.$this->suc_is_on_SAME_pos , $FUNCNAME, 1);
                    debugOut('Material suc_isOnPos? : '.$this->suc_isOnPos , $FUNCNAME, 1);
                    debugOut('Material suc_on_newpos: '.$this->suc_on_newpos , $FUNCNAME, 1);
                   
                }
                
            } 
        
        
        } else {
            if ( $this->parx["aliNum"]> $this->ALI_NUM_MAX ) {
                $error->set( $FUNCNAME, 3, 'Max number of aliquots: '.$this->ALI_NUM_MAX );
                return;
            }
        }
    }
    
    /**
     * transform ALIAS array to POS array
     * @param object $sqlo
     * @param array $alias_arr
     * @return array ALI-NUM => POS
     */
    private function _alias_arr2pos($sqlo, $alias_arr) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $posarr=array();
        
        $max_aliquots= $this->parx['aliNum'];
        $pos = 0;
        
        for( $ali_id=1; $ali_id<=$max_aliquots; $ali_id++ ) {
            
            $pos_alias=trim($alias_arr[$ali_id]);
            if ($pos_alias==NULL) {
                $error->set( $FUNCNAME, 1, 'No Alias for Aliquot '.$ali_id );
                return;
            }
            
            if ($this->pos_method=='ALIAS.PART') {
                $pos = $this->contSubLib->get_pos_by_alias_part($sqlo, $pos_alias, $pos);
                
            } else {
                $pos = $this->contSubLib->get_pos_by_alias($sqlo, $pos_alias);
            } 
            if (!$pos)  {
                $error->set( $FUNCNAME, 2, 'Alias "'.$pos_alias.'" is unknown!' );
                return;
            }
            
            $old_suc = $this->contSubLib->get_SUC_of_Pos($sqlo,$pos);
            if ($old_suc) {
                $error->set( $FUNCNAME, 2, 'Theres is already material on Alias "'.$pos_alias.'" (Pos:'.$pos.').' );
                return;
            }
            
            debugOut('Ali:'.$ali_id.' Alias:'.$pos_alias.' Pos:'.$pos, $FUNCNAME, 1);
            
            $posarr[$ali_id]=$pos;
        }

        
        return $posarr;
    }
    
    private function _addQuantity(&$sqlo) {
        $parx = $this->parx;
        $quantity = $parx["QUANTITY"];
        $pos      = $parx["POS"];
        
        $pos_feats = $this->contSubLib->get_Conc_PosFeats($sqlo, $pos);
        $quantity  = $quantity + $pos_feats['QUANTITY'];
        $arguin =array('QUANTITY'=>$quantity);
        $this->contModLib->updateAliquot($sqlo, $pos, $arguin);
        echo "... updated on pos ".$pos." QUANT_new:".$quantity."<br>";
    }
    
    /**
     * add one substance,
     * - no Aliquot support
     * @param object $sqlo
     */
    function add_single(&$sqlo) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $parx        = $this->parx;
        $pos         = $parx["POS"];
        $quantity    = $parx['QUANTITY'];

        
        if ($this->ABS_CONTAINER_poscnt) {
            
            // has POSEDEF ...
            // $suc_on_newpos = $this->suc_on_newpos;
            
            debugOut('suc_is_on_SAME_pos? '.$this->suc_is_on_SAME_pos , $FUNCNAME, 1);
            
            if ( $this->suc_is_on_SAME_pos) {
                $this->_addQuantity($sqlo);
            } else {
                
                $ali_id=1;
//                 if ($this->pos_method=='ALIAS.PART') {
//                     $pos = $this->contSubLib->get_pos_by_alias_part($sqlo, $pos_alias, 0); 
//                 }

                $this->contModLib->addAliquot( $sqlo, $pos, $this->parx["substid"], $ali_id, $quantity );
                if ($error->Got(READONLY))  {
                    return;
                }
                echo "... inserted on pos ".$pos.". quant: ".$quantity." <br>";
                $pos++;
                
            }
            
        } else {
            
            // no POSDEF
            if ($this->suc_isOnPos) {
                $this->parx['POS'] = $this->suc_isOnPos;
                $this->_addQuantity($sqlo);
            } else {
                // get next position starting at end of container-POS
                $nextFreePos = $this->contModLib->getFreePosEnd($sqlo);
                $ali_id=1;
    
                $this->contModLib->addAliquot( $sqlo, $nextFreePos, $this->parx["substid"], $ali_id, $quantity );
                if ($error->Got(READONLY))  {
                    return;
                }
                echo "... inserted on pos ".$pos." quant: ".$quantity." <br>";

            }
     
        }
        
        $hist_obj = new historyc();
        $hist_obj->historycheck('CONTAINER', $this->parx["CONTAINER_ID"]);
    }
    
    /**
     * add ALIQUOTS ,
     * @param object $sqlo
     */
    function add_ali(&$sqlo, $alias_arr) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $parx        = $this->parx;
        $pos         = $parx["POS"];
        $quantity    = $parx['QUANTITY'];
        $max_aliquots= $parx['aliNum'];
        if ($max_aliquots<=0) $max_aliquots = 1;
        echo "... Aliquot-Number:".$max_aliquots."<br>";
        
        $pos_arr=array();
        if ( $this->pos_method!='AUTO' ) {
            // get / check real positions
            $pos_arr=$this->_alias_arr2pos($sqlo, $alias_arr);
            if ($error->Got(READONLY))  {
                return;
            }
        }
        
        
        for( $ali_id=1; $ali_id<=$max_aliquots; $ali_id++ ) {
            
            // get next position starting at end of container-POS
            if ($this->pos_method=='AUTO') {
                $nextFreePos = $this->contModLib->getFreePosEnd($sqlo);
            } else {
                $nextFreePos = $pos_arr[$ali_id];
            }
            if ($error->Got(READONLY))  {
                return;
            }
            
            
            $this->contModLib->addAliquot( $sqlo, $nextFreePos, $this->parx["substid"], $ali_id, $quantity );
            if ($error->Got(READONLY))  {
                return;
            }
            echo "... inserted on pos ".$pos." quant: ".$quantity." aliquot:".$ali_id."<br>";
            
        }

        
        $hist_obj = new historyc();
        $hist_obj->historycheck('CONTAINER', $this->parx["CONTAINER_ID"]);
    }
    
    function delask(&$sqlo) {
    	// special forms ....
    	require_once ("func_formSp.inc");
    	
    	
    	$formLib = new formSpecialc();
    	$title = "Remove from container";
    	$asktext = "Do you want remove this ".$this->suc_table_nice." at POS:".$this->parx["POS"]." from the container?";
    	$delurl= $_SERVER['PHP_SELF'];
    	
    	$params=NULL;
    	$params["go"]=1;
    	foreach( $this->parx as $key=>$val) {
    		$params["parx[".$key."]"]=$val;
    	}
    	
    	
    	$formLib->deleteForm( $title, $asktext, $delurl, $params );
    }
    
    private function _get_SUC_quant($sqlo, $suc_id) {
        $sqlsel="sum(QUANTITY) from CONT_HAS_CSUBST where  CONCRETE_SUBST_ID=".$suc_id;
        $sqlo->Quesel($sqlsel);
        $sqlo->ReadRow();
        $quant  = $sqlo->RowData[0];
        return $quant;
    }
    
    private function _check_finished($sqlo) {
        
        $suc_id       = $this->contModLib->get_SUC_ID();
        $quant_new    = $this->_get_SUC_quant($sqlo, $suc_id);
        $was_finished = $this->contModLib->has_status($sqlo, 'finished', $suc_id);
        
        if ($quant_new<=0 and !$was_finished) {

           
            echo '<br>';
            cMsgbox::showBox("ok", 'The stock of this '.$this->suc_table_nice.' [ID:'.$suc_id.'] is empty now.'); 
            echo '<br>';
            
            $initarr   = array();
            $initarr["action"]      = 'p.php?mod=DEF/o.CONTAINER.modone'; // !!! other module ..
            $initarr["title"]       = "Do you want to flag the ".$this->suc_table_nice .' as finished?';
            $initarr["submittitle"] = "Set finished";
            $initarr["tabwidth"]    = "AUTO";
            
            $hiddenarr = array();;
            $hiddenarr["id"]=$this->parx['CONTAINER_ID'];
            $hiddenarr["parx[substid]"]=$suc_id;
            $hiddenarr["act"]='FINISH_SUC';
           
            $formobj = new formc($initarr, $hiddenarr, 0);
            
//             $fieldx = array (
//                 "title" => "confirm reopen",
//                 "name"  => "accept",
//                 "object"=> "checkbox",
//                 "notes" => 'Please confirm this action',
                
//             );
//             $formobj->fieldOut( $fieldx );
            
            $formobj->close( TRUE );
            
            $this->_stop_forward=1;
        }
    }
    
    function del(&$sqlo) {
        global $error;
    
    	$this->contModLib->del($sqlo, 1);
    	if ($error->Got(READONLY))  {
    		return;
    	}
    	
    	$this->_check_finished($sqlo);
    }
    
    function del_many_ask(&$sqlo) {
        
        $suc_id = $this->parx['substid'];
        
        $all_pos = $this->contSubLib->get_one_suc_ALL($sqlo, $suc_id);
        
       
        $initarr   = NULL;
        $initarr["action"]      = $_SERVER['PHP_SELF'];
        $initarr["title"]       = "Remove selected Aliquots";
        $initarr["submittitle"] = "Remove";
        $initarr["tabwidth"]    = "AUTO";
        
        $hiddenarr = NULL;
        foreach( $this->parx as $key=>$val) {
            $hiddenarr["parx[".$key."]"]=$val;
        }
        
        
        $formobj = new formc($initarr, $hiddenarr, 0);
        $cnt=0;
        foreach($all_pos as $row) {
        
            $fieldx = array (
                "title" => "Pos ".$row['POS'],
                "name"  => "pos[".$row['POS']."]",
                "object"=> "checkbox",
                "notes" => 'Quantity: '.$row['QUANTITY'],
                "namex" => TRUE 
            );
            $formobj->fieldOut( $fieldx );
            $cnt++;
        }
        
        $formobj->close( TRUE );
        
    }
    
    function del_many(&$sqlo, $pos_arr) {
        global $error;
        
        $suc_id = $this->parx['substid'];
        
        foreach($pos_arr as $pos=>$check_val) {
        
            if ($check_val==1) {
                $cont_parx = array( "POS"=>$pos, "substid"=>$suc_id );
                $this->contModLib->initSubst($cont_parx);
                $this->contModLib->del($sqlo);
                if ($error->Got(READONLY))  {
                    return;
                }
                echo '- Pos: '.$pos.' removed.<br>';
            }
        }
        
        $this->_check_finished($sqlo);
    }
    
    
    
    /**
     * add container to bookmarks
     * @param object $sqlo
     */
    function finishAct(&$sqlo) {
    	$profileLib = new profile_funcs();
    	$projid=$profileLib->getProj($sqlo, 'bookmarks');
    	if (!$projid) return; // no project
    	
    	$CONTAINER_ID = $this->parx["CONTAINER_ID"];
    	if ( !cProjSubs::objectInProject($sqlo, $projid, 'CONTAINER', $CONTAINER_ID) ) {
    		// add
    		$projModlib = new oProjAddElem($sqlo,$projid);
    		$projModlib->addObj($sqlo, 'CONTAINER', $CONTAINER_ID);
    	}
    }

}

// --------------------------------------------------- 
global $error, $varcol;

$error = & ErrorHandler::get();
$sqlo  = logon2( $_SERVER['PHP_SELF'] ); // URL-link for the first db-login; e.g. for object ($_SERVER['PHP_SELF']."?id=".$id)
//$sqlo2 = logon2( );
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();

$parx = $_REQUEST["parx"];
$id   = $parx["CONTAINER_ID"];
$go   = $_REQUEST["go"];

$tablename			 = "CONTAINER";
//$i_tableNiceName 	 = tablename_nice2($tablename);

$title       		 = "manage a ".tablename_nice2('CONCRETE_SUBST') ." on a container: ".$parx["action"];
$infoarr = NULL; 
$infoarr["title"] = $title;
$infoarr["scriptID"] = "";
$infoarr["form_type"]= "obj";
$infoarr["obj_name"] = $tablename;
$infoarr["obj_id"]   = $id;
$infoarr['checkid']  = 1;
$infoarr['version']  = '2022-02-23';
$infoarr['version.info']='Implemented new test: P3-USN-001: '.
    'erlaube hinzufuegen einer MAC, wenn MAC:Protocol-Status = XXX';

$pagelib = new gHtmlHead();
$pagelib->startPage($sqlo, $infoarr);

echo "<ul>\n";

if (!$parx["POS"]) $parx["POS"]='AUTO'; // default

if (!$id) htmlFoot("Error", "Missing CONTAINER_ID.");
if (!$parx["substid"]) htmlFoot("Error", "Missing SUBSTANCE_ID.");
if (!$parx["POS"])     htmlFoot("Error", "Missing POS.");

if ( !glob_table_exists( "CONTAINER") ) {
	htmlFoot("Error","table CONTAINER doeas not exist. Please ask your admin.");
}

$mainlib = new oContainerModiGuiC();
$mainlib->init($sqlo, $parx, $go);
if ($error->Got(READONLY))  {
    $error->printAll();
    htmlFoot();
}

$mainlib->modiCheck($sqlo);
if ($error->Got(READONLY))  {
	$error->printAll();
	htmlFoot();
}


switch ( $parx["action"] ) {
    
    case "ADD":
        
        // get edit-link of BO + NAME + icon (object)
        $objLinkLib = new fObjViewC();
        $htmlTmp = $objLinkLib->bo_display( $sqlo, 'CONCRETE_SUBST', $mainlib->parx['substid'] );
        echo 'Add: '.$htmlTmp.' Quantity: '.$mainlib->parx['QUANTITY']."<br><br>\n";
    	
    	$check1Result = $mainlib->addCheck1($sqlo);
    	if ($error->Got(READONLY))  {
    		$error->printAll();
    		$pagelib->htmlFoot();
    	}
    	
    	
    	if ( $parx['withAli']>0 ) {
    	    
    	    if (!$parx['aliNum'] ) {
        		$mainlib->addAliqForm($sqlo);
        		$pagelib->htmlFoot();
    	    }
    	    
    	    
    	    
    	    if ($mainlib->pos_method!='AUTO' and !is_array($_REQUEST['alias_arr']) ) {
    	        
    	        $alias_start = $parx["alias_start"];
    	        if ($alias_start==NULL) $alias_start='';
    	        $mainlib->alias_check_exists($sqlo, $alias_start);
    	        if ($error->Got(READONLY))  {
    	            $error->printAllEasy();
    	            $pagelib->htmlFoot();
    	        }
    	        
    	        $mainlib->addAliqForm_aliases($sqlo);
    	        $pagelib->htmlFoot();
    	    }
    	    
    	    
    	} else {

        	if ($check1Result['add_single_ask_form']) {
        	    $mainlib->add_single_AskForm();
        		$pagelib->htmlFoot();
        	}
    	}
    	
    	$mainlib->addCheck2($sqlo, $go);
    	if ($error->Got(READONLY))  {
    	    $error->printAll();
    	    $pagelib->htmlFoot();
    	}
    	
    	if ($parx['withAli']>0) {
    	    $mainlib->add_ali($sqlo, $_REQUEST['alias_arr']); 
    	} else {
    	   $mainlib->add_single($sqlo);
    	}
    	$pagelib->chkErrStop();
    	
    	$mainlib->finishAct($sqlo);
    	$pagelib->chkErrStop();
    	break;
	


    case "DEL":
    
        $objLinkLib = new fObjViewC();
        $htmlTmp = $objLinkLib->bo_display( $sqlo, 'CONCRETE_SUBST', $mainlib->parx['substid'] );
        echo 'Remove from container: '.$htmlTmp."<br><br>\n";
        
    	if (!is_numeric($parx['POS'])) {
    		htmlFoot('ERROR', 'Param POS missing.');
    	}
    	
    	if (!$go) {
    		$mainlib->delask($sqlo);
    		htmlFoot();
    	} 
    	$mainlib->del($sqlo);
    	if ($error->Got(READONLY))  {
    		$error->printAll();
    		htmlFoot();
    	}
    	break;

    case "DEL_MANY" :
    
        $objLinkLib = new fObjViewC();
        $htmlTmp = $objLinkLib->bo_display( $sqlo, 'CONCRETE_SUBST', $mainlib->parx['substid'] );
        echo 'Remove from container: '.$htmlTmp."<br><br>\n";
        
        if (!$go) {
            $mainlib->del_many_ask($sqlo);
            htmlFoot();
        }
        
        if (!is_array($_REQUEST["pos"])) {
            htmlFoot('ERROR', 'Param POS missing.');
        }
        
        $mainlib->del_many($sqlo, $_REQUEST["pos"]);
        if ($error->Got(READONLY))  {
            $error->printAll();
            htmlFoot();
        }

        break;

    default:
        htmlFoot('ERROR', 'Action: '.$parx["action"] .' unknown.');
}

if ( $mainlib->stop_forward() ) {
    htmlFoot('<hr>');
    exit;
}

if ($parx["backurl"]!="") {
	$url = urldecode( $parx["backurl"] );
	js__location_replace( $url, "back" );
}
htmlFoot('<hr>');
