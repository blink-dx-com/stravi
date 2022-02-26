<?php
require_once 'o.S_VARIO.subs.inc';
require_once 'o.S_VARIO.mod.inc';
require_once ('f.update.inc');
require_once 'glob.obj.touch.inc';
require_once ("glob.obj.update.inc");

/**
 * sub funcs for glob.objtab.edit.inc
 * @package glob.objtab.edit.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class fBulkUpdateSub {
    
    private $_arguobj; // array of EXTRA_OBJ: NAME=>VAL

function __construct( $tablename, $infolevel, $sqlAfter, $sqlAftNoOrd,&$flushLib,$numelements) {
	$this->tablename = $tablename;
	$this->flushLib  = &$flushLib;
	$this->primary_keys = primary_keys_get2( $tablename );
	$this->infolevel   = $infolevel;
	$this->expectElems = $numelements;
	$this->sqlAfter    = $sqlAfter;
	$this->sqlAftNoOrd = $sqlAftNoOrd;
	$this->debug	   = $_SESSION["userGlob"]["g.debugLevel"];
}

function progessBarShow($expectElems) {
    $prgopt=array();
	$prgopt['objname']= 'rows';
	$prgopt['maxnum'] = $expectElems;
	$this->flushLib->shoPgroBar($prgopt);
}

/**
 * log the action in applog, 
 * log: argument-column names, 
 * expected NUMBER of elements, SQL-condition
 * @param $argu
 * @param $arguobj
 */
function appLogAction($argu, $arguobj) {
	global $error;
	
	$info=NULL;
	if (is_array($argu)) {
		$arguKeys = array_keys($argu);
		$info .= 'changeCols:('.implode(',',$arguKeys).')';
	}
	if (is_array($arguobj)) {
		$arguobjKeys  = array_keys($arguobj);
		$info .= '; changeClsAtr:('.implode(',',$arguobjKeys).')';
	}
	$info .= '; expectElemNum:'.$this->expectElems.'; SQL-cond:'.$this->sqlAfter;
	 
	$error->logxMeta( 'INFO', 3, 'bulk.BulkUpdate', $_SERVER['PHP_SELF'], $info );
	
}

function alivePoint($cnt, $force=0) {
	$this->flushLib->alivePoint($cnt, $force);
}

function infoout($text) {
    echo "<font color=gray>Info:</font> ".$text."<br>\n";
}


function initParams( $extra_obj_col_selected ) {
	$this->extra_obj_col_selected = $extra_obj_col_selected;
}

function getClassFromSel( &$sql, $sqlsafter) {
// FUNCTION: get class-id form first element in selection
    global $varcol;
    
    $classid = 0;
    $sqls = "select x.EXTRA_OBJ_ID from ".$sqlsafter;
    $sql->query("$sqls");
    $sql->ReadRow();
    $extra_obj_id=$sql->RowData[0];
    
    if ($extra_obj_id) {
         $classid = $varcol->obj_id_to_class_id($extra_obj_id);
    }
    return ($classid);
}


function classSelParam( &$sql, $classid ) {
    global $varcol, $_s_i_varcol;
    
    $classname = $varcol->class_id_to_nice_name($classid);
    echo "<input type=hidden name=\"xargu[CLASS]\" value=\"".$classid."\">\n";
    
    echo "<font color=gray>Class:</font> <B>$classname</B><br><br>\n";
    echo "<table cellpadding=1 cellspacing=1 border=0 bgcolor=#B0B0B0>";
    echo "<tr bgcolor=#D0D0D0><td><font color=gray>select</td><td><font color=gray>column</td><td><font color=gray>notes</td></tr>\n";
    
    $attribs = $varcol->get_attrib_nice_names($classid);
    
    $color  = '';
    $color1 = "#EFEFEF";  // SUN violett 
    $color2 = "#EFEFFF";
    foreach( $attribs as $attrid=>$vartext) {
        $tmpnotes = $_s_i_varcol[$classid][$attrid]['comment'];
        if ($color == $color1)  $color = $color2;
        else $color = $color1;  
        echo "<tr bgcolor=\"".$color."\"><td align=center><input type=checkbox name=\"xobjatr[$attrid]\" value=\"1\" checked></td><td><B>" .$vartext. "</B></td><td><font color=gray>".$tmpnotes."</font></td></tr>\n";
    }
    echo "</table>\n";      
}

/**
 * FORM fileds:
 * - vobj
 * - vobjval
 * @param object $sqlo
 * @param int $classid
 */
function vario_params( &$sqlo ) {
    
    $variolib = new oS_VARIO_sub($this->tablename);
    $colarray =  $variolib->getAllKeysNice( $sqlo );
    
    if (!sizeof($colarray)) {
        echo ' No VARIO-columns for this table defined.<br>';
        return;
    }

    

    echo "<input type=hidden name=\"selcol[vario]\" value=\"1\">\n";
    
   
    echo "<table cellpadding=1 cellspacing=1 border=0 bgcolor=#B0B0B0>";
    echo "<tr bgcolor=#D0D0D0><td><font color=gray>select</td><td><font color=gray>column</td><td><font color=gray>value</td></tr>\n";
    
    
    foreach( $colarray as $key=>$nice ) {
        
        $features = $variolib->getColFetaures( $sqlo, $key );
        
        if ( !glob_isAdmin() ) {
            if (!$features['EDITABLE']) continue;
        }

        echo "<tr ><td align=center><input type=checkbox name=\"vobj[$key]\" value=\"1\"></td><td><B>" .
            $nice. "</B></td><td><input type=text name=\"vobjval[$key]\" ></td></tr>\n";
    }
    
    
    echo "</table>\n";
}



function assoc_update( 
			&$sql,  
			&$sql2, 
			$argu 
			) {
	/* - update assoc elements
	   - change the SORT-condition of sqlAfter: sort by FIRST_PK, to prevent multiple TOUCH_ROW actions
	   - return "backInfo" if all objects from ONE mother
	*/		
	global  $error;
	$FUNCNAME="assoc_update";
	
	$tablename   = $this->tablename;
	$sqlAftNoOrd = $this->sqlAftNoOrd;
	$primary_keys= $this->primary_keys;
			
	$edit_allow  = 0;	
	$mothertable = mothertable_get2($tablename);
			
	if ( $mothertable ) $edit_allow=1;
	if ( $_SESSION['sec']['appuser'] == 'root' ) $edit_allow=1;

	if ( !$edit_allow ) {
		$error->set( $FUNCNAME, 1, "Edit not allowed: has no mother-table." );
		return;
	}
	//$mother_nice    = tablename_nice2($mothertable);
	$primary_key1   = $primary_keys[0];
	$primary_key2   = $primary_keys[1];
	if (!empty($primary_keys[2])) {
		$more_keys = ", x.".$primary_keys[2];
		$primary_key3 = $primary_keys[2];
	}
	
	// sort by FIRST_PK, to prevent multiple TOUCH_ROW actions for one mother-object
	$sqlAfterUse = $sqlAftNoOrd. " order by x.".$primary_key1;
	
	$this->infoout("During the update process: the feature list will be sorted by the OBJECT-ID (".$primary_key1.").");
	
	
	$sqls = "select x.".$primary_key1. ", x.$primary_key2 ".$more_keys." from ".$sqlAfterUse;

	if ( $_SESSION['userGlob']["g.debugLevel"] > 0 ) {
		echo "DEBUG: SQL-command: $sqls	<br>";
	}
	$retVal = $sql->query($sqls, $FUNCNAME);
	if ($retVal<=0) {
		$error->set( $FUNCNAME, 2, "SQL-query failed." );
		return;
	} 
	$cnt 	 = 0;
	$this->deny_cnt= 0;
	$last_pk = 0; // last PK
	$mothercnt=0;
	
	while ( $sql->ReadRow() )  {
	
		$prim_id  =  $sql->RowData[0];
		$prim_id2  = $sql->RowData[1];
		
		if ($more_keys) $prim_id3  = $sql->RowData[2];
		if ($last_pk != $prim_id) { 	// is a new primary mother object ???
			$o_rights = access_check( $sql2, $mothertable, $prim_id );
			$mothercnt++;
		}
		
		$pks=array();
		$pks[$primary_key1] = $prim_id;
		$pks[$primary_key2] = $prim_id2;
		if ($more_keys) $pks[$primary_key3] = $prim_id3;
			
		if ( $o_rights["insert"] ) {
		
		    if ($last_pk != $prim_id) {
		        $actarr = array('d'=>array(  'x'=>array( 'ass'=>array('t'=>$tablename) )  ), 'a'=>array('key'=>'mod') );
		        globObjTouch::touch($sql2, $mothertable, $prim_id, $actarr);
		    }
			
			$retval = gObjUpdate::update_row_s( $sql2, $tablename, $argu, $pks );
			if ( $retval<1 ) {
				$error->set( $FUNCNAME, 3, "Object [ID:$prim_id]: update failed. Process stopped." );
				return;
			} 
			
		} else {
			info_out('WARNING', 'Element  [mother-ID:'.$prim_id.']: EDIT not permitted!');
			$this->deny_cnt++;
		}
		$last_pk = $prim_id;
		$cnt++;
	}	
	
	if ( $mothercnt<2 AND $prim_id) {
		// all elememts from one mother
		$backInfo = array( "table"=>$mothertable, "id"=> $prim_id);
	}
	
	
	$retarr = array("cnt"=>$cnt, "deny_cnt"=>$this->deny_cnt, "backInfo"=> $backInfo, "mothercnt"=>$mothercnt );
	return ( $retarr );
}

function _single_prep( &$argu, $extra_class_id ) {
	global $error, $varcol;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	$extra_cols  = ", 1"; 		// dummy val ???
	$this->arguobj_ids = NULL;
	
	if ( $this->extra_obj_col_selected ) {
	
		unset ($argu["EXTRA_OBJ_ID"]);	/* delete from arguments, must be handeled more intelligent ... */
		$extra_cols=", x.EXTRA_OBJ_ID";  /* select also extra_obj_id */
			
		/* transform attrib[NAME] => attrib[ID] */
		$arguobj = &$this->_arguobj;
			
		$this->arguobj_ids=array();
		if (!empty($arguobj)) {
		    foreach( $arguobj as $th0=>$th1) { 
		
				$newid = $varcol->attrib_name_to_id($th0, $extra_class_id);
				if ($error->Got(READONLY))  {
     				$error->set( $FUNCNAME, 1, "Error occurred." );
					return;
				}
				$this->arguobj_ids[$newid] = $th1;
			}
		}
	}  
	return $extra_cols;
}

// ONE object ...
private function _singlePK_ONE( object &$sqlo, $obj_id, $extra_obj_id, &$argu, $extra_class_id ) {
    global $error;
    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
    
    $loop_change_allow = 0;
    $vario_vals = &$this->_vario_vals;
    
    $MAXDENYSHOW  = 20;
    $tablename    = $this->tablename;
    $infolevel    = $this->infolevel;
    $primary_key  = $this->primary_keys[0];
    $extra_obj_col_selected = $this->extra_obj_col_selected;
    
    $argu[$primary_key] = $obj_id;
    
    if ( $obj_id > 0 ) {
        $o_rights = access_check($sqlo, $tablename, $obj_id);
        if ( $o_rights["write"] )
            $loop_change_allow=1;
    }
    
    if ( $loop_change_allow ) {
        
        if ($infolevel > 0) {
            echo "<font color=gray>updated object content ";
            print_r($argu);
            echo "<br>\n";
        }
        
        $upok_feat = 0;
        $args=array();
        
        if ( !empty($argu) ) {
            
            $validLib = new fValidate();
            $validLib->check_columns($sqlo, $tablename, $argu, true); // TBD: must be called inside loop ? may only once at beginnging ???
            if ( $error->Got(READONLY) ) {
                $error->set($FUNCNAME, 1, "validate_columns failed.");
                return;
            }
            
            $upok_feat = 1;
            $args['vals'] = $argu;
        }
        
        if ( $extra_obj_col_selected ) {
            
            if ($_SESSION['userGlob']["g.debugLevel"]>2 ) {
                echo "<B>DEBUG:</B> [ID:$obj_id] has old EXTRA_OBJ_ID:$extra_obj_id <br>\n";
            }
            $args["xobj"] = array(
                'extra_class_id' => $extra_class_id,
                'values'         => $this->_arguobj
            );
        }
        
        if (!empty($args)) {
            
            $UpdateLib = new globObjUpdate();
            $UpdateLib->update_meta( $sqlo, $tablename, $obj_id, $args );
            
            //$retval = gObjUpdate::update_row($sqlo, $tablename, $argu);
            if ($error->Got(READONLY))  {
                $error->set($FUNCNAME, 2, "Object [ID:$obj_id]: VALS update failed.");
                return;
            }
        }
        
        if ( is_array($vario_vals) ) {
            
            if ($_SESSION['userGlob']["g.debugLevel"]>2 ) {
                echo "<B>DEBUG:</B> [ID:$obj_id] has old EXTRA_OBJ_ID:$extra_obj_id <br>\n";
            }
            if ( !$upok_feat ) { // object not touched before ?
                $actarr = array('d'=>array(  'x'=>array( 'var'=>NULL )  ), 'a'=>array('key'=>'mod') );
                globObjTouch::touch($sqlo, $tablename, $obj_id, $actarr);
            }
            
            $vario_lib = new oS_VARIO_mod();
            $var_opt=array('noAccCheck'=>1);
            $vario_lib->setObject( $sqlo, $tablename, $obj_id, $var_opt);
            
            foreach($vario_vals as $vario_key=>$vario_val) {
                $vario_lib->updateKeyVal($sqlo, $vario_key, $vario_val);
                if ( $error->Got(READONLY) ) {
                    $error->set($FUNCNAME, 3, "Object [ID:".$obj_id."]: update of VARIO (".$vario_key.") failed.");
                    return;
                }
            }
            
        }
        
    } else {
        $this->deny_cnt++;
        if ( ($this->deny_cnt<$MAXDENYSHOW) or $this->debug>0) {
            echo "<font color=red>Denied:</font> No update right for Object [ID:".$obj_id."]<br>\n";
        }
        if ($this->deny_cnt==$MAXDENYSHOW) {
            echo "<font color=red>Denied:</font> ... more denied objects are hidden (change g.debugLevel)...<br>\n";
        }
    }
}

/**
 *  single PK update 
 * @param object $sql
 * @param object $sql2
 * @param array $argu
 * @param array $arguobj
 * @param int $extra_class_id
 * @param array $vario_vals : can be NULL
 * @return void|string[]|number[]
 */
function singlePK_update( &$sql, &$sql2, &$argu, &$arguobj, $extra_class_id, &$vario_vals ) {
	
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	$this->progessBarShow($this->expectElems);
	$primary_key  = $this->primary_keys[0];
	
	$this->_vario_vals = &$vario_vals;
	$this->_arguobj = &$arguobj;
	
	
	$sqlAfter	  = $this->sqlAfter;
	
	$extra_cols = $this->_single_prep( $argu, $extra_class_id );
	if ($error->Got(READONLY))  {
    	$error->set( $FUNCNAME, 1, "Error occurred." );
		return;
	}
	
	$sql2->query('SELECT x.'.$primary_key . $extra_cols.' FROM '.$sqlAfter);
	if ($error->Got(READONLY))  {
	    $error->set( $FUNCNAME, 2, "Query failed." );
	    return;
	}
	
	$cnt      = 0;
	$this->deny_cnt = 0;
		
	while ($sql2->ReadRow()) {
 
	  $loop_extra_obj_id = 0;
	  $this->alivePoint($cnt);
      $obj_id            = $sql2->RowData[0];
      if($this->extra_obj_col_selected) $loop_extra_obj_id = $sql2->RowData[1];  	
	  
	  $this->_singlePK_ONE($sql, $obj_id, $loop_extra_obj_id, $argu, $extra_class_id);

	  $cnt++;
	  
	}
	
	$this->alivePoint($cnt,1);
	
	$retarr = array("cnt"=>$cnt, "deny_cnt"=>$this->deny_cnt );
	return ($retarr);
}

}
