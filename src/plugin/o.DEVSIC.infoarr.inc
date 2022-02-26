<?php
require_once ("glob.obj.update.inc");
require_once 'o.DEVSIC.subs.inc';
require_once ("gui/o.DEVSIC.gui.inc");

/**
 * actions for DEVSIC:INFOARR
 * @package o.DEVSIC.infoarr.inc
 * @swreq UREQ:0001458: g > Gerätewartung (Concept)
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param $id
 * @param $row : [index] : array()
 *   't', 'type', 'n'
 */
class o_DEVSIC_infoarr_XPL extends gPlugin {

	
	function register() {
		$tablename	= 'DEVSIC';
		$title		= 'update service info fields';
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'obj'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_id']   = $_REQUEST['id'];
		$this->infoarr['checkid']  = 1;
		
		
		$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('read','write'), 'obj'=>array('write') )
				);
				
	}
	
	function initLib($data_in) {
	    $this->infoarr = NULL;
	    $this->data_in = $data_in;
	    $this->paramProblem=NULL;
	    $this->strore_vars = array();
	}
	
	function _convert_row($i , $row_loop) {
	    
	    $var_loop = trim($row_loop['t']);
	    $tmpNotes = trim($row_loop['n']);
	    $title    = trim($row_loop['x']);
	    $typeNow  = $row_loop['type'];
	    
	    if ($var_loop==NULL and $tmpNotes==NULL) return;
	    
	    if ($var_loop==NULL and $tmpNotes!=NULL)  {
	        // $this->paramProblem[]='Titel fehlt! Wurde kuenstlich vergeben';
	        $var_loop = 'var' . rand ( 0 , 99999 );
	    }
	    
	    $ok = preg_match("/^[A-Za-z0-9_ \-\(\)]*$/", $var_loop);
	    
	    if ( !$ok ) {
	        // repair ':'
	        $problem='Variable "'.$var_loop.'" can contain only Letters, Numbers, underscore "_", minus, brackets "()" or spaces!';
	        $this->paramProblem[]=$problem;
	        $var_loop = str_replace(':','_',$var_loop);
	    }
	    if ( strlen($var_loop) > oDEVSIC_subs::INFOARR_var_MAX_SIZE ) {
	        $problem ='Variable "'.$title.'" too long. (Max: '.oDEVSIC_subs::INFOARR_var_MAX_SIZE.')';
	        $this->paramProblem[]=$problem;
	    }
	    
	    
	    if (in_array($var_loop, $this->strore_vars)) {
	        $this->paramProblem[]='Variable "'.$var_loop.'" taucht im Plan schonmal auf!';
	        $var_loop = substr($var_loop,0,10) . rand ( 0 , 99999 );
	    }
	    
	    $this->strore_vars[] = $var_loop;
	    
	   
	    if ($typeNow==NULL) $typeNow='checkbox';
	    if ($title==NULL) {
	        $title = $var_loop; // copy varname to title
	    }
	    if ( strlen($title) > oDEVSIC_subs::INFOARR_title_MAX_SIZE ) {
	        $problem ='Title "'.$title.'" too long. (Max: '.oDEVSIC_subs::INFOARR_title_MAX_SIZE.')';
	        $this->paramProblem[]=$problem;
	    }
	    if ( strlen($tmpNotes) > oDEVSIC_subs::INFOARR_notes_MAX_SIZE ) {
	        $problem ='Notes "'.$tmpNotes.'" too long. (Max: '.oDEVSIC_subs::INFOARR_notes_MAX_SIZE.')';
	        $this->paramProblem[]=$problem;
	    }
	    
	    
	    $oneRowData = array('t'=>$var_loop, 'x'=>$title, 'type'=>$typeNow, 'n'=>$tmpNotes );
	    
	    $this->infoarr[$i] = $oneRowData;
	}
	


	function startMain() {
		global $error;
		$FUNCNAME='startMain';
		
		$this->objid   = $_REQUEST['id'];
		$go   = $_REQUEST['go'];
		$sqlo = &$this->_sqlo;
		
		$data_in = $_REQUEST['row'];
		if (empty($data_in)) {
		    $data_in=array();
		}
		
		if ( !$go ) {
			$this->htmlFoot('ERROR', 'Missing go parameter');
		}
		
		$this->initLib($data_in);
		
		if ( empty($data_in) ) {
		    
			$this->infoarr=NULL;
			$devsic_mo_lib = new oDEVSIC_mod($this->objid);
			$devsic_mo_lib->update_INFOARR($sqlo, $this->infoarr);
			
			$this->htmlFoot();
			
		} else {
		
			
		    foreach( $data_in as $i=>$row_loop) {
		        $this->_convert_row($i,$row_loop);
			}
			   
			if ( $error->Got(READONLY) or !empty($this->paramProblem)) {
			    
			    $this->_html->messageBox2('error', 'Parameter-Problems');
			    echo "<br>";
			    glob_printr( $this->paramProblem, "Parameter-Probleme:" );
			    echo "<br><br>\n";
			    
			    $helplib = new oDEVSIC_INFOARR_gui($this->objid);			    
			    $helplib->editfrom($data_in);
		    
			    return;
			}
			
			$devsic_mo_lib = new oDEVSIC_mod($sqlo, $this->objid);
			$devsic_mo_lib->update_INFOARR($sqlo, $this->infoarr);
			
			
		}
		
		if ( !$error->Got(READONLY) and empty($this->paramProblem) )  {
			$url='edit.tmpl.php?t=DEVSIC&id='.$this->objid;
			$this->_html->pageForward($url,'back to object');
			return;
		}
		
		
		
		
		
	}
	
}