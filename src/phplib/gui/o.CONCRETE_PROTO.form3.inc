<?php
require_once ("gui/o.PROTO.stepout1.inc");
require_once ("f.objview.inc");	

/**
 * protocol editor FORM3 mashup-component: EXCEL like ...
 * - can be used by other GUI-components 
 * - on submit, calls the DB_save_plugin: obj.concrete_proto.paras.php
 * - option 'backurl' defines the URL, which will be forwarded after the DB_save_plugin
 * @package o.CONCRETE_PROTO.form3.inc 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class oCONCRETE_PROTO_form3Table {
    
    /**
     * 
     * @var array
     *  'head' : list
     *  'rows' : list
     *     '1:AN', '3:S'
     */
    private $layout; 
    
    function __construct($layout, $pra_steps, $c_step_array, $editAllow) {
        $this->layout=$layout; 
        $this->pra_steps=$pra_steps;
        $this->c_step_array=$c_step_array;
        $this->editAllow=$editAllow;
        
        $this->col_num = sizeof($this->layout['head']);
        $this->row_num = sizeof($this->layout['rows']);
        
        $this->jsFormLib = new gJS_edit();
    }
    
    function get_colspan() {
        return  $this->col_num;
    }
    
    private function table_init() {
        echo '<table cellpadding=2 cellspacing=3 style="empty-cells:show;">';
        echo '<tr align=center style="background-color:#E0E0E0;">'."\n";
        echo "<th class=tprot>#</th>"; // line number
        foreach( $this->layout['head'] as $text) {
            echo "<th class=tprot>".$text."</th>";
        }
        echo "</tr>\n";
    }
    
    // PRIVATE ....
    // $table : ['CONCRETE_SUBST'], 'CHIP_READER'
    private function _js_fields_out( $step_nr, $cobj_id, $ca_objname, $modalButId, $tmp_cond, $table=NULL) {
        // - for modal-dialog purposes
        // - the JS-function openproj()  comes from edit.tmpl.php
        
        // $tmplen="";
        // if ( strlen($ca_objname)>5 ) $tmplen = " style=\"width:".(strlen($ca_objname)*0.6)."em;\"";
        if ($table=='') {
            $table   = 'CONCRETE_SUBST';
        }
        
        if ($table=='CHIP_READER') {
            $varname = 'devids['.$step_nr.']';
            $butID  = 'xd'.$modalButId;
        } else {
            $varname = 'concsubst['.$step_nr.']';
            $butID  = 'x'.$modalButId;
        }
        
        $name_out = $ca_objname; // if no object: contains string of SPACES for nice input-button
        if ($cobj_id) {
            $name_out = $cobj_id.": ".$ca_objname;
        }
        
        $this->jsFormLib->setTable($table, $butID);
        $olopt= array('butxtra'=> 'class=tbut');
        $out  = $this->jsFormLib->getObjLnk( $varname, $cobj_id, $name_out, $olopt);
        
        
        $osopt = NULL;
        $out   .=  $this->jsFormLib->getObjSelector( $osopt );
        $butIdTmp = $this->jsFormLib->butid;
        $out   .=  "<a href=\"javascript:openwin('".$table."','".$butIdTmp."','".$tmp_cond."','')\">";
        $out   .=  "<IMG src=\"images/select_abs.svg\" TITLE=\"select by abstract\"></a> ";
        return $out;;
        
    }
    
    private function rowout($row, $linecnt) {
        
        $datarow=array();
        
        foreach($row as $cell) {
            $cellarr=explode(':',$cell);
            $step = $cellarr[0];
            $code = $cellarr[1];
            $cellv='?';
            
            $stepc = &$this->c_step_array[$step];
            $stepa = &$this->pra_steps[$step];
            
            $sua_id = $stepa['ABSTRACT_SUBST_ID'];
            $mv     = $stepa['MANDATORY_VALUES'];
            
            switch ($code) {
                case 'AN':
                    $cellv= $this->pra_steps[$step]['NAME'];
                    break;
                case 'AQ':
                    $cellv= $this->pra_steps[$step]['QUANTITY'];
                    
                    break;
                case 'AS':
                    
                    $sua_id = $this->pra_steps[$step]['ABSTRACT_SUBST_ID'];
                    if ($sua_id) {
                        $nice = $this->pra_steps[$step]['SUA.name'];
                        $cellv = $nice .' ['.$sua_id.']';
                    } else {
                        $cellv= '?no MAA?';
                    }
                    break;
                case 'Q':
                    $cellv= $this->c_step_array[$step]['QUANTITY'];
                    $value_edit=0;
                    if (strstr($mv, 'q')!=NULL ) $value_edit=1; // MANDATORY_VALUE must contaion q
                    if ($value_edit and $this->editAllow) {
                        $cellv="<input name=quanti[".$step."] value=\"".$cellv."\"  onchange=\"myCh(this, 'q')\" size=10>";
                    }
                    break;
                case 'S':
                    if ($sua_id) {
                        
                        $cellv='';
                        
                        $suc_id = $stepc['CONCRETE_SUBST_ID'];
                        $csname = $stepc['SUC.name'];
                        if ($this->editAllow) {
                            $cellv = $stepa['SUA.name']. ' [MAA:'.$sua_id.'] ';
                            if ($suc_id) $cellv .='<br>';
                            $tmp_cond = "&condclean=1&searchAlias=absID&searchtxt=".$sua_id;
                            $cellv .= $this->_js_fields_out($step, $suc_id, $csname, $step, $tmp_cond) . '<br>'.
                                $stepc['SUC.SUA.name'];
                        } else {
                            if ($suc_id) {
                                $cellv = '<b><a href="edit.tmpl.php?t=CONCRETE_SUBST&id='.$suc_id.'">'.$stepc['SUC.name'].
                                ' [MAC:'.$suc_id.']</a></b><br>' . $stepc['SUC.SUA.name']. ' [MAA:'.$stepc['SUC.SUA.id'].']';
                            } else {
                                $cellv .= $stepa['SUA.name']. ' [MAA:'.$sua_id.']';
                            }
                        }
                        
                    } else {
                        $cellv='';
                    }
                    break;
                case 'N':
                    $cellv= $this->c_step_array[$step]['NOTES'];
                    if ($this->editAllow) {
                        $cellv="<input name=newnote[".$step."] value=\"".$cellv."\" size=10>";
                    }
                    break;
                default:
                    if ($cell!='') {
                        $cellv= '?C:'.$code;
                    } else {
                        $cellv='';
                    }
                    break;
            }
            $datarow[]=$cellv;
            
        }
        
        echo '<tr valign="top">'."\n";
        echo "<td>".($linecnt+1)."</td>"; 
        foreach( $datarow as $text) {
            echo "<td>".$text."</td>";
        }
        echo "</tr>\n";
        
    }
    
    function maintable() {

        $this->table_init();

        $i=0;
        while ( $i< $this->row_num ) {
            $row = $this->layout['rows'][$i];
            $this->rowout( $row, $i );
            $i++;
        }
    }
    
    function table_end() {
        echo "</table>\n";
    }
}




/**
 * show protocol features + the protocol step table
 * @author steffen
 *
 */
class oCONCRETE_PROTO_form3 {
	
	
	private $id;         // CONCRETE_PROTO_ID
	private $mo_suc_id;  // mother CONCRETE_SUBST_ID of protocol
	private $_initialized = 0;
	
	/**
	 * 
	 * @var array $c_step_array
	 *  'SUC.name'
	 *  'SUC.SUA.name'
	 *  SUC.SUA.id
	 */
	private $c_step_array;
	
	function __construct($id=NULL) {
		
		$this->withFeatFields = 0;
		if ($id) {
			$this->setProtoID($id);
		}
	}
	
	
	
	/**
	 * init protocol
	 * @param int $c_proto_id
	 * @param number $sucid
	 */
	function setProtoID($sqlo, $c_proto_id, $sucid=0) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    
		$this->mo_suc_id = $sucid;
		$this->id        = $c_proto_id;
		$this->features  = NULL;
		$this->_initialized = 1;
		
		$id = $this->id ;
		$sqlo->Quesel('* FROM CONCRETE_PROTO WHERE CONCRETE_PROTO_ID='.$id);
		$sqlo->ReadArray();
		$this->features = $sqlo->RowData;
		$a_proto_id = $sqlo->RowData['ABSTRACT_PROTO_ID'];

	    // abstract_proto features
	    $sqlo->Quesel('* FROM ABSTRACT_PROTO WHERE ABSTRACT_PROTO_ID='.$a_proto_id);
	    $sqlo->ReadArray();
	    $this->ap_feat = $sqlo->RowData;
	    
	    $this->a_formula_has = 0;
	    if ($this->ap_feat['FORMULA']!=NULL) $this->a_formula_has = 1;

		
		
		$sqlo->Quesel("count(*) from ABSTRACT_PROTO_STEP where ABSTRACT_PROTO_ID=".$a_proto_id);
		$sqlo->ReadRow();
		$this->num_steps=$sqlo->RowData[0];
		if (!$this->num_steps) {
		    $error->set( $FUNCNAME, 1, 'No steps defined.' );
		    return;
		}
		
		$this->protLib = new gProtoOrg( );
		$orgopt=array("noStepScan"=>1);
		$this->protLib->setProto( $sqlo, $a_proto_id, $id, $orgopt);
		
		$this->pra_steps = $this->protLib->get_pra_all_steps($sqlo, $a_proto_id);
		foreach($this->pra_steps as $i=>$row) {
		    if ($row['ABSTRACT_SUBST_ID']) {
		        $this->pra_steps[$i]['SUA.name'] = obj_nice_name ( $sqlo, 'ABSTRACT_SUBST', $row['ABSTRACT_SUBST_ID'] ); 
		    }
		}
		
		$columns=array('QUANTITY','CONCRETE_SUBST_ID', 'NOTES');
		$this->c_step_array = $this->protLib->get_all_data($sqlo, $id, $columns);
		foreach($this->c_step_array as $i=>$row) {
		    if ($row['CONCRETE_SUBST_ID']) {
		        $suc_feat = glob_elemDataGet3( $sqlo, 'CONCRETE_SUBST', array('CONCRETE_SUBST_ID'=>$row['CONCRETE_SUBST_ID']),
		            array('NAME', 'ABSTRACT_SUBST_ID') );
		        $suc_sua_name = obj_nice_name ( $sqlo, 'ABSTRACT_SUBST', $suc_feat['ABSTRACT_SUBST_ID'] );
		        $this->c_step_array[$i]['SUC.name'] = $suc_feat['NAME'];
		        $this->c_step_array[$i]['SUC.SUA.name'] = $suc_sua_name;
		        $this->c_step_array[$i]['SUC.SUA.id']   = $suc_feat['ABSTRACT_SUBST_ID'];
		        
		    }
		}

		
		// check layout$thi
		if ($this->ap_feat['LAYOUT']==NULL) {
		     $error->set( $FUNCNAME, 2, 'No LAYOUT given.' );
		     return;
		}
		$this->layout = json_decode($this->ap_feat['LAYOUT'],TRUE);
		if (empty($this->layout['head'])){
		    $error->set( $FUNCNAME, 3, 'Bad LAYOUT format..' );
		    return;
		}
	}
	

	/**
	 * show the editor form
	 * @param  $sql
	 * @param  int $editAllow 0,1
	 * @param  $option
	 *   'backurl'  : string of backurl
	 * 	 'cs.inp'   : input type: [dblink], text
	 * 	 "showReq"  :    0,1
	 *   "qm_check" :    0,1 do QM-check?
	 *   "nonqm_check" : 0,1 do Non-QM-checks?
	 *   "hiddenFields" : array(key=>val) extra array of hidden form fields
	 *   'mo.CERT_FLAG': optional contains CERT_FLAG of mother; e.g. of SUC
	 */
	function showform( &$sqlo, $editAllow, $option=NULL ) {
		
		if (!$this->_initialized) sys_error_my( 'class not initialized.' );
		
		$id = $this->id;
	    $this->features  = NULL;
	    
		$backurl="";
	    if ( $option["backurl"]!="" ) $backurl = $option["backurl"];
		
	    // for buttons you can define input.tbut
	    echo '<style type="text/css">'."\n";
	    echo "input.tbut { border:solid 1px;  background-color:#DFDFDF; border-color:#808080}\n";
	    echo '</style>'."\n";
	    
	    js__openproj();
		

		if ( !$this->num_steps ) {
			echo "&nbsp;<font color=gray>No steps in the abstract protocol defined...</font>";
			return 0;
		}

		$protoTableLib = new oCONCRETE_PROTO_form3Table( $this->layout, $this->pra_steps, $this->c_step_array, $editAllow);
		
		if ( $editAllow ) {
			echo '<form name="xtraform" ACTION="obj.concrete_proto.paras.php" METHOD=POST ENCTYPE="multipart/form-data">'."\n"; 
		}
	
		$protoTableLib->maintable();
		$colspan = $protoTableLib->get_colspan();
		
		
		
		if ( $editAllow ) {
	            echo "<tr bgcolor=#6699FF align=center height=30><td valign=middle colspan=". $colspan.
				"><input type=submit value=\"   Submit   \"></td></tr>";
		}
	        
		$protoTableLib->table_end();
		
		
		
		if ( $editAllow ) {
	           echo '<input type=hidden name=conc_proto_id value="'.$id.'">'."\n";
	           echo '<input type=hidden name=mo_suc_id value="'.$this->mo_suc_id.'">'."\n";
			if ($backurl!="") echo '<input type=hidden name=backurl value="'.urlencode($backurl).'">'."\n";
			if (is_array($option["hiddenFields"])) {
				reset ($option["hiddenFields"]);
				foreach( $option["hiddenFields"] as $key=>$val) {
					echo '<input type=hidden name="'.$key.'" value="'.$val.'">'."\n";
				}
			}
			echo "</form>";  
	        echo "<ul>";
				
	
	        echo "</ul>\n";
	    }	
		
	
		return 0;
	}
	
	/**
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
	 *  'backurl'
	 */
	function showIntro( &$sqlo, $editAllowIn, $nicename, $optRows=NULL, $optarr=NULL ) {
		$editAllow  = $editAllowIn;
		$tableProto = "CONCRETE_PROTO";
		
		if (!$this->_initialized) sys_error_my( 'class not initialized.' );
		
		$c_proto_id = $this->id;
		$this->withFeatFields = 1; // with feature fields !!!
		
		//$show_proto_info_head = 1; // show protocol info header or Not ?
		if ($optarr['sh_info_head']<0) {
			//$show_proto_info_head = 0;
			$this->withFeatFields = 0;
		}
		
		$optHeadTxt = $optarr['headAdd'];

		
	    $o_rightsProto = access_check($sqlo, $tableProto, $c_proto_id, $this->features['CCT_ACCESS_ID']);
		$acc_write = $o_rightsProto["write"];
		$editAllow = min($editAllow,$acc_write);
	    
		
		echo '<ul style="margin-top:12px;">';
		echo '<img src="images/icon.CONCRETE_PROTO.gif"> ';
		echo '<font style="color:gray; font-weight:bold; font-size:1.2em;">'.
			$nicename.' editor</font> '.$optHeadTxt;
		echo ' <a href="edit.tmpl.php?t=CONCRETE_PROTO&id='.$c_proto_id.'" title="show details"><img src="images/but.view.gif"></a>';
		
		if ($editAllow and $this->a_formula_has) {
		    echo ' <a href="p.php?mod=DEF/o.CONCRETE_PROTO.formula_p&id='.$c_proto_id.'&backurl='.urlencode($optarr["backurl"]).'" title="compute data from formula">Compute</a> ';
		}
		
		echo '<br />';
		
		echo '</ul>'."\n";
		
		return ($editAllow);
}

}
