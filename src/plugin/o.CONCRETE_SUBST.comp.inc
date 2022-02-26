<?php
/**
 * o.CONCRETE_SUBST > list of/ objects > protocol OVERVIEW compare/create
 *    
 * @package o.CONCRETE_SUBST.comp.inc 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq UREQ:0002520: o.CONCRETE_SUBST > list of objects > protocol compare/edit 
 * @param $editmode
 * @param $obj_ref_id ID of model object
 * @param $creaOrd for creation: the XORDER-number
 * @version $Header: trunk/src/plugin/o.CONCRETE_SUBST.comp.inc 59 2018-11-21 09:04:09Z $
 */
require_once ('glob.obj.proto_act.inc');  
require_once 'o.AS_HAS_PR.subs.inc';  
require_once 'o.CS_HAS_PR.subs.inc';  

/**
 * do actions for one substance
 * @author steffen
 *
 */
class oSUC_manageOne {
	
	function __construct() {
		$this->csProtoLogLib = new oCS_HAS_PR_subs();
	}
	
	function initHeader( $abs_proto_arr ) {
		$this->abs_proto_arr = $abs_proto_arr;
	}
	
	/**
	 * show all protocols for one object; create one protocol
	 * @param $sql
	 * @param $sql2
	 * @param $obj_id
	 * @param $bgcolor
	 * @param $table
	 * @param $opt
	 * 		"go" 
            "editmode"
            "extrakey"   : extra string before object name
            "c_model_id" -- concrete_proto_id 
            "a_model_id"
            "is_model"
            "creaOrd"  the XORDER-number
            "creaType" type of protocol: 1,2
        @return array ( XORD => array( 'cpid'=>ID of protocol, 'type'=>1,2 ) )
	 */
	function objProtoOut( 
	    &$sql, 
	    &$sql2, 
	    $obj_id,  // e.g. the CONCRETE_SUBST_ID
	    $bgcolor,
		$opt=NULL   
	    ) { 
	     
	    $access_info = "";
	    // $tmpl_name   = "&nbsp;"; 
	    $table       = 'CONCRETE_SUBST';
	    $tablePkName = $table . "_ID";
	    $retarr      = NULL;
	    
	    // if ( $opt["is_model"] AND $opt["editmode"] == "edit" ) $tmpl_name   = "<font color=red>missing</font>";
		
	    $sqls= "select x.NAME, a.ABSTRACT_SUBST_ID, a.NAME from ".$table." x join ABSTRACT_SUBST a".
	    	" on x.ABSTRACT_SUBST_ID=a.ABSTRACT_SUBST_ID where x.".$tablePkName."=". $obj_id;
	 	$sql->query($sqls);
	 	$sql->ReadRow();
	 	
	 	$objx=array();
	 	$objx["name"]       = $sql->RowData[0];
	 	$objx["asid"]       = $sql->RowData[1];
	 	$objx["abs_name"]   = $sql->RowData[2];
	 	
	 	$this->csProtoLogLib->setObj($obj_id);
	 	$cs_protoArr = $this->csProtoLogLib->getProtoLog($sql);
	    
	    
	    if ( $opt["editmode"] == "edit" AND !$opt["is_model"] ) {
	         $o_rights = access_check( $sql, $table, $obj_id);
	         $access_info = " <font color=green>access</font>"; 
	         if ( !$o_rights["insert"] ) {
	            $access_info = " <font color=red>modification denied</font>";
	         }
	    }
	
		echo "<tr bgcolor=".$bgcolor." valign=top>";
	    
	    // 1. column
	    
	    echo "<td  class=xt1>";  
	    
	    if ($opt["extrakey"]!="") echo $opt["extrakey"];
	    
	    echo "<a href=\"edit.tmpl.php?t=".$table."&id=".$obj_id."\">".$objx["name"]." [ID:".$obj_id."]</a>".$access_info."</td>";
	    echo "<td>".$objx["abs_name"]."</td>";
	    
	    //$c_proto_tmp        = "";
	    // $abstract_proto_id  = "";
	    $tmpcreainfo        = "";  
	    
	    // next columns
	    $retarr= array();
	    
	    if (is_array($this->abs_proto_arr)) {
		    
		    foreach( $this->abs_proto_arr as $oneProtoArr) {
	
		    	$absOrder   = $oneProtoArr['or'];
		    	$objx       = array();
		    	$objx['cpid'] = 0;
		    	$objx['ty']   = $oneProtoArr['ty'];
    	
		    	// get mapping protocol ID
		    	if ( is_array($cs_protoArr) ) {
			    	
			    	foreach( $cs_protoArr as $valarr) {
						$cs_order=$valarr['or'];
						if ($cs_order == $absOrder) {
							$objx['cpid'] = $valarr['cp'];
							$objx['ty']   = $valarr['ty'];
							break;
						}
			    	}
		    	}
		    	
		    	$retarr[$absOrder] = array('cpid'=>$objx['cpid'], 'type'=>$objx['ty'] );
		    	
			    echo "<td  class=xt1>";
			    if ( $objx['cpid'] ) {
			    	
					$sqls= "select c.ABSTRACT_PROTO_ID, a.NAME from  CONCRETE_PROTO c, ABSTRACT_PROTO a ".
			               " where  c.CONCRETE_PROTO_ID=".$objx['cpid']." AND c.ABSTRACT_PROTO_ID=a.ABSTRACT_PROTO_ID";
			  		$sql->query("$sqls");
					$sql->ReadRow();
					$objx["a_proto_id"]   = $sql->RowData[0];
					$objx["a_proto_name"] = $sql->RowData[1];	
			        
			        echo $objx["a_proto_name"];  
			        
				} else {

			        echo "&nbsp;";   		        
			    }
			    
			    /**
			     * for this order in protocol-log ...
			     */
			    if ( $opt["c_model_id"] AND $o_rights["insert"] and $opt['creaOrd']==$absOrder ) {
			    
			        if ( !$objx['cpid'] ) {   // only, if no step exists 
			
			            // COPY steps from model NOW !!!
			            if ( $opt["go"]  ) {
			              
			                $proto_obj = new obj_proto_act();
			               
			               
			                $creopt = array('xord'=>$absOrder, 'type'=>$objx['ty']);
			                $tmpret    = $proto_obj->csubst_create_proto( $sql, $sql2, $table, $obj_id, 
			                                 $opt["a_model_id"], $opt["c_model_id"], $creopt );  
							
			                if ($tmpret<0) $tmpcreainfo = "<font color=red>Error during creation</font>";
			                else $tmpcreainfo = "<font color=green>Created</font>"; 
			
			                echo " ".$tmpcreainfo;                        
			            }
			        }
			    }
			 
			    echo "</td>";
		    }
	    }
		echo "</tr>\n";   
	    
	    return ($retarr);
	}  

}



class o_CONCRETE_SUBST_comp_XPL extends gPlugin {
	var $obj_modelArr; // array ( 'id', 'ord', "NAME" )
	var $first_abs_proto_arr; // init in _generalInit()
	
	function __construct(&$sqlo) {
		parent::__construct($sqlo);
	}
	
	function register() {
		$tablename	= 'CONCRETE_SUBST';
		$title		= 'OverviewCompare: compare/create protocols';
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title']    = $title;
		$this->infoarr['title_sh'] = 'OverviewCompare';
		$this->infoarr['form_type']= 'list'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_cnt']  = 1; 
		$this->infoarr['css']  = '
	th.xt1   { color:white; }
    th.xt2   { color:white; font-weight:normal; }
    th.xprot { color:white; background-color:#008000 }
    td.xt1   { font-size:0.8em; }
    td.xtextm{ font-size:0.8em; color:#606060;}';
		$this->infoarr['javascript']  =
		' function thisCreate( creaOrder ) {
		document.editform2.creaOrd.value=creaOrder; 
        document.editform2.submit();
    	}';
		$this->infoarr2 = array( 
			'accCheck' => array('tab'=>array('read'), 'obj'=>array() ),
			'listChkSelect'=>1
		);
		
	}
	
	/**
	 * select a model
	 */
	function _formModel1 ( $obj_modelArr, $tablename ) {
	    
		$obj_model_name=NULL;
		if ( is_array($obj_modelArr) ) {
			$obj_model_name = $obj_modelArr['NAME'];
		}
		echo "<form method=\"post\"  name=\"editform\"  action=\"p.php?mod=".$this->_mod."\" >\n";
	    echo "<table cellpadding=5 cellspacing=5 border=0 bgcolor=#EFEFEF>\n";
	    echo "<tr><td>\n";
	    echo "<B>Select a model:</B> "; 
	    $model_nameurl = $obj_model_name;
	    
	    if ($model_nameurl=="") $model_nameurl= " --- select ---" ; 
		$jsFormLib = new gJS_edit();
		$fopt = array();
		$answer = $jsFormLib->getAll($tablename, "obj_ref_id", $obj_model['id'], $model_nameurl, 0, $fopt);
		echo $answer;
	    echo "<input type=submit value=\"Take it!\">";
	
	    echo "</td></tr></table>\n";
	    echo "</form>\n"; 
	} 
	
	function _getObjects( &$sqlo, $sqlAfter) {
		// get selected objects
		$tablePkName     = "CONCRETE_SUBST_ID";
		$sqlo->query("SELECT ".$tablePkName." FROM ".$sqlAfter); 
		
		$sel      = array();
		$num_elem = 0;
		while ( $sqlo->ReadRow() ) { 
		    $sel[] = $sqlo->RowData[0];
		    $num_elem++;
		}   
		return $sel;
		
	}
	
	
	function _getAbsProtos(&$sqlo, $sua_id) {
		$suaProtoLib = new oAS_HAS_PR_subs($sua_id);
	    $proto_arr   = $suaProtoLib->getProtoLog($sqlo);
	    return $proto_arr;
	}
	
	/**
	 * init:
	 * - $this->first_abs_proto_arr
	 * - $this->firstFeat
	 * @param object $sqlo
	 */
	function _generalInit(&$sqlo) {
		
		$first_obj = $this->first_obj;
		
		$this->firstFeat = glob_elemDataGet3( $sqlo, 'CONCRETE_SUBST', array('CONCRETE_SUBST_ID'=>$first_obj), 
			array('ABSTRACT_SUBST_ID') ); 
			
			
		$asid = $this->firstFeat['ABSTRACT_SUBST_ID'];	
		$this->first_abs_proto_arr = $this->_getAbsProtos($sqlo, $asid);
		
	}
	
	function _modelShow(&$sql, &$sql2, $obj_modelArr, $editmode) {
		$opt = NULL; 
	    // $opt["extrakey"] = $tmpimg;
	    $opt["is_model"] = 1;
	    $opt["editmode"] = $editmode;
	    $opt["extrakey"] = 'model: ';
	    // array ( XORD=>array('cpid'=>) )
	    $this->obj_model_arr2  = $this->oneSubsLib->objProtoOut( $sql, $sql2, $obj_modelArr['id'], "#99DDFF", $opt );
	        
	    if ($editmode=="edit") { 
	        
	        //
	        // print CREATE button
	        //
	                
	        echo "<tr bgcolor=#99DDFF>";
	        if ( sizeof($this->obj_model_arr2) ) { 
	        
	            echo "<td  class=xt1><font color=gray>Create missing protocols:</font></td>\n";
	            echo "<td  class=xt1>&nbsp;</td>\n";
	            $tmparr = $this->obj_model_arr2;
	            
	            reset ($this->first_abs_proto_arr);
	   			foreach( $this->first_abs_proto_arr as $oneProtoArr) {
	   				$absOrder   = $oneProtoArr['or'];
					echo "<td>";
					
	   				if ( $this->obj_model_arr2[$absOrder]['cpid']>0 ) {  
		                echo "<input type=button name='dummy' value='Create from model' onclick=\"thisCreate(". $absOrder .");\">";
		            } else echo "&nbsp;";
		            
	   				echo "</td>\n";
				}
				

	            
	        }
	        echo "</tr>\n\n";
	    }
		     
	}
	
	/**
	 * get model protocol
	 * set on $opt: "c_model_id", "a_model_id", "go", "creaOrd", "creaType"
	 * @return $opt
	 */
	function _createCheck(&$sqlo, $opt) {
		
		$tmparr = $this->obj_model_arr2;
		$ord_id = $this->obj_modelArr['ord'];
		
		$cpid   = $tmparr[$ord_id]['cpid'];
	    $opt["c_model_id"] = $cpid;           // the model protocol   
	    $protoType = $tmparr[$ord_id]['type'];

	    if (!$opt["c_model_id"]) {    
	    
	        $this->go = 0;   // fall back
	        echo "<tr bgcolor=#FFD0D0>";
	        echo "<td  class=xt1 colspan=".$this->tab_colspan.">"; 
	        echo " <font color=red>Error:</font> concrete protocol from model missing";
	        echo "</td>";
	        echo "</tr>"; 
	        
	    } else {
		    $abs_proto_id  = glob_elementDataGet( $sqlo, 'CONCRETE_PROTO', 'CONCRETE_PROTO_ID', 
		    	$opt["c_model_id"], 'ABSTRACT_PROTO_ID');
		    $opt["a_model_id"] = $abs_proto_id;     // the model abstract protocol
		    $opt["creaOrd"]  = $this->obj_modelArr['ord'];
		    $opt["creaType"] = $protoType;
	    }
	      
	    $opt["go"] = $this->go; 
	    
	    return $opt;
	}
	
	/**
	 * show the table
	 * @param $sqlo
	 * @param $sel
	 * @param $editmode
	 */
	function _showTable(&$sql, &$sql2, &$sel, $editmode) {
		$go = $this->go;
		$tablex      = 'CONCRETE_SUBST';
		$obj_modelArr   = $this->obj_modelArr;
			
		
		$this->_generalInit($sql);
		
		
		if (!sizeof($this->first_abs_proto_arr)) {
			echo "<br>Warning: no protocols on first object found.<br>\n";
			return;
		}
		
		
		$this->oneSubsLib = new oSUC_manageOne();
		
		$this->oneSubsLib->initHeader( $this->first_abs_proto_arr );
		
		$this->tab_colspan = 2 + sizeof($this->first_abs_proto_arr);
		
		if ( $editmode=="edit" ) {
		    echo "\n\n<form method=\"post\"  name=\"editform2\"  action=\"".$this->selfUrl."\" >\n"; 
		}        
		    
		echo "<table border=0 cellpadding=1 cellspacing=1 bgcolor=#FFFFFF>\n";
		
		echo "<tr bgcolor=#336699>";
		echo "<th class=xt1>Substance</th>"; 
		echo "<th class=xt1>(abstract)</th>";
		for ( $i=0; $i<sizeof($this->first_abs_proto_arr ); $i++ ) {
			echo "<th class=xprot>Proto ".($i+1)."</th>\n";
		}
		echo "</tr>\n"; 
		
		$f_absid = $this->firstFeat['ABSTRACT_SUBST_ID'];
		$f_absName = obj_nice_name ( $sql, 'ABSTRACT_SUBST', $f_absid ); 
		
		echo "<tr bgcolor=#336699>";
		echo "<th class=xt1   > abstract subst from first </th>"; 
		echo "<th class=xt1   > ".$f_absName."</th>"; 
		
	   	foreach( $this->first_abs_proto_arr as $dummy=>$oneProtoArr) {
	   		$absOrder   = $oneProtoArr['or'];
	   		$apid = $oneProtoArr['ap'];
	   		if ($apid) {
	   			$f_apid_Name = obj_nice_name ( $sql, 'ABSTRACT_PROTO', $apid ); 
	   		} else {
	   			$f_apid_Name = '&nbsp;';
	   		}
	   		echo "<th class=xt2> ".$f_apid_Name."</td>"; 
	   	}
		echo "</tr>\n";   
		          
		// show model details
		
		$tmpimg = "<img src=\"images/icon.".$tablex.".gif\" border=0> <B>Model:</B> ";
		if ( is_array($obj_modelArr) and $obj_modelArr['id']>0 ) { 
		   $this->_modelShow($sql, $sql2, $obj_modelArr, $editmode);    
		} else {
		    echo "<tr bgcolor=#99DDFF>";
		    echo "<td  class=xt1 colspan=".$this->tab_colspan.">".$tmpimg." no model";
		    echo "</td>";
		    echo "</tr>";
		}
		
		$color1 = "#EFEFEF";  // SUN violett 
		$color2 = "#EFEFFF";   
		
		$opt = NULL;
		$opt["editmode"]   = $editmode;
		
		if ( $go ) {
			$opt = $this->_createCheck($sql, $opt); 
			if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
				glob_printr( $opt, "_showTable: creation-opt info" );
			}
		}  
		
		$color=NULL;
		foreach( $sel as $th0=>$th1) {
		
		    $obj_idx   = $th1; 
		    if ($color == $color1)  $color = $color2;
		    else $color = $color1;
		
		    $this->oneSubsLib->objProtoOut( $sql, $sql2, $obj_idx, $color, $opt );
		     
		} 
		echo "</table>\n"; 
		
		if ($editmode=="edit") {
			echo "<input type=hidden name='tablename' value='".$tablex."'>\n";
			echo "<input type=hidden name='creaOrd' value=''>\n";
			echo "<input type=hidden name='go' value='1'>\n";
		    echo "</form>\n\n";
		} 
	}

	function startMain() {
		global $error;
		
		$FUNCNAME='startMain';
		
		$tablename="CONCRETE_SUBST";
		$OBJ_MODEL_CNAME = "model_substance";
		$tablex          = "CONCRETE_SUBST";
		$tablePkName     = "CONCRETE_SUBST_ID";
		$this->tablexNice      = "substance (concrete)";
		
		$this->selfUrl = 'p.php?mod='.$this->_mod;
		
		$editmode  = $_REQUEST['editmode'];
		$obj_ref_id= $_REQUEST['obj_ref_id'];
		$go        = $_REQUEST['go'];
		$creaOrder = $_REQUEST['creaOrd'];
		
		$this->go  = $go;
		
		js_formAll();
		
		$sqlo  = &$this->_sqlo;
		$sqlo2 = &$this->_plug_regSql2();
		
		
		
		$KEY_EXISTOBJ_ID = "o.".$tablex.".create.obj_id";  // keyword of session variable
		$obj_modelArr    = $_SESSION['s_sessVars'][$KEY_EXISTOBJ_ID];
		if (!isset($obj_modelArr)) $obj_modelArr=array();
		if ( isset($obj_ref_id) ) {
			$obj_modelArr = array(); // full reset
			$obj_modelArr['id']  = $obj_ref_id;
			
		}
		if ( isset($creaOrder) )  $obj_modelArr['ord'] = $creaOrder;
		$_SESSION['s_sessVars'][$KEY_EXISTOBJ_ID] = $obj_modelArr;
   		$this->obj_modelArr = $obj_modelArr;
		
		if ( isset($editmode) )   $_SESSION['s_sessVars']['o.'.$tablex.'.editmode'] = $editmode; 
		$editmode        = $_SESSION['s_sessVars']['o.'.$tablex.'.editmode'];
		if ( $editmode == "" )  $editmode = "view";  
		
		gHtmlMisc::func_hist( "o.CONCRETE_SUBST.comp", 'subst: OverviewCompare',  "p.php?mod=DEF/o.CONCRETE_SUBST.comp" );      
		
		
		if ( $obj_modelArr['id'] ) {       
		    $sqls= "select NAME from ".$tablex." where ".$tablePkName ."=". $obj_modelArr['id'];
		    $sqlo->query($sqls);
		    if ( $sqlo->ReadRow() ) {               
		        $this->obj_model_arr["NAME"]  = $sqlo->RowData[0];
		        
		    }
		}
		  
		echo "[<a href=\"obj.concrete_proto.m_comgui.php?tablename=".$tablex."\">&lt;&lt; Back to comparison</a>]"; 
		echo " <font color=gray>Mode:</font> <B>$editmode</B> ";
		if ($editmode=="view") echo " [<a href=\"".$this->selfUrl."&editmode=edit&tablename=".$tablex."\">Edit mode</a>]";  
		else                   echo " [<a href=\"".$this->selfUrl."&editmode=view&tablename=".$tablex."\">View mode</a>]";
		echo "<br>";   
		
		if ($go) {
			echo "Create protocol from model<br>\n"; 

			if (!$obj_modelArr['id'] and !$obj_modelArr['ord']) {
				$this->htmlFoot('ERROR', 'model and position in model missing.');
			}
		}
		
		$sqlAfter     = $this->listLib->getSqlAfter();
		
		$sel = $this->_getObjects($sqlo, $sqlAfter);
		$num_elem = sizeof($sel) ;
	
		if ( !$num_elem ) {
			htmlFoot("Attention", "no objects selected to compare.");
		}            
		
		$this->first_obj = current($sel);
		
		
		if ( $editmode=="edit" ) {
			
			if ( !$this->obj_modelArr['id'] ) {      
		    	echo "<br>\n";
		    	htmlErrorBox("Missing", "Need a ". $OBJ_MODEL_CNAME ." for the protocol update! Please select one");
			}
		    $this->_formModel1 ( $this->obj_modelArr, $tablex );
		} 
		
		// START FORM 
		
		$this->_showTable( $sqlo, $sqlo2, $sel, $editmode);
		
		
		
	}
	
}