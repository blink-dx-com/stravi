<?php
require_once ("lev1/glob.obj.assocadd.inc");	
//require_once 'f.sql_query.inc';
require_once ("f.objview.inc");
require_once ("f.assocUpdate.inc");
require_once 'f.clipboard.inc';

/**
 * Add children-SUAs (from clipboard) to a META-substance
 *    
 * @package o.ABSTRACT_SUBST.metaAdd.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param $_REQUEST['id'] id of ABSTRACT_SUBST 
 * @param $action
 *    add
 *    delete
 * @param $ch[OBJ-ID] = 1 needed for delete
 */

class o_ABSTRACT_SUBST_metaAdd_XPL extends gPlugin {

	function __construct(&$sqlo) {
		parent::__construct($sqlo);
	}
	
	function register() {
	    
	    $action = $_REQUEST['action'];
		$tablename	= 'ABSTRACT_SUBST';
		$title		= 'Add children to META substance';
		
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title']    = $title;
		$this->infoarr['title_sh'] = 'Add children';
		$this->infoarr['form_type']= 'obj'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_id']   = $_REQUEST['id'];
		$this->infoarr['checkid']  = 1;
		
		if ($action=='delete') {
		    $this->infoarr['title']	   = 'Remove children from META substance';
		    $this->infoarr['title_sh'] = 'Remove children';
		}
		
		$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('read','write'), 'obj'=>array('write') )
				);	
	}
	
	function form_add() {
	    
	    $initarr   = array();
	    $initarr["title"]       = "Add following children to META";
	    $initarr["submittitle"] = "Add"; // title of submit button
	    $initarr["tabwidth"]    = "AUTO";   // table-width: AUTO
	    
	    $hiddenarr = NULL; 
	    $hiddenarr["id"]     = $this->objid; 
	    $hiddenarr["action"] = $this->action;  
	    
	   
	    $formobj = $this->_html->formInit($initarr, $hiddenarr, 0);

	    $formobj->close( TRUE );
	    $this->_html->line_out('');	  
	    
	}
	
	function form_delete($children) {
	    
	    $initarr   = array();
	    $initarr["title"]       = "Remove following children from META";
	    $initarr["submittitle"] = "Remove"; 
	    $initarr["tabwidth"]    = "AUTO"; 
	    
	    $hiddenarr = NULL;
	    $hiddenarr["id"]     = $this->objid;
	    $hiddenarr["action"] = $this->action;
	    
	    foreach($children as $child_id=>$val) {
	        if (!$val) continue;
	        $hiddenarr['ch['.$child_id.']'] = 1; 
	    }
	    
	    
	    $formobj = $this->_html->formInit($initarr, $hiddenarr, 0);
	    
	    $formobj->close( TRUE );
	    $this->_html->line_out('');
	    
	}
	
	function act_delete($sqlo, $sqlo2, $children, $go) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    $assoclib = new  fAssocUpdate();
	    $assoclib->setObj( $sqlo, 'ABS_CHILD_SUBST', $this->objid );
	    if ($error->Got(READONLY))  {
	        $error->set( $FUNCNAME, 1, 'Error on Init.' );
	        return;
	    }
	    
	    if (!$go) {
	        $this->form_delete($children);
	    }
	    
	   
	    $objLinkLib = new fObjViewC();
	   echo "<br>";
	    
	    foreach($children as $child_id=>$val) {
	        if (!$val) continue;
	        $idarr = array('CHILD_SUBST_ID'=>$child_id);
	        
	        $html_tmp = $objLinkLib->bo_display( $sqlo, 'ABSTRACT_SUBST', $child_id );
	        echo "- ".$html_tmp."<br>";
	        if ($go>0) {
	           $assoclib->delOneRow($sqlo, $idarr);
	        }
	    }
	    echo '<br>';
	    
 
	}
	
	function act_add($sqlo, $sqlo2, $go) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    $MAXATTACH=1000;
	    $mtable ='ABSTRACT_SUBST';
	    $asoctab='ABS_CHILD_SUBST';
	    $asoccol='POS';
	    $ch_tablename='ABSTRACT_SUBST';
	    
	    $clip_lib = new clipboardC();
	    $new_obj_ids = $clip_lib->getAllObjects($ch_tablename);
	    if (empty($new_obj_ids)) {
	        $this->_html->messageBox2('warning', 'Please copy '.tablename_nice2($ch_tablename).'s to the clipboard');
	        return;
	    }
	    $probe_cnt=sizeof($new_obj_ids);
	   
// OLD:
// 	    $utilLib = new fSqlQueryC($mtable);
// 	    if ($utilLib->get_sql_info() =='') {
// 	        $this->htmlFoot('ERROR','Please select some children.');
// 	    }
// 	    $ord_opt = array("order"=>1);
// 	    $sqlAfterSort = $utilLib->get_sql_after( $ord_opt);
// 	    $sqlAfterNoOrder = $utilLib->get_sql_after();
// 	    $sqls = "count(1) from ".$sqlAfterNoOrder;
// 	    $sqlo->Quesel($sqls);
// 	    $sqlo->ReadRow();
// 	    $probe_cnt = $sqlo->RowData[0];
	    
	    
	    if ( $probe_cnt>$MAXATTACH ) {
	        $this->_html->messageBox2('warning', 'Max '.$MAXATTACH.' elements allowed. Please ask your admin.');
	        return;
	    }
	    
	    $mainLib = new gObj_assocAdd($sqlo, $mtable, $this->objid, $asoctab, $asoccol, $ch_tablename);
	    if ($error->Got(READONLY))  {
	        $error->set( $FUNCNAME, 6, 'Error on Init.' );
	        return;
	    }
	    
	    if (!$go) {
	        $this->form_add();
	    }
	    
	    
	    foreach($new_obj_ids as $chi_sua_id) {
	        $mainLib->add_one($sqlo, $chi_sua_id, $go);
	    } 
	    if ($error->Got(READONLY))  {
	        $error->set( $FUNCNAME, 10, 'Error on adding.' );
	        return;
	    }
	}

	function startMain() {
		global $error;
		$FUNCNAME='startMain';
		
		$tablename	= 'ABSTRACT_SUBST';
		$table_nice = tablename_nice2($tablename);

		
		$this->objid   = $_REQUEST['id'];
		$go = $_REQUEST['go'];
		$this->action = $_REQUEST['action'];
		
		$sqlo = &$this->_sqlo;
		$sqlo2  = &$this->_plug_regSql2();
		
		$metaInfo = glob_elementDataGet( $sqlo, 'ABSTRACT_SUBST', 'ABSTRACT_SUBST_ID', $this->objid, 'IS_META');
		if ($metaInfo<=0) {
		    $error->set( $FUNCNAME, 1, 'the '.$table_nice.' must be flagged as "META"-'.$table_nice.' to add/delete children!' );
			return;
		}
		
		if ($this->action=='add') {
		    $this->act_add($sqlo, $sqlo2, $go);
		}
		if ($this->action=='delete') {
		    $children = $_REQUEST['ch'];
		    if (!is_array($children)) {
		        $this->htmlFoot('ERROR', 'Not input given.');
		    }
		    $this->act_delete($sqlo, $sqlo2, $children, $go);
		}
		
		if ($error->Got(READONLY))  {
		    return;
		}
		
		if ($go) {
		  $this->_html->line_out('OK');	  
		  $this->_html->pageForward('edit.tmpl.php?t=ABSTRACT_SUBST&id='.$this->objid , 'Back to Mother.');
		}
		
		
	}
	
}
