<?php
require_once ("o.proj.profile.inc"); 
require_once ('o.PROJ.subs.inc');
require_once ('o.PROJ.addelems.inc');
require_once 'gui/o.PROJ.explorer.inc';

/**
 * add OBJECT to favorites
 * @package g.obj.favact.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param $t: tablename
 * @param $id: ID of object
 * @param $act
 *   'add'
 *   'del' delete
 *   'remove' TBD:
 *   'show'
 *   'go_proj' : go to favorites project
 */
class g_obj_favact_XPL extends gPlugin {
	
	function register() {
		$title		  = 'Manage/Show your favorites';
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'tool'; 
	}
	

	function startMain() {
		global $error;
		//$FUNCNAME='startMain';
		
		$table = $_REQUEST['t'];
		$objid = $_REQUEST['id'];
		$act   = $_REQUEST['act'];
		$sqlo  = &$this->_sqlo;
		$sqlo2 = &$this->_plug_regSql2();
		
		$profileLib = new profile_funcs();
		$projid     = $profileLib->getProj($sqlo, 'bookmarks', 'CREATE');
		if (!$projid) {
		    $this->_html->line_out('Could not create a bookmarks folder!');
		    return; 
		}
		
		$dest_url = 'edit.tmpl.php?t='.$table.'&id='.$objid;
	    $go_forward = 1;
		
		switch ($act) {
		    case "add":
		        
		        if ( !cProjSubs::objectInProject($sqlo, $projid, $table, $objid) ) {
		            // add
		            $projModlib = new oProjAddElem($sqlo,$projid);
		            $projModlib->addObj($sqlo, $table, $objid);
		        }
		        
		        if (!$error->Got(READONLY))  {  
		          echo '... added.<br>';
		        }
		        break;
		        
		    case "del":
		        
		        if ( !cProjSubs::objectInProject($sqlo, $projid, $table, $objid) ) {
		            return; // no action
		        }
		        
		        $projModlib = new oProjAddElem($sqlo,$projid);
		        $projModlib->unlinkObj($sqlo, $table, $objid);
		        
		        if (!$error->Got(READONLY))  {
		            echo '... removed.<br>';
		        }
		        break;
		        
		    case "go_proj":
		        $dest_url = 'edit.tmpl.php?t=PROJ&id='.$projid;
		        break;
		    case "show":
		        
		        echo '&nbsp;<a href="'.$this->_plug_get_URL().'&act=go_proj"><img src="res/img/settings.svg" title="Favorite settings: add, remove" width=30></a><br><br>';
		        
		        $go_forward = 0;
		        $pmode='';
		        $projLib    = new oPROJ_navC($projid, $pmode);
		        $projTables = $projLib->get_proj_tables($sqlo);
		        
		        echo '<table style="width:100%"><!-- (71) -->'."\n";
		        
		        $singOpt=array();
		        $popt   =array("noCheck"=>1);
		        $target ='';
		        $projLib->show_sub_projs($sqlo, $sqlo2, $singOpt, $popt, $target);
		        if ($error->Got(READONLY))  {
		            return;
		        }
   
	            $iopt=NULL;
	            $iopt["maxob"]    = 200;
	            $iopt["noCheck"]  = 1;
	            $projLib->bos_infos( $sqlo, $sqlo2, $projTables, $iopt );
		            

                echo "</table>\n";
                
		        
		        break;
		    default:
		        $error->set( 'MAIN', 1, 'Action "'.$act.'" unknown' );
		        return;
		}
		
		if ($error->Got(READONLY))  {
		    return;
		}
		
		if ($go_forward) {
		  $this->_html->pageForward($dest_url, '... forward');
		}
	}
	
}