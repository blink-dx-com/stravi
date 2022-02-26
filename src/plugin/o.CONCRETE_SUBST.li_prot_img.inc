<?php
/**
 * show graphics of LIST of SUCs : protocol steps
 * @package o.CONCRETE_SUBST.li_prot_img.inc
 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param $parx  -- diagram visualization definition
 *   pq_one_config_STRUCT
 *
 *     "abs_obj_id" => e.g. SUA-ID
 "objects":  array()
     1 => array(
         table:  CONCRETE_SUBST
         cols => array() # add these columns to the object NAME
         )
     2 => array(
         table:  CONCRETE_PROTO
         src: array(
              't'  => 'ABSTRACT_PROTO'
               "abs_id"
         ),
         "steps": => array(
            array( 
              'st'=>STEPNR, 
              'v'=>"qnsd",  --- 'v' is OPTIONAL
              'box'=>array('min_st'=>, 'max_st'=>, 'CV_st'=>) --- box-plot min-max OR  CV_st values STEP_NR
            ) 
         )
 )
 
 * $objsel - object selection
 *   'num' : max number of objects
     'cond' : condition array
        'col', 'val'
     'sort' : e.g. x:CONCRETE_SUBST DESC
 */

require_once('f.sql_query.inc');
require_once 'gui/f.html.diagram.inc';
require_once ("f.objview.inc");

require_once 'o.CONCRETE_SUBST.proto.inc';
require_once 'o.PROTO.steps.inc';

$proto_inc = dirname(__FILE__).'/../www/pionir/impexp/protoquant/';
require_once $proto_inc . 'o.PROTO.quant_help.inc';
require_once $proto_inc . 'o.PROTO.quant.img2.inc';

class o_CONCRETE_SUBST_li_prot_HELP {
    /**
     * - get a_proto_params
     - order steps by MAIN_STEP_NR

     
     * @param object $sql
     * @param int $aprotoid
     * @return array(
     *   "NAME" => $name, 
     *   "features"=> array(STEP_NR => STEP_ROW)
     *   )
     */
    static function get_aproto_infos( &$sql, $aprotoid) {
        // glob_printr($this->aProtoInfo, "thisAprotoInfo");
        
        $sqls = "select NAME from ABSTRACT_PROTO where ABSTRACT_PROTO_ID=".$aprotoid;
        $sql->query($sqls);
        $sql->ReadRow();
        $name = $sql->RowData[0];
        $steparr = NULL;
        
        
        $steparr = oProtoQuant_help::_getAbsStepsNorm($sql, $aprotoid);
        $new_steparr=array();
        
        if ( sizeof($steparr)) {
            
            foreach( $steparr as $num=>$row) {
                
               
                
                if ($row["H_UNIT_ID"]) {
                    $sqlsel = "NAME from H_UNIT where H_UNIT_ID=".$row["H_UNIT_ID"];
                    $sql->Quesel($sqlsel);
                    $sql->ReadRow();
                    $h_name = $sql->RowData[0];
                    $row["H_UNIT"] = $h_name;
                }
                if ($row["ABSTRACT_SUBST_ID"]) {
                    $sqlsel = "NAME from ABSTRACT_SUBST where ABSTRACT_SUBST_ID=".$row["ABSTRACT_SUBST_ID"];
                    $sql->Quesel($sqlsel);
                    $sql->ReadRow();
                    $su_name = $sql->RowData[0];
                    $row["ABSTRACT_SUBST"] = $su_name;
                }
                
                $new_steparr[$row["STEP_NR"]] = $row;
            }
            
        }
        
        
        return array("NAME"=>$name, "features"=> $new_steparr);
    }
}

class o_CONCRETE_SUBST_prot_HELP {
    
    function __construct($parx, $pra_arr) {
        $this->_pra_arr = $pra_arr;
        $this->parx = $parx;
        $this->proto_lib = new oCONCRETE_SUBST_proto();
    }
    
    function set_suc_id($suc_id) {
        $this->suc_id = $suc_id;
        $this->proto_lib->set_obj($suc_id);
        $this->data_band=array();
    }
    
    private function box_data_add($sqlo, $cp_id, $out_arr_index, $step_data, $box_def) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        debugOut('(122) box_data_add: '.print_r($box_def,1), $FUNCNAME, 2);
       
        if (!empty($box_def['CV_st'])) {
            $step_CV=$box_def['CV_st'];
            if (!$step_CV) {
                throw new Exception("[1] ['box'] 'CV_st' is missing.");
            }
            $step_data_CV = gProtoOrg::cStep_pure( $sqlo, $cp_id, $step_CV );
            $q_CV = $step_data_CV['QUANTITY'];
            
            // need $step_data['QUANTITY']; ...
            if (!is_numeric($step_data['QUANTITY'])) {
                return;
            }
            
            $mean = $step_data['QUANTITY'];
            $sigma = $mean * $q_CV;
            $q_min = $mean-$sigma;
            $q_max = $mean+$sigma;
            
            
        } else {
            $step_min=$box_def['min_st'];
            $step_max=$box_def['max_st'];
            if (!$step_min or !$step_max) {
                throw new Exception("[2] ['box'] 'min_st' or 'max_st' or 'CV_st' are missing.");
            }
            $step_data_min = gProtoOrg::cStep_pure( $sqlo, $cp_id, $step_min );
            $q_min = $step_data_min['QUANTITY'];
            $step_data_max = gProtoOrg::cStep_pure( $sqlo, $cp_id, $step_max );
            $q_max = $step_data_max['QUANTITY'];
        }
        $this->data_band[$out_arr_index] = array('min'=>$q_min, 'max'=>$q_max);
    }
    
    /**
     * get object name an optional more features
     * @param object $sqlo
     * @return string
     */
    function  get_obj_name_feat($sqlo) {
        
        $table_main='CONCRETE_SUBST';
        $name_out = obj_nice_name( $sqlo, $table_main,  $this->suc_id  );
        
        
        if (is_array($this->parx['objects'][1]['cols'])) {
            $obj_cols_def  = $this->parx['objects'][1]['cols'];
            
            $extra = glob_elemDataGet3( $sqlo, $table_main, array('CONCRETE_SUBST_ID'=>$this->suc_id), $obj_cols_def );
            foreach($obj_cols_def as $col_loop) {
                $val = $extra[$col_loop];
                if ($col_loop=='SOME_DATE') {
                    $val = substr($val,0,10);
                }
                $name_out .= '; '.$val;
            }
        }
        return $name_out;
    }
    
    /**
     * 
     * @param object $sqlo
     * @return NULL[]|mixed[]
     */
    function get_data($sqlo) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $proto_def  = $this->parx['objects'][2];
        $pra_id     = $proto_def['src']['abs_id'];
        $steps_want = $proto_def["steps"];
        
        //$pra_feats = $this->_pra_arr[$pra_id];
        //$step_feats = $pra_feats['features'];
        
        $out_arr = array( $this->suc_id );
        
        $cp_id = $this->proto_lib->get_c_proto_by_pra($sqlo, $pra_id);
        
        foreach( $steps_want as $row ) {
            
            $step_nr=$row['st'];
            
            if ($row['v']) $step_types = str_split($row['v']);
            else $step_types=array('q');
            // get only the QUANTITIES

            foreach($step_types as $type) {
                
                //$name = $step_feats[$step_nr]['NAME'];
                if ($type=='q') {
                    $step_data = gProtoOrg::cStep_pure( $sqlo, $cp_id, $step_nr );
                    $out_arr[] = $step_data['QUANTITY'];
                }
            }
            
            $out_arr_index = sizeof($out_arr)-2; // minus 2 because the first object is SUC_ID ...
            
            // 'box'=>array('min_st'=>, 'max_st'=>, 'CV_st'=>)
            if (!empty($row['box'])) {      
                $this->box_data_add($sqlo, $cp_id, $out_arr_index, $step_data, $row['box']);     
            }
        }
        
        return $out_arr;
    }
    
    // return 0 or 1
    function has_data_band() {
        return !empty($this->data_band);
    }
    
    function get_data_band() {
        return $this->data_band;
    }
}


class o_CONCRETE_SUBST_li_prot_img_XPL extends gPlugin {
    
    private $_pra_arr;// 'features'=>array(steps)
	
	function register() {
		$tablename	= 'CONCRETE_SUBST';
		$title		= 'show graph of protocol steps';
		
		$objsel  = $_REQUEST['objsel'];
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'list'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;
		
		$this->infoarr2 = array(
		    'accCheck' => array('tab'=>array('read'), 'obj'=>array() ),
		);
		
		if (is_array($objsel)) {
		    // nothing 
		} else {
		  $this->infoarr['obj_cnt'] = 1;
		  $this->infoarr2['listChkSelect'] = 1;
		}
		$this->infoarr['jsFile'] = f_html_diagram::get_js_script();
		
		
		
		
	}
	
	
	
	
	function _init($sqlo, $parx) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    $this->parx = $parx;
	    $this->axis_info = array();
	    
	    
	    if (!$parx['abs_obj_id']) {
	        $error->set( $FUNCNAME, 1, 'no abstract object ID defined' );
	        return;
	    }
	    $this->sua_id = $parx['abs_obj_id'];
	    
	    if (!is_array($parx['objects'][2])) {
	        $error->set( $FUNCNAME, 2, 'no Protocol DEF defined' );
	        return;
	    }
	    
	    $proto_def = $parx['objects'][2];
	    $aprotoid = $proto_def['src']['abs_id'];
	    if (!$aprotoid) {
	        $error->set( $FUNCNAME, 3, 'no PRA-ID defined' );
	        return;
	    }
	    
	    $steps_want = $proto_def["steps"];
	    
	    $pra_feats = o_CONCRETE_SUBST_li_prot_HELP::get_aproto_infos($sqlo, $aprotoid);
	    $this->_pra_arr[$aprotoid] = $pra_feats; // 'features'=>array(steps)
	    $step_feats = $pra_feats['features'];
	    
	    foreach( $steps_want as $row ) {
	        
	        $step_nr=$row['st'];
	        if ($row['v']) $step_types = str_split($row['v']);
	        else $step_types=array('q');
	        
	        // get only the QUANTITIES

	        foreach($step_types as $type) {
	            
	            $name = $step_feats[$step_nr]['NAME'];
	            if ($type=='q') {
	                $dataarr_tmp=array('NAME'=>$name, 'TYPE'=>'q');
	                $this->axis_info[]=$dataarr_tmp;
	            }
	           
	        }
	    }
	    
	}
	
	private function  _build_sql($sqlo, $objsel) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    if (!is_array($objsel['cond'])) {
	        $error->set( $FUNCNAME, 2, 'Table condition missing.' );
	        return;
	    }
	    if ($objsel['sort']==NULL) {
	        $error->set( $FUNCNAME, 2, 'Table sort missing.' );
	        return;
	    }
	    
	    $colvals = array();
	    
	    foreach($objsel['cond'] as $row) {
	        
	        $col_code = $row['col'];
	        $val      = trim($row['val']);
	        if ($val==='') {
	            $error->set( $FUNCNAME, 2, 'Condition for Column "'.$col_code.'" contains no value.' );
	            return;
	        }
	        
	        $col_pure = substr($col_code,2);
	        $prefix   = substr($col_code,0,1);
	        switch ($prefix) {
	            case 'x':
	                
	                if ( !glob_column_exists($this->table_main, $col_pure) ) {
	                    $error->set( $FUNCNAME, 3, 'Column "'.$col_code.'" not defined' );
	                    return;
	                }
	               break;
    	        case 'a':
    	            if ( !glob_column_exists('CCT_ACCESS', $col_pure) ) {
    	                $error->set( $FUNCNAME, 4, 'Column "'.$col_code.'" not defined' );
    	                return;
    	            }
    	            break;
    	        default: 
    	            $error->set( $FUNCNAME, 5, 'Col-Prefix "'.$prefix.'" not defined' );
    	            return;
	        }
	        
	        $colvals[] = '('.$col_code.'='.$sqlo->addQuotes($val).')';
	    }
	    
	    $where = implode(' and ',$colvals);
	    
	    $sqlAfter = $this->table_main.' x join CCT_ACCESS a on x.CCT_ACCESS_ID=a.CCT_ACCESS_ID where '.$where;
	    $sqlAfter .= ' order by '.$objsel['sort'];
	    
	    return $sqlAfter;
	}

	function startMain() {
		global $error;
		
		$FUNCNAME='startMain';
		$this->MAX_NUM = 500;
		
		$parx  = $_REQUEST['parx'];
		$sqlo  = &$this->_sqlo;
		$sqlo2 = &$this->_plug_regSql2();
		
		$this->table_main = 'CONCRETE_SUBST';
		
		$objsel  = $_REQUEST['objsel'];
		if (is_array($objsel)) {
		    
		    $this->MAX_NUM = $objsel['num'];
  
		    $this->sqlAfter  = $this->_build_sql($sqlo, $objsel);
		    
		} else {
		     $this->sqlAfter  = $this->listLib->getSqlAfter();
		}
		
		if ($error->Got(READONLY))  {
		    $error->set( $FUNCNAME, 1, 'Error on SQL init.' );
		    return;
		}
		
		$this->_init($sqlo, $parx);
		if ($error->Got(READONLY))  {
		    $error->set( $FUNCNAME, 2, 'Error on Init.' );
		    return;
		}
		

		$html_tmp = fObjViewC::bo_display( $sqlo, 'ABSTRACT_SUBST', $this->sua_id );
		if($this->_pra_arr) {
		    foreach($this->_pra_arr as $pra_id=>$dummy) {
		        $html_tmp .= '; Protocol: '.fObjViewC::bo_display( $sqlo, 'ABSTRACT_PROTO', $pra_id );
		    }
		}
		$this->_html->line_out('Selected MACs of MAA: '.$html_tmp);
		$this->_html->line_out('');

		$this->img_lib2 = new oProtoQuant_img2C();
		
		$this->suc_one_lib = new o_CONCRETE_SUBST_prot_HELP($parx, $this->_pra_arr);
		
		$sqlsel = "x.CONCRETE_SUBST_ID, x.ABSTRACT_SUBST_ID from ". $this->sqlAfter;
		$sqlo2->Quesel($sqlsel);
		
		$cnt=0;
		while ( $sqlo2->ReadRow() ) {
		    
		    if ($cnt >= $this->MAX_NUM) {
		        break;
		    }

		    $suc_id = $sqlo2->RowData[0];
		    $sua_id = $sqlo2->RowData[1];
		    
		    $this->suc_one_lib->set_suc_id($suc_id);
		    $dataarr = $this->suc_one_lib->get_data($sqlo);
		    
		    debugOut('- dataarr:'.print_r($dataarr ,1), $FUNCNAME, 1);
		    
		    $objxname = $this->suc_one_lib->get_obj_name_feat($sqlo); 
		    
		    if ($sua_id!=$this->sua_id) {
		        $dataarr=array($suc_id);
		        $objxname .=' ERR: bad MAA-ID!';
		    }
		    
		    $data_band=array();
		    
		    if ($this->suc_one_lib->has_data_band()) {
		        $data_band = $this->suc_one_lib->get_data_band();
		        $this->img_lib2->set_option('data_band_box', 1);
		        if ( $_SESSION["userGlob"]["g.debugLevel"]>1 ) {
		            debugOut('(462) data_band:'.print_r($data_band,1), $FUNCNAME, 2); 
		        }
		    }
		   
		    $this->img_lib2->add_datarow($dataarr, $objxname, $data_band);
		    $cnt++;
		}
		
		$img_title='TITELX';
		
		if ( $_SESSION["userGlob"]["g.debugLevel"]>1) {
		    debugOut('(478) axis_info:'.print_r($this->axis_info,1), $FUNCNAME, 1);
		    debugOut('parx:'.print_r($parx ,1), $FUNCNAME, 1);
		}
		
		$this->img_lib2->img_init($this->axis_info, $img_title);
		$this->img_lib2->show_img();
	}
	
}
