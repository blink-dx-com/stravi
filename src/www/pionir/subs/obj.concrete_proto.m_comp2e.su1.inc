<?php
/**
 * - class: prepare update of one STEP for a selection of PROTOS with same values
 * - class: update steps
 * - used by package:   obj.concrete_proto.m_comp2e.php
 * GLOBAL $_SESSION['s_formState']["proto.m_comp2e"]
 * 
 * @package obj.concrete_proto.m_comp2e.su1.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param string INPUT 
 */
require_once 'o.CS_HAS_PR.subs.inc';
require_once 'o.EXP.proto.inc';
require_once ('f.msgboxes.inc');
require_once ("f.objview.inc");

/**
 * sub functions for protcol compare
 * - get selected protocols
 * 
 * @author steffen
 *
 */
class oProtoCompLib {

    function __construct($proj_id, $tablename, $parx, $step) {
    	$this->proj_id   = $proj_id;
    	$this->tablename = $tablename;
    	$this->parx=$parx;
    	$this->step=$step;
    }
    
    /**
     * RETURN: &$sel (as REF)
     * @param object $sql
     * @param object $sel
     * @param string $sqlAfterOrder
     * @param int $go
     * @return number
     */
    function getSel( &$sql, &$sel, $sqlAfterOrder, $go ) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
    	
    	$num_elem=0;
    	if ( $go<=0 ) {
    		
    		$sql->query( "SELECT ".$this->tablename."_ID FROM ".$sqlAfterOrder, $FUNCNAME ); 
    		$sel      = array();
    		while ( $sql->ReadRow() ) { 
    			$sel[$sql->RowData[0]]=1;
    			$num_elem++;
    		}
    		
    	} else {
    		// get objects from variables, because, if tablename==CONCRETE_SUBST => 
    		//	  the $_SESSION['s_tabSearchCond'] changes due to searches for substances in edit-mode !!!
    		$num_elem = sizeof($sel);
    	}
    	return ($num_elem);
    }
    
    /**
     * 	get one protocol of the object $obj_id
     * return array('name'=>$obj_name, 'cpid'=>$c_proto_id, 'apid'=>$a_proto_id, 'ap_name'=>$a_proto_name)
     */
    function obj_one_proto_get(  &$sql, $obj_id, $alter_pra_id=0  ) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;

        $def_pra_id   = $this->parx["a_proto_id"]; 
        if ($alter_pra_id) $def_pra_id=$alter_pra_id;
        $a_proto_id   = 0;
        $c_proto_id   = 0; 
        $a_proto_name = "";
    	$table        = $this->tablename;
    	$pk           = PrimNameGet2($table);
        
        $sqls= "select NAME from ".$table." where ".$pk."=".$obj_id ;
        $sql->query("$sqls");
        $sql->ReadRow();
        $obj_name = $sql->RowData[0];
    	
    	if ($obj_name=="") {
    		$obj_name = "[$obj_id]";
    	}
    	
    	debugOut('(87) obj_id: '.$obj_id.' def_pra_id:'.$def_pra_id, $FUNCNAME, 2);
        
    	switch ($table) {
    	
    		case "EXP":
    		    if (!$def_pra_id) {
     				 $error->set( $FUNCNAME, 1, 'obj_one_proto_get() pra_id missing.' );
     				 return;
    			}
    		    $exp_lib = new oEXPprotoC();
    		    $exp_lib->getConcProtoByAbstract($sql, $obj_id, $def_pra_id);
    			$c_proto_id=$sql->RowData[0];
         		break;
    			
        	case "CONCRETE_PROTO":
    			$c_proto_id = $obj_id;
           		break;
    			
        	case "CONCRETE_SUBST":
        	    if (!$def_pra_id) {
        	        $error->set( $FUNCNAME, 1, 'obj_one_proto_get() pra_id missing.' );
        	        return;
        	    }
        	    $suc_lib = new oCS_HAS_PR_subs($obj_id);
        	    $c_proto_id = $suc_lib->getProtoByPRA($sql, $def_pra_id);
        	    break;
    			
    		case "CHIP_READER":
    			$sqls= "select CONCRETE_PROTO_ID from CHIP_READER where CHIP_READER_ID=".$obj_id;
    			$sql->query("$sqls");
    			$sql->ReadRow();
    			$c_proto_id=$sql->RowData[0];
           		break; 
    	
    	 	
    	}
        
        if ($c_proto_id) {
            $sqls= "select ABSTRACT_PROTO_ID from CONCRETE_PROTO where CONCRETE_PROTO_ID=".$c_proto_id;
            $sql->query("$sqls");
            $sql->ReadRow();
            $a_proto_id=$sql->RowData[0];
            
            $sqls= "select NAME from abstract_proto where ABSTRACT_PROTO_ID=".$a_proto_id ;
            $sql->query("$sqls");
            $sql->ReadRow();
            $a_proto_name=$sql->RowData[0];	  
        }
        return ( array('name'=>$obj_name, 'cpid'=>$c_proto_id, 'apid'=>$a_proto_id, 'ap_name'=>$a_proto_name) );
    }     
    
    /**
     * do initial parameter checks
     */
    function InitialChecks() {
    	global  $error;
    	$FUNCNAME = __CLASS__.':'.__FUNCTION__;
    	
    	$t = $this->tablename;
    	
    	if (  $this->parx["action"] != "update" AND !is_array($this->step) ) {
    		$error->set( $FUNCNAME, 1, 'Need at least one protocol STEP_NR.' );
    		return;
    	}   
    	
    	if ( ($t=="EXP" or $t=="CONCRETE_SUBST") AND !$this->parx['a_proto_id'] ) {
    		$error->set( $FUNCNAME, 2,  "Input missing: need a a_proto_id.");         
    		return;
    	} 
    	
    	
    	
    	if ( !$this->parx["ref_exp"] ) {
    		$error->set( $FUNCNAME, 3, "Need a reference object ID.") ;         
    		return; 
    	}           
    	
    	if ( is_array($this->parx) ) {  // save parameters
    		
    // 		$_SESSION['s_formState']["proto.m_comp2e"]["parx"] = $this->parx; 
    // 		$_SESSION['s_formState']["proto.m_comp2e"]["step"] = $this->step;
    // 		$_SESSION['s_formState']["proto.m_comp2e"]["allParam"]= array(
    // 			 "proj_id" => $this->proj_id
    // 			);
    	}   
    }

}


/**
 * 	update protocol steps
 */
class oCprotoUpStepC {
    
    var $updateInfo;

			
    function __construct( &$mainLib ) {
    	$this->mainLib = &$mainLib;
    	$this->tablename = $this->mainLib->tablename;
    	$this->protoManiLib = new oCprotoMani();
    }
    
    /**
     * 
     * @param object $sql
     * @param int $obj_id
     * @param array $step_args of array(STEP=>array())
     */
    private function one_obj( object $sql, int $obj_id, array $step_args, $onl_arg_keys_up=0 ) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $a_proto_id = $this->a_proto_id;
        $tablenice  = tablename_nice2($this->tablename); 
        
        $this->updateInfo[$obj_id] = "";
        $proto_arr_loop = $this->mainLib->obj_one_proto_get( $sql, $obj_id, $a_proto_id );
        $c_proto_id =$proto_arr_loop['cpid'];
        $a_proto_tmp=$proto_arr_loop['apid'];
        
        if ( !$c_proto_id ) {
            $this->updateInfo[$obj_id] = "<font color=red>".$tablenice." has no protocol</font>";
            return; // next object
        }
        
        if ( $a_proto_id != $a_proto_tmp   ) {
            $this->updateInfo[$obj_id] = "<font color=red>other protocol (abstract) [$a_proto_tmp]</font>";
            return; // next object
        }
        
        $acccheck = 1;
        $this->protoManiLib->setCprotoID ( $sql, $c_proto_id, $a_proto_tmp, $acccheck );
        if ( $error->got(READONLY) ) {
            $errLast   = $error->getLast();
            $error_txt = $errLast->text;
            $error->reset();
            $this->updateInfo[$obj_id] = "<font color=red>".$error_txt."</font>";
            return;  // next object
        }
        
       
        
        foreach( $this->step as $step_nrx=>$dummy) {
            
            if ($this->rememberStep[$c_proto_id][$step_nrx] > 0 ) { // TBD: why [$c_proto_id][$step_nrx] ???
                $this->updateInfo[$obj_id] = "<font color=red>Warning: No update! Protocol touched before.</font>";
                continue; // next step
            }
            
            $newarg = $step_args[$step_nrx];
 
            if ( $_SESSION['userGlob']["g.debugLevel"]>2 ) {
                echo "DEBUG: c_proto_id:$c_proto_id, step_nr:$step_nrx args:<br><pre>";
                print_r($newarg);
                echo "</pre>\n";
            }
            $opt=array();
            if ($onl_arg_keys_up) {
                $opt["defcols"]=1;
            }
            $upinfo = $this->protoManiLib->oneStepSave( $sql, $newarg, $step_nrx, $opt );
            if ( $_SESSION['userGlob']["g.debugLevel"]>1 ) {
                debugOut('(247) Update_Info: CPID:'.$c_proto_id.' StepNr:'.$step_nrx.': '. $upinfo, $FUNCNAME);
            }
            
            
            if ( $error->got(READONLY) ) {
                $errLast   = $error->getLast();
                $error_txt = $errLast->text;
                $error->reset();
                $this->updateInfo[$obj_id] = "<font color=red>".$error_txt."</font>";
            } else {
                
                $this->updateInfo[$obj_id] = "<font color=green>ok</font>";
            }
            $this->rememberStep[$c_proto_id][$step_nrx] = 1;   // remember
        }
        $this->protoManiLib->close($sql); // needed for mod log
        
        $this->ok_cnt++;
    }
    
    /**
     * update steps with INDIVIDUAL values per object
     * 
     * @global array $_REQUEST[] "quanti" "concsubst" "newnote" "not_done", "devids"
     * @return 
     */
    function update_individualVals( &$sql, &$sel, &$step, $parx ) {
    	# global $error;
    	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
    	
    	$this->step = $step;
    	$this->updateInfo = NULL; 
        echo "<span>Update steps, individual values ...</span><br><br>\n";
    	$this->rememberStep = NULL; // remember, that step was updated before ...
    	$this->a_proto_id=$parx["a_proto_id"];
    	
    	$this->ok_cnt=0;
        foreach( $sel as $obj_id=>$dummy) { // for each object ...
            
            $step_args=array();
            
            foreach( $this->step as $stnr =>$dummy) {
                
                $one_step_arr=array();
                $one_step_arr["quantity"] = $_REQUEST["quanti"]   [$obj_id][$stnr];
                $one_step_arr["subst"]    = $_REQUEST["concsubst"][$obj_id][$stnr];
                $one_step_arr["notes"]    = $_REQUEST["newnote"]  [$obj_id][$stnr];
                $one_step_arr["not_done"] = $_REQUEST["not_done"] [$obj_id][$stnr];
                $one_step_arr["dev"]      = $_REQUEST["devids"]   [$obj_id][$stnr];
                
                $step_args[$stnr] = $one_step_arr;
            }

            $this->one_obj( $sql, $obj_id, $step_args );
            
        }
        
        if ($this->ok_cnt) {
            echo "<br>";
            cMsgbox::showBox("ok", "<B>".$this->ok_cnt."</B> Protocols updated."); 
            echo "<br>";
        }
        
 
    }
    
    /**
     * update with SAME values
     * @return NULL|string
     */
    function update_SameVals( &$sql, &$sel, &$step, $parx, $checkbox, $values_in ) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $this->step = $step;
        $this->updateInfo = NULL;
        echo "<span>Update steps, individual values ...</span><br><br>\n";
        $this->rememberStep = NULL; // remember, that step was updated before ...
        
        $this->a_proto_id=$parx["a_proto_id"];
   
        $short2mod_key= array(
            'qu'=>"quantity",
            'cs'=>"subst",  
            'cn'=>"notes", 
            'dea'=>"not_done",
            'dev'=>"dev"
        );
        
        $step_args=array();
        foreach( $this->step as $stnr =>$dummy) {

            $chk_step  = $checkbox[$stnr];
            $vals_step = $values_in[$stnr];
            
            $one_step_arr=array();
            foreach($chk_step as $col_short=>$chkval) {
                if ($chkval) {
                    $val_in = $vals_step[$col_short];
                    $col_mod = $short2mod_key[$col_short];
                    $one_step_arr[$col_mod] = $val_in;
                }
            }
            if ($vals_step['dea']==='0' or $vals_step['dea']==='1') {
                # special rule for NOT_DONE ...
                $col_short='dea';
                $col_mod = $short2mod_key[$col_short];
                $one_step_arr[$col_mod] = $vals_step[$col_short];
            }

            $step_args[$stnr] = $one_step_arr;
        }
        
        if ( $_SESSION['userGlob']["g.debugLevel"]>1 ) {
            debugOut('(357) step_args: <pre>'. print_r($step_args,1).'</pre>', $FUNCNAME);
            
        }
        
        
        $this->ok_cnt=0;
        foreach( $sel as $obj_id=>$dummy) { // for each object ...
            $this->one_obj( $sql, $obj_id, $step_args, 1 );
        }
        
        echo "<b>Results:</b><br>";
        foreach( $sel as $obj_id=>$dummy) { // for each object ...
            
            $objnice = fObjViewC::bo_display( $sql, $this->tablename, $obj_id); 
            echo $objnice.': '.$this->updateInfo[$obj_id]."<br>";
        }
        echo "<br>\n";
        
        
        if ($this->ok_cnt) {
            echo "<br>";
            cMsgbox::showBox("ok", "<B>".$this->ok_cnt."</B> Protocols updated.");
            echo "<br>";
        }
        
        
    }

}

