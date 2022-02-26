<?php
/**
 * widget
 *
 * @package g.widget_editor.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param $go 0,1
 * @param $parx
 *     widget_id    widget ID
 * @param $parx2[]
 *   edit_is_new : 0,1

 * @param $wind    widget ID
 * @param $action
 *   widget_add
 *   widget_edit : widget_id
 *   list_resort
 * 
 */
require_once ('func_form.inc');
require_once ('gui/f.widget_MGR.inc');
require_once ('gui/f.layer_my.inc');
require_once ("javascript.inc" );


class g_widget_editor_XPL extends gPlugin {

	function register() {
		$title		= 'Widget Configuration';
		
		$this->infoarr			 = NULL;
		$this->infoarr['title']    = $title;
		//$this->infoarr['title_sh'] = 'Set Workflow';
		$this->infoarr['form_type']= 'tool'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['locrow']   = array(
		    array('home.php','Home'),
		    array('p.php?mod=DEF/g.layer.my_config','Configuration')
		    
		);
	}
	
	private static function _fields_sub(&$formobj, &$parx, &$in_arr_def, $basevar) {
	    
	    if (!empty($in_arr_def)) {
	        
	        foreach($in_arr_def as $row) {
	            
	            if (!empty($row['_dict_'])) {
	                
	                $fieldx = array ( 
	                    "title"  => $row['title'],
	                    "object" => "info",
	                );
	                $formobj->fieldOut( $fieldx ); 
	                
	                self::_fields_sub($formobj, $parx[$row['var']], $row['_dict_'], $row['var']);
	                
	            } else {
	                
	                $fieldx = array ( // form-field definition
	                    "title"  => $row['title'],
	                    "name"   => $row['var'],
	                    "object" => "text",
	                    "val"    => $parx[$row['var']],
	                    "notes" => $row['notes']
	                );
	                
	                if ($basevar) {
	                    $fieldx['name'] = 'parx['.$basevar.']['.$row['var'].']';
	                    $fieldx['namex'] = TRUE;
	                }
	                
	                if ($row['req']) {
	                    $fieldx['req']=TRUE;
	                }
	                
	                $formobj->fieldOut( $fieldx ); // output the form-field
	            }
	            
	        }
	        
	    }
	    
	}
	
	/**
	 * array
	 *   'w' : widget name
	 *   'wopt' : widget-specific options
	 *
	 * @param object $sqlo
	 */
	function form2_wid_conf($wind, $parx, &$in_arr_def, $ign_conf_test) {

	    // echo $add_button;
	    echo "<br>\n";

	    $initarr   = array();
	    $initarr["title"]       = "Configure Widget"; // title of form
	    $initarr["submittitle"] = "Save"; // title of submit button
	    $initarr["tabwidth"]    = "AUTO";   // table-width: AUTO
	    
	    $hiddenarr = NULL;
	    $hiddenarr["action"]  = 'widget_edit';
	    $hiddenarr["wind"]     = $wind;
	    $hiddenarr["ign_conf_test"] = $ign_conf_test;
	    
	    
	    
	    // init form, uses class formc
	    $formobj = $this->_html->formInit($initarr, $hiddenarr, 0);
	    
	    self::_fields_sub($formobj, $parx, $in_arr_def, NULL);
	    
	    $formobj->close( TRUE ); // close form, sshow submit-button
	    
// 	    $formobj = new formPure($initarr, $hiddenarr, 0);
// 	    $formobj->start();
// 	    echo '<input class="yButton" type=submit value="Save">'."\n";
// 	    $formobj->close();
	}
	
	function form1_add($sqlo, $go, $parx)  {
	    
	    $all_info = fWidgets_all_MGR::get_all_widget_info($sqlo);
	    
	    

	    $selarr = array();
	    foreach($all_info as $row) {
	        $selarr[$row['wid']] = $row['name'];
	    }
	    
	    $initarr   = array();
	    $initarr["title"]       = "Add a widget"; // title of form
	    $initarr["submittitle"] = "Next &gt;"; // title of submit button
	    $initarr["tabwidth"]    = "AUTO";   // table-width: AUTO
	    
	    $hiddenarr = NULL; // hidden form variables
	    $hiddenarr["action"]     = 'widget_add'; // is a required input parameter
	    
	    // init form, uses class formc
	    $formobj = $this->_html->formInit($initarr, $hiddenarr, 0);
	    
	    $fieldx = array ( // form-field definition
	        "title"   => "Widget",
	        "name"  => "widget_id",
	        "object"=> "select",
	        "val"    => NULL,
	        "inits"    => $selarr,
	        "notes" => ""
	    );
	    $formobj->fieldOut( $fieldx ); // output the form-field
	    
	    $formobj->close( TRUE ); // close form, sshow submit-button
	}
	
	private function _main_widget_init($sqlo, $widget_id, $in_arr) {
	    $one_widget_lib = new fWidgets_MGR($widget_id);
	    $one_widget_lib->load($sqlo, $in_arr);
	    $infoarr = $one_widget_lib->get_infoarr();
	    return $infoarr;
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
	
	private function widget_test($sqlo, $widget_id, $in_arr) {
	    
	    $one_widget_lib = new fWidgets_MGR($widget_id);
	    $one_widget_lib->load($sqlo, $in_arr);
	    $infoarr = $one_widget_lib->get_infoarr();
	    echo '.. perform config test of Widget: '.$infoarr['title']."<br>\n";
	    $one_widget_lib->test_config();
	    
	}
	
	function resort($sqlo, &$windarr_raw) {
	    $wind_arr = array_keys($windarr_raw);
	    $this->my_world_lib->resort_all($wind_arr);
	    $this->my_world_lib->save($sqlo);
	    $this->_html->messageBox2('ok', 'saved.');
	}
	
	function wid_conf_save($sqlo, $wind, $parx, $ign_conf_test) {
	    //TBD: check data ...
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    debugOut('(135) parx: '.print_r($parx,1), $FUNCNAME, 1);
	    
	    $this->my_world_lib->widget_set_conf($wind, $parx);
	    
	    if ($ign_conf_test) {
	        // nothing
	    } else {
    	    $widget_id = $this->my_world_lib->get_wid($wind);
    	    debugOut('(185) widget_id: '.$widget_id, $FUNCNAME, 1);
    	    $this->widget_test($sqlo, $widget_id, $parx);
	    }
	    $this->my_world_lib->save($sqlo);
	    
	    $this->_html->messageBox2('ok', 'saved.');
	}
	
	function widget_list($sqlo) {
	    
	    
	    echo 'My Widgets: <br>';
	    
	    $my_widgets = $this->my_world_lib->get_my_widgets();
	    if(empty($my_widgets)) return;
	    
	    $initarr   = array();
	    $initarr["title"]       = "Configure my world order"; 
	    $initarr["submittitle"] = "Save"; 
	    $initarr["tabwidth"]    = "AUTO"; 
	    $initarr["action"]    = "p.php"; 
	    
	    
	    $hiddenarr = NULL;
	    $hiddenarr["action"]  = 'list_resort';
	    $hiddenarr["mod"]       = $this->_mod; 
 
	    
	    $formobj = new formPure($initarr, $hiddenarr, 0);
	    $formobj->start();
	    
	    
	    
	    echo '<table id="xtable" >'."\n";

	    $base_url=$this->_plug_get_URL();
        
        foreach($my_widgets as $row) {
            
            $wind= $row['wind'];
            $widget_id= $row['wid'];
            $info_loop = $this->widget_info($sqlo, $widget_id, $row['conf']);
            $widg_nice = $info_loop['wid_info']['title'];

            // $t_icon = $objLinkLib->_getIcon($table_loop);
            
            echo '<tr class="sortx"><td>'."\n";
            echo '<input type=hidden name="windarr['.$wind.']" value="1">'."\n";
            echo '<img src="images/ic.modpx9.gif" style="float:left; padding-left:2px;"> ';
            echo '</td>';
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
        echo "<input type=\"submit\" value=\"Save sorting\" class='yButton'>";
        $formobj->close();
        
        js_table_sortable();
	}
	
	function startMain() {
		global $error;
		$FUNCNAME='startMain';
		
		$action = $_REQUEST['action'];
		$go     = $_REQUEST['go'];
		$parx   = $_REQUEST['parx'];
		$parx2  = $_REQUEST['parx2'];
		$wind   = $_REQUEST['wind'];
		$sqlo = &$this->_sqlo;
		
		//$widget_mgr_lib = new fWidgets_MGR();
		$this->my_world_lib = new fWidgets_my_show();
		$my_widgets = $this->my_world_lib->get_my_widgets();
		if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
		    debugOut('my_widgets: '.print_r($my_widgets,1), $FUNCNAME);
		}
		
		$urlbase=$this->_plug_get_URL();
		$editor_home='<a href="'.$urlbase.'"><img src="res/img/home.svg" height=24> Widget Config home</a>';
		$this->add_button='<a href="'.$urlbase.'&action=widget_add"><img src="images/ic.plus.png"> add widget</a>';
		
		echo $editor_home.' &nbsp;&nbsp;&nbsp;'. $this->add_button."<br><br>\n";
		
		$ign_conf_test = 0;
		if ($_REQUEST['ign_conf_test']>0) $ign_conf_test = 1;

		if ($action=='widget_add') {
		    
		    if (!$go) {
		        $this->form1_add($sqlo, $go, $parx);
		        return;
		    }
		    if ($go and $parx['widget_id']) {
		        
		        $widget_id = $parx['widget_id'];
		        
		        // continue aczions ...
		        $action='widget_edit';
		        $go = 0;
		        $wind = $this->my_world_lib->add($widget_id);
		        $dummy= array();
		        $ign_conf_test=1;
		        $this->wid_conf_save($sqlo, $wind, $dummy, $ign_conf_test);
		    }
		}
		
		if ($action=='widget_edit') {
		   
		    $in_arr=array();
		    $widget_info = $this->my_world_lib->get_all_of_wid($wind);
		    $wid_infoarr = $this->_main_widget_init($sqlo, $widget_info['wid'], $in_arr);
		    echo 'Widget-title:'.$wid_infoarr['title']."<br><br>\n";
		    
		    if ($go) {
		        $this->wid_conf_save($sqlo, $wind, $parx, $ign_conf_test);
		        $ign_conf_test = 0; // reset to normal
		    } else {
		        $wid_conf = $this->my_world_lib->get_wid_conf($wind);
		        $this->form2_wid_conf($wind, $wid_conf, $wid_infoarr['in_arr_def'], $ign_conf_test); 
		        return;
		    }
		}
		
		if ($action=='widget_del') {
		    if ($wind) {
		        $this->my_world_lib->delete($wind);
		        $this->my_world_lib->save($sqlo);
		    }
		}
		
		if ($action=='list_resort') {
		    //echo "DDDX:".print_r($_REQUEST['windarr'],1)."<br>";
		    $this->resort($sqlo, $_REQUEST['windarr']);
		}
		
		$this->_html->line_out('<br>');
		$this->widget_list($sqlo);
	}
	
}