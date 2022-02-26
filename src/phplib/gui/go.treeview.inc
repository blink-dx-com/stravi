<?php
require_once 'glob.obj.conabs.inc';

/**
 * manage COMPONENT tree analysis/view for an OBJECT

 * 
 * GLOBALS:
      $_SESSION['s_sessVars']["boThis"]
      $_SESSION['s_tree'] = tree2_struct
	  $_SESSION["userGlob"]["g.debugLevel"] > 0
	    - show table.nicename after each object
   
 * @package go.treeview.inc
 * @swreq SREQ:0000036 tree-view
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @example <pre>
 * $no_session_var = 1;
 * $treeobj = new treeViewC( $this->_plug_get_URL().'&t='.$tablename.'&id='.$id, $no_session_var );
   $treeobj->set_root($tablename, $id);
   
   
   $level=1;
   $treeobj->treeshow_start($sh_opt);
   $treeobj->treeshow( $sqlo, $id, $tablename, $level);
   </pre>
   @var  array tree2_struct  --- stores the full component tree
      array of
          't'    => table, 
          'id'   => ID of object, 
          'lea'  => leave, 
          'state'=> state
          'lev'  => level,
          'tou'  => 0,1 -- touched - just an info
          'elems'=> array( - sub elements
              array('t'=>table, 'id'=> ID)
           )
      LEGEND:   
  		1. 'lea' -- leave (first char):   
  		   "l" leave, 
  		   "x": end point !!!
		2. 'state' -- state (second char): 
		    "s": from now hide 
		    "h": hidden, not yet analysed
		    "e": show
		3. 'lev' -- level INT: 0,1,2, ..
 */
class treeViewC {
    
    private $_pointer;
    // public  $rootTab; // no meaning this time
    //OLD: public  $tree_struct;
    
    public  $tree2_struct;
   
    private $show_options=array();
    private $IGNORE_TABLES=array(
        'USER_GROUP'
    );

    /**
     * 
     * @param string $treeurl
     * @param int $no_session_var
     */
    function __construct($no_session_var=0) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;

        if ($no_session_var) {    
            $this->tree2_struct= array();
        } else {
            
            $this->tree2_struct= &$_SESSION['s_tree'];
        }
        
        //$this->_UID=1;	
    	// $this->hist_obj = new historyc();
    	
    	
    	if ( $_SESSION["userGlob"]["g.debugLevel"]>1 ) {
    	    echo "DEBUG:$FUNCNAME: RAW data at start ... <br>";
    	    
    	    $this->show_tree_raw();
    	}
    	
    }
    
    public function get_root_obj() {
        if (!is_array($this->tree2_struct)) return array();
        
        reset($this->tree2_struct);
        $first_obj = current($this->tree2_struct);
        return $first_obj;
    }
    
    
    
    /**
     * add  structure to $tmp_tree
     * @param array $tmp_tree -- OUTPUT
     * @param string $table
     * @param int $id
     * @param string $leave
     * @param int $state
     * @param int $level
     * @return array
     */
    private function _get_struct(&$tmp_tree, $table, $id, $leave, $state, $level) {
        if (in_array($table, $this->IGNORE_TABLES)) {
            debugOut("(139) ignore TABLE:".$table, $FUNCNAME, 2);
            return;
        }
        $tmp_tree[] = array ( 't'=>$table , 'id' => $id , 'lea'=> $leave, 'state'=>$state, 'lev'=>$level );
    }
    
    /**
     * init the whole tree
     * @param object $sql
     * @param object $sql2
     * @param int $id
     * @param string $tab_name
     */
    function _init_tree($sql, $sql2, $id, $tab_name) {
        
        if ( empty($this->tree2_struct) and $tab_name!='' and $id ) {
            $level = 1;
            $this->set_element(array('t'=>$tab_name, 'id'=>$id, 'lea'=>"l", 'state'=>'s', 'lev'=>$level), 'init' );
            $this->hiddenelems($sql, $sql2, $tab_name, $id, $level, 'init' );
            $this->tree_open=1;
        }
    }
    
    
    function set_root($tablename, $id) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $this->root_table = $tablename;
        $this->root_id = $id;
        
        $s_tree_tmp = $this->tree2_struct;
        if (!is_array($s_tree_tmp)) return;
        
        reset($s_tree_tmp);
        $row_dict = current( $s_tree_tmp );
        
        if ($row_dict['t']!=$tablename or $row_dict['id']!=$id ) {
            debugOut("set new TREE ($tablename, $id)!".print_r($row_dict,1) .' ALL:'.print_r($s_tree_tmp,1), $FUNCNAME, 3);
            $this->tree2_struct = NULL;
        }
 
    }
    
    public function show_tree_raw() {
        
        echo "Raw_data<br>";
        //print_r($this->tree_struct);
        //echo "<br><br>";
        $th_tmp=array();
        $index=0;
        
        if (sizeof($this->tree2_struct)) {
        
            foreach ($this->tree2_struct as $index => $row ) {
                
                $dict1  = $row;
                $tab    = $row['t'];
                $loopid = $row['id'];
                
                $spacer = str_repeat ("+", $dict1['lev']);
                
                echo $index. ' '.$spacer." |$tab: ". $loopid.': '.$row['lea'].$row['state'].$row['lev'].' ori:'.$row['ori'].' uid:'.$row['uid'].
                  ''."<br>";
                if (!empty($row['elems'])) {
                    foreach($row['elems'] as $subrow) {
                        echo " - ".$subrow['t'].':'.$subrow['id']."<br>";
                    }
                }
            }
            
        }
        echo "---<br>";
    }
    
   
   
    
    
    
    public static function _calc_index($row) {
        $index = $row['t'].':'.$row['id'];
        return $index;
    }
    
    /**
     * set element in stack
     * @param int $index
     * @param array $row
     * @return int $is_new
     */
    private function set_element($row, $set_info='') {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $index  = self::_calc_index($row);
        $oldrow = $this->tree2_struct[$index];
        $is_new = 1;
        
        
        debugOut( "(289): ind:$index data:".print_r($row,1). ' info:'.$set_info, 'set_element', 3);
        
        if (is_array($oldrow)) {
            // echo "DDDCVG: ok $index ". print_r($this->tree_struct[$index],1)."<br>";         
            $is_new = 0;
        } else {
            $this->tree2_struct[$index] = $row;
        }
 
        
        
        return $is_new;
    }
    
    /**
     * 
     * @param int $index
     * @param string $key
     * @param variant $val
     */
    private function _set_one_param($row, $key, $val) {

        $index = $this->_calc_index($row);
        $this->tree2_struct[$index][$key] = $val;      
        // echo "DDDX30:_set_one_param index:$index key:$key val:".$this->tree2_struct[$index][$key]."<br>";
       
    }
    
    
    /**
     * insert a stack of elements
     * @param int $index
     * @param array $elements
     */
    private function set_elements_insert( $table, $id, $elements ) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        debugOut("(325)  $table:$id NUM_ELEMS:".sizeof($elements), $FUNCNAME, 2);
        $index = $this->_calc_index(array('t'=>$table, 'id'=>$id));
        $this->tree2_struct[$index]['elems'] = $elements; // TBD: ...
        
    }
    
    private function exists($row) {
        $index = $this->_calc_index($row);
        $answer = 0;
        if (is_array($this->tree2_struct[$index])) $answer = 1;
        return $answer;
    }
    
    /**
     * add children
     * @param object $sql
     * @param object $sql2
     * @param array $s_tree
     * @param string $tab_key
     * @param int $id
     * @param int $level - level of SUB-elements
     * @param string $hinfo - info to element
     * OLD: @param int $new_index : start of MOTHER
     * @return number $slice_num
     */
    private function hiddenelems(&$sql, &$sql2,  $tab_key, $id, $level, $hinfo='' ) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $numslices    = 0;

    	# echo  "PRESUME: $tab_key($id)=".$s_tree[$new_index][$tab_key][$id]."<br>";
    	
    	// set mother
    	$row_tmp    = array('t'=>$tab_key, 'id'=>$id);
    	$exists     = $this->exists( $row_tmp );
    	
    	debugOut("(353) set: $tab_key:$id  exists:$exists info:$hinfo", $FUNCNAME, 2);
    	
    	
    	$tmp_tree   = array();
    	$cct_ref_tab=array();
    		
    	$key_num = countPrimaryKeys($tab_key);
    	
    	
    	if ( ($id>0) && ($key_num<2) ) {
            
    	    $col_names    = "";
    	    $spacer="";
    	    
    		// analyse CHILD-links of BO
    		if ($tab_key!="PROJ") { // do not analyse PROJ, due to SAVING TIME (SQL-queries)
    			$sqls="select COLUMN_NAME, CCT_TABLE_NAME from CCT_COLUMN  where TABLE_NAME='".$tab_key."' AND CCT_TABLE_NAME>'0' and VISIBLE>0";
    			$sql->query("$sqls");
    			$cnt=0;
    			while ($sql->ReadRow() ) {
    				
    				if ( ($sql->RowData[0]!="CCT_ACCESS_ID") ) {
    					$ref_tab = $sql->RowData[1];
    					// check, if this is a BO
    					$sqls="select COLUMN_NAME from CCT_COLUMN  where TABLE_NAME='".$ref_tab."' AND COLUMN_NAME='CCT_ACCESS_ID'";
    					$sql2->query("$sqls");
    					if ( $sql2->ReadRow() ) { /* only business objects */
    						$col_names = $col_names . $spacer . $sql->RowData[0];
    						$cct_ref_tab[$cnt]=$ref_tab;
    						$cnt++;
    						$spacer =",";
    					}
    				}
    			}
    		}
    		
    		if ( $col_names!="" ) {
    		    
    		    // get CHILD-links of BO
    			$sqls="select ".$col_names." from $tab_key where ".$tab_key."_ID=".$id;
    			$sql->query($sqls);
    			$sql->ReadRow();
    			for ($i=0; $i< sizeof($cct_ref_tab); $i++ ) {
    				$valer = $sql->RowData[$i];
    				if ($valer=="") $valer="0";
    				else {
    					// NEW: only add leave, if object exists
    				    $this->_get_struct( $tmp_tree, $cct_ref_tab[$i], $valer, "l", "h", $level);
    					$numslices++;
    				}
    			}
    			# echo "slice_START:".($new_index+1). " new:".$numslices."<br>";
    		}
    		
    		switch ($tab_key) {
    			case "ABSTRACT_PROTO":
    				$sqls="ABSTRACT_SUBST_ID from ABSTRACT_PROTO_STEP where ".
    					" ABSTRACT_PROTO_ID=".$id." AND ABSTRACT_SUBST_ID>0";
    				$sql->Quesel($sqls);
    				while ( $sql->ReadRow()) {
    				    $this->_get_struct( $tmp_tree, "ABSTRACT_SUBST", $sql->RowData[0], "l", "h", $level); 
    					$numslices++;
    				}
    				break;
    			case "ABSTRACT_SUBST":
    				$sqls="ABSTRACT_PROTO_ID from AS_HAS_PR where ABSTRACT_SUBST_ID=".$id. " order by XORDER";
    				$sql->Quesel($sqls);
    				while ( $sql->ReadRow()) {
    				    $this->_get_struct( $tmp_tree, "ABSTRACT_PROTO", $sql->RowData[0], "l", "h", $level);
    					$numslices++;
    				}
    				
    				$sqls="COMPO_SUA_ID from SUA_BOM where ABSTRACT_SUBST_ID=".$id. " order by POS";
    				$sql->Quesel($sqls);
    				while ( $sql->ReadRow()) {
    				    $this->_get_struct( $tmp_tree, "ABSTRACT_SUBST", $sql->RowData[0], "l", "h", $level); 
    				    $numslices++;
    				}
    				
    				break;

    			case "CONCRETE_PROTO":
    				$sqls="CONCRETE_SUBST_ID from CONCRETE_PROTO_STEP where ".
    					" CONCRETE_PROTO_ID=".$id. " AND CONCRETE_SUBST_ID>0";
    				$sql->Quesel($sqls);
    				while ( $sql->ReadRow()) {
    				    $this->_get_struct( $tmp_tree, "CONCRETE_SUBST", $sql->RowData[0], "l", "h", $level);
    					$numslices++;
    				}
    				
    				$sqls="DEV_ID from CONCRETE_PROTO_STEP where ".
        				" CONCRETE_PROTO_ID=".$id. " AND DEV_ID>0";
    				$sql->Quesel($sqls);
    				while ( $sql->ReadRow()) {
    				    $this->_get_struct( $tmp_tree, "CHIP_READER", $sql->RowData[0], "l", "h", $level);
    				    $numslices++;
    				}
    				break;
    				
    			case "CONCRETE_SUBST":
    			    $sqls="CONCRETE_PROTO_ID from CS_HAS_PR where CONCRETE_SUBST_ID=".$id. " order by STEP_NO";
    			    $sql->Quesel($sqls);
    			    while ( $sql->ReadRow()) {
    			        $this->_get_struct( $tmp_tree, "CONCRETE_PROTO", $sql->RowData[0], "l", "h", $level);
    			        $numslices++;
    			    }
    			    
    			    $sqls="COMPO_SUC_ID from SUC_BOM where CONCRETE_SUBST_ID=".$id. " order by POS";
    			    $sql->Quesel($sqls);
    			    while ( $sql->ReadRow()) {
    			        $this->_get_struct( $tmp_tree, "CONCRETE_SUBST", $sql->RowData[0], "l", "h", $level);
    			        $numslices++;
    			    }
    			    
    			    break;
    			
    			case "EXP":

    				$sqls="select CONCRETE_PROTO_ID from EXP_HAS_PROTO where EXP_ID=".$id;
    				$sql->query("$sqls");
    				while ( $sql->ReadRow() ) {
    				    $subobj_id = $sql->RowData[0];
    				    $this->_get_struct( $tmp_tree, "CONCRETE_PROTO", $subobj_id, "l", "h", $level); 
    				    $numslices++;
    				}

    				
    				break;
    			
    				
    			case "PROJ":
    				
    				$sqls="select PROJ_ID from PROJ where PRO_PROJ_ID=".$id. " order by NAME";
    				$sql->query("$sqls");
    				while ( $sql->ReadRow()) {
    				    $this->_get_struct( $tmp_tree, "PROJ", $sql->RowData[0], "l", "h", $level);
    					$numslices++;
    				}
    				break;
    				
    			case "SOCKET":
    			    $sqls="CONCRETE_SUBST_ID from CART_H_SUC where SOCKET_ID=".$id. " order by POS";
    			    $sql->Quesel($sqls);
    			    while ( $sql->ReadRow()) {
    			        $this->_get_struct( $tmp_tree, "CONCRETE_SUBST", $sql->RowData[0], "l", "h", $level);
    			        $numslices++;
    			    }
    			    
    			    break;
    		}
    		
    		$this->_set_one_param( $row_tmp, 'state', 's');
    		
    		if ($numslices) {
    		    $this->_set_one_param( $row_tmp, 'lea', 'l'); // set leave
    		 
    		    // debugOut( "DDDX3: add SLICE", 3);

    		    $this->set_elements_insert(  $tab_key, $id, $tmp_tree );
    		    
    		} else {
    		    $this->_set_one_param( $row_tmp, 'lea', 'x'); // end point
    		}
    			
    	}
    	
    	return $numslices;
    }
    
    /**
     * TBD: ...
     * @param int $uid
     * @return number|array|NULL
     */
    private function _get_ind_of_uid($uid) {
        $found_index = -1;
        $tmp_pointer = &$this->tree2_struct;
        foreach( $tmp_pointer as $loop_ind => $loop_row )  {
            
            if ( $loop_row['uid']==$uid ) {
                $found_index=$loop_ind;
                break;
            }
        }
        return $found_index;
    }
    
    
    /**
     * - do PLUS or MINUS
     * - can be called multiple times
     * 
     *  --- INIT
     * 0 : ls1 suc1
     * 1 : lh2  proto1  
     * 2 : lh2  proto2  
     * 
     * --- next => suc1 plus
     * le1  suc1
     *  ls2  proto1
     *   lh3 sua2
     *   lh3 sua4
     *  ls2  proto2 
     *   lh3 sua5
     *   lh3 sua6 
     * PLUS:
     *   - set_element(object) status: e
     *   - hiddenelems()       status: s
     * @param object $sql
     * @param object $sql2
     * @param int $id
     * @param string $tab_name
     * @param string $action
     * @param string $info_str
     */
    function do_action(&$sql, &$sql2, $id, $tab_name, $action, $info_str='') {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        
        debugOut( "(579) START: $id, $tab_name, $action info:$info_str", 'do_action', 2);
        
        $this->tree_open=0;
        $this->_init_tree($sql, $sql2, $id, $tab_name);

        
        $cnt    =0;
        $s_tree_tmp = $this->tree2_struct;
        // $tree_len   = sizeof($this->tree2_struct);
       
        
        $index    = $this->_calc_index(array('t'=>$tab_name, 'id'=>$id));
        $mo_row_dict = $s_tree_tmp[$index];
        
        if (empty($mo_row_dict)) {
            echo "Object not exists: $tab_name:$id <br>";
            return;
        }
        
        $elems = $mo_row_dict['elems'];
        
        if ( $action == "plus" ) {
            $this->_set_one_param( $mo_row_dict, 'state', 'e', 'PLUS' );
        }
        if ( $action == "minus" ) {
            $this->_set_one_param( $mo_row_dict, 'state', 's', 'PLUS' );
        }
        
        if (!empty($elems)) debugOut( "(607) elems: has_elems:".sizeof($elems), 'do_action', 2);
        if (!$elems) {
            return;
        }
       
        // analyse sub-elements
        foreach( $elems as $row_dict ) {

   
            $leave   = $row_dict['lea'];
            $state   = $row_dict['state'];
            $tmp_tab = $row_dict['t'];
            $tmp_id  = $row_dict['id'];
            $level   = $row_dict['lev'];
            
            if (in_array($tmp_tab, $this->IGNORE_TABLES)) {
                debugOut("(627) ignore TABLE ".$row_dict['t'].":".$row_dict['id'], $FUNCNAME, 2);
                continue;
            }

            // echo "DDDX2:do_action:START dict:".print_r($row_dict,1)." action: $action  <br>\n";
            
            if ( $action == "plus" ) {
  
                $is_new = $this->set_element( array('t'=>$tmp_tab, 'id'=>$tmp_id, 'lea'=>$leave, 'state'=>'s'), 'act_plus' );
   
                if ( $is_new ) { // the next elements are the hidden leaves
                    
                    // scan the following sub elements 
                    $more_slices = $this->hiddenelems( $sql, $sql2, $tmp_tab, $tmp_id,  $level, 'next_level' );
                    if (!$more_slices) {
                        $this->_set_one_param( $row_dict, 'lea', 'x', 'after_hiddenelems' );    
                    }

                } 
                // $this->_set_one_param( $row_dict, 'state', 'e', 'act_plus_FLAG_only' );
                
            }
            
            if ( $action == "minus" ) { 
                $this->_set_one_param( $row_dict, 'state', 's', 'act_minus' );          
            }
            
            $cnt=$cnt+1;
         
        }
        
        debugOut( "DDDX:END:<br>", 'do_action', 2);
        if ( $_SESSION["userGlob"]["g.debugLevel"]>2 ) {
            echo "DDX:END OF do_action:<br>";
            $this->show_tree_raw();
        }
    }
    
   
    
    function start_loop() {
        $this->_pointer = $this->tree2_struct;
    }
    
    function read_row() {
        
        $result = current($this->_pointer);
        next($this->_pointer);
        if (!is_array($result)) {
            return FALSE;
        }
        $row    = $result;
        
        return $row;
    }
    
    
    


}

/**
 * support AUTO scan of an object -- get the FULL component tree
 * @author skube
 *
 */
class treeView_allC {
    function __construct(&$treeobj) {
        $this->treeobj = $treeobj;
        $this->concabs_lib = new gConcAbsC();
    }
    
    /**
     * TBD: recode
     * @param object $sql
     * @param object $sql2
     */
    private function do_all_oneloop(&$sql, &$sql2) {
        
        
        $cnt    =0;
        $s_tree_tmp = $this->treeobj->tree2_struct;
        $tree_len   = sizeof($this->treeobj->tree2_struct);
        $tree_old_cnt = 0;
        $act_now_ALL = 0;
        
        while ( $tree_old_cnt < $tree_len ) { # last element not in each
            
            $index    = key( $s_tree_tmp );
            $row_dict = current( $s_tree_tmp );
            next($s_tree_tmp);
            
            $leave   = $row_dict['lea'];
            $state   = $row_dict['state'];
            $level   = $row_dict['lev'];
            $tmp_tab = $row_dict['t'];
            $tmp_id  = $row_dict['id'];
            $elems   = $row_dict['elems'];
            
            
            $act_now=0;
            
            if ($state=='s') {
                $act_now=1;
            }
            
            $do_scan = 1; // scna this element ?
            
            $concabs_type = $this->concabs_lib->table_type($tmp_tab);
            if ($this->subaction=='all_conc') {
                if ($concabs_type=='abs') {
                    $do_scan = 0;
                    $act_now = 0;
                    $this->treeobj->tree2_struct[$index]['tou'] = 1; // touched
                }
            }
            
            // echo "<b>DDDX6:</b> index:$index do_scan:$do_scan act_now:$act_now row:".print_r($row_dict,1)." <br>";
            
            if (!empty($elems) and $do_scan) {
                foreach($elems as $loop_row) {
                    $tmp_index = $this->treeobj->_calc_index($loop_row);
                    if (!is_array($this->treeobj->tree2_struct[$tmp_index])) {
                        $act_now=1;
                    }
                }
            }
            
            if ($act_now) {
                $this->treeobj->do_action($sql, $sql2, $row_dict['id'], $row_dict['t'], 'plus', 'from_do_all');
                $act_now_ALL=1;
            }
            
            $cnt=$cnt+1;
            $tree_old_cnt++;
        }
        
        
        
        if ( $_SESSION["userGlob"]["g.debugLevel"]>1 ) {
            echo "DDX:END OF do_all_oneloop:<br>";
            $this->treeobj->show_tree_raw();
        }
        
        return $act_now_ALL;
    }
    
    /**
     * get all leaves of an object
     * @param object $sql
     * @param object $sql2
     * @param string $subaction
     *  'all_conc' - only concrete objects ...
     */
    function  do_all(&$sql, &$sql2, $subaction='' ) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $this->subaction = $subaction;
        $doit    = 1;
        $MAX_LOOP=20;
        
        $loopcnt = 0;
        
        $this->treeobj->_init_tree($sql, $sql2, $this->treeobj->root_id , $this->treeobj->root_table);
        
        while ($doit) {
            
            if ($loopcnt>$MAX_LOOP) {
                echo "Warning: MAX number (".$MAX_LOOP.") of loops reached ...<br>";
                break;
            }
            
            debugOut("Loop_cnt: $loopcnt", $FUNCNAME, 2);
            
            $did_action = $this->do_all_oneloop($sql, $sql2);
            
            $doit = 0;
            if ($did_action) {
                $doit = 1;
            }
            
            
//             if (!empty($this->treeobj->tree_struct)) {
//                 debugOut("Loop_END: size(struct):".sizeof($this->treeobj->tree_struct), $FUNCNAME, 2);
//             }
            
            
            $loopcnt++;
            
        }
        
    }
    
}

/**
 * the visualization class
 */
class treeView_SHOW {
    
    private $tree2_struct;
    private $intcnt;  // for GUI
    private $_tree_showinit = 0;
    private $infox;
    private $out_type; // ['show'], 'cache'
    
    function __construct(treeViewC &$treeobj, $treeurl) {
        $this->tree2_struct = &$treeobj->tree2_struct;
        
        $this->treeurl = $treeurl;
        $this->_tree_showinit = 0;
        $this->intcnt = 0;
        $this->infox    = array(); // info array [tablename] = array ( "icon"=> ...)
    }
    
    //
    // GUI functions
    //
    
    private function print_js() {
        
        $treeurl = $this->treeurl;
        
        $url_delimiter='?';
        if ( strstr( $treeurl, '?' )!=NULL ) $url_delimiter = '&'; // add ampersand
        $treeurl .= $url_delimiter;
        
        
        ?>
    	<script language="JavaScript">
      	<!--
            
    	function plusbo(tablename, id) {
    		location.href="<?echo $treeurl?>action=plus&tree_tab="+tablename+"&tree_oid="+id ;
    	}
    	function minusbo(tablename, id) {
    		location.href="<?echo $treeurl?>action=minus&tree_tab="+tablename+"&tree_oid="+id ;
    	}
    
      	//-->
    	</script> 
    	<?php
    	
    }
    
    private function tableInit($table) {
        $icon="images/icon.".$table.".gif";
        if ( !file_exists($icon) ) $icon="images/icon.UNKNOWN.gif";
        $alt_txt = tablename_nice2($table);
        $this->infox[$table] = array("icon"=> $icon, "nicename"=>$alt_txt);
    }
    
    /**
     *
     * @param object $sql
     * @param string $tablename
     * @param int $id
     * @param number $with_abstract
     * @return string[]|number[]|string
     */
    private static function obj_getname(&$sql, $tablename, $id, $with_abstract=0 ) {
        
        $idname = PrimNameGet2($tablename);
        $key_num = countPrimaryKeys($tablename);
        if ($key_num > 1) {
            $nice_name = tablename_nice2($tablename);
            
            $sql->query('SELECT COUNT(1) FROM '.$tablename.' WHERE '.$idname.' = '.$sql->addQuotes($id));
            $sql->ReadRow();
            $text = $nice_name.': '.$sql->RowData[0].' elements';
            
        } else {
            
            $main_name = importantNameGet2($tablename);
            if ( $main_name == '') {
                $main_name = $idname;
            }
            if ($tablename=="CONCRETE_PROTO") {
                $sqls="select a.name from CONCRETE_PROTO c, ABSTRACT_PROTO a ".
                    " where c.CONCRETE_PROTO_ID=".$id. " AND c.ABSTRACT_PROTO_ID=a.ABSTRACT_PROTO_ID";
                $sql->query($sqls);
                $sql->ReadRow();
                $text = isset($sql->RowData[0]) ? $id.":".$sql->RowData[0] : '';
            } else {
                $sql_add_col='';
                if ($with_abstract) {
                    if ($tablename=='CONCRETE_SUBST') {
                        $sql_add_col=', ABSTRACT_SUBST_ID';
                    }
                }
                
                $sql->query('SELECT '.$main_name. $sql_add_col.' FROM '.$tablename.' WHERE '.$idname.' = '.$sql->addQuotes($id));
                $sql->ReadRow();
                $text = isset($sql->RowData[0]) ? $sql->RowData[0] : '';
                $abs_name='';
                if ($with_abstract) {
                    $abs_id = $sql->RowData[1];
                    if ($tablename=='CONCRETE_SUBST') {
                        $abs_name=obj_nice_name ( $sql, 'ABSTRACT_SUBST', $abs_id );
                    }
                }
            }
            
        }
        return array($text, $key_num, $abs_name);
    }
    
 
    /**
     * old: show_start
     * @param array $options
     *  'with_tab_name' : 0,1
     *  'show_types' : string hide some object types ???
     *       ['ALL'], 
     *      'all_conc' : show only concrete ones, hide some ABSTRACT objects ...
     *   'out_type' :
     */
    function show_start( $options=array() ) {
        
        $this->obj_touched=array();
        $this->_hide_abstracts=0;
        $this->_tree_showinit = 1;
        $this->intcnt = 0;
        $this->show_options = $options;
        if ( $this->show_options['show_types']=='all_conc') {
            $this->_hide_abstracts=1;
        }
        if ( $options['out_type']) {
            $this->out_type = $options['out_type'];
        }
        $this->print_js();
    }
    
    /**
     * old: treeshow
     * show an object tree
     * will be called RECURSIVE !
     * @param object $sql
     * @param int $id  : show this object
     * @param string $tab_name : show this object
     * @param int $level
    
     */
    function show( &$sql, $id, $tab_name, $level=1 ) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        if (!$this->_tree_showinit) {
            throw new Exception('treeshow method must be initialized!');
        }

    	if ( empty($this->tree2_struct) ) {
    		return;
    	}
    	
//     	if ( $_SESSION["userGlob"]["g.debugLevel"]>=2 ) {
//     	    debugOut('tree2_struct:', $FUNCNAME);
//     	    echo "<pre>".print_r($this->tree2_struct,1).'</pre>';
//     	}

    	
    	//$this->rootTab = NULL;
    	$histNowTab = $_SESSION['s_sessVars']["boThis"]["t"];
    	$histNowID  = $_SESSION['s_sessVars']["boThis"]["id"];
    	$with_tablenice = $this->show_options['with_tab_name'];


        $from_now_hide = 0;
        $index = treeViewC::_calc_index(array('t'=>$tab_name, 'id'=>$id));
        $row_dict = $this->tree2_struct[$index];
        $row_tmp  = $row_dict;
        unset($row_tmp['elem']);
        
        //debugOut( "DDDX1: ind:$index level:$level", 'treeshow', 2);
        
 
        if ($this->intcnt>200) { // TBD: ...
            echo "INFO: TOO many objects. <br>";
            return;
        }
        $this->intcnt++;

		$leave = $row_dict['lea'];
		$state = $row_dict['state'];
		$tmp_tab = $row_dict['t'];
		$tmp_id  = $row_dict['id'];
		$elems   = $row_dict['elems'];
	
    	if ( !is_array($this->infox[$tmp_tab]) ) $this->tableInit($tmp_tab);
    	
    	if ($this->_hide_abstracts) {
    	    if ($tmp_tab=='ABSTRACT_SUBST' or $tmp_tab=='ABSTRACT_PROTO') {
    	        return; // hide ...
    	    }
    	}
	
    		
    	$output_level = $level; // used for the output ...
    		
    	$from_now_hide = 0;
		if ( $state=="s" ) $from_now_hide=1;

		$imgbutt ="plus";
		$javafuun="plusbo";
		if ($state=="e") {
			$imgbutt ="minus";
			$javafuun="minusbo";
		}
			
		echo "<img src=\"images/but.blank.gif\" width=".(($output_level-1)*17)." height=1>";
			

		if ($tmp_id) {
			
				$icon	  = $this->infox[$tmp_tab]["icon"];
				$alt_txt  = $this->infox[$tmp_tab]["nicename"];
				$tmpstyle = "";
				if ( ($histNowTab==$tmp_tab) AND ($histNowID==$tmp_id) ) {   // is current object ?
					 $tmpstyle = " class=thmark name=\"theobject\""; // give also a jump-mark
				}
				
				list ($obj_text,  $key_num, $abstract_name) = self::obj_getname($sql, $tmp_tab, $tmp_id, $this->_hide_abstracts);
			    if ($obj_text=="") $obj_text ="[$tmp_id]";
			    
			    
				if ( $leave=="l" ) {
					echo "<a href=\"javascript:".$javafuun."('".$tmp_tab."', '".$tmp_id."')\">";
					echo "<img src=\"images/but.".$imgbutt.".gif\" border=0></a> ";
				} else {
					echo "<img src=\"images/but.blank.gif\" width=10 height=1> ";
				}
				if ( $key_num < 2) 
                    echo "<a href=\"edit.tmpl.php?t=".$tmp_tab."&id=".$tmp_id."\" target=\"unten\" ".$tmpstyle.">";
                    // echo "<a href=\"javascript:gobo('".$tmp_tab."', '".$tmp_id."')\"> ";
				else echo "<a href=\"javascript:golist('".$tmp_tab."', '".$tmp_id."')\"> ";
				
				echo "<img src=\"".$icon."\" border=0 TITLE=\"$alt_txt\"> ".$obj_text."</a>";
				if ($abstract_name!='') {
				    echo " (".$abstract_name.") ";
				}
				if ($with_tablenice) {
				    echo ' &nbsp;&nbsp;&nbsp; <span class="yGgray">('.$alt_txt.')</span>';
				}
		} else {
			  echo "<img src=\"images/but.blank.gif\" width=10 height=1> ";
			  echo '<img src="'.$icon.'" TITLE="'.$alt_txt.'"> <span style="color:#BFBFBF">[not defined]</span>';
		}
		
		echo "<br>\n";
		
		if ($from_now_hide) return;
		
		if (!empty($elems)) {
		    foreach($elems as $obj_ids) {
		        // debugOut( "DDD2: data: ".$obj_ids['t'].':'.$obj_ids['id'], 'treeshow', 2);
		        $this->show( $sql, $obj_ids['id'], $obj_ids['t'], $level+1 );
		    }
		        
		}

    	
    }
    
    /**
     * 
     * show object as LIST
     * will be called RECURSIVE !
     * use: $this->obj_touched
     * @param object $sql
     * @param int $id  : show this object
     * @param string $tab_name : show this object
     * @param int $level
     
     */
    function show_list( &$sql, $id, $tab_name, $level=1 ) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        if (!$this->_tree_showinit) {
            throw new Exception('treeshow method must be initialized!');
        }
        
        if ( empty($this->tree2_struct) ) {
            return;
        }
        $this->_hide_abstracts = 1;
        
        //     	if ( $_SESSION["userGlob"]["g.debugLevel"]>=2 ) {
        //     	    debugOut('tree2_struct:', $FUNCNAME);
        //     	    echo "<pre>".print_r($this->tree2_struct,1).'</pre>';
        //     	}
        
        
        //$this->rootTab = NULL;
        $histNowTab = $_SESSION['s_sessVars']["boThis"]["t"];
        $histNowID  = $_SESSION['s_sessVars']["boThis"]["id"];
        $with_tablenice = $this->show_options['with_tab_name'];
        
        
        $from_now_hide = 0;
        $index = treeViewC::_calc_index(array('t'=>$tab_name, 'id'=>$id));
        
        if ($this->obj_touched[$index]) {
            $this->obj_touched[$index]++;
            return;
        }
        $this->obj_touched[$index] = 1; // touch now
        
        $row_dict = $this->tree2_struct[$index];
        $row_tmp  = $row_dict;
        unset($row_tmp['elem']);
        
        //debugOut( "DDDX1: ind:$index level:$level", 'treeshow', 2);
        
        
        if ($this->intcnt>200) { // TBD: ...
            echo "INFO: TOO many objects. <br>";
            return;
        }
        
        
        $leave = $row_dict['lea'];
        $state = $row_dict['state'];
        $tmp_tab = $row_dict['t'];
        $tmp_id  = $row_dict['id'];
        $elems   = $row_dict['elems'];
        
        if ( !is_array($this->infox[$tmp_tab]) ) $this->tableInit($tmp_tab);

        $good_tables=array('CONCRETE_SUBST', 'CONCRETE_PROTO');
        if (!in_array($tmp_tab, $good_tables) ) {
            return; // do not show
        }

        if ($tmp_id and  $tmp_tab=='CONCRETE_SUBST') {
            
            
            
            $icon	  = $this->infox[$tmp_tab]["icon"];
            $alt_txt  = $this->infox[$tmp_tab]["nicename"];
            $tmpstyle = "";
            if ( ($histNowTab==$tmp_tab) AND ($histNowID==$tmp_id) ) {   // is current object ?
                $tmpstyle = " class=thmark name=\"theobject\""; // give also a jump-mark
            }
            
            list ($obj_text,  $key_num, $abstract_name) = self::obj_getname($sql, $tmp_tab, $tmp_id, $this->_hide_abstracts);
            if ($obj_text=="") $obj_text ="[$tmp_id]";

            if ( $key_num < 2) {
                echo ($this->intcnt+1).'. ';
                echo "<a href=\"edit.tmpl.php?t=".$tmp_tab."&id=".$tmp_id."\" target=\"unten\" ".$tmpstyle.">";
            }
                
            echo "<img src=\"".$icon."\" border=0 TITLE=\"$alt_txt\"> ".$obj_text."</a>";
            if ($abstract_name!='') {
                echo " (".$abstract_name.") ";
            }
            if ($with_tablenice) {
                echo ' &nbsp;&nbsp;&nbsp; <span class="yGgray">('.$alt_txt.')</span>';
            }
            echo "<br>\n";
            
            $this->intcnt++;
        }
        
       
        
        if ($from_now_hide) return;
        
        if (!empty($elems)) {
            foreach($elems as $obj_ids) {
                // debugOut( "DDD2: data: ".$obj_ids['t'].':'.$obj_ids['id'], 'treeshow', 2);
                $this->show_list( $sql, $obj_ids['id'], $obj_ids['t'], $level+1 );
            }
            
        }
  
    }
    
    function get_select($sqlo, $tablename) {
        
        $cnt=0;
        $cache=array();
        foreach( $this->tree2_struct as $row_dict ) {
            $tmp_tab = $row_dict['t'];
            $tmp_id  = $row_dict['id'];
            if ($tmp_tab==$tablename) {
                $cache[] = $tmp_id;
                $cnt++;
            }
        }
        
        return $cache;
        
    }
        
        

}
