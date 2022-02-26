<?php
/**
 * - manage CONCRETE_SUBST in CONTAINER
 * - substances with aliquots can not perform a new ADD action
 * - it contains code, which will be included in edit.tmpl.php, when the rider is set to "MODE"
 * - support time stamps: CSUB_HAS_STAMP
 * @package    edit.tmpl.php
 * @subpackage obj.concrete_subst.xmode.stor.inc
 * @swreq UREQ:0001107: o.CONTAINER > add one substance to storage 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
// $Header: trunk/src/www/pionir/obj.concrete_subst.xmode.stor.inc 59 2018-11-21 09:04:09Z $
 

require_once ("f.objview.inc");	
require_once ("class.history.inc");
require_once ('func_form.inc');
require_once ('f.help.inc');
require_once 'o.CONTAINER.subs.inc';

class oContManage {
    
    public $aliquotExists;
    public $container_nice;
    private $substid;
    private $has_store_entries;
    private $in_containers;

    function __construct( &$sqlo, $substid ) {
        
        $this->container_nice = tablename_nice2('CONTAINER');
    	$this->substid = $substid;
    	$this->backurl = urlencode("edit.tmpl.php?t=CONCRETE_SUBST&id=".$this->substid);
    	$this->H_UNIT  = NULL;
    	$this->containerCache = array();
    	
    	$conFeats   = glob_elemDataGet3( $sqlo, 'CONCRETE_SUBST', array('CONCRETE_SUBST_ID'=>$substid), array('ABSTRACT_SUBST_ID') ) ;
    	$absFeats   = glob_elemDataGet3( $sqlo, 'ABSTRACT_SUBST', array('ABSTRACT_SUBST_ID'=>$conFeats['ABSTRACT_SUBST_ID']), array('NAME', 'H_UNIT_ID') ) ;
    	$H_UNIT_ID = $absFeats['H_UNIT_ID'];
        if ($H_UNIT_ID) {
        	$this->H_UNIT = obj_nice_name ( $sqlo, 'H_UNIT', $H_UNIT_ID );
        }
        
        // all distinct containers ...
        $this->in_containers = array();
        $sqlsel="DISTINCT(CONTAINER_ID) from CONT_HAS_CSUBST where CONCRETE_SUBST_ID=". $substid;
        $sqlo->Quesel($sqlsel);
        while ( $sqlo->ReadArray() ) {

            $contid = $sqlo->RowData['CONTAINER_ID'];
            // $pos    = $sqlo->RowData['POS'];
            // $aliq   = $sqlo->RowData['ALIQUOT_NO'];
            //$quant  = $sqlo->RowData['QUANTITY'];
            $this->in_containers[] = $contid;
        }
    }
    
    function get_in_containers() {
        return $this->in_containers;
    }
    
    private function getDelUrl($contid, $pos) {
    	$delurl="obj.container.addsub.php?parx[substid]=".$this->substid.
    			"&parx[CONTAINER_ID]=".$contid."&parx[POS]=".$pos."&parx[action]=DEL".
    			"&parx[backurl]=".$this->backurl;
    	return ($delurl);
    	
    }
    
    function getTakeoutUrl($contid, $pos) {
        $url="p.php?mod=DEF/o.CONTAINER.modone&id=".$contid."&act=mod&pos=".$pos;
        return ($url);
    }
    
    private function _getfromBooks(&$sqlo) {
    	require_once ("o.proj.profile.inc");
    	$profileLib = new  profile_funcs ();
    	
    	$action  = "READONLY";
    	$projid  = $profileLib->getProjBySyn( $sqlo, "bookmarks", $action);
    	if (!$projid) return;
    	$contList = $profileLib->getTabList ( $sqlo, $projid, "CONTAINER");
    	return ($contList);
    }
    
    private function _cont_add_loc(&$sqlo, &$cont_list) {
        
        if(empty($cont_list)) return;
        
        foreach($cont_list as $cont_id => $name) {
            $loc = oCONTAINER_SubsC::get_location($sqlo, $cont_id);
            if($loc!=NULL) {
                if (strlen($loc)>15) $loc=substr($loc,0,15).'...';
                $cont_list[$cont_id] = $name.' &nbsp;&nbsp;('.$loc.')';
            }
        }
    }
    
    /**
     * get abstract container
     * @param  $sqlo
     * @param  $contid
     * @return array('posNice')
     */
    private function getContInfo(&$sqlo, $contid, $pos) {
    	
    	if (!$pos) return;
    	
    	if (!is_array($this->containerCache[$contid])) {
    		// get abstract container
    		$contFeats = glob_elemDataGet3( $sqlo, 'CONTAINER', array('CONTAINER_ID'=>$contid), array('ABS_CONTAINER_ID') ) ;
    		$absCont   = $contFeats['ABS_CONTAINER_ID'];
    		$this->containerCache[$contid] = array('ABS_CONTAINER_ID'=>$absCont);
    	}
    	
    	$continfo = $this->containerCache[$contid];
    	if (!$continfo['ABS_CONTAINER_ID']) return;
    	
    	// get pos nice
    	
    	$sqlsel = 'ALIAS from ABS_CONT_ENTRY where ABS_CONTAINER_ID='.$continfo['ABS_CONTAINER_ID'].' and POS='.$pos;
    	$sqlo->Quesel($sqlsel);
    	$sqlo->ReadRow();
    	$alias = $sqlo->RowData[0];
    	
    	return array('posNice'=>$alias);
    }
    
    function showStoreEntries( &$sqlo, &$sqlo2 ) {
        
    	$contid = NULL;
    	$this->aliquotExists     = 0;
    	$this->has_store_entries = 0;
    	$this->container_cache=array();
    	
    	$cnt= 0;
    	$id = $this->substid;
    	$aliquotExists=0;
    	
    	$sqlsel="count(1) from CONT_HAS_CSUBST where CONCRETE_SUBST_ID=". $id;
    	$sqlo2->Quesel($sqlsel);
    	$sqlo2->ReadRow();
    	$this->has_store_entries = $sqlo2->RowData[0];
    	if (!$this->has_store_entries) return 0;
    	
    	
    	echo "<table cellpadding=1 cellspacing=1 border=0>\n";
    	
    	$sqlsel="* from CONT_HAS_CSUBST where CONCRETE_SUBST_ID=". $id;
    	$sqlo2->Quesel($sqlsel);
    	while ( $sqlo2->ReadArray() ) { 

    		
    		$contid = $sqlo2->RowData['CONTAINER_ID'];
    		$pos    = $sqlo2->RowData['POS'];
    		$aliq   = $sqlo2->RowData['ALIQUOT_NO'];
    		$quant  = $sqlo2->RowData['QUANTITY'];
	
    		
    		if ($aliq) $aliquotExists=1;
    		if ( !$this->container_cache[$contid] ) $this->container_cache[$contid]=0;
    		$this->container_cache[$contid]++;
    		
    		$aliasInfo = $this->getContInfo($sqlo, $contid, $pos);
    		
    		// get edit-link of BO + NAME + icon (object)
    		
    		$opts=NULL;
    		
    		$bolink = fObjViewC::bo_display( $sqlo, "CONTAINER", $contid, $opts );
    		//OLD: echo 'Pos: <a href="edit.tmpl.php?t=CONT_HAS_CSUBST&id='.$contid.'&primasid[1]='.$pos.'">=&gt; <b>'.$pos.'</b></a>';
    		
    		echo "<tr><td><b>".$bolink."</b> &nbsp;&nbsp;";
    		if ($aliq and $this->has_store_entries>1) echo " Ali: <b>$aliq</b>"; // show only, if more than ONE aliquots in storage
    		
    		echo ' <span class="yGgray">Quantity:</span> <span style="font-weight:bold; font-size:1.2em;">'.$quant.'</span> '.$this->H_UNIT;
    		
    		echo ' &nbsp;&nbsp; <span class="yGgray">Pos: '.$pos.'</span>';
    		if ($aliasInfo['posNice']!=NULL) {
    		    echo ' (<b>'.$aliasInfo['posNice'].'</b>)';
    		}
    		
    		echo "</td><td>&nbsp;&nbsp;&nbsp;&nbsp;";
    		
    		$take_url = $this->getTakeoutUrl($contid, $pos); 
    		$remurl   = $this->getDelUrl($contid, $pos); 
    		echo " <a href=\"".$take_url.'"><img src="res/img/upload.svg" height=20> take part</a> &nbsp;&nbsp;';
    		echo " <a href=\"".$remurl."\"><img src=\"res/img/x.svg\"> take ALL</a>";
    		echo "</td></tr>\n";
    		$cnt++;
    	}
    	
    	echo "</table>\n";
    	
    	$show_many_link=0;
    	$komma='';
    	foreach($this->container_cache as $cont_id => $ali_cnt) {
    	    if ($ali_cnt>1) {
        	    $delurl = "obj.container.addsub.php?parx[substid]=".$this->substid.
        	       "&parx[CONTAINER_ID]=".$cont_id."&parx[action]=DEL_MANY".
        	       "&parx[backurl]=".$this->backurl;
        	    $bolink = obj_nice_name ( $sqlo, "CONTAINER", $cont_id );
        	    echo $komma." <a href=\"".$delurl."\"><img src=\"res/img/x.svg\"> take out many</a> from storage ".$bolink."";
        	    $komma=' &nbsp;|&nbsp; ';
        	    $show_many_link=1;
    	    }
    	}
    	if ($show_many_link) echo"<br>\n";

    	$this->aliquotExists = $aliquotExists;
    	return $this->has_store_entries;
    }
    
    private function _cont_name($sqlo, $cnt_id) {
        return glob_elementDataGet($sqlo, 'CONTAINER', 'CONTAINER_ID', $cnt_id, 'NAME');
    }
    
    function form1( &$sqlo, $last_containers ) {
    	$backurl = $this->backurl;
    	
    	
    	$seltext = "... storages ...";	
    	$conarr = array();
    	
//     	if (is_array($last_containers)) {
//     	    foreach( $last_containers as $tmpid) {
//     	        $conarr[] = array($tmpid, $this->_cont_name($sqlo, $tmpid));
//     	    }
//     	}
    	
    	$has_bookmarks = 0;
    	$conarr[] = array(-1, "--- from bookmarks ---");
    	$tmpArr = $this->_getfromBooks($sqlo);
    	$this->_cont_add_loc($sqlo, $tmpArr);
    	
    	if ( !empty($tmpArr) ) {
    	    $has_bookmarks = 1;
    		foreach( $tmpArr as $tmpid => $tmpname) {
    			$conarr[] = array($tmpid, $tmpname);
    		}
    	}
    	
    	
    	
    	// get/show ALL stroages from DB
    	//
    	$get_all=1;
    	if ( $get_all ) {
    	    
    	    $conarr[] = array(-1, "--- all in database ---");
    	    
    	    $sqlsel = "x.CONTAINER_ID, x.NAME, c.LOCATION from CONTAINER x ".
        	   " LEFT JOIN CHIP_READER c on x.CHIP_READER_ID=c.CHIP_READER_ID".
    		   " order by x.NAME";
    		$sqlo->Quesel($sqlsel);
    		$cnt = 0;
    		$max_show= 100;
    		
    		while ( $sqlo->ReadRow() ) {
    		    
    		    if ($cnt>$max_show) {
    		        $conarr[]  = array(-1,"... too many ...");
    		        break;
    		    }
    			$contid = $sqlo->RowData[0];
    			$cont_name = $sqlo->RowData[1];
    			$cont_loc  = $sqlo->RowData[2];
    			if ($cont_loc) {
    			    if (strlen($cont_loc)>15) $cont_loc=substr($cont_loc,0,15).'...';
    			    $cont_name .= ' &nbsp;&nbsp;('.$cont_loc.')';
    			}
    			
    			$conarr[] = array($contid, $cont_name);
    			$cnt++;
    			
    		}
    	}
    	
    	$preselected = NULL;

    	
    	//$requ_icon = ' <img src="images/redstar.gif" border=0> ';
    	
    	$selopt  = array("selecttext"=>$seltext, "req"=>1, "field.type"=>'complex' );
    	$seltext = formc::selectFget( "parx[CONTAINER_ID]", $conarr, $preselected, $selopt ); 
    	
    	htmlInfoBox( "Add to storage", "", "open", "FORM" );
    	echo "<form style=\"display:inline;\" method=\"post\" ".
    		 " name=\"editform\"  action=\"obj.container.addsub.php\" >\n";
    	echo "<input type=hidden name='parx[go]' value='1'>\n";
    	echo "<input type=hidden name='parx[substid]' value='".$this->substid."'>\n";
    	echo "<input type=hidden name='parx[backurl]' value='".$backurl."'>\n";
    	echo "<input type=hidden name='parx[action]' value='ADD'>\n";
    	echo '<input type=submit value="&nbsp; Add &nbsp;" class="yButSmall">'."\n"; // SUBMIT, yButSmall
    	
    	echo " &nbsp;&nbsp;" .  $seltext . " ";
    	// echo "&nbsp;&nbsp;Pos: "   . "<input type=text name='parx[POS]' value='AUTO'      size=3>\n";
    	echo "&nbsp;&nbsp;Quantity: " . "<input type=text name='parx[QUANTITY]' value='' size=5 required>\n"; // optional the quantity
    	echo ' '.$this->H_UNIT.'';
    	echo '&nbsp;&nbsp;&nbsp;&nbsp;<span style="color:gray;">Produce Aliquots?</span> ' . "<input type=checkbox name='parx[withAli]' value='1'>\n";
    	
    	echo "</form>\n<br>";
    	htmlInfoBox( "", "", "close" );
    }
    
    function showTimeStamps( &$sqlo ) {
    	$id = $this->substid;
    	if (!table_exists2('CSUB_HAS_STAMP')) return;
    	
    	$hasEntries=0;
    	$sqlsel = '1 from CSUB_HAS_STAMP where CONCRETE_SUBST_ID=' .$id;
    	$sqlo->Quesel($sqlsel);
    	if ($sqlo->ReadRow() ) $hasEntries=1;
    	
    	if (!$hasEntries) return; // do not show any text ...
    	
    	echo "<br><font color=gray>storage time stamps:</font> ";
    	
    	if ($hasEntries) {
    		$lnkTxt = '<a href="p.php?mod=DEF/o.CONTAINER.stamp02&id=' .$id. '">show statistics</a>';
    	} else $lnkTxt = 'no events.';
    	echo $lnkTxt . "<br>\n";
    }
    
    function help_text() {
        $helpLib = new fHelpC();
        $hlpopt = array ( 'location'=>'pionir', "object"=>"icon" );
        $helpText = $helpLib->link_show("o.CONCRETE_SUBST.tab.stor.html", "help for Storage", $hlpopt );
        echo $helpText. '&nbsp;  <span style="color:gray">inventory, storage management [<a href="view.tmpl.php?t=CONTAINER">storage list</a>]:</span>'."\n";
    }

}

// ----------
		
class oCONCRETE_SUBST_xmode_stor {
	
    function __construct( &$editLib ) {
    }		
    
    function xmode_start( &$sql, &$sql2, $id, &$arguByKey, &$x_rights, &$extraobj_o, &$tabArgu, $editAllow) {

    	echo "<ul>";
    
    	$mainlib = new oContManage( $sql, $id );
    	
    	
    	$get_last_containers=0; // NO
    	$lastContainers=array();
    	if ($get_last_containers) {
    	    // get last storages 
        	$hist_obj = new historyc();
        	$cont_history = $hist_obj->bos_get_type('CONTAINER');
        	$lastContainers  = $mainlib->get_in_containers();
        	if (is_array($cont_history)) {
        	    $lastContainers = array_unique($lastContainers + $cont_history);
        	}
    	}
    	
    	$has_entries = $mainlib->showStoreEntries( $sql, $sql2 );
    	
    	if (!$has_entries) {
    	    echo '<span style="color:gray;"><img src="res/img/info.svg" height=20> not found in a '.$mainlib->container_nice.'</span>.<br><br>'."\n";
    	    $mainlib->form1( $sql, $lastContainers );
    	} else {
    		echo "<br>";
    		/*
    		if ($mainlib->aliquotExists) {
    			echo "Info: material has Aliquots. Can not be added to new storage-locations.<br>\n";
    		} else {
    			$mainlib->form1( $sql, $lastCont );
    		}*/
    		$mainlib->form1( $sql, $lastContainers );
    	}
    	
    	echo "<br>\n";
    	$mainlib->help_text();
    	
    	$mainlib->showTimeStamps( $sql );
    }

}