<?php
/**
 * show modification log for one object
 * @package obj.cct_acc_up.showobj.php
 * @swreq UREQ:0001760: o.CCT_ACC_UP > show modification log for one object
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param  $id - objid
  		   $t - tablename
		   $parx["raw"] = 0,1
   @global $_SESSION['s_formState']['cct_acc_up.show'] : 0,1 show advanced modification log ?
 */
session_start(); 

require_once 'subs/glob.obj.superhead.inc';
require_once ("visufuncs.inc");

require_once ("date_funcs.inc");
require_once ("o.CCT_ACC_UP.human.inc");
require_once ( 'glob.obj.access.head.inc');
require_once ( 'subs/glob.obj.propShow.inc');
require_once 'f.advmod_log.inc';


class fVisuFuncX2 extends visufuncs {

    function table_row($dataArr, $colarr=NULL) {
	
    	$thisbgcolor = $this->cellColor;
    	echo "<tr valign=top bgcolor=".$thisbgcolor.">"; 
    	$i=0;
    	foreach( $dataArr as $tmptxt) {
    		$tmptd = "";
    		if ($colarr[$i]!=NULL)  $tmptd = $colarr[$i];
    		echo "<td ".$tmptd.">".$tmptxt."</td>";
    		$i++;
    	}
    	echo "</tr>\n";
    	
    }

}

class oCCT_ACC_UP_tab {

    function __construct( &$sql_cached, $tablename, $objid, $cct_access_id, $parx) {
    
    	$this->tablename = $tablename;
    	$this->id        = $objid;
    	$this->cct_access_id = $cct_access_id;
    	$this->app_advmod = $_SESSION['globals']['app.advmod'];
    	
    	$this->acUpHumLib = new oCCT_ACC_UP_hum();
    
    	$this->showraw = $parx["raw"];
    
    	$this->actcolor = array(
    		"new"=>array("#DDFFDD", "new"), 
    		"mod"=>array("#FFFFDD","modified"),
    		"del"=>array("#FFDDDD","deleted"),
    		"arch"=>array("#FFAAAA","archived")
    		);
    	
    	
    	$this->advMod_gui = new advMod_guiC('norm');
    	$ok = $this->advMod_gui->set_obj($sql_cached, $cct_access_id, $tablename, $objid);
    	if (!$ok) {
    	    $this->app_advmod=0;
    	}
    }
    
    function showNavTab(){
    	$accHeadLib = new gObjAccessHead( 'mod', $this->tablename, $this->id );
    	$accHeadLib->showNavTab();
    }
    
    function tabStart() {
    
    	$this->tabobj = new fVisuFuncX2();
    	
    	$headOpt = array( "title" => "Object modification log");
    	$headx   = array ("Pos", "User", "Date", "Action", "Data Col/Tab","Misc");
    	if ($this->showraw) $headx[]='raw info';
    	if ($this->app_advmod) {
    	    $this->ADV_OLD = sizeof($headx);
    	    $this->ADV_NEW =  $this->ADV_OLD + 1;
    	    $headx[]='advanced mod OLD';
    	    $headx[]='advanced mod NEW';
    	}
    	$this->tabobj->table_head($headx,  $headOpt);
    }
    
    function tabRow( &$dataArr ) {
    
        $date_str=$dataArr[2];
    	$colarr = NULL;
    	
    	if ( $dataArr[3]!="" ) {
    
    		$infor  = $dataArr[3];
    		$resarr = $this->acUpHumLib->extract($infor);
    		$dataa  = &$resarr["a"];
    		 
    		$aact   = $dataa["key"];
    		$aniceA  = $this->actcolor[$aact];
    		if ($aniceA==NULL) $anice = $aact;
    		else {
    			$anice     = $aniceA[1];
    			$colarr[3] = "bgcolor=".$aniceA[0];
    		}
    		if ($dataa['func']!=NULL) {
    			$anice .= ' <font color=gray>func:</font>'.$dataa['nam'];
    		}
    		$dataArr[3] = $anice;
    	
    		// data col
    		$datax = &$resarr["d"];
    		$dx_str='';
    		if (is_array($datax['x'])) {
    		    
    		    $dx_arr=array();
        		foreach($resarr["d"]['x'] as $key=>$val) {
        		    switch ($key) {
            			case "fea":
            				$datao = '<span style="color:gray">features';
            				if (is_array($val)) {
            				    $datao .= ': '.implode(', ',$val);
            				}
            				$datao .= '</span>';
            				break;
            			case "xob":
            			    $datao = "<font color=gray>object-class</font>";
            			    break;
            			case "ass":
            				$datao = "<font color=gray>assoc:</font>"; 
            				$datao .= tablename_nice2( $val["t"] );
            				
            				if(!empty($val)) {
                				if ($val['pr']!=NULL) {
                				    $datao .= ' <font color=gray>pos:</font>'.$val['pr'];
                				}
                				if ($val['po']!=NULL) {
                				    $datao .= ' <font color=gray>pos:</font>'.$val['po'];
                				}
            				}
            				break;
            			case "atx":
            				$datao = "<font color=gray>attachment</font>";
            				if(!empty($val)) {
            				    if ($val['po']!=NULL) {
            				        $datao .= ' <font color=gray>pos:</font>'.$val['po'];
            				    }
            				}
            				break;
            			case "var":
            				$datao = "<font color=gray>vario-values</font>";
            				break;
            			default: 
            			    $datao = $key."???";
            		}
            		$dx_arr[] = $datao;
        		}
        		$dx_str = implode(', ',$dx_arr);
    		}
    		
    		$dataArr[4] = $dx_str;
    		$dataArr[5] = NULL;
    		
    		if ( !empty($resarr["x"]) ) {
    			$dataArr[5] =  glob_array2String( $resarr["x"],1, "," );
    		}
    		if ($this->showraw) {
    			$dataArr[6] = htmlspecialchars($infor);
    		} 
    		
    		if ($this->app_advmod) {
    		    $date_str_ISO = str_replace(' ','T', $date_str);
    		    $adv_answer = $this->advMod_gui->search_one_date($date_str_ISO);
    		    if ($adv_answer[0]) {
    		        $old_new_arr = $this->advMod_gui->entry_make_nice($adv_answer[1]);
    		        $dataArr[$this->ADV_OLD] = $old_new_arr['old'];
    		        $dataArr[$this->ADV_NEW] = $old_new_arr['new'];
    		    } else {
    		        $dataArr[$this->ADV_OLD] = '';
    		        $dataArr[$this->ADV_NEW] = '';
    		    }
    		}
    
    	}
    	$this->tabobj->table_row ($dataArr, $colarr);
    }
    
    function tabClose() {
    	$this->tabobj->table_close();
    }
    
    function accessIntro( &$sqlo ) {
    
    	$propLib = new gObjPropShowC();
    	$propLib->initObj( $sqlo, $this->tablename, $this->id );
    	$propLib->objAccTabGet( $sqlo );
    	echo '<br>';
    }

}

class advMod_ASSOC_table {
    
    static function nice_elements($sqlo, $assoctable, $assoc_elems) {
        
        $out = '[Associated list] > '.tablename_nice2($assoctable)."<br>";
        if (empty($assoc_elems)) {
            $out .= '<span style="color:gray;">empty</span>';
            return $out;
        }
        

        $pks = primary_keys_get2($assoctable);
        $pk2 = $pks[1];
        $pk2_features = colFeaturesGet2($assoctable, $pk2);
        
        
        foreach($assoc_elems as $key=>$valarr) {
            
            $out .= '&nbsp;&nbsp;'.$key.': <br>';

            foreach($valarr as $xcol=>$val) {
                $xcol_features = colFeaturesGet2($assoctable, $xcol);
                $out .= '&nbsp;&nbsp; - '.$xcol_features['NICE_NAME'].': ';
                if ($xcol_features['CCT_TABLE_NAME'] and $val) {
                    $fkt = $xcol_features['CCT_TABLE_NAME'];
                    if (gObject_exists ($sqlo, $fkt, $val) ) {
                        $val = obj_nice_name ( $sqlo, $fkt, $val ). ' ['.$val.']';
                    } else {
                        // nothing
                    }
                   
                }
                $out .= $val."<br>";
            }
            
        }
        return $out;
    }
}

/**
 * ADVANCED modification log VIEWER
 * @author skube
 *
 */
class advMod_guiC {
	function __construct($mode) {
		$this->mode=$mode;
		if ($this->mode==NULL) $this->mode='norm';
	}
	
	function _show_raw() {
		
		// $cct_access_id=$this->cct_access_id;
		
		$headOpt = array( "title" => "Advanced modification log; Differences - RAW");
		$headx   = array ("Date", "Differences");
		$tabobj  = new visufuncs();
		$tabobj->table_head($headx,  $headOpt);
		$rowopt = array('trOpt'=>'valign=top');
		do {
			// array('f'=>$found_flag, 'date'=>date-string, 'dict'=>$entryDict)
			$answer = $this->testLib->queryRow();
			if (!$answer['f']) break;
			
			$dataArr = array($answer['date'], "<pre>".print_r($answer['dict'],1)."</pre>" );
			$tabobj->table_row ($dataArr, $rowopt);

		} while(1);
		
		$tabobj->table_close();
	}
	
	/**
	 * produce HTML
	 * @param $raw
	 */
	function _cpstep_niceOne($raw) {
		
		$sqlo = &$this->sqlo;
		
		$komma=NULL;
		$htmlout=NULL;
		
		if (!is_array($raw)) {
			return 'bad data format';
		}
		
		
		
		reset ($raw);
		foreach( $raw as $step=>$valarr) {
			$htmlout .= $komma . 'step: ('.$step.') '.$this->abproto_stnames[$step].': ';
			
			reset ($valarr);
			foreach( $valarr as $key=>$val) {
				$htmlout .= " ".$key.'=<span class=xBlue>'.$val.'</span>';
			}
			$komma="<br />\n";
		} 
		
		return $htmlout;
	}
	
	/**
	 * make CONCRETE_PROTO_STEP nice
	 * @param $dictx
	 */
	function _cpstep_makeNice( &$dictx ) {
		
		// get step names
		$sqlo = &$this->sqlo;
		// get abstract_proto_id
		$table_tmp='CONCRETE_PROTO_STEP';
		
		$this->abproto_stnames = array();
		$apid = glob_elementDataGet( $this->sqlo, $this->tablename, 'CONCRETE_PROTO_ID', $this->objid, 'ABSTRACT_PROTO_ID');
		if ($apid) {
			$sqlsel = "STEP_NR, NAME from ABSTRACT_PROTO_STEP where ABSTRACT_PROTO_ID=".$apid." order by STEP_NR";
			$sqlo->Quesel($sqlsel);
			while ( $sqlo->ReadRow() ) {
			    $this->abproto_stnames[$sqlo->RowData[0]] = $sqlo->RowData[1];
			}
		}
		
		$nice_old = '['.tablename_nice2($table_tmp).']<br />' . $this->_cpstep_niceOne($dictx['old']['ass']['CONCRETE_PROTO_STEP']);
		$nice_new = '['.tablename_nice2($table_tmp).']<br />'. $this->_cpstep_niceOne($dictx['new']['ass']['CONCRETE_PROTO_STEP']);
		
		return array($nice_old, $nice_new);
	}
	
	
	
	function _ass_help1($assoctable, &$dictx, $oldnew) {
		return '['.tablename_nice2($assoctable).']<br />' ."<pre>".htmlentities(print_r($dictx[$oldnew]['ass'][$assoctable],1))."</pre>";
	}
	
	/**
	 * get NICE output for assoc elements
	 * @param array $dictx
	 * @return array
	 */
	function _ass_makeNice( &$dictx ) {
		
		$sqlo = &$this->sqlo;
		$keysx = array_keys($dictx['old']['ass']);
		
		$nice_old = NULL;
		$nice_new = NULL;
		
		foreach( $keysx as $assoctable) {
		    $nice_old .= advMod_ASSOC_table::nice_elements($sqlo, $assoctable, $dictx['old']['ass'][$assoctable]); // $this->_ass_help1($assoctable, $dictx, 'old');
		    $nice_new .= advMod_ASSOC_table::nice_elements($sqlo, $assoctable, $dictx['new']['ass'][$assoctable]);// $this->_ass_help1($assoctable, $dictx, 'new');
		}
		
		return array($nice_old, $nice_new);
	}
	
	private static function _makeNiceOne( &$dictOne ) {
		$nicestr = NULL;
		if (!is_array($dictOne)) {
			return NULL;
		}
		$komma=NULL;
		foreach( $dictOne as $key=>$val) {
			$nicestr .= $komma . $key.'=<span class=xBlue>'.htmlentities($val).'</span>';
			$komma='<br />';
		}
		return $nicestr;
	}
	
	// get NICE feature columns
	private function _fea_makeNiceOne( &$dictOne ) {
	    $new_dict = array();
	    foreach($dictOne as $key=>$val) {
	        $key_nice = columnname_nice2($this->tablename, $key) ;
	        if ($key_nice==NULL) $key_nice=$key;  // fall back ...
	        $new_dict[$key_nice]=$val;
	    }
	    
	    $nice_arr = self::_makeNiceOne( $new_dict );
	    return $nice_arr;
	}
	
	function _fea_makeNice( &$dictx ) {
		$nice_old = $this->_fea_makeNiceOne( $dictx['old']['vals'] );
		$nice_new = $this->_fea_makeNiceOne( $dictx['new']['vals'] );
		return array($nice_old, $nice_new);
	}
	function _xobj_makeNice( &$dictx ) {
	    $nice_old = self::_makeNiceOne( $dictx['old']['xobj'] );
	    $nice_new = self::_makeNiceOne( $dictx['new']['xobj'] );
	    return array($nice_old, $nice_new);
	}
	function _vario_makeNice( &$dictx ) {
	    $nice_old = self::_makeNiceOne( $dictx['old']['vario'] );
	    $nice_new = self::_makeNiceOne( $dictx['new']['vario'] );
	    return array($nice_old, $nice_new);
	}
	
	/**
	 * analyse 
	 * 	 'vals'
	 *   'ass'
	 *   '...'
	 * @param $answer
	 *   'dict'
	 * @param $oldval
	 * @param $newval
	 */
	function _transformDict( &$answer, &$oldval, &$newval) {
		$dictx = &$answer['dict'];
		$pre_LF='';
		
		if ( is_array($dictx['old']['vals']) or is_array($dictx['new']['vals'])) {
			list ($oldvalx, $newvalx) = $this->_fea_makeNice( $dictx );
			$oldval .= '<span class="yGgray">[features]</span><br />'.$oldvalx;
			$newval .= '<span class="yGgray">[features]</span><br />'.$newvalx;
		}
	
		if ( is_array($dictx['old']['xobj']) or is_array($dictx['new']['xobj'])) {
		    list ($oldvalx, $newvalx) = $this->_xobj_makeNice( $dictx );
		    if ($newval) $pre_LF='<br>';
		    $oldval .= $pre_LF.'<span class="yGgray">[class-params]</span><br />'.$oldvalx;
		    $newval .= $pre_LF.'<span class="yGgray">[class-params]</span><br />'.$newvalx;
		}
		
		if ( is_array($dictx['old']['vario']) or is_array($dictx['new']['vario'])) {
		    list ($oldvalx, $newvalx) = $this->_vario_makeNice( $dictx );
		    if ($newval) $pre_LF='<br>';
		    $oldval .= $pre_LF.'<span class="yGgray">[vario]</span><br />'.$oldvalx;
		    $newval .= $pre_LF.'<span class="yGgray">[vario]</span><br />'.$newvalx;
		}
		
		// ASSOC elements
		if ( is_array($dictx['old']['ass']) or is_array($dictx['new']['ass'])) {
			if ( is_array( $dictx['old']['ass']['CONCRETE_PROTO_STEP'] ) ) {
				list ($oldvalx, $newvalx) = $this->_cpstep_makeNice( $dictx );
				
			} else {
				list ($oldvalx, $newvalx) = $this->_ass_makeNice( $dictx );
			}
			if ($newval) $pre_LF='<br>';
			$oldval .= $pre_LF.$oldvalx;
			$newval .= $pre_LF.$newvalx;
		}
	}
	
	function _show_norm() {
		
		$sqlo= $this->sqlo;
		$cct_access_id=$this->cct_access_id;
		
		$headOpt = array( "title" => "Advanced modification log; Differences");
		$headx   = array ("Date / User", "Old Values", "New Values");
		$tabobj  = new visufuncs();
		$tabobj->table_head($headx,  $headOpt);
		$rowopt = array('trOpt'=>'valign=top');
		do {
			// array('f'=>$found_flag, 'date'=>date-string, 'dict'=>$entryDict)
			$answer = $this->testLib->queryRow();
			if (!$answer['f']) break;
			
			$oldval=NULL;
			$newval=NULL;
			$this->_transformDict($answer, $oldval, $newval);
			
			
			// search entry in CCT_ACC_UP
			$timestamp = date_str2unix( $answer['date'], 6 );
			$user_id   = 0;
			$user_nick = NULL;
			$sqls    = "POS, DB_USER_ID, UPINFO ".
					 " from CCT_ACC_UP where cct_access_id=".$cct_access_id. " and MODI_DATE=".$sqlo->Timestamp2Sql($timestamp);
			$sqlo->Quesel($sqls);
			if ( $sqlo->ReadRow() ) {
				$user_id = $sqlo->RowData[1];
				if ($user_id) {
					$userarr = glob_elemDataGet3( $sqlo, "DB_USER", array("DB_USER_ID"=>$user_id), 
						array('NICK') );
					$user_nick = $userarr['NICK'];
				}
			}
			
			$dataArr = array(
				$answer['date']."<br />".$user_nick, 
				$oldval, 
				$newval 
				);
			$tabobj->table_row ($dataArr, $rowopt);

		} while(1);
		
		$tabobj->table_close();
	}
	
	/**
	 * 
	 * @param object $sqlo - cahced SQL object !
	 * @param int $cct_access_id
	 * @param string $tablename
	 * @param int $id
	 * @return $ok:
	 */
	function set_obj(&$sqlo_cached, $cct_access_id, $tablename, $id) {
	    
	    $this->sqlo = &$sqlo_cached;
	    $this->cct_access_id= $cct_access_id;
	    $this->tablename = $tablename;
	    $this->objid = $id;
	    
	    $this->testLib = new f_advmod_log();
	    $ok = $this->testLib->queryStart($this->cct_access_id);
	    return $ok;
	}
	
	function entry_make_nice(&$entry_dict) {
	    $oldval=NULL;
	    $newval=NULL;
	    $entry_full=array('dict'=>$entry_dict);
	    $this->_transformDict($entry_full, $oldval, $newval);
	    return array('old'=>$oldval, 'new'=>$newval);
	}
	
	function search_one_date($date_str) {
	    return $this->testLib->searchLine($this->cct_access_id, $date_str);
	}
	
	function show(  ) {

		if ($this->mode=='norm') {
			$this->_show_norm();
		}
		if ($this->mode=='raw') {
			$this->_show_raw();
		}

		// $infoarr = $testLib->searchLine($cct_access_id, $date_str);
	}
}

class OMLog_table {
    
    function __construct($sqlo, $tablename, $objid) {
        $prim_name = PrimNameGet2($tablename);
        
        $this->cct_access_id = glob_elementDataGet( $sqlo, $tablename, $prim_name, $objid, 'CCT_ACCESS_ID'); 
        $this->tablename=$tablename;
        $this->objid=$objid;
    }
    
    function show($sql, $sql2, $parx) {
        
        $cct_access_id = $this->cct_access_id;
        $tablename =$this->tablename;
        $objid = $this->objid;
        
        $tabShowLib = new oCCT_ACC_UP_tab( $sql2, $tablename, $objid, $cct_access_id,  $parx);
        $tabShowLib->accessIntro( $sql );
        
        
        $sqls = "select POS, ".$sql->Sql2DateString ( "MODI_DATE", 1).", DB_USER_ID, UPINFO ".
            " from CCT_ACC_UP where cct_access_id=".$cct_access_id. " order by POS desc";
        $sql->query($sqls);
        $SHOWMAX = 50;
        $cnt=0;
        while ( $sql->ReadRow() ) {
            
            if ($cnt> $SHOWMAX ) {
                $dataArr=array("...","...", "...more...");
                $tabShowLib->tabRow ($dataArr);
                break;
            }
            
            $pos   = $sql->RowData[0];
            $datex = $sql->RowData[1];
            $userx = $sql->RowData[2];
            $upinf = $sql->RowData[3];
            
            $sqls = "select nick from db_user where db_user_id=".$userx;
            $sql2->query($sqls);
            $sql2->ReadRow();
            $nickx = $sql2->RowData[0];
            $dataArr = array($pos, $nickx, $datex, $upinf, '' );
            
            if ( !$cnt ) {
                $tabShowLib->tabStart();
            }
            
            $tabShowLib->tabRow($dataArr);
            $cnt++;
        }
        
        if ( $cnt ) {
            $tabShowLib->tabClose();
            
        } else {
            echo "<font color=gray><b>No entry</b> in the modification log for this object.</font><br>";
        }
        
        echo "<br>";
    }
}

// --------------------------------------------------- 
global $error, $varcol;

$error = & ErrorHandler::get();
$sql    = logon2( $_SERVER['PHP_SELF'] );
$sql2  = logon2(  );
if ($error->printLast()) htmlFoot();
// $varcol = & Varcols::get();

$tablename=$_REQUEST['t'];
$parx = $_REQUEST['parx'];
$objid = $_REQUEST['id'];

$gui_lib = new glob_obj_superhead($tablename, $objid);
$gui_lib->page_open($sql, '0modlog');

echo '<style type="text/css">'."\n";
echo '.xBlue  { color: #0000FF }'."\n";
echo '</style>'."\n";

echo "<ul>\n";

$isbo = cct_access_has2($tablename);
if (!$isbo) {
    htmlFoot('ERROR', 'Only business objects can use this tool.');
}

$prim_name = PrimNameGet2($tablename);
$cct_access_id = glob_elementDataGet( $sql, $tablename, $prim_name, $objid, 'CCT_ACCESS_ID');
if (!$cct_access_id) {
    htmlFoot('ERROR', 'Objects has no CCT_ACCESS_ID!');
}

$main_table_lib = new OMLog_table($sql, $tablename, $objid);
$main_table_lib->show($sql, $sql2, $parx);

echo "[<a href=\"".$_SERVER['PHP_SELF']."?t=$tablename&id=$objid&parx[raw]=1\">Show RAW Info-Code</a>] ";

$showAdvmodDict = $_SESSION['s_formState']['cct_acc_up.advmod'];
$backurl         = urlencode($_SERVER['PHP_SELF']."?t=".$tablename."&id=".$objid);
if (!$showAdvmodDict['show']) {
	echo '[<a href="f.s_formState.set.php?key=cct_acc_up.advmod&subkey=show&val=1&backurl='.$backurl.'">Expert view of Advanced Modification Log</a>] &nbsp;';
} else {
	echo '[<a href="f.s_formState.set.php?key=cct_acc_up.advmod&subkey=show&val=0&backurl='.$backurl.'">Hide Advanced Modification Log</a>]';
	$normLink = '&nbsp;&nbsp;[<a href="f.s_formState.set.php?key=cct_acc_up.advmod&subkey=mode&val=norm&backurl='.$backurl.'">Normal Mode</a>]';
	$rawlink  = '&nbsp;&nbsp;[<a href="f.s_formState.set.php?key=cct_acc_up.advmod&subkey=mode&val=raw&backurl='.$backurl.'">Raw Mode</a>]';

	if ($showAdvmodDict['mode']=='raw') $rawlink = '<b>'.$rawlink.'</b>';
	else $normLink = '<b>'.$normLink.'</b>';
	
	echo $normLink . $rawlink;
}
echo "<br>";

if ($showAdvmodDict['show']) {	
	$advMod_gui = new advMod_guiC($showAdvmodDict['mode']);
	$ok = $advMod_gui->set_obj($sql, $cct_access_id, $tablename, $objid);
	if ($ok) $advMod_gui->show();
}

echo "</ul><br>";

$gui_lib->page_close($sql);
