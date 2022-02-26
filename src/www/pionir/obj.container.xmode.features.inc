<?php
/**
 * container.xedit extension
 * @package obj.container.xedit.php
 * @swreq UREQ:0001118: o.CONTAINER : single object extension 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param array $_REQUEST['vopt']
 *   main view options
 */

require_once ("visufuncs.inc");
require_once 'f.rider.inc';
require_once ("f.objview.inc");	
require_once ("func_form.inc");
require_once 'o.H_UNIT.siSubs.inc';
require_once 'o.CONTAINER.subs.inc';
require_once 'o.CONCRETE_SUBST.stockwarnC.inc';
require_once 'gui/o.CONTAINER.one_show.inc';

class oContXedit {

    var $withTimeStamp; // 0,1
    var $cntSubst;
    var $cnt_CHA_SUA;
    private $cont_features;  // array

function __construct(&$sqlo, $id, $viewmode) {
    $this->id = $id;
    $this->viewmode = $viewmode;
    
    $this->cont_features = array();
    $this->cont_location = '';
    
    $this->contLib = new oCONTAINER_SubsC();
    $this->contLib->setContainer($sqlo, $this->id);
    
    $query_arr = array('vals'=>array('CHIP_READER_ID') );
    $tmp_vals = $this->contLib->getFeature( $sqlo, $query_arr );
    $this->cont_features['CHIP_READER_ID']=$tmp_vals['CHIP_READER_ID'];
    
    $this->cont_features['ABS_CONTAINER']=array();
    $this->cont_features['ABS_CONTAINER']['posnum'] = $this->contLib->getAbsPosNum($sqlo);
    $this->cont_features['ABS_CONTAINER_ID'] = $this->contLib->get_ABS_CONTAINER_ID();
    
    $this->sqls1 = "select CONCRETE_SUBST_ID from  CONT_HAS_CSUBST where CONTAINER_ID=".$this->id ; 
    $this->backurlEnc = urlencode('edit.tmpl.php?t=CONTAINER&id='.$this->id );
    
 
	$this->cntSubst    = $this->contLib->count_SUC($sqlo);
	$this->cnt_CHA_SUA = $this->contLib->count_CHA_SUA($sqlo);
	
	$varioLib = new oS_VARIO_sub('CONTAINER');
	$this->withTimeStamp = intval($varioLib->getValByKey( $sqlo, $id, 'showCheckinCol' ) );
	
	$CHIP_READER_ID = $this->cont_features['CHIP_READER_ID'];
	if ($CHIP_READER_ID) {
	   $this->cont_location = glob_elementDataGet( $sqlo, 'CHIP_READER', 'CHIP_READER_ID', $CHIP_READER_ID, 'LOCATION');
	}
}

function deviceInfo(&$sqlo) {
    $location = $this->cont_location;
    $dev_short = globTabMetaByKey('CHIP_READER', 'SHORT');
    echo '<span style="color:gray;">'.$dev_short.'-ID: <b>'. $this->cont_features['CHIP_READER_ID'] .'</b></span> ';
	echo '<span style="color:gray;">location:</span> <b>'.$location.'</b> ';
	echo ' &nbsp;|&nbsp;&nbsp;&nbsp;';
}

function children_info(&$sqlo) {
    
    $children = $this->contLib->get_cont_children($sqlo);
    if (empty($children)) return;
    
    $tab_nice = tablename_nice2('CONTAINER');
    $objlnkLib    = new fObjViewC();
    echo '<span style="color:gray;">Has sub-'.$tab_nice.'s:</span><ul>'."\n";
    foreach($children as $cont_id) {
        
        $contLib_tmp = new  oCONTAINER_SubsC();
        $contLib_tmp->setContainer($sqlo, $cont_id);
        $cnt = $contLib_tmp->count_SUC($sqlo);
        echo $objlnkLib->bo_display( $sqlo, 'CONTAINER', $cont_id ).' ('.$cnt.' MACs)' . "<br>\n";
    }
    echo '</ul>'."\n";
    echo "<br>\n";
    
}

private function _setvars_url($val, $comment) {
    return '<a href="sessVars_set.php?variable=o.CONTAINER.sumShow&val='.$val.'&backurl='.
        $this->backurlEnc.'"><img src="images/icon.ABSTRACT_SUBST.gif" border=0> '.$comment.'</a>';
}

function headline() {
	
	echo "<a href=\"view.tmpl.php?t=CONCRETE_SUBST&tableSCond=".
    	rawurlencode ( "CONCRETE_SUBST_ID in (".$this->sqls1 .")" ).
        "&condclean=1\" > ".
        '<img src="images/icon.CONCRETE_SUBST.gif" border=0 TITLE="DETAILED list"> 
    	'.tablename_nice2('CONCRETE_SUBST').' list</a>  &nbsp;|&nbsp;&nbsp; ';
    
    echo '  <a href="p.php?mod=LAB/o.CONTAINER.stockwarn&id='.$this->id.'"><img src="images/icon.CONTAINER.gif" border=0 TITLE="Storage quantity warnings">
    		Stock quantity warnings</a>';

    echo "<br>\n";
}

/**
 * get stamp features
 */
function _getCheckinFeats(&$sqlo, $substid) {

	if (!$substid) return array();
	$sqlsel = 'POS, DATEX, KEY from CSUB_HAS_STAMP where CONCRETE_SUBST_ID=' .$substid. ' order by DATEX DESC';
	$sqlo->Quesel($sqlsel);
	$sqlo->ReadRow();
	$feat=array();
	$feat['POS']   = $sqlo->RowData[0];
	$feat['DATEX'] = $sqlo->RowData[1];
	$feat['KEY']   = $sqlo->RowData[2];
	return $feat;
}
/**
 * get checkin link
 * @param  int $substid can be NULL
 * @return string checkin-link
 */
function _getCheckinLinks($feat, $contPos, $substid) {

	if (!$substid) return NULL;
	$basex = 'p.php?mod=DEF/o.CONTAINER.stamp02&id='.$substid.'&parx[cont_id]='. $this->id . 
		     '&action=STAMP&parx[key]=';
	$lnk[0]=$basex.'1'; // do checkin
	$lnk[1]=$basex.'2'; // do checkout
	if ($feat['KEY']==2) {
		$lnkTxt = '<a href="'.$lnk[0].'">checkin</a>|checkout';
	} else $lnkTxt = 'checkin|<a href="'.$lnk[1].'">checkout</a>';
	return $lnkTxt;
	
}

function _getCheckStats($checkInFeats, $substid){
	if ($checkInFeats['POS']>0) {
		$lnkTxt = '<a href="p.php?mod=DEF/o.CONTAINER.stamp02&id=' .$substid. '">stats</a>';
	} else $lnkTxt = NULL;
	return $lnkTxt;
}

/**
 * show normal substance fill list
 * @swreq UREQ:0001118:001 view current substances in container
 *          SUBREQ:004: zeige audit-Status
 * @swreq UREQ:0002617: o.CONTAINER > View: show Notes
 * @param  $sqlo
 * @param  $sqlo2
 */
function container_list_norm(&$sqlo, &$sqlo2) {
    
    
    
	$MAX_SHOW_NUM      = 40;   // default max show
	$MAX_SHOW_SUPER_NUM= 1000; // default SUPER max show
	$cont_id = $this->id;
	
	$cont_has_posdef = $this->cont_features['ABS_CONTAINER']['posnum'] ;
	
	$max_show_now = $MAX_SHOW_NUM;
	$tmpMax = $_SESSION['s_sessVars']['o.CONTAINER.fillShow'];
	if ($tmpMax>$max_show_now) $max_show_now = $tmpMax;
	if ( $max_show_now>$MAX_SHOW_SUPER_NUM ) $max_show_now = $MAX_SHOW_SUPER_NUM;
	
	if ($max_show_now>$MAX_SHOW_NUM) {
	    $backurlEnc=''; //TBD:
		echo '[<a href="sessVars_set.php?variable=o.CONTAINER.fillShow&val='.$MAX_SHOW_NUM.'&backurl='.
            			$backurlEnc.'">show per page: '.$MAX_SHOW_NUM.'</a>]';
        echo "<br />\n";
	}
	
	$checkin   = 0;
	$editAllow = $this->viewmode == 'edit' ? 1 : 0;
	if ($editAllow and $this->withTimeStamp ) $checkin = 1;
	
	$tabobj = new visufuncs();
	$headOpt = array( "title" => "Material details in container; all positions");
	if ($editAllow) {
		
		$headx  = array ("#", "#");
		if ($checkin) $headx[]="checkin";
		
		echo '<form style="display:inline;" method="post" '.
		 	' name="form2"  action="p.php?mod=DEF/o.CONTAINER.modone" >'."\n";
		echo '<input type=hidden name="id" value="'.$cont_id.'"  value="1">'."\n";
		
		
	} else $headx  = array ();
	
	$headx[]='pos-ID *';
	if ($cont_has_posdef) {
	    $headx[]='pos-ALIAS';
	}
	
	// "Aliquot", "mod date",
	$headx  = array_merge($headx, array ('MAC-ID', "material", "material template", "Storage-<br>Status", "quantity", "unit"));
	

	
	if ($this->withTimeStamp) $headx[]='checkinStats';
	$tabobj->table_head($headx,   $headOpt);
	
	
	
	$objlnkLib    = new fObjViewC();
	$auditLogSub  = new oAccLogC();
	$stockWarnLib = new o_CONCRETE_SUBST_stockwarnC();
	
	$auditColors = oH_ALOG_ACT_subs::$auditColor;
	
	$sqlsel = "c.* from CONT_HAS_CSUBST c where c.CONTAINER_ID=" . $cont_id . ' and c.CONCRETE_SUBST_ID>0  order by c.POS DESC'; 
	
	if ($cont_has_posdef) {
	    // with ALIAS ...
	    $sqlsel = "c.*, a.ALIAS from CONT_HAS_CSUBST c join ABS_CONT_ENTRY a on c.POS = a.POS ".
	   	    " where c.CONTAINER_ID=" . $cont_id . ' and a.ABS_CONTAINER_ID='. $this->cont_features['ABS_CONTAINER_ID'] .' and c.CONCRETE_SUBST_ID>0  order by c.POS DESC'; 
	}
	
	$sqlo2->Quesel($sqlsel);
	$cnt=0;
	while ( $sqlo2->ReadArray() ) {
	    
	    $featLoop = $sqlo2->RowData;
	    
	    
	    $row_features = array();
	    
	    
		if ( $cnt > $max_show_now ) {
			$backurlEnc = urlencode('edit.tmpl.php?t=CONTAINER&id='.$cont_id );
            $dataArr = array("...","...");
            if ($editAllow) {   	
            	$dataArr[]='...';
            	$dataArr[]='...';
            	$dataArr[]='...';
            }
            
            if ($max_show_now<$MAX_SHOW_SUPER_NUM) {
            	$dataArr[] = '<b><a href="sessVars_set.php?variable=o.CONTAINER.fillShow&val='.$MAX_SHOW_SUPER_NUM.'&backurl='.
            			$backurlEnc.'">... show more ...</a></b> (max: '.$MAX_SHOW_SUPER_NUM. ')';
            } else {
            	$dataArr[] = '... Max number shown: '.$MAX_SHOW_SUPER_NUM.' ...';
            }
            $tabobj->table_row ($dataArr);
            break;
        }
        
        if ($this->withTimeStamp) {
        	$checkInFeats = $this->_getCheckinFeats($sqlo, $featLoop['CONCRETE_SUBST_ID']);
        }
        
        $suc_id_loop = $featLoop['CONCRETE_SUBST_ID'];
	    
        if ( $suc_id_loop>0 ) {
	    	$outColArr  = array('ABSTRACT_SUBST_ID', 'NAME', 'CCT_ACCESS_ID');
	    	$substFeats = glob_elemDataGet3( $sqlo, 'CONCRETE_SUBST', array('CONCRETE_SUBST_ID'=>$suc_id_loop), $outColArr );
			$absFeats   = glob_elemDataGet3( $sqlo, 'ABSTRACT_SUBST', array('ABSTRACT_SUBST_ID'=>$substFeats['ABSTRACT_SUBST_ID']), array('NAME', 'H_UNIT_ID') ) ;
			$row_features['SUA.link']   = $objlnkLib->bo_IDLink( "CONCRETE_SUBST", $suc_id_loop, $substFeats['NAME'] );
	    	
	    	$row_features['SUA.NAME'] = $absFeats['NAME'];
	    	$H_UNIT_ID = $absFeats['H_UNIT_ID'];
	    	if ($H_UNIT_ID) {
	    		$featLoop ['H_UNIT'] = obj_nice_name ( $sqlo, 'H_UNIT', $H_UNIT_ID );
	    	}
	    	
	    	$auditLogSub->setObject( $sqlo, 'CONCRETE_SUBST', $suc_id_loop, $substFeats['CCT_ACCESS_ID'] );
	    	$tmpStatus   = $auditLogSub->getLastLogName($sqlo);
	    	$audColor    = $auditColors[$tmpStatus];
	    	$row_features['SUC.audit_status_txt'] = "<span style='background-color: ". $audColor."'>".$tmpStatus."</span>";
	    	
	    	$quant_sum = $this->contLib->get_one_suc_QUANTITY($sqlo, $suc_id_loop);
	    	
	    	$store_status_arr = $stockWarnLib->get_suc_info($sqlo, $suc_id_loop, $quant_sum);
	    	$row_features['SUC.store_status'] = $store_status_arr['ICON'];
	    }
	    
	    if ($editAllow) {
	    	$posLink = '<a href="p.php?mod=DEF/o.CONTAINER.modone&id='.$cont_id.'&act=mod&pos='.$featLoop['POS'].'">'.
	    	'<img src="images/but.edit.ac.gif" border=0></a>';
	    	
	    	/*$delLink = '&nbsp;<a href="obj.container.addsub.php?parx[substid]='.$featLoop['CONCRETE_SUBST_ID'].
	    		'&parx[CONTAINER_ID]='.$cont_id .'&parx[POS]='.$featLoop['POS'].'&parx[action]=DEL&parx[backurl]='.
	    		urlencode('edit.tmpl.php?t=CONTAINER&id='.$cont_id).'">'.
	    		'<img src="images/but9.del.gif" TITLE="remove from cont." border="0"></a>&nbsp;';
	    		*/
	    	$delLink = '<input type=checkbox name="posar['.$featLoop['POS'].']" value=1>';
	    	$dataArr=array( $posLink, $delLink);
	    	if ($checkin) {
	    		$checkText = $this->_getCheckinLinks($checkInFeats, $featLoop['POS'], $featLoop['CONCRETE_SUBST_ID']);
	    		$dataArr[] = $checkText;
	    	}
	    } else {
	    	$dataArr=array();
	    }
	    
	    $dataArr[]=$featLoop['POS'];
	    if ($cont_has_posdef) {
	        $dataArr[]=$featLoop['ALIAS'];
	    }
	    
	    /**
	     *   $row_features['SUC.audit_status_txt']
	     *   $row_features['SUC.store_status'],
	     *   
	     *   $featLoop['ALIQUOT_NO'], 
	    	 $featLoop['MOD_DATE'],
	    	 $featLoop['NOTES']
	     */
	    $dataArr = array_merge( $dataArr, 
	    	array(
	    		$featLoop['CONCRETE_SUBST_ID'],
	    	    $row_features['SUA.link'] , 
	    	    $row_features['SUA.NAME'],
	    	    $row_features['SUC.store_status'],
	    		$featLoop['QUANTITY'], 
	    		$featLoop['H_UNIT']
	    		
	    		) 
	    	);
	    if ($this->withTimeStamp) $dataArr[] = $this->_getCheckStats($checkInFeats, $featLoop['CONCRETE_SUBST_ID']);
	    
        $tabobj->table_row ($dataArr);
        $cnt++;
	}
	
	if ($editAllow) {
	    
		$colopt = array(1=>'colspan=2');
		$tabobj->setColOpt($colopt);
		
		$selbody = "onChange=\"this.form.submit();\"";
		$feld    = array('delli'=>'delete');
		$soption = array("selbody"=>$selbody);
		$selbox  = formc::selectFget( 'act', $feld, '', $soption);
		
		$num_cols=sizeof($headx);
		$add_col_num = $num_cols -3;
		$dataArr = array('', $selbox );
		for($i=0; $i<$add_col_num; $i++) $dataArr[]='';
		
		$rowopt  = array("bgcolor"=>'E0E0FF');
		$tabobj->table_row ($dataArr, $rowopt);
	}
	
	$tabobj->table_close();
}

/**
 * show SUMMARY of concrete substs
 * @param object $sqlo
 * @param object $sqlo2
 */
function SUC_summary(&$sqlo, &$sqlo2) {
    
    $MAX_SHOW_ENTRY=200; // max shown number
    $tablename='CONCRETE_SUBST';
    
    $cont_id = $this->id;
    $sqlsel_sub = "select distinct(c.CONCRETE_SUBST_ID) from CONT_HAS_CSUBST c where c.CONTAINER_ID=" . $cont_id . 
        ' and c.CONCRETE_SUBST_ID>0'; 
    $sqlsel = 'CONCRETE_SUBST_ID from CONCRETE_SUBST where CONCRETE_SUBST_ID in ('.$sqlsel_sub.') order by NAME';
    
    $sqlo2->Quesel($sqlsel);
    
    $tabobj = new visufuncs();
    $headOpt = array( "title" => "Summary of MACs in container");
    $head1  = array ("#");
    $head3  = array("MAC-ID", "MAC-Name *", "MAA", "Aliquots", "Quantity", "unit");
    
    
    $headx = array_merge($head1, $head3);
    
    $tabobj->table_head($headx,   $headOpt);
    $objlnkLib = new fObjViewC();
    
    $measUnitLib  = new oH_UNIT_siSubs();
    // $stockWarnLib = new o_CONCRETE_SUBST_stockwarnC();
    
    $cnt=0;
    while ( $sqlo2->ReadRow() ) {
        
        $sucid     = $sqlo2->RowData[0];
        // $suaname   = $sqlo2->RowData[1];
        if ($cnt>$MAX_SHOW_ENTRY) {
            $dataArr=array("...", "... more ...","");
            $tabobj->table_row ($dataArr);
            break;
        }
        
        $suc_feats = glob_elemDataGet3( $sqlo, 'CONCRETE_SUBST', array('CONCRETE_SUBST_ID'=>$sucid), array('ABSTRACT_SUBST_ID', 'NAME') );
        $suaid     = $suc_feats['ABSTRACT_SUBST_ID'];
        $SUA_feats = glob_elemDataGet3( $sqlo, 'ABSTRACT_SUBST', array('ABSTRACT_SUBST_ID'=>$suaid), array('NAME', 'H_UNIT_ID') );
        
        $substlnk = $objlnkLib->bo_IDLink($tablename, $sucid, $suc_feats['NAME']);
        $unit_id  = $SUA_feats['H_UNIT_ID'];
        
        $sqlsel = 'count(CONCRETE_SUBST_ID), sum(QUANTITY) from CONT_HAS_CSUBST where ' .
            'CONTAINER_ID='.$cont_id .' and CONCRETE_SUBST_ID='.$sucid;
        $sqlo->Quesel($sqlsel);
        $sqlo->ReadRow();
        
        $suc_cnt  = $sqlo->RowData[0];
        $quantSum = $sqlo->RowData[1];
        
        $unitName='';
        if ($unit_id) $unitName    = $measUnitLib->id2name($sqlo, $unit_id);
        

        $dataArr=array($cnt+1, $sucid, $substlnk, $SUA_feats['NAME'], $suc_cnt, $quantSum, $unitName);
        $tabobj->table_row ($dataArr);
        $cnt++;
    }
    
    $tabobj->table_close();
}

function SUC_box_summary(&$sqlo, &$sqlo2) {
    
    $MAX_SHOW_ENTRY=400; // max shown number
    $tablename='CONCRETE_SUBST';
    
    $cont_id = $this->id;
    // $sqlsel_sub = "select distinct(c.ALIAS) from CONT_HAS_SUB_VIEW c where c.CONTAINER_ID=" . $cont_id;
    $sqlsel = "distinct(c.ALIAS) from CONT_HAS_SUB_VIEW c where c.CONTAINER_ID=" . $cont_id ." order by c.ALIAS";
    $sqlo2->Quesel($sqlsel);
    
    $tabobj  = new visufuncs();
    $headOpt = array( "title" => "Inventory list: Content by Box");
    $headx   = array( "Box *", "MAC", "MAC-ID", "MAA", "Aliquots"); //  "Quantity" , "unit"

    $tabobj->table_head($headx,   $headOpt);
    $objlnkLib = new fObjViewC();
    
    $measUnitLib  = new oH_UNIT_siSubs();
    // $stockWarnLib = new o_CONCRETE_SUBST_stockwarnC();
    
    
    
    $cnt=0;
    while ( $sqlo2->ReadRow() ) {
        
        $alias  = $sqlo2->RowData[0];
        
        $tmp_suc_arr=array();
        $sqlsel = "max(c.CONCRETE_SUBST_ID), count(c.CONCRETE_SUBST_ID) from CONT_HAS_SUB_VIEW c where c.CONTAINER_ID=" . $cont_id .
            " and c.ALIAS=". $sqlo->addQuotes($alias) .
            " group by c.CONCRETE_SUBST_ID" .
            " order by c.CONCRETE_SUBST_ID";
        $sqlo->Quesel($sqlsel);
        while ( $sqlo->ReadRow() ) {
            $tmp_suc_arr[]=array('suc'=>$sqlo->RowData[0], 'cnt'=>$sqlo->RowData[1]);
        }
        
        
        foreach($tmp_suc_arr as $i=>$row) {
        
            $sucid    = $row['suc'];
            $suc_cnt  = $row['cnt'];
            
            
            if ($cnt>$MAX_SHOW_ENTRY) {
                $dataArr=array("...", "... more ...","");
                $tabobj->table_row ($dataArr);
                break;
            }
            
            $suc_feats = glob_elemDataGet3( $sqlo, 'CONCRETE_SUBST', array('CONCRETE_SUBST_ID'=>$sucid), array('ABSTRACT_SUBST_ID', 'NAME') );
            $suaid     = $suc_feats['ABSTRACT_SUBST_ID'];
            $SUA_feats = glob_elemDataGet3( $sqlo, 'ABSTRACT_SUBST', array('ABSTRACT_SUBST_ID'=>$suaid), array('NAME', 'H_UNIT_ID') );
            
            $substlnk = $objlnkLib->bo_IDLink($tablename, $sucid, $suc_feats['NAME']);
            // $unit_id  = $SUA_feats['H_UNIT_ID'];
            //$unitName='';
            //if ($unit_id) $unitName    = $measUnitLib->id2name($sqlo, $unit_id);
            
            
            $dataArr=array($alias, $substlnk, $sucid, $SUA_feats['NAME'], $suc_cnt);
            $row_opt=array();
            if (!$i) {
                $row_opt['bgcolor']='#F0F0AA';
            }
            $tabobj->table_row ($dataArr,$row_opt);
            $cnt++;
        }
        
    }
    
    $tabobj->table_close();
}

/**
 * show SUMMARY of abstract substs
 * @param object $sqlo
 * @param object $sqlo2
 */
 function SUA_summary(&$sqlo, &$sqlo2) {
	
	$MAX_SHOW_ENTRY=200; // max shown number
	

	$subsel =	
		" select distinct(ABSTRACT_SUBST_ID) from CONCRETE_SUBST where ".
    	" CONCRETE_SUBST_ID in (".
    	"  select CONCRETE_SUBST_ID from CONT_HAS_CSUBST where CONTAINER_ID=".$this->id .
    	")"; // order by a.NAME";
    
    $sqls2 = "ABSTRACT_SUBST_ID from ABSTRACT_SUBST where ABSTRACT_SUBST_ID in (". $subsel.")".
    	" order by NAME";
    
    $sqlo2->Quesel($sqls2);
    
    $tabobj = new visufuncs();
	$headOpt = array( "title" => "Summary of material types in container");
	$head1  = array ("#");
	$head3  = array( "MAA *", "num of MACs", "Storage-Status", "Quantity", "unit");
	
	
	$headx = array_merge($head1, $head3);
	
	$tabobj->table_head($headx,   $headOpt);
	$objlnkLib = new fObjViewC();
	
	$measUnitLib  = new oH_UNIT_siSubs();
	$stockWarnLib = new o_CONCRETE_SUBST_stockwarnC();
	
    $cnt=0;
    while ( $sqlo2->ReadRow() ) {
    	
        $suaid     = $sqlo2->RowData[0];
        // $suaname   = $sqlo2->RowData[1];
        if ($cnt>$MAX_SHOW_ENTRY) {
            $dataArr=array("...", "... more ...","");
            $tabobj->table_row ($dataArr);
            break;
        }
        $substlnk = $objlnkLib->bo_display( $sqlo, "ABSTRACT_SUBST", $suaid  );
        
        $sqlsel = 'count(distinct(CONCRETE_SUBST_ID)) from CONT_HAS_CSUBST where ' .
        	 'CONTAINER_ID='.$this->id .' and CONCRETE_SUBST_ID in ('.
        	   'select CONCRETE_SUBST_ID from CONCRETE_SUBST where ABSTRACT_SUBST_ID='.$suaid .
        	 ')';
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		
		$subscnt= $sqlo->RowData[0];
		
		// get quantity summary
		$sqlsel = 'sum(QUANTITY) from CONT_HAS_CSUBST where ' .
				'CONTAINER_ID='.$this->id .' and CONCRETE_SUBST_ID in ('.
				'select CONCRETE_SUBST_ID from CONCRETE_SUBST where ABSTRACT_SUBST_ID='.$suaid .
				')';
		
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$quantSum = $sqlo->RowData[0];
		
		$unitName    = $measUnitLib->getUnitNameFromSUA($sqlo, $suaid);
		
		$store_status_arr = $stockWarnLib->get_sua_info($sqlo, $suaid, $quantSum);
		$status_icon = $store_status_arr['ICON'];
        
        $dataArr=array($cnt+1, $substlnk, $subscnt, $status_icon, $quantSum, $unitName);
        $tabobj->table_row ($dataArr);
        $cnt++;
    }
    
    $tabobj->table_close();
}


}

class obj_edit_ext extends gObj_edit_feat_ABS { 
    
    /**
     * main tab
     */
    function main_tab() {
        
        $opt = $_REQUEST['vopt'];
        $opt_str =  $_SESSION['userGlob']['o.CONTAINER.mainview'];
        if ($opt_str!=NULL) {
            $opt = unserialize($opt_str);
        }
        if (is_array($_REQUEST['vopt'])) {
            if ($_REQUEST['vopt']['view_type']) {
                $opt['view_type']=$_REQUEST['vopt']['view_type'];
                $_SESSION['userGlob']['o.CONTAINER.mainview'] = serialize($opt);
            }
        }
        
        $main_view_type=$opt['view_type'];
        if ($main_view_type=='') $main_view_type='SUC';
        
        $url='edit.tmpl.php?t=CONTAINER&id='.$this->objid;
        $xmodes=array(
            'SUC'=>array('All batches and aliquots', $url.'&vopt[view_type]=SUC'),
            'SUC_sum'=>array('MAC summary', $url.'&vopt[view_type]=SUC_sum'),
            'SUA'=>array('MAA summary', $url.'&vopt[view_type]=SUA'),
            'REG_SUA'=>array('Registered MAAs', $url.'&vopt[view_type]=REG_SUA'),
            'SUC_box'=>array('Box inventory', $url.'&vopt[view_type]=SUC_box'),
        );
        $tab_lib = new fRiderC();
        $tab_lib->riderShow($xmodes, $main_view_type, 'Views');
        echo "<br>";
        
        return $main_view_type;
    }
    
    function rightWindow( &$sqlo, &$sqlo2) {
        
        $id = $this->id;
        $argu  = &$this->argu;
        $viewmode = $this->viewmode;
       
        $this->objid=$id;
    	echo "<ul>";
        $mainlib = new oContXedit($sqlo, $id, $viewmode);
        
    	$CHIP_READER_ID   = $argu["CHIP_READER_ID"]; 
    	
    	if ($CHIP_READER_ID) {
    		$mainlib->deviceInfo($sqlo, $CHIP_READER_ID);
    	}
    
    	$mainlib->headline();
    	
    	$mainlib->children_info($sqlo);
        
    	if ($mainlib->cntSubst or $mainlib->cnt_CHA_SUA) {
            
            $main_view_type = $this-> main_tab();
            
            if ($main_view_type=='SUC') {
    	       $mainlib->container_list_norm($sqlo, $sqlo2);
            }
            if ($main_view_type=='SUC_sum') {
                $mainlib->SUC_summary($sqlo, $sqlo2);
            }
            if ($main_view_type=='SUA') {
    	       $mainlib->SUA_summary($sqlo, $sqlo2);
            }
            if ($main_view_type=='REG_SUA') {
                $cont_lib = new oCONTAINER_one_show($id);
                $cont_lib->get_CHA_Summary($sqlo, $sqlo2);
            }
            if ($main_view_type=='SUC_box') {
                $mainlib->SUC_box_summary($sqlo, $sqlo2);
            }
        }
        
     
        
    	echo "</ul>\n";
    }

}
