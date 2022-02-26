<?php
require_once('gui/go.treeview.inc');
require_once 'glob.obj.conabs.inc';
require_once 'f.rider.inc';
require_once 'f.sql_query.inc';

/**
 * tree of object
 * @package g.obj.tree.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   string $t tablename 
 * @param   int $id object id 
 * --- activity parameters ---
 * @param $tree_tab : table of object
 * @param $tree_oid : ID of object
 * @param $action : plus, minus
 * @param $show   : 
 *    ['one']
 *    'all' : show full tree
 *    'all_conc' : show full tree of concrete objects
 *    'all_auto' : show all or  all_conc by table type
 *    'x_list' : show all MACs as list
 *    'x_list_sel' : bring all MACs to list view
 */ 
class g_obj_tree_XPL extends gPlugin {
	
	
	function register() {
		
		$title		= 'tree view of object';
		
		$this->infoarr			 = NULL;
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'obj'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $_REQUEST['t'];
		$this->infoarr['obj_id']   = $_REQUEST['id'];
		$this->infoarr['checkid']  = 1;
		
		$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('read'), 'obj'=>array('read') )
			);
	}
	

	
	function startMain() {
	    
		global $error;
		$FUNCNAME='startMain';
		
		$sqlo = &$this->_sqlo;
		$sqlo2= &$this->_plug_regSql2();
		
		$tablename = $_REQUEST['t'];
		$id        = $_REQUEST['id'];
		
		$child_tab   = $_REQUEST['tree_tab'];
		$child_id    = $_REQUEST['tree_oid'];
		
		if ($child_tab==NULL) {
		    $child_tab   = $tablename;
		    $child_id    = $id;
		}
		
		
		$action   = $_REQUEST['action'];
		$show_key = $_REQUEST['show'];
		
		$concabs_lib = new gConcAbsC();
		$concabs_type = $concabs_lib->table_type($tablename);
		
		$extra_action='all';
		if ($show_key==NULL) {
		    $show_key = 'one'; 
		}
 		
		if ($show_key=='all_auto') {
		    if ( $concabs_type == 'conc') {
		        $show_key='all_conc';
		    } else {   
		        $show_key='all';
		    }
		}
		
		if ($show_key=='all') {
		    $extra_action='all';
		}
		if ($show_key=='all_conc') {
		    $extra_action='all_conc';
		}
		if ($show_key=='x_list') {
		    $extra_action = 'all_conc';
		}
		
		$xmodes=array(
		    'all'     => array('Show all', $this->_plug_get_URL().'&t='.$tablename.'&id='.$id.'&show=all')
		);
		
		if ($concabs_type=='conc') {
		    $xmodes['all_conc'] =  array('Show all concrete objects', $this->_plug_get_URL().'&t='.$tablename.'&id='.$id.'&show=all_conc');
		}
		
		if ($tablename=='CONCRETE_SUBST') {
		    $xmodes['x_list'] = array('All MACs as LIST', $this->_plug_get_URL().'&t='.$tablename.'&id='.$id.'&show=x_list');
		    $xmodes['x'] = array('Material usage: forward explosion', 'p.php?mod=DEF/o.CONCRETE_SUBST.prot_follow&id='.$id);
		}
		
		$rider_lib = new fRiderC();
		$rider_lib->riderShow($xmodes, $show_key, 'Tree-Mode: ');
		
		
		
		$this->_html->line_out('');
		
		$tree_URL = $this->_plug_get_URL().'&t='.$tablename.'&id='.$id;
		
		
		if ($extra_action=='all' or $extra_action=='all_conc') {

		    $no_session_var = 1;
		    $treeobj = new treeViewC( $no_session_var );
		    $treeobj->set_root($tablename, $id);
		    
		    $tree_all_lib = new treeView_allC($treeobj);
		    $tree_all_lib->do_all($sqlo, $sqlo2, $extra_action);

		} 
		
		if ($extra_action=='one') {
		  $no_session_var=0;
		  $treeobj = new treeViewC( $no_session_var );
		  $treeobj->set_root($tablename, $id);
		  $treeobj->do_action($sqlo, $sqlo2, $child_id, $child_tab, $action);
		}
		
		
		if ($show_key=='x_list') {
		    $level=1;
		    $sh_opt=array();
		    $sh_opt['show_types']='all_conc';
		    
		    echo '<a href="'.$this->_plug_get_URL().'&t='.$tablename.'&id='.$id.
		       '&show=x_list_sel"><img src="images/but.list2.gif"> '.
		      'Bring MACs to list view</a><br><br>';
		    
		    $tree_show_lib = new treeView_SHOW( $treeobj, $tree_URL );
		    $tree_show_lib->show_start($sh_opt);
		    $tree_show_lib->show_list( $sqlo, $id, $tablename, $level);
		    return;
		}
		
		if ($show_key=='x_list_sel') {
		    $level=1;
		    $sh_opt=array();
		    $sh_opt['show_types']='all_conc';

		    $tree_show_lib = new treeView_SHOW( $treeobj, $tree_URL );
		    $suc_ids = $tree_show_lib->get_select( $sqlo, 'CONCRETE_SUBST');
		    
		    if(empty($suc_ids)) {
		        $this->htmlFoot('WARNING','MAC contains to components.');
		    }
		    
		    $utilLib = new fSqlQueryC('CONCRETE_SUBST');
		    $utilLib->cleanCond();
		    $sql_Where = 'x.CONCRETE_SUBST_ID in ('.implode(',',$suc_ids).')';
		    
		    $utilLib->addCond( $sql_Where, '', 'BOM-MACs' );
		    $sqlAfter = $utilLib->get_sql_after( );
		    $utilLib->queryRelase();

		    $this->_html->pageForward('view.tmpl.php?t=CONCRETE_SUBST', 'go to object list view');
		    return;
		}

		$sh_opt=array('with_tab_name'=>1);
		debugOut( "Show-action: $extra_action", $FUNCNAME, 1);
		if ($extra_action=='all_conc') {
		    $sh_opt['show_types']=$extra_action;
		}
		$level=1;
		$tree_show_lib = new treeView_SHOW( $treeobj, $tree_URL );
		$tree_show_lib->show_start($sh_opt);
		$tree_show_lib->show( $sqlo, $id, $tablename, $level);
		
	}
}
