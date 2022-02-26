<?php
/**
 * - special code for document (LINK) specials
 * - handle class 'query'
 * - handle version control (oLINKvershist)
 * $Header: trunk/src/www/pionir/obj.link.xmode.specials.inc 59 2018-11-21 09:04:09Z $
 * @package    edit.tmpl.php 
 * @subpackage obj.link.xmode.specials.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com) 
 */
require_once ("o.LINK.vershist.inc");

class oLinkSpecTab {

    function __construct($id) {
    	$this->id = $id;
    }
    
    function f_query( &$sql, $extra_obj_id, $myqu_go, $myquvarval) {
    	require_once ("subs/obj.link.c_query_sub2.inc"); 
        global  $varcol; 
    	
        echo "<B>Execute query</B> ";
        echo "[<a href=\"obj.link.c_query_mylist.php\">my searches</a>] ";  
        
        $values_tmp = $varcol->select_by_name ($extra_obj_id);
        $values     = &$values_tmp["values"];
        
        if ( $values["table"]!="" ) {
            $tmpNiceName =  tablename_nice2($values["table"]); 
            echo "&nbsp;&nbsp;[<a href=\"view.tmpl.php?t=".$values["table"]."\">list '".$tmpNiceName."'</a>]";
        } 
        echo "&nbsp;&nbsp;[<a href=\"help/robo/o.LINK.class.query.html\" target=help>help</a>]";
        echo "<br>\n";
        if ( strlen($tmpnotes) ){
            $html_notes = htmlspecialchars( $tmpnotes );
            echo "<font color=gray>Notes:</font> <I>$html_notes</I><br>"; 
        }
        echo "<br>\n"; 
        $queryForm = new oLinkQueryGui();
    	$foropt    = array( 'formVarBase'=> 'tabArgu'); // add this name-base to the form-variables
        $queryForm->form_manage( $sql, $values, $myquvarval, "edit.tmpl.php?t=LINK&id=".$this->id, 
    					$extra_obj_id, $myqu_go, $foropt );
                                                                                 
        echo "<br>\n";
    }
    
    function user_view_show($sqlo, $sqlo2) {
        
        require_once 'gui/g.objtab.list_user_lib.inc';
        
        echo 'Shows the configured "'.g_objtab_list_user_lib::TOOL_NAME.'" <br><br>'."\n";
        
        $docid = $this->id;
        $main_lib = new g_objtab_list_user_lib($sqlo, $docid);
        $main_lib->init_table($sqlo);
        $main_lib->show($sqlo, $sqlo2);
        
    }
    
    function special_SUA_REF_config($sqlo) {
        echo '<b>Shows a Material Template Reference Config</b> &nbsp; [<a href="p.php?mod=DEF/o.LINK.SUC_REF_config&id='.$this->id.'">Edit Reference-Config</a>]<br><br>'."\n";
        
        require_once dirname(__FILE__).'/impexp/suc_ref/o.LINK.TTC.GUI.inc';
        $config_show_lib = new o_LINK_SUC_REF_config_SHOW($sqlo, $this->id, '', NULL, array() );
        $config_show_lib->view_structure($sqlo);
    }
    
    function labbook_show($sqlo, $sqlo2) {
        require_once('gui/o.JOUR_ENTRY.gui.inc');
        
        oEXPlabjourC::labBookTrack($sqlo, $this->id);
    }

}

class oLINK_xmode_specials {
	
	
	
	/**
	* @param array $tabArgu  'myqu_go', 'myquvarval' variables for the SQL-query form
	* @todo need $sql2 !!!
	*/
	function xmode_start( &$sql, &$sql2, $id, &$arguByKey, &$x_rights, &$extraobj_o, &$tabArgu, $editAllow) {
		global $varcol;
	
		echo "<ul>\n";  
		
		$tabLib = new oLinkSpecTab($id);
		
		$extra_obj_id =  $arguByKey['EXTRA_OBJ_ID'];
		$tmpnotes     = &$arguByKey['NOTES'];
		
		if ( $extra_obj_id ) $extra_class = $varcol->obj_id_to_class_name ( $extra_obj_id );
		
		if ( $extra_class=="query" ) { 
			$tabLib->f_query( $sql, $extra_obj_id, $tabArgu['myqu_go'], $tabArgu['myquvarval'] );
		}
		
		if ( $extra_class=="user_view" ) {
		    $tabLib->user_view_show($sql, $sql2);
		}
		
		if ( $extra_class=="labbook" ) {
		    $tabLib->labbook_show($sql, $sql2);
		    return;
		}
		
		if ( $extra_class=="SUA_REF_config" ) {
		    $tabLib->special_SUA_REF_config($sql);
		}
		
// 		if ( $extra_class=="queryAdv" ) { 
// 			echo '<b><a href="obj.link.c_queryAdv_exec.php?id='.$id.'">Execute Advanced query</a><br /></b>';
// 			echo "<br /><br />\n";
			
// 			$tmptext = "Generate authorization key";
// 			if ( $editAllow>0 ) {
// 				//check rights for this option
// 				echo '<a href="obj.link.c_KeyGen_exec.php?id='.$id.'">'.$tmptext.'</a> (need special role-right)';
// 				// $QueryAdvForm = new advForm();
// 		    } else {
// 				echo '<font color=gray>'.$tmptext.' (use editmode)</font><br />';
// 			}
// 		    return;
// 		}
		
		$versionObj = new oLINKversC();
		if ( $versionObj->isPossible() ) {
			if ( $versionObj->isUnderControl($sql, $id) ) {
				$versHistObj = new oLINKvershist($versionObj);
				$versHistObj->show( $sql, $sql2, $id);
			}
		}
		
		echo '<br><span style="color:gray; font-size:0.8em;">This page shows special behaviour for: document: class="query" OR class="User-List-View" OR version control</span>'."\n";
	}

}
