<?php 
/**
 * analyse MAA - META objects
 * - detect Cyclic references of objects ...
 * @package o.ABSTRACT_SUBST.meta_tree.inc
 * @author Steffen
 * @param $id SUC-ID
 */

//require_once 'o.CONCRETE_SUBST.proto.inc';
require_once 'o.ABSTRACT_SUBST.subs.inc';
require_once ("f.objview.inc");
require_once ("lev1/gGraphRelWork.inc");
require_once ('f.msgboxes.inc');



class o_ABSTRACT_SUBST_meta_tree_XPL extends gPlugin {
	
    private $_suc_cache;      // $one_suc_id => array('lev'=>$level, 'par'=>$parent_id);
    private $_suc_more_cache; // $one_suc_id => array( 'cnt'=>$children['cnt'] );  // more SUCs exist ...
	
	function register() {
	    
		$tablename	= 'ABSTRACT_SUBST';
		$title		= 'META material - tree analysis';
		
		$this->infoarr			 = NULL;
		//$this->infoarr['scriptID'] = 'forward explosion';
		//$this->infoarr['title']    = 'analyse material in material usage : forward explosion';
		$this->infoarr['title'] = $title;
		$this->infoarr['form_type']= 'obj'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_id']   = $_REQUEST['id'];
		$this->infoarr['checkid']  = 1;
		
		
		$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('read'), 'obj'=>array() )
				);

	}
	
	private function _warning($text) {
	    $this->warnarr[]=$text;
	}
	
	
	private function analyse_one_rec($sqlo, $one_suc_id, $level, $parent_id) {
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    if ( $_SESSION["userGlob"]["g.debugLevel"]>1 ) {
	        if (sizeof($this->_suc_cache)> 100) {
	            debugOut('STOP searching du to DEBUG ...', $FUNCNAME, 1);
	            return;
	        }
	    }
	    
	    if (array_key_exists($one_suc_id, $this->_suc_cache) ){
	        $this->_warning('MAA '.$one_suc_id.' already analysed.');
	        return;
	    }
	    $this->_suc_cache[$one_suc_id]=array('lev'=>$level, 'par'=>$parent_id);
	    
	    $level = $level + 1;
	    
	    //TBD: ....
	    $children =  oABSTRACT_SUBST_subs::get_META_children($sqlo, $one_suc_id);
	    if ( !empty($children) ) {
	        debugOut('followers: '.print_r($children,1), $FUNCNAME, 1);
	        foreach($children as $loop_sua_id) {
	            $this->analyse_one_rec($sqlo, $loop_sua_id, $level, $one_suc_id);
	        }

	    }
	}
	
	private function _sua_info($sqlo, $sua_id) {

	    $sua_nice  =   	obj_nice_name ( $sqlo, 'ABSTRACT_SUBST', $sua_id );
	    $out_name=$sua_nice."\n[".$sua_id.']';
	    $this->obj_gr_features[$out_name] = array('href'=>'edit.tmpl.php?t=ABSTRACT_SUBST&id='.$sua_id); 
	    return $out_name;
	}
	

	private function _ana_suc3($sqlo) {
	    global $error;
	    
	    $out_arr = array();

	    foreach($this->_suc_cache as $loop_suc => $row) {
	        
	        //$level      = $row['lev'];
	        $mother_suc = $row['par'];
	        if ($mother_suc) {
	            
	            $suc_mo_txt  = $this->_sua_info($sqlo, $mother_suc);
	            $suc_chi_txt = $this->_sua_info($sqlo, $loop_suc);
	        
	            $out_arr[]=array( $suc_mo_txt, $suc_chi_txt);
	            
	        } else {
	            // nothing ...
	        }
	        
	        if ($error->Got(READONLY))  {
	            return;
	        }
	        
	    }
	    
	    // create the BOXES for "more ..."
	    foreach( $this->_suc_more_cache as $mother_suc => $row ) {
	        $suc_mo_txt  = obj_nice_name ( $sqlo, 'ABSTRACT_SUBST', $mother_suc ). ' ['.$mother_suc.']';
	        $out_arr[]= array($suc_mo_txt, "...(Total:".$row['cnt'].")" );
	    }
	    
	    //$out_str=implode("\n",$out_arr);
	    
	    return $out_arr;
	}
	
	

	/**
	 * {@inheritDoc}
	 * @see gPlugin::startMain()
	 */
	function startMain() {
		global $error;
		$FUNCNAME='startMain';
		
		// $go  = $_REQUEST['go'];
	
		$tablename	= 'ABSTRACT_SUBST';
		$this->objid = $_REQUEST['id'];
		
		$this->_suc_cache = array();
		$this->_suc_more_cache = array();
		$this->warnarr = array();
		$this->obj_gr_features = array();
		
		$sqlo  = &$this->_sqlo;
		//$sqlo2 = &$this->_plug_regSql2();
		
		echo 'The tool shows all child material templates and sub-METAs<br><br>';
		
		
		
		$this->analyse_one_rec($sqlo, $this->objid, 0, 0);
		
		if (sizeof($this->_suc_cache)<=1) {
		    echo 'No Followers found';
		}


		
		/**
		 * st1    st2   st3     st4
		 * s1  ->  s2 -> s4  -> s8
		 *                      s9
		 *               s5
		 *         s3 -> s6
		 *               s7
		 *
		 */
		
// 		
// 		$tabarr=array(array(), array());
// 		echo '<tr>';
// 		for ($i=0; $i<$maxlevel; $i++) {
// 		    echo '<td>Stage '.($i+1).'</td>';
// 		}
// 		echo '</tr>';

		if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
		    echo "DEBUG: _suc_cache:<br>";
		    $objLinkLib = new fObjViewC();
		    
		    //echo '<table>'."\n";
         	foreach($this->_suc_cache as $loop_suc => $row) {
    		    $html_tmp = $objLinkLib->bo_display( $sqlo, $tablename, $loop_suc );
    		    echo $html_tmp.': '.print_r($row,1)."<br>";
         	}
		
		    //echo '</table>'."\n";
		  
         	
		}
		
		if (!empty($this->warnarr)) {
		    cMsgbox::showBox("warning", "Cyclic references of objects found!"); 
		    foreach($this->warnarr as $text) {
		        echo '&nbsp; - '.$text."<br>";
		    }
		    echo "<br>";
		}
		
		$datax = $this->_ana_suc3($sqlo);
		
		$parx=array();
		$parx['obj_features'] = $this->obj_gr_features;
		$parx['out_type']  = 'svg';
		$parx['graphType'] = 'neato';
		$graphLib = new gGraphRelWork('graph_META', $datax, $parx);
		$graphLib->start_calc();
		$graphLib->graph_create($sqlo);
		$filename = $graphLib->get_graph_filename_full();
		
		if (!file_exists($filename))  {
		    $error->set( $FUNCNAME, 1, 'Graph-file not found. ('.$filename.')' );
		    return;
		}
		
		echo file_get_contents($filename);
	}
	
}
