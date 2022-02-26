<?php
/**
 * @package layer.my.subs.inc
 */

require_once ("f.textOut.inc");
require_once ("o.DB_USER.subs2.inc");
require_once 'o.LINK.list_user.inc';
require_once ("f.objview.inc");
require_once 'o.LINK.list_user.inc';
require_once 'f.html_img.inc';

require_once ('gui/f.widget_MGR.inc');
require_once ('gui/f.layer_my.inc');
require_once  'gui/g.modfavs.inc';

require_once 'impexp/protoquant/o.PROTO.def_min.inc';

class g_theme_my_cls {
    
    private $_config_mode=0;
    
    function set_config_mode($flag) {
        $this->_config_mode=$flag;
    }
    
    function list_views_user($sqlo) {
        global $error;
        
        $misc_folder = oLINK_list_user::get_STD_folder($sqlo);
        if ($error->Got(READONLY))  {
            $error->printAll();
            return;
        }
        
        
        $objLinkLib = new fObjViewC();
        
        $doc_table='LINK';
        $docs = cProjSubs::get_OBJECTS_all ( $sqlo, $misc_folder, 'LINK' ); // TBD: SUB-select class
        echo "<b>User-List-View collection:</b> ";
        echo '&nbsp; [<a href="p.php?mod=DEF/g.objtab.list_user&action=doc_crea">Create new</a>]'."\n";
        echo "<br><br>\n";
        
        echo '<table>'."\n";
        
        if(!empty($docs)) {
            foreach($docs as $doc_id) {
                
                $table_loop=NULL;
                $doc_nice = obj_nice_name ( $sqlo, $doc_table, $doc_id );
                $base_url='p.php?mod=DEF/g.objtab.list_user&id='.$doc_id;
                
                try {
                    $doc_conf = oLINK_list_user::doc_load($sqlo, $doc_id);
                } catch (Exception $e) {
                    echo 'Exception: Config-Document:"'.$doc_nice.'": ',  $e->getMessage(), "<br>\n";
                }
                
                if (!empty($doc_conf)) {
                    $table_loop = $doc_conf['t'];
                }
                
                $t_icon = $objLinkLib->_getIcon($table_loop);
                
                echo '<tr><td>'."\n";
                echo '<img src="'.$t_icon.'">  <a href="'.$base_url.'&action=show">'.$doc_nice.'</a><br>'."\n";
                echo '</td>';
                echo '<td style="padding-left:20px;">';
                echo '<a href="'.$base_url.'"><img src="res/img/settings.svg" height=18></a>';
                echo '</td>';
                echo '</tr>'."\n";
            }
        }
        echo '</table>'."\n";
        
    }
    
    private function widget_run($sqlo, $sqlo2, $widget_id, $in_arr) {
        
        $one_widget_lib = new fWidgets_MGR($widget_id);
        $one_widget_lib->load($sqlo, $in_arr);
        $one_widget_lib->set_sqlo2($sqlo2);
        $one_widget_lib->test_config();
        $infoarr = $one_widget_lib->get_infoarr();
        echo '<span class="yGgray">Widget: </span>'.$infoarr['title']."<br>\n";
        $one_widget_lib->run($sqlo);
        
    }
    
    private static function widget_info($sqlo, $widget_id, $in_arr) {
        $error_txt='';
        try {
            $one_widget_lib = new fWidgets_MGR($widget_id);
            $one_widget_lib->load($sqlo, $in_arr);
            $one_widget_lib->test_config();
            $infoarr = $one_widget_lib->get_infoarr();
        } catch (Exception $e) {
            
            $trace_string = $e->getTraceAsString();
            $trace_string = str_replace("\n", "<br>", $trace_string);
            $error_txt = 'Widget:"'.$widget_id.'": '.  $e->getMessage();
        }
        
        return array('wid_info'=>$infoarr, 'error_txt'=>$error_txt);
    }
    
    function widget_edit_list($sqlo) {
        
        $base_url='p.php?mod=DEF/g.widget_editor';
        
        echo '<br>'."\n";
        echo '<span class=""><b>My Widgets:</b></span>'."\n";
        echo   ' &nbsp;<a href="'.$base_url.'&action=widget_add"><img src="images/ic.plus.png"> add widget</a>';
        echo "<br><br>\n";
        
        $my_world_lib = new fWidgets_my_show();
        $my_widgets = $my_world_lib->get_my_widgets();
        if(empty($my_widgets)) return;
        
        
        echo '<table id="xtable" >'."\n";

        foreach($my_widgets as $row) {
            
            $wind= $row['wind'];
            $widget_id= $row['wid'];
            $info_loop = self::widget_info($sqlo, $widget_id, $row['conf']);
            $widg_nice = $info_loop['wid_info']['title'];
            
            // $t_icon = $objLinkLib->_getIcon($table_loop);
            
            echo '<tr>'."\n";
           
            echo '<td style="padding-left:20px;">';
            echo '<a href="'.$base_url.'&wind='.$wind.'&action=widget_edit"><img src="res/img/settings.svg" height=20></a>  '.$widg_nice;
            if ( $info_loop['error_txt']) {
                echo '<span style="color:red;">ERROR: </span>'.$info_loop['error_txt'];
            }
            echo '</td>';
            echo '<td style="padding-left:20px;">';
            echo '<a href="'.$base_url.'&wind='.$wind.'&action=widget_del"><img src="res/img/x.svg" height=20 title="delete"></a>';
            echo '</td>';
            echo '</tr>'."\n";
        }
        
        echo '</table>'."\n";

    }
    
    
    function widget_list($sqlo, $sqlo2) {
        global $error;
        $my_world_lib = new fWidgets_my_show();
        
        // echo 'My Widgets: <br>';
        
        $my_widgets = $my_world_lib->get_my_widgets();
        if(empty($my_widgets)) {
            echo '<br>No widgets configured.<br>'."\n";
            return;
        }
        
        foreach($my_widgets as $row) {
            
            $widget_id= $row['wid'];
            try {
                $this->widget_run($sqlo, $sqlo2, $widget_id, $row['conf']);
            } catch (Exception $e) {
                
                $trace_string = $e->getTraceAsString();
                $trace_string = str_replace("\n", "<br>", $trace_string);
                echo '<b>ERROR:</b> Widget:"'.$widget_id.'": ',  $e->getMessage(), "\n";
            }
            if ($error->Got(READONLY))  {
                $error->printAll();
                $error->reset();
            }
            
            
            echo "<br><br>\n";          
        }
 
    }
    
    function favs_show($sqlo) {
        
        // <span class="yGlarger">
        echo '<span>'."\n";
        echo '<b>Plugin favorites</b><br>'."\n";
        
        $fav_arr = g_modfavs::get_my_favs($sqlo);
        if (empty($fav_arr)) {
            echo '<span style="color:gray;">No favorites found.</span><br>';
        } else {
            
            echo "<ul>\n";
            
            foreach($fav_arr as $fav_id) {
                
                $tmp_lib  = new g_modfavs_one($sqlo, $fav_id);
                $fav_info = $tmp_lib->get_data();
                $mod_id   = $fav_info['mxid'];
                $params   = $tmp_lib->get_url_params();
                
                $mod_info = g_modfavs::get_mod_info($sqlo, $mod_id);
                $icon = fObjViewC::getIcon($mod_info['t']);
                
                echo '<a href="'.$mod_info['url']. $params. '">'.
                    '<img src="'.$icon.'"> '.
                    $fav_info['title'].'</a>';
                if ($this->_config_mode) {
                    echo' <a href="p.php?mod=DEF/g.plugin.addfav&favid='.$fav_id.'&action=edit" title="configuration">'.
                    '<img src="res/img/settings.svg"></a>';  
                }
                echo "<br>\n";
            }
            echo "</ul>\n";
        }
        echo '</span>'."\n";
    }
    
    /**
     * show all
     * @param object $sqlo
     * @param object $sqlo2
     */
    function show($sqlo, $sqlo2)  {
        
        $text_opt = array(); //array('font-size'=> 'large');
        $textoutObj = new textOutC($text_opt);
        
        echo '<div style="padding-left:10px;">'."\n";
        $action=$_REQUEST['action'];
        if ($action==NULL) $action='STD';
        
        if ($this->_config_mode) {
            
            $widget_base_url='p.php?mod=DEF/g.widget_editor';
            $editor_home  ='<a href="p.php?mod=DEF/g.layer.my_config"><img src="res/img/home.svg" height=24> Config home</a>';
            $editor_home .= ' &nbsp;&nbsp;&nbsp; [<a href="p.php?mod=DEF/g.plugin.addfav">Config my Plugins</a>]';
            
            echo $editor_home."<br><br>\n";
            
        }
        
        if ($action=='user-list-coll') {
            echo "<br><br>\n";
            $this->list_views_user($sqlo);
        }
        
        if ($action=='STD') {
            
            $home_proj_id = oDB_USER_sub2::userHomeProjGet( $sqlo );
            // $protoquant_proj = fQuant_defs::get_profile_PROJ($sqlo);
            
            ?>
        	<div class="container" style="display: flex; ">
        	<div style="width: 50%; flex-grow: 1;">
        	<?php
        	
        	
        	//echo '<table border=0 cellpadding=10><tr valign=top>';
        	//echo '<td width=30%>'."\n";
        	
        	$icon_height=25;
        	
        	$flist=array();
        	$flist[] = array("ty"=>"head", "txt"=>'Main links', "lay"=>"1" );
        	if ($home_proj_id) $flist[] = array("ty"=>"lnk", "txt"=>'my folder',  "href"=>'edit.tmpl.php?t=PROJ&id='.$home_proj_id, 'iheight'=>$icon_height,
        		"iicon"=>'icon.PROJ.svg', 'li'=>'br');
        	else  $flist[] = array("ty"=>"lnk", "txt"=>'my folder (not exists)',  "href"=>'', "iicon"=>'icon.PROJ.gif', 'li'=>'br' );
        	$flist[] = array("ty"=>"lnk", "txt"=>'my object favorites',  "href"=>'p.php?mod=DEF/g.obj.favact&act=show', "icon"=>'images/but.heart.png', 'iheight'=>$icon_height, 'li'=>'br' );
        	// $flist[] = array("ty"=>"lnk", "txt"=>'my tool favorites',  "href"=>'p.php?mod=DEF/modfavs', "icon"=>'images/but.heart.png', 'iheight'=>$icon_height, 'li'=>'br' );
        	
        	$flist[] = array("ty"=>"lnk", "txt"=>'my account',  "href"=>'obj.db_user.settings.php', "iicon"=>'icon.DB_USER.svg', 'iheight'=>$icon_height, 'li'=>'br');
        	$flist[] = array("ty"=>"lnk", "txt"=>'my objects statistics',  "href"=>'p.php?mod=DEF/o.DB_USER.obj_my&user_id=me', 
        	    "icon"=>'images/ic.24.diagram.png', 'iheight'=>$icon_height, 'li'=>'br' );
    
        	$flist[] = array("ty"=>"lnk", "txt"=> 'lab journal',  "href"=>'obj.jour_entry.list.php?parx[action]=search&parx[sea_my]=1', "iicon"=>'icon.JOUR_ENTRY.svg', 
        	    'iheight'=>$icon_height, 'notes'=>' &nbsp;&nbsp;<a href="obj.jour_entry.ed1.php?action=create"><img src="res/img/plus.svg" title="Create new entry"></a>',
        	    'li'=>'br');
        	$flist[] = array("ty"=>"lnk", "txt"=>'my searches',  "href"=>'obj.link.c_query_mylist.php', "icon"=>'images/ic.myqueryLogo.40.png', 'iheight'=>$icon_height, 'li'=>'br' );
        	
        	$tmp_arr = array("ty"=>"lnk", "txt"=>'Protoquant for MAC',  "href"=>'obj.protocol.quant.php?q_table=CONCRETE_SUBST', "icon"=>'images/ic.protoquant.svg',
        	    'iheight'=>$icon_height, 'li'=>'br' );
        	$flist[] = $tmp_arr;
        	$tmp_arr = array("ty"=>"lnk", "txt"=>'User-List-View collection',  "href"=>'home.php?action=user-list-coll', "iicon"=>'but.listshort.gif',
        	    'iheight'=>$icon_height, 'li'=>'br' );
        	$flist[] = $tmp_arr;
        	
        	$flist[] = array("ty"=>"headclose"); 
        	$textoutObj->linksOut($flist);
        	
        	
        	echo '</div>'."\n";
        	echo '<div style="flex-grow: 3;">'."\n";
        	
    
//         	echo '</td><td>&nbsp;</td>';
//         	echo '<td width=70%>'."\n";
        	
        	$this->favs_show($sqlo);
        	
        	echo '</div>'."\n";
        	echo '</div>'."\n";
        	
//         	echo '</td>'."\n";
//         	echo '</tr></table>'."\n";
        	if ($this->_config_mode) {
        	    $this->widget_edit_list($sqlo);
        	} else {
        	   $this->widget_list($sqlo, $sqlo2);
        	}
    	}
    	
    	echo '</div>'."\n";
    }
}
