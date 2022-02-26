<?php
require_once 'gui/g.modfavs.inc';
require_once ('func_form.inc');
require_once ('f.msgboxes.inc');

/**
 * manage favorite module;
 * favs are shown in plugin DEF/modfavs.inc
 * @swreq UREQ:14206  g.plugin > Plugin-Favorites for Users
 * @package g.plugin.addfav.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param $id: ID of module (for 'add')
 * @param $action
 *   ['show'] -- show all favs
 *   'add'    -- needs $id
 *   'remove'
 *   'edit'   -- needs $favid
 * @param favs array
 * @param favid -- ID of FAV
 * @param $parx
 * @param $favpar -- favorite parameters -- "name"
 * @var in_arr_def_STRUCT see g.modfavs.inc
 */


/**
 * manage one favorite
 * saved in USER_PREF2 : KEY='fav.plugin'
 * 
 * @author skube
 * 
 */
class g_one_plugin {
    
    private $one_lib;
    
    function __construct($fav_id) {
        $this->fav_id=$fav_id; 
    }
    
    function init($sqlo) {
       
        $this->one_lib = new g_modfavs_one($sqlo, $this->fav_id);
        $data=$this->one_lib->get_data();
        return $data;
    }
    function get_plugin_config($sqlo) {
        return $this->one_lib->get_plugin_config($sqlo);
    }
    /**
     * get ALL data
     * 
     * @return void|mixed
     */
    function get_fav_data() {
        return $this->one_lib->get_data();
    }
    function get_fav_config() {
        return $this->one_lib->get_fav_config();
    }
    

    /**
     * array
     *   'w' : widget name
     *   'wopt' : widget-specific options
     *
     * @param object $sqlo
     */
    function form2_wid_conf($parx, &$in_arr_def, $fav_data) {
        
        
        // echo $add_button;
        echo "<br>\n";
        
        if (empty($in_arr_def)) {
            cMsgbox::showBox("warning", "No Config-parameters defined for this plugin."); 
            echo "<br>";
        }
        
        
        
        $initarr   = array();
        $initarr["title"]       = "Configure Favorite plugin"; 
        $initarr["submittitle"] = "Save"; 
        $initarr["tabwidth"]    = "AUTO";   
        
        $hiddenarr = NULL;
        $hiddenarr["action"]  = 'edit';
        $hiddenarr["fav_id"]  = $this->fav_id;
        $hiddenarr["mod"]     = 'DEF/g.plugin.addfav';
 
        $formobj = new formc($initarr, $hiddenarr, 0);
        
        $fieldx = array ( // form-field definition
            "title"  => 'Name of favorite',
            "name"   => 'favpar[title]',
            "object" => "text",
            "val"    => $fav_data['title'],
            "notes"  => '',
            'req'    => TRUE,
            'namex'  => TRUE
        );
        $formobj->fieldOut( $fieldx ); // output the form-field
        
        if (!empty($in_arr_def)) {
            self::_fields_sub($formobj, $parx, $in_arr_def, NULL);
        }
        
        $formobj->close( TRUE ); 
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
    
    private function _check_input_sub(&$formobj, &$parx, &$in_arr_def, $basevar) {
        
       
        if (empty($in_arr_def)) return;
        
            
        foreach($in_arr_def as $row) {
            
            if (!empty($row['_dict_'])) {

                self::_fields_sub($formobj, $parx[$row['var']], $row['_dict_'], $row['var']);
                
            } else {
                

                if ($basevar) {
                    $val_in = $parx[$basevar][$row['var']];
                    
                } else {
                    $varname = $row['var'];
                    $val_in = $parx[$varname];
                }
                
                if ($row['req']) {
                    if($val_in===NULL or $val_in==='') {
                        $this->error_arr[]=$row['title'].': value missing!';
                        continue;
                    }
                }
                if (!empty($val_in)) {
                    //      'type' : 'INT', 'STRING', 'LIST_OF_INT', 'LIST_OF_STRING'
                    $type_def=$row['type'];
                    switch ($type_def) {
                        case 'INT':
                            if ( filter_var($val_in, FILTER_VALIDATE_INT)===FALSE )  {
                                $this->error_arr[]=$row['title'].': value "' .$val_in. '" is not INTEGER!';
                            }
                            break;
                        case 'LIST_OF_INT':
                            $val_arr=explode(',',$val_in);
                            foreach($val_arr as $val0) {
                                $val0=trim($val0);
                                if ( filter_var($val0, FILTER_VALIDATE_INT)===FALSE )  {
                                    $this->error_arr[]=$row['title'].': value "'.$val0.'" is not INTEGER!';
                                }
                            }
                            break;
                    }
                }

            }
            
        }
            
       
        
    }
    
    
    private function check_input($sqlo, &$parx, &$in_arr_def) {
        $this->error_arr=array();
        self::_check_input_sub($formobj, $parx, $in_arr_def, NULL);
    }
    
    function fav_conf_save($sqlo, $parx, $in_arr_def, $favpar) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        debugOut('(135) parx: '.print_r($parx,1), $FUNCNAME, 1);
        
        $favpar['title'] = trim($favpar['title']);
        if ( $favpar['title']==NULL) {
            $this->error_arr[]='Favorite TITLE is missing.';
        }
        
        $this->check_input($sqlo, $parx, $in_arr_def);
        if (!empty($this->error_arr)) {
            htmlInfoBox( "Input-Parameter Errors", "", "open", "ERROR" );
            echo implode("<br>",$this->error_arr);
            htmlInfoBox( "", "", "close" );
            echo "<br>\n";
            $this->form2_wid_conf($fav_conf, $in_arr_def);
            
            return;
            
        }
        $this->one_lib->update_title($favpar['title']);
        $this->one_lib->update_conf($sqlo, $parx);
        cMsgbox::showBox('ok', 'saved.');
    }
}


class g_plugin_addfav_XPL extends gPlugin {
	
	function register() {
		$title		  = 'Manage Plugin Favorites';
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'tool'; 
		$this->infoarr['locrow']   = array(  // defines the tool location-path
		    array('p.php?mod=DEF/g.widget_editor', 'My World Configuration'),
		); 
		
		$action  = $_REQUEST['action'];
		if ($action=='edit') {
		    $this->infoarr['title']    = 'Edit on Favorite';
		    $this->infoarr['locrow'][]   = 
		    array($this->_plug_get_URL(), $title);
		    
		}
	}
	
	function show_list($sqlo, &$fav_arr) {
	    $initarr   = NULL;
	    $initarr["action"]      = $_SERVER['PHP_SELF'];
	    $initarr["title"]       = "Favorite overwiew";
	    $initarr["submittitle"] = "Remove";
	    $initarr["tabwidth"]    = "AUTO";
	    
	    $hiddenarr = NULL;
	    $hiddenarr['mod'] = $this->_mod;
	    $hiddenarr['action'] = 'remove';
	    
	    $formobj = new formPure($initarr, $hiddenarr, 0);
	    $formobj->start();
	    foreach($fav_arr as $fav_id) {
	        
	        $tmp_lib  = new g_modfavs_one($sqlo, $fav_id);
	        $fav_info = $tmp_lib->get_data();
	        $mod_id_loop= $fav_info['mxid'];
	        $mod_info = g_modfavs::get_mod_info($sqlo, $mod_id_loop);
	        
	        echo '<input type="checkbox" name="favs['.$fav_id.']"> ';
	        echo ' <a href="'.$this->_plug_get_URL().'&favid='.$fav_id.'&action=edit" title="configuration">'.
	   	        '<img src="res/img/settings.svg"></a> ';
	        echo $fav_info['title'];
	        
	        echo "<br>";
	    }
	    echo '<input type=submit value="Remove" class="yButton">'."\n";
	    $formobj->close();
	}
	
	function form_remove_all() {
	    
	    $initarr   = array();
	    $initarr["title"]       = "Remove all my favorites?"; // title of form
	    $initarr["submittitle"] = "Remove all!"; // title of submit button
	    $initarr["tabwidth"]    = "AUTO";   // table-width: AUTO
	    
	    $hiddenarr = NULL; 
	    $hiddenarr["action"]     = 'remove_all'; 
	    $formobj = $this->_html->formInit($initarr, $hiddenarr, 0);
	    $formobj->close( TRUE ); // close form, sshow submit-button
	}
	

	function startMain() {
		global $error;
		$FUNCNAME='startMain';
		
		$objid   = $_REQUEST['id'];
		$go      = $_REQUEST['go'];
		$action  = $_REQUEST['action'];
		$favs_arr_in   = $_REQUEST['favs'];
		$parx    = $_REQUEST['parx'];
		$sqlo = &$this->_sqlo;
		
		echo '[<a href="'.$this->_plug_get_URL().'">Favorites HOME</a>] &nbsp;';
		echo '[<a href="'.$this->_plug_get_URL().'&action=remove_all">Remove all</a>]<br><br>';
		
		if ($action==NULL) $action='show';
		
		if ($action=='remove_all') {
		    if (!$go) {
		        $this->form_remove_all();
		        return;
		    }
	        g_modfavs::remove_all($sqlo);
	        echo "all your favorites were removed.<br>";
		    return;
		}
		
		$fav_arr = g_modfavs::get_my_favs($sqlo);
		
		switch ($action) {
		    case 'add':
        		$add_info = g_modfavs::add_to_favs($sqlo, intval($objid) );
        		if ($error->Got(READONLY))  {
        		    $error->set( $FUNCNAME, 1, 'Error on favorite adding.' );
        		    return;
        		}
        		$added = $add_info[1];
        		$new_fav_id = $add_info[0];
        		
        		if ($added==1) {
        		    echo "Plugin ".$objid." is saved in your favorites.<br>";
        		    $this->_html->pageForward($this->_plug_get_URL().'&action=edit&favid='.$new_fav_id, $link_text);
        		}
        		if ($added==2) {
        		    echo "Plugin ".$objid." is already in your favorites.<br>";
        		}
        		break;
        	    
		    case 'remove':
		        
		        if (!$go) {
		           
		        } else {
		            
		            if (!is_array($favs_arr_in)) {
		                echo 'No favorites selected.<br>';
		                return;
		            }
		            
		            $fav_arr_del = array();
		            foreach($favs_arr_in as $favid=>$flag) {
		                if($flag) $fav_arr_del[]=$favid;
		            }
		           
		            g_modfavs::remove_my_many($sqlo, $fav_arr_del);
		            
		            echo "Plugins: ".implode(', ',$fav_arr_del)." were removed from your favorites.<br>";
		        }
		        break;
		        
		     case 'edit':
		         
		        $fav_id = $_REQUEST['favid'];
		        if (!$fav_id) {
		            $this->htmlFoot('ERROR', 'Input favid missing.');
		        }
		      
	            //$in_arr=array();
	            $one_plug_lib = new g_one_plugin($fav_id);
	            $wid_infoarr = $one_plug_lib->init($sqlo);
	            echo 'FAV-ID: '.$fav_id.' Widget-title: <b>'.$wid_infoarr['title']."</b><br><br>\n";
	            $plugin_config = $one_plug_lib->get_plugin_config($sqlo);
	            $fav_data_old      = $one_plug_lib->get_fav_data();
	            
	            if ($go) {
	               
	                $one_plug_lib->fav_conf_save($sqlo, $parx, $plugin_config, $_REQUEST['favpar']);
	            } else {  
	                $fav_conf = $one_plug_lib->get_fav_config();           
	                $one_plug_lib->form2_wid_conf($fav_conf, $plugin_config, $fav_data_old);
	                return;
	            }
		        break;
		}
		
		$fav_arr = g_modfavs::get_my_favs($sqlo);
		if (empty($fav_arr)) {
		    echo "No favorites found.<br> ";
		} else {
		    if ($action!='edit') {
		        echo "<ul>\n";
		        $this->show_list($sqlo, $fav_arr);
		        echo "</ul>\n";
		    }
		}
	}
	
}