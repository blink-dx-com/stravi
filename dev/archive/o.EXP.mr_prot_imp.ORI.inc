<?php
/**
 * EXPERIMENT multireactor protocol import
 * - import a MultiReactorProtoFile
 * - can create CHILDREN or Update CHILDREN protocols ...
 * @package  o.EXP.mr_prot_imp.inc
 * @author  Steffen Kube (steffen@clondiag.com)
 * @param 
 *   $id  -- mother experiment
	[$go]    ==   0,1,2
	[$parx]
	
   @param $_FILES['userfile'] - uploaded file
   
   
   @var array pra_step_arr_STRUCT
      array:  array('fpos'=>$i, 'st'=>$step_nr, 'ty'=>$data_type, 'name'=>$pra_name, 'error'=>$xerror);
 */

require_once ('func_form.inc');
require_once ("f.upload.inc");
require_once ('f.msgboxes.inc');
require_once ("visufuncs.inc");
require_once ("f.objview.inc");	


require_once 'f.debug.inc';

require_once 'o.ABSTRACT_PROTO.stepx.inc';
require_once 'o.CONCRETE_SUBST.proto.inc';
require_once 'o.EXP.mr.inc';
require_once 'o.EXP_TMPL.proto.inc';
require_once 'o.EXP.proto.inc';
require_once 'o.EXP.proto_mod.inc';

require_once dirname(__FILE__).'/../lablib/MR/f.mrpf_imp_file.inc';
require_once dirname(__FILE__).'/_subs/o.EXP.mr_prot_help.inc';





/**
 * ONE experiment
 */
class oEXP_mr_ONE {
    
    /**
     * 
     * @param array $pra_step_arr  array('hpos'=>$i, 'st'=>$step_nr, 'ty'=>$data_type);
     * @param int $pra_id
     */
    function __construct(array $pra_step_arr, int $pra_id, int $step_no) {
        $this->pra_step_arr=$pra_step_arr;
        $this->pra_id  = $pra_id;
        $this->step_no = $step_no;
        
        $this->prc_lib = new oPRC_mr_one($this->pra_step_arr);
    }

    function get_error_data() {
        return $this->prc_lib->get_error_data();
    }
    
    /**
     * 
     * @param object $sqlo
     * @param int $sub_exp_id
     * @param array $datafields_raw
     * @param int $last_prc_id
     * @param int $do_create
     */
    function handle_replica($sqlo, int $sub_exp_id, array $datafields_raw, int $last_prc_id, int $do_create) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $exp_lib = new oEXP_proto_mod();
        $exp_lib->set_exp($sqlo, $sub_exp_id);
        
        foreach($this->pra_step_arr as $ind => $row) {
  
            $val   = $datafields_raw[$ind];
            $val   = trim($val);
            if ($val==='' or $val===NULL) {
                // ok
            } else {
                $error->set( $FUNCNAME, 10, 'A REPLICA column must NOT contain any data!' );
                return;
            }
        }
        
        if ($do_create) {
            
            if(!$last_prc_id) {
                $error->set( $FUNCNAME, 100, 'Input: PRC-ID missing.' );
                return;
            }

            if ($exp_lib->step_exists($sqlo, $this->step_no) ) {
                
                $ori_prc_id = $exp_lib->get_cpid_of_step($sqlo, $this->step_no);
                debugOut('(81) old PRC-ID exists:'.$ori_prc_id, $FUNCNAME, 2);
                if ($ori_prc_id!=$last_prc_id) {
                    $error->set( $FUNCNAME, 100, 'Bizzare: Current PRC-ID ('.$ori_prc_id.') is not the same like new PRC-ID ('.$last_prc_id.')' );
                    return;
                }
             } else {
                debugOut('(87) REPLICA: use PRC-ID:'.$last_prc_id, $FUNCNAME, 2);
                $exp_lib->add_existing($sqlo, $last_prc_id, $this->step_no);
             }
            
        }
    }
    
    /**
     * import ONE protocol ...
     * @param object $sqlo
     * @param object $sqlo2
     * @param int  $sub_exp_id
     * @param array $datarow
     * @param int $do_create  0,1
     * @return void|number[]
     */
    function import(object $sqlo, $sqlo2, $sub_exp_id, $datarow, $do_create) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $this->_error_data=NULL;
        
        // search protocol
        $exp_prc_lib = new oEXPprotoC();
        $prc_id = $exp_prc_lib->getConcProtoByAbstract($sqlo, $sub_exp_id, $this->pra_id);
        if ($prc_id) {
            $error->set( $FUNCNAME, 100, 'Protocol already exists on EXP.' );
            // this is a WARNING ?
            return;
        }
        
        $step_params = $this->prc_lib->raw2step_data($sqlo, $datarow);
        
        
        if ($do_create==1) {
        
            // echo print_r($step_params,1)."<br>";
            $status='?';
            
            $exp_lib = new oEXP_proto_mod();
            $exp_lib->set_exp($sqlo, $sub_exp_id);
            
            if ($exp_lib->step_exists($sqlo, $this->step_no) ) {
                $prc_id = $exp_lib->get_cpid_of_step($sqlo, $this->step_no);
                debugOut('(183) CPID already exists. Get this ID:'.$prc_id.'.', $FUNCNAME, 2);
                $status='UPDATE:';
            } else {
                debugOut('(186) CREATE new CPID.', $FUNCNAME, 2);
                $status='CREATE:';
                $prc_id = $exp_lib->create($sqlo, $sqlo2, $this->pra_id, $this->step_no);
                if ($error->Got(READONLY))  {
                    $error->set( $FUNCNAME, 10, 'Error on protocol creation' );
                    return;
                }
            }
            
            $proto_mod_lib = new oConcProtoUpSteps();
            $proto_mod_lib->setProto($prc_id);
            $proto_mod_lib->protoUpdate($sqlo, $this->pra_id, $step_params);
            if ($error->Got(READONLY))  {
                $error->set( $FUNCNAME, 11, 'Error on protocol STEP update' );
                return;
            }
        }
        

        return array('prc_id'=>$prc_id, 'status'=>$status);
    }
    
}

class o_EXP_mr_prot_imp_C {
    
    private $do_create=0;
    private $exp_id;
    private $exp_chi_arr;  // list of EXP of the MOTHER: can be empty on import ...
    private $exp_list;     // list of EXP in the FILE:  array [] = array( 'id'=>EXP_ID )
    
    private $pra_step_arr; // pra_step_arr_STRUCT
    private $pra_id;
    
    function __construct() {
    }
    
    function init($sqlo, $exp_id) {
        $this->exp_id= $exp_id;
        $this->exp_mr_lib = new oEXP_mr($exp_id);
        $this->exp_chi_arr = $this->exp_mr_lib->get_children($sqlo);
        
        $help_lib = new o_EXP_mr_prot_imp_help();
        $help_lib->init($sqlo, $exp_id);
      
        $this->pra_id  = $help_lib->pra_id;
        $this->step_no = $help_lib->step_no;

        $this->has_children_on_start = 0;
        if(!empty($this->exp_chi_arr )) {
            $this->has_children_on_start = 1;
        }
        
//         if(empty($this->exp_chi_arr )) {
//             $error->set( $FUNCNAME, 1, 'This experiment has no children.' );
//             return;
//         }
        
    }

    /**
     * create one 
     * @param object $sqlo
     * @param string $well_name
     */
    private function _subexp_create($sqlo, $abs_cart_lib, string $well_name) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;

        $exp_pos  =  $abs_cart_lib->wellAlias2Pos($well_name);
        $new_name = 'EXP'.$this->exp_id.' '.$well_name;
        
        $new_args = array();
        $new_args['NAME']    = $new_name;
        $new_args['MO_POS']  = $exp_pos;
        
        debugOut('(592) new_args:'.print_r($new_args,1), $FUNCNAME, 2);
        $sub_exp_id = 0;
        if ($this->do_create) {
            $sub_exp_id = $this->exp_mod_lib->add_sub_exp($sqlo, $new_args);
            if ($error->Got(READONLY))  {
                $error->set( $FUNCNAME, 1, 'Sub-experiment creation pos='.$exp_pos.' failed.' );
                return;
            }
        }
        return $sub_exp_id;
    }
    

    /**
     * import the protocol data
     * @param object $sqlo
     * @param object $sqlo2
     * @param array $proto_cell_data
     */
    private function _import_data($sqlo, $sqlo2) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $proto_cell_data = &$this->proto_cell_data;
        
        $this->one_exp_lib = new oEXP_mr_ONE( $this->pra_step_arr, $this->pra_id, $this->step_no);
        
        $tabobj = new visufuncs();
        $headOpt = array( "title" => "Import Overview" );
        
        $header_tab = array('#', 'Experiment','WGR', 'Status', 'Errors', 'Details');
        
        $tabobj->table_head($header_tab,   $headOpt);
        
        $last_wgr      = -1;
        $last_prc_id   = 0;
        $all_error_got = 0;
        $this->all_error_det = array(); // array [exp_index] = $error_row
        
        $wgr_data     = &$this->header_obj_info['WGR'];
        $obj_ori_data = &$this->header_obj_info['OBJ_ori'];
        
        $i=0;
        foreach( $this->exp_list as $exp_info )  {
            
            $sub_exp_id = $exp_info['id'];
            $html_tmp   = $obj_ori_data[$i];
            $wgr_now    = $wgr_data[$i];
            
            if (!$sub_exp_id) {

                $status='not exists';
                $allErrTxt='';
                $exp_det_info=array();
                
            } else {
            
                $allErrTxt= '';
                $datafields_raw  = $proto_cell_data[$i];
                $status   ='?';
                $exp_det_info = array();
                
                
                $do_import=1;
  
                // check for replica
                
                
                if ($wgr_now>0) {
                    
                    if ( $last_wgr==$wgr_now) {

                        // handle as REPLICA
                        $this->one_exp_lib->handle_replica($sqlo, $sub_exp_id, $datafields_raw, $last_prc_id, $this->do_create);
                        
                        $do_import=0;
                        if ($error->Got(READONLY))  {
                            $all_error_got=1;
                            $allErr    = $error->getTextAsArray();
                            $allErrTxt = implode('; ',$allErr);
                            $error->reset();
                            
                            $erros_row = $this->one_exp_lib->get_error_data();
                            $this->all_error_det[$i] = $erros_row;
                            $status='<span style="color:red;">ERROR</span>';
                            
                        } else {
                            $status = 'REPLICA, OK';
                        }
                    }
                    $last_wgr = $wgr_now;
                    
                } else {
                    // if WGR==0  =>  no import 
                    $do_import = 0;
                    $status='NOT imported';
                }
                
                
                if ($do_import) {
                    $exp_det_info = $this->one_exp_lib->import($sqlo, $sqlo2, $sub_exp_id, $datafields_raw, $this->do_create);
                    $status   = $exp_det_info['status'];
                    
                    if ($error->Got(READONLY))  { 
                        $all_error_got=1;
                        $allErr    = $error->getTextAsArray();
                        $allErrTxt = implode('; ',$allErr);
                        $error->reset();
                        
                        $erros_row = $this->one_exp_lib->get_error_data();
                        $this->all_error_det[$i] = $erros_row;
                        $status='<span style="color:red;">ERROR</span>';
                        
                    } else {
                        $status   .='OK';
                        $last_prc_id = $exp_det_info['prc_id'];
                        if(!$last_prc_id) $last_prc_id=0;
                    }
                }
                
                if ($sub_exp_id) 
                    $html_tmp = fObjViewC::bo_display( $sqlo, 'EXP', $sub_exp_id );
            }
            
            $data_tmp=array($i+1, $html_tmp, $wgr_now, $status, $allErrTxt, print_r($exp_det_info,1) ); 
 
            $tabobj->table_row ($data_tmp);

            $i++;
            
        }
        $tabobj->table_close();
        
        echo "<br><br>\n";
        
        
        
        if ($all_error_got) {
            $error->set( $FUNCNAME, 1, 'Errors detected.' );
            return;
        } 
    }
    
    private function _analyse_exp_header($sqlo) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $exp_features = $this->exp_mr_lib->get_mr_details($sqlo);
        
        $this->exp_mod_lib  = new oEXP_mr_mod($sqlo, $this->exp_id);
        
        $ABS_SOCKET_ID = $exp_features['ABS_SOCKET_ID'];
        $abs_cart_lib  = new oABS_SOCKET_subs();
        $abs_cart_lib->init($sqlo, $ABS_SOCKET_ID);
        
        /* - now analyse the file-header: Experiment names
         * - if no CHILDREN were available at start:
         *     ==> YES: only update
         *     ==> NO:  create children now
         * -
         */

        $well_names = &$this->header_obj_info['OBJ'];
        
        $tmperr=array();
        
        foreach($well_names as $i => $well_name) {
            
            $exp_id    = $this->exp_mr_lib->find_exp_by_NAME_pattern($sqlo, '%'.$well_name );
            
            if ($this->has_children_on_start) {
                
                if (!$exp_id or $exp_id<0) {
                    $tmperr[$i]=$well_name.': not found';
                }
                
            } else {
                
                do {
                    
                    if ($exp_id<0) {
                        $tmperr[$i]=$well_name.': more than one found';
                        break;
                    }
                    if ($exp_id>0) {
                        // exists ...
                        break;
                    }
                    
                    // create
                    $exp_id = $this->_subexp_create($sqlo, $abs_cart_lib, $well_name);
                    if ($error->Got(READONLY))  {
                        $error->set( $FUNCNAME, 3, 'Error on Exp-creation of Well '.$well_name );
                        return;
                    }
                    
                } while (0);
                
            }
            
            $this->exp_list[] = array('id'=>$exp_id);
            
            
        }
        
        $this->exp_row_errors = $tmperr;
        
        if ( !empty($tmperr) ) {
            echo "Some Experiments could not be found:<ul>";
            foreach($tmperr as $text) {
                echo "<li>{$text}</li>";
            }
            echo "</ul>\n";
            echo "<br>\n";
            $error->set( $FUNCNAME, 3, 'Some Experiments were not found.' );
            return;
                
        }
        
        if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
            
            echo "<br>Debug: Experiment list:<ul>";
            foreach( $this->exp_list as $row) {
                $sexp_id=$row['id'];
                echo "<li>{$sexp_id}</li>";
            }
            echo "</ul><br>\n";
        }
        
    }
    
    function set_pra_data(&$pra_step_arr) {
        $this->pra_step_arr = $pra_step_arr;
        //$this->pra_step_ori = $pra_step_ori;
    }
    

    
    /**
     * import file and create steps
     *
     * @param $sql
     * @param $sql2
     * @param $tempfile
     * @param $go
     * @return NULL
     */
    function import_data( &$sqlo, &$sqlo2, $do_create, &$header_obj_info, &$proto_cell_data) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $this->do_create = $do_create;
        
        $this->header_obj_info = &$header_obj_info;
        $this->proto_cell_data = &$proto_cell_data;

        $this->_analyse_exp_header($sqlo);
        if ($error->Got(READONLY))  {
            $error->set( $FUNCNAME, 1, 'Header analysis error.' );
            return;
        }
        $this->_import_data($sqlo, $sqlo2);

    }
    
    function get_pra_id() {
        return $this->pra_id;
    }
    function get_all_error_det() {
        return $this->all_error_det;
    }
}


class o_EXP_mr_prot_imp2_XPL extends gPlugin {

	function register() {
		$tablename	= 'EXP';
		$title		= 'Multireactor Protocol import';
		
		$this->infoarr			 = NULL;
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'obj';
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_id']   = $_REQUEST['id'];
		$this->infoarr['checkid']  = 1;

		$this->infoarr2 = array( 
		    'accCheck' => array('tab'=>array('write'), 'obj'=>array('read', 'write') )
			);
	}
	
	function help() {
	    
     	echo '<br><br>';
     	
    	htmlInfoBox( "Short help", "", "open", "HELP" );
    	?>
    	<ul>
    	<LI>Version: 10.03.2021 18:36</LI>
     	<LI>Import MultiReactorProtoFile</LI>
     	<LI>[<a href="">Download Excel-Example-File</a>]</LI>
		</ul>
		<?php
    	htmlInfoBox( "", "", "close" );
    }
    
	
	function form1() {

	    $initarr   = array();
	    $initarr["title"]       = "Give Excel-file";
	    $initarr["submittitle"] = "Upload file"; // title of submit button
	    $initarr["tabwidth"]    = "AUTO";   // table-width: AUTO
	    $initarr["ENCTYPE"]     = "multipart/form-data";
	   
	    
	    $hiddenarr = array(); // hidden form variables
	    $hiddenarr['id']=$this->exp_id;

	    $formobj = $this->_html->formInit($initarr, $hiddenarr);
	    
	    $fieldx = array ( // form-field definition
	        "title" => "Protocol-File",
	        "name"  => "userfile",
	        "namex" => TRUE,
	        "object"=> "file",
	        "val"   => '',
	        
	    );
	    $formobj->fieldOut( $fieldx ); // output the form-field
	    
	    $formobj->close( TRUE );
	    
	    
	}
	
	function form2($tmpfile) {
	    $initarr   = array();
	    $initarr["title"]       = "Import Protocol data";
	    $initarr["submittitle"] = "Import now!"; 
	    $initarr["tabwidth"]    = "AUTO";  

	    $hiddenarr = array(); 
	    $hiddenarr["userfile"]     = $tmpfile;
	    $hiddenarr["short_file_name"] = $this->newName;
	    $hiddenarr['id']=$this->exp_id;

	    $formobj = $this->_html->formInit($initarr, $hiddenarr, 1);
	    $formobj->close( TRUE );

	}
	
	function form3_restart() {
	    $initarr   = array();
	    $initarr["title"]       = "Back to start";
	    $initarr["submittitle"] = "Back to start";
	    $initarr["goNext"]      = 0;
	    $initarr["tabwidth"]    = "AUTO"; 
	    $hiddenarr = array();
	    $hiddenarr['id']=$this->exp_id;
	    $formobj = $this->_html->formInit($initarr, $hiddenarr, 0);
	    $formobj->close( TRUE );
	    
	}
	

	
	function startMain() {
		global $error;
		$FUNCNAME='startMain';

		$sqlo = &$this->_sqlo;
		$sqlo2= &$this->_plug_regSql2();
		
		$go      = $_REQUEST['go'];
		$this->exp_id  = $_REQUEST['id'];
		
		// $parx    = $_REQUEST['parx'];
		
		$short_file_name = $_REQUEST['short_file_name'];
		
		$this->uploadLib = new uploadC();
		$this->tmp_sub_dir = str_replace('/', '_',  $this->_mod );
		
		$mochi_type = oEXP_mr_STAT::get_mochi_type($sqlo, $this->exp_id);
		if ($mochi_type!=oEXP_mr_STAT::MOCHI_TYPE_MO) {
		    $error->set( $FUNCNAME, 1, 'This experiment is not an MTP-mother.' );
		    return;
		}
		
		$import_lib = new o_EXP_mr_prot_imp_C();
		$import_lib->init($sqlo, $this->exp_id);
		if ($error->Got(READONLY))  {
		    $error->set( $FUNCNAME, 1, 'Error on init.' );
		    return;
		}
		
		if ($go>0) {
		    echo '[<a href="'.$this->_plug_get_URL().'&id='.$this->exp_id.'">Restart</a>] ';
		}
		
		$pra_id = $import_lib->get_pra_id();
		$html_tmp = fObjViewC::bo_display( $sqlo, 'ABSTRACT_PROTO', $pra_id );
		$this->_html->line_out( 'Planned Protocol: '. $html_tmp);
		$this->_html->line_out( '');
		
		
		if (empty($go)) $go = 0;

		
		if ( !$go  ) {
		    $this->form1();
		    $this->help();
		    return;
		}
		
		
		
		if ( $go==1 ) {

		    $userfile_size = $_FILES['userfile']['size'];
		    $userfile      = $_FILES['userfile']['tmp_name'];
		    $userfile_name = $_FILES['userfile']['name'];

		    $this->newName = $userfile_name;
		    $tmpfile = $this->uploadLib->mvUpload2Tmp( $this->tmp_sub_dir, $this->newName, $userfile, $userfile_name, $userfile_size );
		    if ($error->Got(READONLY))  {
		        $this->form3_restart();
		        return;
		    }

		}
		
		if ( $go == 2 ) {
		    
		    $this->newName = $short_file_name;
		    $tmpfile = $this->uploadLib->getFileFromTmp($this->tmp_sub_dir, $this->newName);
		    if ($error->Got(READONLY))  {
		        $this->form3_restart();
		        return;
		    }
		}
		
		$file_lib = new f_mrpf_imp_file($tmpfile);
		$file_lib->set_pra_id($pra_id);
		$file_lib->import_start($sqlo);
		
		//$pra_step_arr    = $file_lib->get_pra_step_arr();
		$header_obj_info = $file_lib->get_header_obj_info();
		
		
		if ($error->Got(READONLY))  {
		    $error->set( $FUNCNAME, 2, 'Error on file read - header.' );
		    return;
		}
		
// 		$file_lib->import_proto_data();
// 		$proto_cell_data = $file_lib->get_proto_cell_data();
// 		if ($error->Got(READONLY))  {
// 		    $error->set( $FUNCNAME, 3, 'Error on file read - the protocol data' );
// 		    return;
// 		}
		
// 		$do_create = 0;
// 		if ($go==2) $do_create = 1;
		
// 		//WEG: $import_lib->set_pra_data($pra_step_arr);
// 		$import_lib->import_data( $sqlo, $sqlo2, $do_create, $header_obj_info, $proto_cell_data);
		
		if ($error->Got(READONLY))  {
		    
		    $this->form3_restart();
		    
		    $all_error_det = $import_lib->get_all_error_det();
		    $file_lib->show_full_table_frCache($all_error_det);

		    return;
		   
		} else {
		    if ( $go==1 ) $this->form2($tmpfile);
		    else {
		        
		        $this->_html->line_out('');

		        
		        $this->_html->messageBox2('ok', 'Updates ok.');
		        
		        $this->_html->line_out('');
		        $this->_html->line_out('');
		        $this->form3_restart();
		    }
		}

	}
}
