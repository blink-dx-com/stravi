<?php 
/**
 * analyse SUA follower- analyse : PROTOCOL: SAMPLE flag
 * @package o.ABSTRACT_SUBST.prot_follow.inc
 * @author Steffen
 * @param $id SUA-ID
 */

require_once 'o.ABSTRACT_SUBST.proto.inc';
require_once ("f.objview.inc");
require_once ("lev1/gGraphRelWork.inc");

class oSUA_one_follow_ana {
    
    function __construct() {
        $this->sua_proto_lib = new oABSTRACT_SUBST_proto();
    }
    
    function set_sua($sua_id) {
        $this->sua_id = $sua_id;
    }
    
    /**
     * get all SUAs where SAMPLE_FLAG = 1
     * @param object $sqlo
     * @return array of SUA_ID
     */
    function get_followers(&$sqlo) {
        $this->sua_proto_lib->set_obj($sqlo, $this->sua_id);
        $followers = $this->sua_proto_lib->obj_tracking_sua($sqlo);
        
        return $followers;
    }
}

class o_ABSTRACT_SUBST_prot_follow_XPL extends gPlugin {
	
	
	function register() {
	    
		$tablename	= 'ABSTRACT_SUBST';
		$title		= 'analyse material usage : forward explosion';
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = 'forward explosion';
		$this->infoarr['title']    = $title;
		$this->infoarr['title_sh'] = '';
		$this->infoarr['form_type']= 'obj'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_id']   = $_REQUEST['id'];
		$this->infoarr['checkid']  = 1;
		
		
		$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('read'), 'obj'=>array() )
				);

	}
	
	
	private function analyse_one_rec($sqlo, $one_sua_id, $level, $parent_id) {
	    
	    $this->_sua_cache[$one_sua_id]=array('lev'=>$level, 'par'=>$parent_id);
	    
	    $level = $level + 1;
	    
	    $this->ana_lib->set_sua($one_sua_id);
	    $followers =  $this->ana_lib->get_followers($sqlo);
	    if ( $followers['cnt'] ) {
	        
	        foreach($followers['suas'] as $loop_sua_id) {
	            $this->analyse_one_rec($sqlo, $loop_sua_id, $level, $one_sua_id);
	        }
	        
	    }
	}
	
	private function _ana_sua_cache() {
	    
	    $cache2 = array(); // sua_id=> array of CHILDREN
	    
	    foreach($this->_sua_cache as $loop_sua => $row) {
	        
	        $level      = $row['lev'];
	        $mother_sua = $row['par'];
	        if ($mother_sua) {
	            if ( !isset($cache2[$mother_sua]) )  $cache2[$mother_sua]=array();
	            $cache2[$mother_sua][]=$loop_sua;
	        }
	    }
	    
	    foreach($cache2 as $loop_sua => $children) {
	        
	        $level      = $row['lev'];
	        $mother_sua = $row['par'];
	        if ($mother_sua) {
	            if ( !isset($cache2[$mother_sua]) )  $cache2[$mother_sua]=array();
	            $cache2[$mother_sua][]=$loop_sua;
	        }
	    }
	}
	
	private function _ana_sua1() {
	    
	    // first cache analysis
	    $maxlevel=0;
	    $level_cache = array();  // number of entries per level
	    
	    /**
	     * SUA-ID => $sua_layer_id ( id of output table row )
	     */
	    $sua_cache2   = array();
	    $sua_layer_id = 0;
	    $last_mother_sua = 0;
	    
	    foreach($this->_sua_cache as $loop_sua => $row) {
	        
	        $level      = $row['lev'];
	        $mother_sua = $row['par'];
	        if ($mother_sua) {
	            
	            if ($last_mother_sua==$mother_sua) {
	                $sua_layer_id = $sua_layer_id + 1;
	            }
	            
	            
	            $sua_layer_id = $sua_cache2[$mother_sua];
	        } else {
	            // nothing ...
	        }
	        
	        $last_mother_sua = $mother_sua;
	        $this->_sua_cache[$loop_sua]['lay'] = $sua_layer_id;
	        
	        if (!isset($level_cache[$level]) ) $level_cache[$level]=0;
	        $level_cache[$level] = $level_cache[$level] + 1;
	        
	        $maxlevel = max($maxlevel, $level);
	    }
	    
	    $max_rows_per_level = max($level_cache);
	}
	
	private function _ana_sua3($sqlo) {
	    
	    $out_arr = array();
	    
	    foreach($this->_sua_cache as $loop_sua => $row) {
	        
	        $level      = $row['lev'];
	        $mother_sua = $row['par'];
	        if ($mother_sua) {
	            
	            $sua_mo_txt = obj_nice_name ( $sqlo, 'ABSTRACT_SUBST', $mother_sua ). ' ['.$mother_sua.']';
	            $sua_chi_txt = obj_nice_name ( $sqlo, 'ABSTRACT_SUBST', $loop_sua ). ' ['.$loop_sua.']';
	        
	            $out_arr[]=$sua_mo_txt."\t".$sua_chi_txt;
	            
	        } else {
	            // nothing ...
	        }
	        
	        
	    }
	    
	    $out_str=implode("\n",$out_arr);
	    
	    return $out_str;
	}

	/**
	 * {@inheritDoc}
	 * @see gPlugin::startMain()
	 */
	function startMain() {
		global $error;
		// $FUNCNAME='startMain';
		
		$go  = $_REQUEST['go'];
	
		
		$this->objid = $_REQUEST['id'];
		$this->_sua_cache = array();
		
		$sqlo  = &$this->_sqlo;
		//$sqlo2 = &$this->_plug_regSql2();
		
		echo 'The tool shows the sequence of all materials in all levels, where this material was used.<br><br>';
		
		$this->ana_lib = new oSUA_one_follow_ana();
		
		$this->analyse_one_rec($sqlo, $this->objid, 0, 0);
		
		if (sizeof($this->_sua_cache)<=1) {
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
		
// 		echo '<table>'."\n";
// 		$tabarr=array(array(), array());
// 		echo '<tr>';
// 		for ($i=0; $i<$maxlevel; $i++) {
// 		    echo '<td>Stage '.($i+1).'</td>';
// 		}
// 		echo '</tr>';
		
//  	foreach($this->_sua_cache as $loop_sua => $row) {
// 		    $html_tmp = $objLinkLib->bo_display( $sqlo, 'ABSTRACT_SUBST', $loop_sua );
// 		    echo $html_tmp.': '.print_r($row,1)."<br>";
//  	}
		
// 		echo '</table>'."\n";

		$datax = $this->_ana_sua3($sqlo);
		$parx=array();
		$graphLib = new gGraphRelWork('graph1', $datax, $parx);
		$graphLib->start($sqlo);
	}
	
}
