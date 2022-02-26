<?php

require_once ("gui/o.PROTO.stepout1.inc");
require_once ("gui/o.PROTO.stepout2.inc");
require_once ("o.CONCRETE_PROTO.policy.inc");
require_once ("lev1/o.CONCRETE_SUBST.plan_check.inc");
require_once 'o.S_VARIO.subs.inc';
require_once ("f.objview.inc");	
require_once ("f.text_html.inc");
require_once ('object.info.inc');

/**
 * protocol editor mashup-component
 * - can be used by other GUI-components 
 * - on submit, calls the DB_save_plugin: obj.concrete_proto.paras.php
 * - option 'backurl' defines the URL, which will be forwarded after the DB_save_plugin
 * @package o.CONCRETE_PROTO.edit.inc  
 * @swreq SREQ:0001016: g > protocol editor modul 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @unittest exists, GUI-test: misc/plugin/phplib/gui/UT_o.PROTO.stepout1.inc
 */
class oConcProtoEdform {
	
    private $features;    // features of CONCRETE_PROTO
    private $ap_features; // features of ABSTRACT_PROTO
    private $mo_obj; // array('t'=>tablename, 'id'=>ID of mother)
    
	var $withFeatFields; // 0,1 : show protocol feature fields ?
	private $mo_CERT_FLAG;
	private $PROTO_FEAT_sh_DATAURL;
	private $SUC_plan_check_Lib;
	
    function __construct($sqlo, $id=NULL) {
        $this->SUC_plan_check_Lib = NULL;
    	$this->withFeatFields = 0;
    	if ($id) {
    	    $this->setProtoID($sqlo, $id);
    	}
    }
    
    function setProtoID($sqlo, $c_proto_id) {
    	$this->id       = $c_proto_id;
    	$this->features = NULL;
    	$this->ap_features  = array();
    	$this->mo_CERT_FLAG = NULL;
    	$this->PROTO_FEAT_sh_DATAURL = 0;
    	
    	$objInfoLib = new objInfoC();
    	$raw_opt=array('vario'=>1);
    	$this->features =$objInfoLib->getParamsRaw ( $sqlo, 'CONCRETE_PROTO', $this->id, $raw_opt);
    	$a_proto_id = $this->features['vals']['ABSTRACT_PROTO_ID'];
    	if ($a_proto_id) {
    	    // abstract_proto features
    	    $sqlo->Quesel('* FROM ABSTRACT_PROTO WHERE ABSTRACT_PROTO_ID='.$a_proto_id);
    	    $sqlo->ReadArray();
    	    $this->ap_features = $sqlo->RowData;
    	    
    	    $this->PROTO_FEAT_sh_DATAURL = oS_VARIO_sub::getValByTabKey($sqlo, 'ABSTRACT_PROTO', $a_proto_id, 'inp.DATAURL');
    	    
    	}
    }
    function set_mo_CERT_FLAG($mo_CERT_FLAG) {
        $this->mo_CERT_FLAG = $mo_CERT_FLAG;
    }
    function set_mo_obj($t, $id) {
        $this->mo_obj=array('t'=>$t, 'id'=>$id);
        $this->SUC_plan_check_Lib = NULL;
        
    }
    
    function _featColFlat($key, $valout) {
        $keynice = columnname_nice2('CONCRETE_PROTO', $key);
        echo '<span style="color:gray;">'.$keynice.':</span> '.$valout.'&nbsp;&nbsp; ';
    }
    
    private function _featRow($keynice, $valout) {
    	echo '   <tr><td valign=top align=right><span style="color:gray;">'.$keynice.'</span>:&nbsp;</td><td>'.$valout.'</td></tr>'."\n";
    }
    
    /**
     * show feature fields
     * 
     * @param object $sqlo
     * @param $editAllow
     */
    private function _showFeatFields( &$sqlo, $editAllow) {
    	
        $allowArr_EDIT = array( 'EXEC_DATE', 'NOTES');
        $allowArr_READ = array( 'NOTES', 'VARIO.DATAURL' ); // 'EXEC_DATE': will be shown above ...
    	$xargu         = &$this->features['vals'];
    	
    	if ($this->PROTO_FEAT_sh_DATAURL) {
    	    $allowArr_EDIT[]='VARIO.DATAURL';
    	}
    	
    	echo '<ul>' . "\n";
    	
        if ($editAllow) {
            foreach( $allowArr_EDIT as $key) {
    		    $shown=0;
    			$val = $xargu[$key];
    			switch ( $key ) {
    				case 'EXEC_DATE': 
    					$valout = '<input type=text name="xargu[EXEC_DATE]" value="'.$val.'" size=20>'.
    					' <span style="color:gray;">(format: YYYY-MM-DD)</span>';
    					break;
    
    				case 'NOTES':
    				    $tmpsize = ' cols="80"'; // rows="1"
    					$valout = '<textarea name="xargu[NOTES]" '.$tmpsize.' style="height:1.2em; width:500px;">'.$val.'</textarea>';
    					break;
    				case 'VARIO.DATAURL':
    				    $val = $this->features['vario']['DATAURL'];
    				    $keynice='Raw-Data-URL';
    				    $valout = '<input type="text" name="xargu[VARIO.DATAURL]"  value="'.$val.'" style="width:600px;">';
    				    echo '<br><span style="color:gray;">'.$keynice.':</span> '.$valout;
    				    $shown=1;
    				    break;
    			}
    			
    			if (!$shown) $this->_featColFlat($key, $valout);
       		}
    		
    		
        } else {
            
            echo "<table border=0 cellspacing=0 cellpadding=0 ><tr><td>\n"; // bgcolor=#E8E8E8 white bgcolor
            foreach( $allowArr_READ as $key) {
    		    $valout= NULL;
    			$val   = $xargu[$key];
    			$show_default=1;
    			switch ( $key ) {
    				case 'EXEC_DATE': 
    					$valout = $val;
    					break;
    					
    				case 'NOTES':
    					$valout = "<pre>".htmlspecialchars($val)."</pre>";
    					break;
    				case 'VARIO.DATAURL':
    				    $show_default=0;
    				    $keynice = 'Raw-Data-URL';
    				    $val  = $this->features['vario']['DATAURL'];
    				    $valout = $val;
    				    break;
    			}
    			
    			
    			if ($val!=NULL) {
    			    // just show the data, if a value is given ...
    			    if ($show_default) $keynice = columnname_nice2('CONCRETE_PROTO', $key);
    			    $this->_featRow($keynice, $valout);
    			}
    		}
    		echo "</td></tr></table>\n";
    	}
    
        echo '</ul>' . "\n";
    }
    
    /**
     * show policy warnings
     * @param  $sqlo
     * @param  $cpid
     * @param int $mo_CERT_FLAG
     */
    static function policyAnalysis(&$sqlo, $cpid, $mo_CERT_FLAG ) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        debugOut('(111) do policyAnalysis.', $FUNCNAME, 2);
        
    	$releaseCheckLib = new oPRC_policy($sqlo);
    	
    	$cpOpt = NULL;
    	if ($mo_CERT_FLAG)
    	    $cpOpt = array( 'mo.CERT_FLAG'=>$mo_CERT_FLAG );
    	
    	$infoarr = $releaseCheckLib->policyInfo( $sqlo, $cpid, 0, $cpOpt );
    	if ($infoarr['err']>0) {
    		echo "<ul>";
    		htmlInfoBox( "QM Warning", "", "open", "WARN" );
    		echo $infoarr['txt'];
    		htmlInfoBox( "", "", "close" );
    		echo "</ul>\n";
    	}
    }
    
    private function _nonQmWarn($sqlo, $cpid) {
    	
    	$releaseCheckLib = new oPRC_policy($sqlo);
    	
    	$infoarr = $releaseCheckLib->nonQmInfo( $sqlo, $cpid );
    	if ($infoarr['err']>0) {
    		echo "<ul>";
    		htmlInfoBox( "Non-QM Warnings", "", "open", "INFO" );
    		echo $infoarr['txt'];
    		htmlInfoBox( "", "", "close" );
    		echo "</ul>\n";
    	}
    }
    
    // @swreq REQ-0028 o.MORDER > define EXPECTED protocols STEP DETAILS
    private function _plan_check($sqlo) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        debugOut('(203) do protocol planned parameter check', $FUNCNAME, 2);
        
        if ($this->SUC_plan_check_Lib==NULL) {
            // initialize ...
            $this->SUC_plan_check_Lib = new oCONCRETE_SUBST_plan_check($sqlo, $this->mo_obj['id'] );
        }
        
        debugOut('(210) has defs? '.$this->SUC_plan_check_Lib->has_def_params , $FUNCNAME, 2);
        if (!$this->SUC_plan_check_Lib->has_def_params) return;
        
        $infoarr = $this->SUC_plan_check_Lib->check_proto( $sqlo, $this->id );
        if ($infoarr['err']===0) {
            echo "<ul>";
            htmlInfoBox( "Planned Parameters check", "", "open", "CALM" );
            echo 'Check ok. '.$infoarr['txt'];
            htmlInfoBox( "", "", "close" );
            echo "</ul>\n";
        }
        if ($infoarr['err']>0) {
            echo "<ul>";
            htmlInfoBox( "Planned Parameters check", "", "open", "WARN" );
            echo $infoarr['txt'];
            htmlInfoBox( "", "", "close" );
            echo "</ul>\n";
        }
    }
    
    /**
     * show the editor form
     * @param  $sql
     * @param  int $dummy -- param not used anymore
     * @param  int $editAllow 0,1
     * @param  array $option
     *   'backurl'  : string of backurl
     * 	 'cs.inp'   : input type: [dblink], text
     * 	 "showReq"  :    0,1
     *   "qm_check" :    0,1 do QM-check?
     *   "nonqm_check" : 0,1 do Non-QM-checks?
     *   "plan_check"  : 0,1 do protocol detail plan check? works only with $this->mo_obj
     *   "hiddenFields" : array(key=>val) extra array of hidden form fields
     *   'mo.CERT_FLAG': optional contains CERT_FLAG of mother; e.g. of SUC
     */
    function showform( &$sql, $editAllow, $option=NULL ) {
    	
    	
    	$id = $this->id;
    	$protoFeatFields = $this->withFeatFields;
       
        $this->mo_CERT_FLAG = $option['mo.CERT_FLAG'];
        
    	$backurl="";
        if ( $option["backurl"]!="" ) $backurl = $option["backurl"];
    	
    	$proShLib = new oProtocolShowC();
    	
        // for buttons you can define input.tbut
        echo '<style type="text/css">'."\n";
        echo "input.tbut { border:solid 1px;  background-color:#DFDFDF; border-color:#808080}\n";
        echo '</style>'."\n";
        
        js__openproj();
    	
    	$proShLib->writeJavascript();
    
    	$optShow = unserialize($_SESSION['userGlob']["o.CONCRETE_PROTO.show"]);
    
    	
    	$a_proto_id = $this->features['vals']['ABSTRACT_PROTO_ID'];
    	
    	$sqls= "select count(*) from CONCRETE_PROTO_STEP where CONCRETE_PROTO_ID=".$id;
     	$sql->query("$sqls");
     	$sql->ReadRow();
     	$c_steps=$sql->RowData[0];
    	
    	$sqls= "select count(*) from ABSTRACT_PROTO_STEP where ABSTRACT_PROTO_ID=".$a_proto_id;
     	$sql->query("$sqls");
     	$sql->ReadRow();
     	$steps=$sql->RowData[0];
    	if ( !$steps ) {
    		echo "&nbsp;<font color=gray>No steps in the abstract protocol defined...</font>";
    	} else {
    		$stepOpt = NULL;
    		$stepOpt["abstrStepsShow"] = $optShow["abstrStepsShow"];
            $stepOpt["showGoCol"] 	   = 0;
    		$stepOpt["showStepLnk"]    = 1;
    		if ($option['cs.inp']!=NULL) $stepOpt['cs.inp'] =$option['cs.inp'];
    		if ($option["showRequ"]!=0)   $stepOpt['showRequ']=$option["showRequ"];
    		$stepOpt["qc"]    = 1; //NEW:2020
    		$dummy2=NULL;
    		$proto_step_c = new  protostep( $sql, $dummy2, $a_proto_id, $editAllow, 0, $id,  $stepOpt);
    		
    		if ( $editAllow ) {
    			echo '<form name="xtraform" ACTION="obj.concrete_proto.paras.php" METHOD=POST>'."\n"; 
    		}
    		
    		if ( $protoFeatFields ) {
    			$this->_showFeatFields($sql, $editAllow);
    		}
    		
    		$step_arrayX  = $proto_step_c->getStepArray();
    		$proto_step_c->table_init();
    		$step_count = sizeof( $step_arrayX ) ;	
    		$i=0;
     		while ( $i< $step_count ) {
    			$step_nr = $step_arrayX[$i];
    			$proto_step_c->outstep( $step_nr, $sql );
    			$i++;
    	  	}
    		
    		if ( $editAllow ) {
                echo '<tr bgcolor=#EFEFEF align=center height=30><td valign=middle colspan='.$proto_step_c->colspan.
    				'><input type=submit value="Submit" class="yButton"></td></tr>';
    		}
            
    		$proto_step_c->table_end();
    		
    		if ( $option["qm_check"] ) {
    		    $this->policyAnalysis($sql, $id, $this->mo_CERT_FLAG);
    		}
    		if ( $option["nonqm_check"] ) {
    			$this->_nonQmWarn($sql, $id);
    		}
    		if ( $option["plan_check"] ) {
    		    $this->_plan_check($sql);
    		}
    		
    		if ( $editAllow ) {
                echo '<input type=hidden name=conc_proto_id value="'.$id.'">'."\n";
    			if ($backurl!="") echo '<input type=hidden name=backurl value="'.urlencode($backurl).'">'."\n";
    			if (is_array($option["hiddenFields"])) {
    				reset ($option["hiddenFields"]);
    				foreach( $option["hiddenFields"] as $key=>$val) {
    					echo '<input type=hidden name="'.$key.'" value="'.$val.'">'."\n";
    				}
    			}
    			echo "</form>";  
                echo "<ul>";
    			
    			/*
                $topt=NULL;
                $topt["width"]="AUTO";
    			$topt["fcolor"]="#D0D0D0";
    			$topt["color"] ="#F8F8F8";
    			echo "<br>";
                htmlInfoBox( "Legend for icons", "", "open", "CALM", $topt ); 
                echo "<font color=gray>";
    			echo "<img src=\"images/but.gocp13.gif\"> link to the concrete protocol step (only shown, if an entry exists)<br>";
    		    echo "<img src=\"images/but9.del.gif\"> mark step inactive<br>\n";
    			echo "</font>";
    		    htmlInfoBox( "", "", "close" );
    			*/
                echo "</ul>\n";
            }	
    	}
    
    	return 0;
    }
    
    /**
     * OLD: $proto_id, $mother_table
     * - manage the head lines of the protocol editor
     * - checks for access rights of the protocol
     * @return 
     * @param object $sqlo
     * @param int $c_proto_id
     * @param int $editAllowIn [0,1] : 1 can be downgraded to 0, so if no right to edit the mother,
     * 			you have also no right to edit the protocol
     * @param string $nicename
     * @param array  $optRows optional info-rows array()= array($key, $val)
     * @param array  $optarr options: 
     * 	'headAdd' : addtional string behind TITLE
     * 	'sh_info_head' : [1] : show it, -1: do not show it
     * 	'sh:AP:NOTES' : 0,1 : show ABSTRACT_PROTO:NOTES
     *  'backurl' back url
     */
    function showIntro( &$sqlo, $editAllowIn, $nicename, $optRows=array(), $optarr=array() ) {
    	$editAllow  = $editAllowIn;
    	$tableProto = "CONCRETE_PROTO";
    	
    	$c_proto_id = $this->id;
    	$this->withFeatFields = 1; // with feature fields !!!
    	
    	$show_proto_info_head = 1; // show protocol info header or Not ?
    	if ($optarr['sh_info_head']<0) {
    		$show_proto_info_head = 0;
    		$this->withFeatFields = 0;
    	}
    	
    	$optHeadTxt = $optarr['headAdd'];

        $cp_feat       = &$this->features['vals'];
        $a_proto_id    = $cp_feat['ABSTRACT_PROTO_ID'];
        $a_formula_has = 0;
 
        $ap_feat = &$this->ap_features;
    	if ($ap_feat['FORMULA']!=NULL) $a_formula_has = 1;
        
        
        $o_rightsProto = access_check($sqlo, $tableProto, $c_proto_id, $cp_feat['CCT_ACCESS_ID']);
    	$acc_write = $o_rightsProto["write"];
    	$editAllow = min($editAllow,$acc_write);
        
    	if ( !$acc_write ) {
    		$accInfo=' (no write access!)';
    		if ($editAllowIn) $accInfo .= ' <img src="images/i13_warning.gif">';	
    	} else  $accInfo=NULL;
    	
    	echo '<ul style="margin-top:12px;">';
    	echo '<img src="images/icon.CONCRETE_PROTO.gif"> ';
    	echo '<font style="color:gray; font-weight:bold; font-size:1.2em;">'.
    		$nicename.' editor</font> '.$optHeadTxt;
    	echo ' <a href="edit.tmpl.php?t=CONCRETE_PROTO&id='.$c_proto_id.'" title="show details"><img src="images/but.view.gif"></a>';
    	if ($editAllow and $a_formula_has) {
    	    echo ' <a href="p.php?mod=DEF/o.CONCRETE_PROTO.formula_p&id='.$c_proto_id.'&backurl='.urlencode($optarr["backurl"]).'" title="compute data from formula">Compute</a> ';
    	}
    	echo '<br />';
    	
    	
    	
    	if ($show_proto_info_head) {
    	
    		//echo '<font color=gray>protocol (concrete):</font> '.
    		//	'[<a href="edit.tmpl.php?t=CONCRETE_PROTO&id='.$c_proto_id.'" style="color:gray;">PRC-ID:'.$c_proto_id.'</a>]'.
    		//	$accInfo;
    			
    		if ($cp_feat['ACCEPT_PROT_ID']>0) {
    			// show acceptance protocol
    			echo '&nbsp;&nbsp;&nbsp; <font color=gray>acceptance protocol :</font> [ID:'.$cp_feat['ACCEPT_PROT_ID'].']'.
    				' [<a href="edit.tmpl.php?t=CONCRETE_PROTO&id='.$c_proto_id.'&xmode=qc">show Limits</a>]';
    			
    		}
    		echo '<br /><span style="color:gray;">'.tablename_nice2('ABSTRACT_PROTO') .':</span> '.  
    			obj_nice_name ( $sqlo, 'ABSTRACT_PROTO', $a_proto_id ).' '. 
    			'[<a href="edit.tmpl.php?t=ABSTRACT_PROTO&id='.$a_proto_id.'" style="color:gray;">PRA-ID:'.$a_proto_id.'</a>]';
    		
			if ( $this->features['vals']['EXEC_DATE']) {
			    $keynice = columnname_nice2('CONCRETE_PROTO', 'EXEC_DATE');
			    echo ' &nbsp;&nbsp;&nbsp;<span style="color:gray;">'.$keynice.':</span> '.$this->features['vals']['EXEC_DATE'];
			}
    		echo "<br />\n"; 
    		
    		if (is_array($optRows)) 
        		if (sizeof($optRows)) {
        			foreach( $optRows as $dummy=>$valarr) {
        				echo '<span style="color=gray">'.$valarr[0].':</span> '.$valarr[1]. "<br />\n"; 
        			}
        			
        		}
    		
    		if ($optarr['sh:AP:NOTES']>0 and $ap_feat['NOTES']!='') {
    			// show NOTES
    			echo '<span style="color=gray">protocol (abstract):notes: </span>'."<br />\n";
    			f_text_html::notes_out($sqlo, $ap_feat['NOTES'], 1);
    		}
    	
    	}
    		
    	echo '</ul>'."\n";
    	
    	return ($editAllow);
    }

}
