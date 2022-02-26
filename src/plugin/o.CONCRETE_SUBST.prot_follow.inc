<?php 
/**
 * analyse SUC follower- analyse : PROTOCOL
 * - detect Cyclic references of objects ...
 * @package o.CONCRETE_SUBST.prot_follow.inc
 * @author Steffen
 * @param $id SUC-ID
 */

require_once 'o.CONCRETE_SUBST.proto.inc';
require_once ("f.objview.inc");
require_once ("lev1/gGraphRelWork.inc");
require_once ('f.msgboxes.inc');

class oSUC_one_follow_ana {
    
    function __construct() {
        $this->suc_proto_lib = new oCONCRETE_SUBST_proto();
    }
    
    function set_suc($suc_id) {
        $this->suc_id = $suc_id;
    }
    
    /**
     * get all SUCs where SAMPLE_FLAG = 1
     * @param object $sqlo
     * @return array of SUC_ID
     */
    function get_followers(&$sqlo) {
        $MAX_STORE=20;
        $this->suc_proto_lib->set_obj($this->suc_id);
        $followers = $this->suc_proto_lib->obj_tracking_suc($sqlo, $MAX_STORE);
        
        return $followers;
    }
}

class o_CONCRETE_SUBST_prot_follow_XPL extends gPlugin {
	
    private $_suc_cache;      // $one_suc_id => array('lev'=>$level, 'par'=>$parent_id);
    private $_suc_more_cache; // $one_suc_id => array( 'cnt'=>$followers['cnt'] );  // more SUCs exist ...
	
	function register() {
	    
		$tablename	= 'CONCRETE_SUBST';
		$title		= 'Usage: forward explosion';
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = 'forward explosion';
		$this->infoarr['title']    = 'analyse material in material usage : forward explosion';
		$this->infoarr['title_sh'] = $title;
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
	        $this->_warning('MAC '.$one_suc_id.' already analysed.');
	        return;
	    }
	    $this->_suc_cache[$one_suc_id]=array('lev'=>$level, 'par'=>$parent_id);
	    
	    $level = $level + 1;
	    
	    $this->ana_lib->set_suc($one_suc_id);
	    $followers =  $this->ana_lib->get_followers($sqlo);
	    if ( $followers['cnt'] ) {
	        debugOut('followers: '.print_r($followers,1), $FUNCNAME, 1);
	        foreach($followers['sucs'] as $loop_suc_id) {
	            $this->analyse_one_rec($sqlo, $loop_suc_id, $level, $one_suc_id);
	        }
	        
	        if ($followers['cnt'] > sizeof($followers['sucs']) ) {
	            $this->_suc_more_cache[$one_suc_id]=array( 'cnt'=>$followers['cnt'] );  // more SUCs exist ...
	        }
	        
	    }
	}
	
	private function _suc_info($sqlo, $suc_id) {
	    
	    $suc_mo_txt  = obj_nice_name ( $sqlo, 'CONCRETE_SUBST', $suc_id ). ' ['.$suc_id.']';
	    $sua = glob_elementDataGet( $sqlo, 'CONCRETE_SUBST', 'CONCRETE_SUBST_ID', $suc_id,  'ABSTRACT_SUBST_ID');
	    $sua_nice  =   	obj_nice_name ( $sqlo, 'ABSTRACT_SUBST', $sua );
	    
	    $out_name=$suc_mo_txt."\n(".$sua_nice.')';
	    
	    $this->obj_gr_features[$out_name] = array('href'=>'edit.tmpl.php?t=CONCRETE_SUBST&id='.$suc_id); 
	    
	    return $out_name;
	}
	

	private function _ana_suc3($sqlo) {
	    global $error;
	    
	    $out_arr = array();
	    
	    
	    
	    
	    foreach($this->_suc_cache as $loop_suc => $row) {
	        
	        //$level      = $row['lev'];
	        $mother_suc = $row['par'];
	        if ($mother_suc) {
	            
	            $suc_mo_txt  = $this->_suc_info($sqlo, $mother_suc);
	            $suc_chi_txt = $this->_suc_info($sqlo, $loop_suc);
	        
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
	        $suc_mo_txt  = obj_nice_name ( $sqlo, 'CONCRETE_SUBST', $mother_suc ). ' ['.$mother_suc.']';
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
	
		
		$this->objid = $_REQUEST['id'];
		
		$this->_suc_cache = array();
		$this->_suc_more_cache = array();
		$this->warnarr = array();
		$this->obj_gr_features = array();
		
		$sqlo  = &$this->_sqlo;
		//$sqlo2 = &$this->_plug_regSql2();
		
		echo 'The tool shows the sequence of all material batches in all levels, where this material was used by other batches.<br><br>';
		
		$this->ana_lib = new oSUC_one_follow_ana();
		
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
    		    $html_tmp = $objLinkLib->bo_display( $sqlo, 'CONCRETE_SUBST', $loop_suc );
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
		$parx['out_type'] = 'svg';
		$graphLib = new gGraphRelWork('graph1', $datax, $parx);
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
