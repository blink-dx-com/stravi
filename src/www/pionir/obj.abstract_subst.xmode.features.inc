<?php
/**
 * used by edit.tmpl.php
 * @author Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @module obj.abstract_subst.xedit.php
 */

require_once 'o.S_OBJLINK.subs.inc';
require_once ("f.objview.inc");	
require_once ("visufuncs.inc");
require_once 'o.ABSTRACT_SUBST.proto.inc';
require_once 'o.ABSTRACT_SUBST.bom.inc';
require_once 'o.ABSTRACT_SUBST.logi.inc';
require_once 'o.CONTAINER.subs.inc';
require_once 'subs/o.ABSTRACT_SUBST_bom_form.inc';


class oABSTRACT_SUBST_xHelp {
    
    const PREF_VAR_NAME='o.ABSTRACT_SUBST.xspref';
    public $user_prefs;
    
    function __construct($id, $argu, $editAllow) {
		$this->id   = $id;
		$this->argu = $argu;
		$this->editAllow = $editAllow;
		$this->user_prefs = unserialize($_SESSION['userGlob'][self::PREF_VAR_NAME]);
	}
	
	function sub_title($title) {
	    echo '&nbsp;&nbsp;<span style="color:gray;">'.$title.':</span>';
	}
	
	function show_objlinks($sqlo) {
	    $objlink_lib = new oS_OBJLINK_subs();
	    $children = $objlink_lib->getChiObjByKey($sqlo, 'ABSTRACT_SUBST', $this->id, 'ERPchild');
	    if (!is_array($children)) return;
	    if (!sizeof($children)) return;
	    $objLinkLib = new fObjViewC();
	    
	    echo '<br><span style="color:gray;">Linked objects:</span><ul>'."\n";
	    foreach($children as $child) {
	        $tempHtml = $objLinkLib->bo_display( $sqlo, $child[0], $child[1] );
	        echo tablename_nice2($child[0]) ." ".$tempHtml."<br>";
	    }
	    echo '</ul>'."\n";
	}
	
	function show_jour_entry($sqlo) {
	    
	    require_once 'o.JOUR_ENTRY.subs2.inc';
	    require_once 'gui/o.JOUR_ENTRY.gui2.inc';
	    
	    $jou_obj_arr = oJOUR_ENTRY_subs::get_JOU_links($sqlo, 'ABSTRACT_SUBST', $this->id );
	    if (empty($jou_obj_arr)) {
	        return;
	    }

	    $this->sub_title('Related '.tablename_nice2('JOUR_ENTRY').'s');
	    echo '<ul>'."\n";
	    oJOUR_ENTRY_gui2::show_list1($sqlo, $jou_obj_arr);
	    echo "</ul>\n";
	}
	
	function showMetaChildren(&$sqlo) {
		$MAXSHOW=40;
		$sqlsel = "cs.CHILD_SUBST_ID, cs.NOTES_SH, cs.POS from ".
			"ABS_CHILD_SUBST cs join ABSTRACT_SUBST xs on cs.CHILD_SUBST_ID=xs.ABSTRACT_SUBST_ID ".
			"where cs.ABSTRACT_SUBST_ID=".$this->id.' order by xs.NAME';
		$sqlo->Quesel($sqlsel);
		$cnt=0;
		$substArr=NULL;
		while ( $sqlo->ReadRow() ) {
			if ($cnt>$MAXSHOW) {
				break;
			}
		    $substLoop = $sqlo->RowData[0];
		    $notesLoop = $sqlo->RowData[1];
		    $pos = $sqlo->RowData[2];
		    $substArr[]=array($substLoop, $notesLoop, $pos);
		    $cnt++;
		}
		
		if (empty($substArr)) {
			echo '<span style="color: gray;">... META-material has no children.</span><br />';
			return;
		}
		
		// select children
		$condparam = urlencode('x.ABSTRACT_SUBST_ID in (select CHILD_SUBST_ID from '.
			' ABS_CHILD_SUBST where ABSTRACT_SUBST_ID='.$this->id.')');
		$linktmp = '[<a href="view.tmpl.php?t=ABSTRACT_SUBST&condclean=1&tableSCond='.$condparam.'">Select all children</a>]'."<br />\n";
		
		if ($this->editAllow) {
    		echo '<form style="display:inline;" method="post" '.
    		  		' name="editform"  action="p.php" >'."\n";
    		echo '<input type=hidden name="mod" value="DEF/o.ABSTRACT_SUBST.metaAdd">'."\n";
    		echo '<input type=hidden name="action" value="delete">'."\n";
    		echo '<input type=hidden name="id" value="'.$this->id.'">'."\n";
    		
		}
		$tabobj = new visufuncs();
		$headOpt = array( "title" => "children of META", 'title_notes'=>'&nbsp;&nbsp; '.$linktmp);
		$headx   = array ('', 'Material <img src="images/but.asc.gif">', "Link-Notes", "Pos");
		$tabobj->table_head($headx,   $headOpt);
		$objLinkLib = new fObjViewC();
		
		foreach( $substArr as $substarr ) {
			
			$substLoop = $substarr[0];
			$notes     = $substarr[1];
			$pos       = $substarr[2];
			$substHtml = $objLinkLib->bo_display( $sqlo, 'ABSTRACT_SUBST', $substLoop );
			
			$postxt = $pos==1 ? $pos .' <img src="images/i13_infog.gif" title="most important child (Pos=1)">' : $pos;
			if ($this->editAllow) {
			    $checkbox='<input type="checkbox" name="ch['.$substLoop.']" value="1">';
			} else $checkbox='';
			
			$dataArr=array($checkbox, $substHtml, $notes, $postxt);
			$tabobj->table_row ($dataArr);
		}
		
		if ($cnt>$MAXSHOW) {
			$tabobj->table_row (array('......'));
		}
		$tabobj->table_close();
		if ($this->editAllow) {
		    echo '<input type=submit class="yButSmall" value="Remove selected children">'."\n"; // SUBMIT
		    echo '</form>'."\n";
		}
	}
	
	// analyse mother
	function anaMotherSubst(&$sqlo) {
		$MAXSHOW=5;
		$sqlsel = "ABSTRACT_SUBST_ID from ABS_CHILD_SUBST where CHILD_SUBST_ID=".$this->id;
		$sqlo->Quesel($sqlsel);
		$cnt=0;
		$substArr=array();
		while ( $sqlo->ReadRow() ) {
			if ($cnt>$MAXSHOW) {
				break;
			}
		    $substLoop = $sqlo->RowData[0];
		    $substArr[]= $substLoop;
		    $cnt++;
		}
		if (empty($substArr)) return;
		
		$komma=NULL;
		$objLinkLib = new fObjViewC();
		
		foreach( $substArr as $substid ) {
			$substNames  .= $komma . $objLinkLib->bo_display( $sqlo, 'ABSTRACT_SUBST', $substid );
			$komma=', ';
		}
	
		return $substNames;
		
	}
	
	// show thumbnail
	function showThumbnail(&$sqlo) {
    	
	    $tablename='ABSTRACT_SUBST';
    	require_once('gui/o.SATTACH.imgshow.inc');
    	$attachLib = new oSATTACH_imgshow();
    	$relid = $attachLib->getThumbAttach( $sqlo, $tablename, $this->id);
    	if ($relid) {
    	    $htmlTmp = $attachLib->getImgTag($relid);
    	    echo '<br><span style="color:gray;">Thumbnail:</span><ul>'.$htmlTmp.'</ul>'."\n";
    	}
	}
	
	function get_protos($sqlo) {
	    $proto_lib = new oABSTRACT_SUBST_proto();
	    $proto_lib->set_obj($sqlo, $this->id);
	    $protos = $proto_lib->get_protos($sqlo);
	    
	    return $protos;
	}
	
	function show_CAS($sqlo) {
	    $CAS = $this->argu['CAS'];

	    echo 'Search in <a href="http://www.commonchemistry.org/ChemicalDetail.aspx?ref='.$CAS.'" target=_new>CAS-Query-Database</a> for reagent details + Wikipedia'."\n";
	}
	
	private function _container_info($sqlo, $cont_id) {
	    
	    $cont_lib   = new oCONTAINER_SubsC();
	    $objLinkLib = new fObjViewC();
	    $html_tmp = $objLinkLib->bo_display( $sqlo, 'CONTAINER', $cont_id );
	    $cont_lib->setContainer( $sqlo, $cont_id);
	    $dev_info = $cont_lib->device_info($sqlo);
	    
	    $html_all = $html_tmp . ' (Loc: '.$dev_info['LOCATION'].')';
	    return $html_all;
	}
	
	function containers_show($sqlo) {
	    $logisticsLib = new oABSTRACT_SUBST_logi( $this->id );
	    $sua_containers = $logisticsLib->get_storage_CHA($sqlo);
	    if (!empty($sua_containers)) {
	        $this->sub_title('Registered for '.tablename_nice2('CONTAINER') .' ');
	        if (sizeof($sua_containers)==1) {
	            $cont_id = current($sua_containers);
	            $html_tmp = $this->_container_info( $sqlo, $cont_id );
	            echo $html_tmp."<br>\n";
	        } else {
	            echo "<ul>";
	            foreach($sua_containers as $cont_id) {
	                echo $this->_container_info( $sqlo, $cont_id ) ."<br>\n";
	            }
	            echo "</ul>";
	        }
	    }
	}
}


class obj_edit_ext extends gObj_edit_feat_ABS {
    
    function rightWindow( &$sqlo, &$sqlo2) {
        $id = $this->id;
        $argu= $this->argu;
        $o_rights = $this->o_rights;
        $viewmode = $this->viewmode;
       
    	$editAllow = $o_rights["write"];
    	if ($viewmode!= "edit" ) $editAllow = 0;
    	
    	$a_proto_id = 0;
    	
    	$subLib = new oABSTRACT_SUBST_xHelp($id, $argu, $editAllow);
    	
    	if ($subLib->user_prefs['JOUR_ENTRY.sh']) {
    	    $subLib->show_jour_entry($sqlo);
    	}
    	
    	$subLib->showThumbnail($sqlo);
    	
    	$subLib->show_objlinks($sqlo);
    	
    	$specialIcons = "";
    	// <span style="color:gray; font-weight:bold;">QC:</span>
    	//if ( $argu['QC_FLAG']>0 ) {
    	//	$specialIcons .= "<img src=\"images/i40.cert.gif\" title=\"under QC-control\" hspace=1>"; 	
    	//}
    	if ( $argu['IS_META']>0 ) {
    		$specialIcons .= "<img src=\"images/i40.SUA.meta.gif\" title=\"is a META-material\" hspace=1>";
    		$specialIcons .= ' &nbsp;&nbsp; <a href="p.php?mod=DEF/o.ABSTRACT_SUBST.meta_tree&id='.$id.'">'.
    		   '<img src="images/ic.tree1.svg" title="show as tree/network" hspace=1 height=40></a>';
    		
    		if ($editAllow) {
    		    $specialIcons .= '&nbsp;&nbsp; <a href="p.php?mod=DEF/o.ABSTRACT_SUBST.metaAdd&id='.$id.'&action=add">'.
        		    '<img src="res/img/plus.svg" title="add" hspace=1 height=40>'.
    		    'Add children (from clipboard)</a> ';
    		}
    	}
    	if ( table_exists2( 'ABS_CHILD_SUBST' ) ) {
    		$motherInfo = $subLib->anaMotherSubst($sqlo);
    		if ($motherInfo!=NULL) {
    			$specialIcons .= '<img src="images/i40.SUA.metaChild.gif" title="has META-parents" hspace=1> parent(s): '.$motherInfo; 	
    		}
    	}
    	if ($specialIcons!=NULL) echo ' &nbsp; '.$specialIcons.'<br />'."\n";
    	
    	if ( $argu['VERSFLAG']==1 ) {
    		require_once 'gui/o.SUA.versman.gui.inc';
    		$versGuiLib = new oSUA_gui();
    		$versGuiLib->showVersTable($sqlo, $sqlo2, $id, 0);
    	}
    	
    	$do_logistics=1;
    	if ($do_logistics) {
    	    // show CONTAINERs of SUA
    	    $subLib->containers_show($sqlo);
    	}
    	
    	if ( $argu['IS_META']>0 ) {
    		echo '<ul>';
    		$subLib->showMetaChildren($sqlo);
    		echo "</UL>\n";
    	}
    
    	$protos = $subLib->get_protos($sqlo);
    	if (!empty($protos)) {
    
    	    // require_once ("gui/o.PROTO.stepout1.inc");
    		require_once ("gui/o.PROTO.stepout2.inc");		
    		
    		$protoShowObj = new oProtocolShowC();
    		$protoShowObj->writeJavascript();
    		
    		$subLib->sub_title('Preparation protocols');
    		echo "<br>\n";
    		
    		foreach($protos as $proto_row) {
    		    $a_proto_id = $proto_row['ap'];
    		    $protoShowObj->proto_abstract_show($sqlo, $sqlo2, $a_proto_id);
        		echo "<br>\n";
    		}
        }
        
        $sua_bom_lib = new oABSTRACT_SUBST_bom($id);
        $sua_compo_pos  = $sua_bom_lib->get_compo_for_form($sqlo);
        
        if (!empty($sua_compo_pos)) {
            echo '<ul>'."\n";
            $form_sua_lib = new oABSTRACT_SUBST_bom_form($id);
            $form_sua_lib->form1($sqlo, $sua_compo_pos, 0);
        }
    	
    	
    	return 0;
      }

}



