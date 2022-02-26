<?php
/**
 * import protocol steps from CSV-file
 * @namespace core::obj:ABSTRACT_SUBST
 * @package  o.ABSTRACT_PROTO.import.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param 
 *  $id
	[$go]    ==   0,1,2
	
	$short_file_name : short filename
	$action: 
	    optional: 'download' (template for import)
	[$parx]
	  'use_step_nr' : 0,1 use original step_nr ?
	  [infolevel]
	  [subst_create] 0|1  create substance, if not found
	  old_steps_keep : 0|1
    @param $_FILES['userfile'] - uploaded file
   @unittest exists (data file: "plugin\o.ABSTRACT_PROTO.import_01.xlsx" )
 */
require_once 'f.debug.inc';
require_once ('func_form.inc');
require_once ('insert.inc');
require_once ("insertx.inc");
require_once ("f.assocUpdate.inc");
require_once ("f.upload.inc");
require_once ('f.msgboxes.inc');


require_once ("import/f.read_file_meta.inc");
require_once ("import/f.spreadsheet_ana.inc");
require_once ('lev1/f.exportDataFile.inc');
require_once ("export/o.ABSTRACT_PROTO.steps.inc");

/**
 * manage import of abstract protocol steps
 * @author steffen
 *
 */
class oAbstract_protoImpC {
    var $useIDs;    // 0,1 : use substance IDs, device IDs to identify?
    private $encoding_key;
    private $proto_id;
    private $newName;
    
    private $pra_info;
    private $sum;
    /**
     * 
     * @var array
     * 'use_step_nr' : 0,1 use original step_nr ?
     *  old_steps_keep
     */
    private $options;
    
    /**
     * 
     * @param int $proto_id
     * @param string $scriptid
     * @param array $options
     */
    function __construct( $sqlo, $proto_id, $scriptid, $options ) {
        $this->proto_id = $proto_id;
        $this->options = $options;
        $this->insertSubstAllowed = 0;
        $this->useIDs = 0;
        
        $this->_subst_create = $this->options['subst_create'];
        
        $this->newName   = '';
        $this->scriptid  = $scriptid;
        $this->uploadLib = new uploadC();
        
        $this->tmp_sub_dir = str_replace('/', '_',  $this->scriptid );
        
        $this->analyse_old_pra($sqlo);
    }
    
    private function analyse_old_pra($sqlo) {
        
        $this->pra_info=array();
        
        $pra_id = $this->proto_id;
        // last MAIN_NR
        $sqls= "select max(MAIN_STEP_NR) from ABSTRACT_PROTO_STEP where ABSTRACT_PROTO_ID=".$pra_id;
        $sqlo->query($sqls);
        $sqlo->ReadRow();
        $this->pra_info['MAIN_STEP_NR.max']=$sqlo->RowData[0];
        
        $sqls= "select max(STEP_NR) from ABSTRACT_PROTO_STEP where ABSTRACT_PROTO_ID=".$pra_id;
        $sqlo->query($sqls);
        $sqlo->ReadRow();
        $this->pra_info['STEP_NR.max']=$sqlo->RowData[0];    
    }
    
    
    
    function GoInfo($go) {
        
       
        $goArray   = array( "0"=>"Give file", 1=>"Prepare Import", 2=>"Create steps" );
        $extratext = '[<a href="'.$_SERVER['PHP_SELF'].'?mod=DEF/o.ABSTRACT_PROTO.import&id='.$this->proto_id.'">Start again</a>]';
        
        $formPageLib = new FormPageC();
        $formPageLib->init( $goArray, $extratext );
        $formPageLib->goInfo( $go );
        
    }
    
    private function _info_out($key, $val) {
        echo $key.": ".$val."<br>\n";
    }
    
    function getObjId( &$sql, $subst_name, $table) {
        /* RETURN: 0 : no subst
         -1: more than one found
         >0: subst_id
         */
        
        $pkname  = PrimNameGet2($table);
        $nameCol = importantNameGet2($table);
        
        $sqls = "select ".$pkname." from ".$table." where ".$nameCol."=".$sql->addQuotes($subst_name);
        $sql->query($sqls);
        $cnt = 0;
        $subst_id = '';
        while ($sql->ReadRow()) {
            $subst_id = $sql->RowData[0];
            $cnt++;
        }
        if ($cnt>1) $subst_id=-1;
        return ($subst_id);
    }
    

    
    /**
     * test, if a substance can be inserted
     * @param  $sqlo
     * @return boolean 0 : allow; 1 : deny
     */
    function _canInsertSubst( &$sqlo ) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        if ( $this->insertSubstAllowed>0 ) return (1);
        
        $t_rights = tableAccessCheck( $sqlo, 'ABSTRACT_SUBST' );
        $right    = "insert";
        if ( $t_rights["insert"] != 1 ) {
            $answer = getTableAccessMsg( 'ABSTRACT_SUBST', $right );
            $error->set( $FUNCNAME, 1, $answer );
            return 0;
        }
        $this->insertSubstAllowed = 1;
        return ($this->insertSubstAllowed);
    }
    
    function createsubst( &$sqlo, $subst_name ) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        if (!$this->_canInsertSubst($sqlo) ) {
            $error->set( $FUNCNAME, 1, 'can not insert substance "'.$subst_name.'"' );
            return;
        }

        $insertlib = new insertC();
        $args = array( 
            "vals"=> array(
                'NAME' => $subst_name
            )
 
        );
        $subst_id = $insertlib->new_meta($sqlo, 'ABSTRACT_SUBST', $args);
       
        return ($subst_id);
    }
    
   
    
    function _objOut( $table, $objname, $objid, $xerrx ) {
        echo "<td>";
        if ( $xerrx == "" ) echo $objname;
        else echo "<font color=gray>$objname</font> ".$xerrx;
        if ($objid) echo " <font color=gray>[<a href=\"edit.tmpl.php?tablename=".$table.
            "&id=$objid\">ID:$objid</a>]</font>";
        echo "</td>";
    }
    
    // get device info
    private function _getDevice( &$sqlo, $datafields ) {
        
        $objid = NULL;
        $name  = NULL;
        $xerr  = '';
        
        $name_or_id = $datafields['device'];
        if ($name_or_id===NULL) {
            return array ( $objid, $name, $xerr );
        }
        
        $is_ID = gObj::is_ID($name_or_id);
        
        if ($is_ID) {
            
            $objid = $name_or_id;
            if ($objid==NULL) return;
            if (!gObject_exists ($sqlo, 'A_CHIP_READER', $objid) ) {
                $xerr="<br><font color=red>ID ".$objid." not found in DB.</font><br>";
                return array ( NULL, $objid, $xerr );
            }
            $name = obj_nice_name ( $sqlo, 'A_CHIP_READER', $objid );
            
        } else {
            
            $name = $name_or_id;
            if ($name==NULL) return;
            
            $objid = $this->getObjId( $sqlo, $name, 'A_CHIP_READER');
            
            if ($objid==0) {
                $xerr="<br><font color=red>Name not found in DB.</font><br>";
            }
            if ($objid<0) {
                $xerr="<br><font color=red>Ambigous: name more than once in DB.</font><br>";
                $objid="";
            }
        }
        return array ( $objid, $name, $xerr );
    }
    
    static function replace_bad_excel_chars($text) {
        $text = preg_replace('/[\xc2\xa0]/', '', $text);
        return $text;
    }
    
    /**
     * manage substance ID <=> name
     * @param object $sqlo
     * @param array $datafields
     * @return array
     */
    function _substManage(&$sqlo, &$datafields) {
        
        $xerrSubst = NULL;
        $subst_id  = 0;
        $subst_do_create_now=0;
        $subst_name= NULL;
        
        $name_or_id = $datafields['substance'];
        $name_or_id=trim($name_or_id);
        if ( $name_or_id==NULL ) return array(NULL, $subst_name, $subst_do_create_now, $xerrSubst);
        
        $name_or_id=self::replace_bad_excel_chars($name_or_id);
        
        $is_ID = gObj::is_ID($name_or_id);
        
        if ($is_ID) {
            
            $subst_id = $name_or_id;
            
            if ( !is_numeric($subst_id)) {
                $xerrSubst="<br><font color=red>is not a number.</font><br>".$this->_tmp_empty_str;
                return array(NULL, $subst_name, $subst_do_create_now, $xerrSubst);
            }
            
            if (!gObject_exists ($sqlo, 'ABSTRACT_SUBST', $subst_id) ) {
                $xerrSubst="<br><font color=red>ID ".$subst_id." not found in DB.</font><br>".$this->_tmp_empty_str;
                return array(NULL, $subst_name, $subst_do_create_now, $xerrSubst);
            }
            $subst_name = obj_nice_name ( $sqlo, 'ABSTRACT_SUBST', $subst_id );
            
        } else {
            
            $subst_name = $name_or_id;
            
            $subst_id = $this->getObjId($sqlo, $subst_name, 'ABSTRACT_SUBST');
            $subst_do_create_now = 0;
            
            if ( !$subst_id ) {
                if ($this->_subst_create) {
                    $xerrSubst="<br><font color=#808000>Name not found in DB.<br><font color=green>Create a new one.</font>";
                    $subst_do_create_now=1;
                } else {
                    $xerrSubst="<br><font color=red>Name not found in DB.</font><br>";
                    $xerrSubst=$xerrSubst . $this->_tmp_empty_str;
                }
            }
            if ($subst_id<0) {
                $xerrSubst="<br><font color=red>Ambigous: name more than once in DB.</font><br>".$this->_tmp_empty_str;
                $subst_id="";
            }
            
        }
        return array($subst_id, $subst_name, $subst_do_create_now, $xerrSubst);
    }
    
    static private function _check_flags($flags) {
        
        $allow_keys=array('s','q', 'n', 'd');
        $bad=array();
        $good=array();
        $flags=trim($flags);
        if ($flags==NULL) return array(NULL, '');
        $flags_arr=str_split($flags);
        foreach($flags_arr as $flag_loop) {
            if (!in_array($flag_loop, $allow_keys)) {
                $bad[]=$flag_loop;
            }
            if ($good[$flag_loop]>0) {
                $xerr='<br><font color=red>Error:</font> key "'.$flag_loop.'" is only allowed once in the string.';
                return array(NULL, $xerr);
            }
            $good[$flag_loop]=1;
        }
        
        if(!empty($bad)) {
            $xerr='<br><font color=red>Error:</font> Following keys are not allowed "'.implode(',',$bad).'".';
        }
        
        return array($flags, $xerr);
    }
    
    /**
     * handle one row
     * INPUT:
     *   $this->go
     *   $this->options
     *   $this->sum
     *   $this->tab_len
     * @param object $sqlo
     */
    private function _row_one($sqlo, $sqlo2, $file_lib, $spread_lib) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $proto_id = $this->proto_id;
        $datafields_raw  = $file_lib->get_data_row();
        $datafields      = $spread_lib->getDataByDefColName( $datafields_raw );
        
        $retval_tmp=0;
        
        // trim
        $no_trim=array('name'); // except the name
        foreach($datafields as $index=>$val_raw) {
            if (in_array($index, $no_trim)) {
                //
            } else {
                $datafields[$index]=trim($val_raw);
            }
        }
        
        // check content
        $has_data=0;
        foreach($datafields as $cell) {
            if(trim($cell)!=='') $has_data=1;
        }
        
        // $insert_possible=1;
        if ( !$has_data ) {
            return;
        }
            
        $sub_step_nr  = 0;
        $xerr=array();
        $xerr['NAME'] ="";
        $xerr['SUBST']="";
        $xerr['UNIT'] ="";
        $xerr['DEV']  ="";
        $xerr['QUANT'] ="";
        $xerr['FLAGS'] ="";
        
        $use_val=array();
        
        $dev_id   = '';
        $subst_id = '';
        $unit_id  = 0;
        $subst_do_create_now = 0;
        
        if ($this->options['use_step_nr']) {
            $this->sum['step_nr'] = $datafields['step_nr'];
            if (!$this->sum['step_nr']) {
                $error->set( $FUNCNAME, 2, 'Line '.($this->tab_len+1).' Step_nr missing.' );
                return;
            }
        }
        
        
        
        $name = $datafields['name'];
        
        if ($this->head_col_block_has) {
            
            $block = trim($datafields['block']);
            if ($block!=NULL) {
                $name = $block;  // use this as name, $sub_step_nr=0
            } else {
                $sub_step_nr=1;
                if ( substr($name, 0,3)== "   " ) { // analyse first 3 chars: insert a sub-step
                    $sub_step_nr=2;
                }
            }
            
            
        } else {
            
            if (trim($name)=="") {
                $retval_tmp=-1;
                $xerr['NAME']="<br><font color=red>No name<br>[step not imported]</font>";
            } 
        
            if ( substr($name, 0,3)== "   " ) { // analyse first 3 chars: insert a sub-step
                $sub_step_nr=1;
            }
            if ( substr($name, 0,6)== "      " ) { // analyse first 6 chars: insert a sub-step
                $sub_step_nr=2;
            }
        }
        $name = trim($name);
        
        
        $xerr['SUBST']=NULL;
        list($subst_id, $subst_name, $subst_do_create_now, $xerr['SUBST']) = $this->_substManage($sqlo, $datafields);
        list($dev_id, $dev_name, $xerr['DEV'] ) = $this->_getDevice($sqlo, $datafields);
        
        $quanti = $datafields['quantity'];
        $munit  = $datafields['measurement unit'];
        
        $unit_id = glob_elementDataGet( $sqlo, 'H_UNIT', 'NAME', $munit, 'H_UNIT_ID');
        if ( ($munit!="") && ($unit_id<=0)) {
            $xerr['UNIT']="<br><font color=red>unit not found in DB.</font><br>".$this->_tmp_empty_str;
        }
        
        if ( $quanti!=NULL and !is_numeric($quanti) ) {
            $xerr['QUANT']="<br><font color=red>quantity nmust be a number.</font><br>".$this->_tmp_empty_str;
        } else {
            $use_val['QUANTITY'] = $quanti;
        }
        
        list($flags, $xerr['FLAGS'] ) = $this->_check_flags($datafields['flags']);
        
        $notes  = $datafields['notes'];
        
        $name_add_str="";
        if ($sub_step_nr>0) {
            $name_add_str="<font color=gray>___</font>";
        }
        if ($sub_step_nr>1) {
            $name_add_str="<font color=gray>______</font>";
        }
        
        
        echo "<tr>";
        echo "<td>";
        if ($retval_tmp>=0) echo $this->sum['step_nr'];
        echo "</td>";
        
        echo "<td>$name_add_str";
        if ( $xerr['NAME']=="" ) echo $name;
        else echo "<font color=gray>$name</font> ".$xerr['NAME'];
        echo "</td>";
        
        $this->_objOut('ABSTRACT_SUBST', $subst_name, $subst_id, $xerr['SUBST'] );
        $this->_objOut('A_CHIP_READER' , $dev_name,   $dev_id,   $xerr['DEV']   );
        
        
        echo "<td>";
        if ( $xerr['QUANT']==NULL )  echo $quanti;
        else echo "<font color=gray>$quanti</font> ".$xerr['QUANT'];
        echo "</td>";
        
        echo "<td>";
        if ( $xerr['UNIT']=="" )  echo $munit;
        else echo "<font color=gray>$munit</font> ".$xerr['UNIT'];
        echo "</td>";
        
        echo "<td>$notes</td>";
        echo "<td>";
        
        echo "<td>";
        if ( $xerr['FLAGS']=="" )  echo $flags;
        else echo "<font color=gray>$flags</font> ".$xerr['FLAGS'];
        echo "</td>";
        
        echo "<td>";
        
        if ($retval_tmp>=0) {
            
            if ($subst_do_create_now>0) $this->sum['substToCreate'] ++;
            
            $do_action='insert';
            
            if ($this->options['use_step_nr']) {
                // check step exists .
                if ( gElement_exists2($sqlo, 'ABSTRACT_PROTO_STEP', array('ABSTRACT_PROTO_ID'=>$proto_id, 'STEP_NR'=>$this->sum['step_nr'])) ) {
                    $do_action='update';
                }      
            }
            
            if ( $this->go==2 ) {
                
                $argu=NULL;
                $argu['STEP_NR']=		$this->sum['step_nr'];
                $argu['MAIN_STEP_NR']=	$this->sum['main_step_nr'];
                $argu['SUB_STEP_NR']=	$sub_step_nr;
                $argu['NAME']=			$name;
                $argu['ABSTRACT_SUBST_ID']=	$subst_id;
                $argu['ABS_DEV_ID'] =	$dev_id;
                $argu['QUANTITY']=		$use_val['QUANTITY'];
                $argu['H_UNIT_ID']=		$unit_id;
                $argu['NOTES']=			$notes;
                $argu['MANDATORY_VALUES']=	$flags;
                
                if ( $subst_do_create_now and $subst_name!=NULL ) {
                    $subst_id = $this->createsubst( $sqlo, $subst_name );
                    if ($subst_id<=0) {
                        $error->set( $FUNCNAME, 1, 'Error at insert of substance: name:'.$subst_name);
                        return;
                    }
                    else $argu['ABSTRACT_SUBST_ID'] = $subst_id;
                }
                
                if ($do_action=='insert') {
                    $this->assoclib->insert( $sqlo2, $argu);
                    if ($error->Got(READONLY))  {
                        $error->set( $FUNCNAME, 1, 'Error on insert of step '.$this->sum['step_nr'] );
                        return;
                    }
                }
                if ($do_action=='update') {
                    $idarr=array('STEP_NR'=>$this->sum['step_nr']);
                    unset($argu['STEP_NR']);
                    $this->assoclib->update( $sqlo2, $argu, $idarr);
                    if ($error->Got(READONLY))  {
                        $error->set( $FUNCNAME, 5, 'Error on update step '.$this->sum['step_nr'] );
                        return;
                    }
                }
            }
            $this->sum['step_nr']++;
            $this->sum['main_step_nr']++;
        }
        
        if ( $this->go<2 ) {
            if ($retval_tmp<0) {
                echo "<font color=red>WARNING:</font> step not valid!";
            } else {
                echo "<font color=green>ready for <b>".$do_action."</b>.</font>";
                $this->sum['steps_inserted']++;
            }
        } else {
            if ($retval_tmp<0) {
                echo "<font color=red>WARNING:</font> did not ".$do_action."!";
            } else {
                echo "<font color=green>step ".$do_action." ok.</font>";
                $this->sum['steps_inserted']++;
            }
        }
        
        echo "</td>";
        echo "</tr>\n";
        $this->tab_len++;
        
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
    function import_file( &$sql, &$sql2, $tempfile, $go) {
        global $error;
        $FUNCNAME= 'import_file';
        
        $this->go = $go;
        $proto_id = $this->proto_id;
 
        $colmapp=array();
        $colmapp['NAME']		    ='name';
        $colmapp['ABSTRACT_SUBST']  ='substance';
        $colmapp['DEVICE']          ='device';
        $colmapp['QUANTITY']	    ='quantity';
        $colmapp['H_UNIT']	        ='measurement unit';
        $colmapp['NOTES']		    ='notes';
        $colmapp['MANDATORY_VALUES']='flags';
        
        
        
        $colsExpect = array (
            'step_nr'     =>array('col.name'=>'step_nr',  'req'=>1 ),
            'block'       =>array('col.name'=>'block',       'req'=>1 ),
            'name'        =>array('col.name'=>'name',        'req'=>2 ),
            'substance'   =>array('col.name'=>'substance',   'req'=>2),
            'device'      =>array('col.name'=>'device',      'req'=>1),
            'quantity'    =>array('col.name'=>'quantity',    'req'=>2),
            'measurement unit'=>array('col.name'=>'measurement unit','req'=>2),
            'notes'       =>array('col.name'=>'notes',       'req'=>2),
            'flags'       =>array('col.name'=>'flags',       'req'=>2)
        );
        
        
        
        $file_lib = new f_read_file_meta($tempfile);
        $file_lib->open_file();
        $headers  = $file_lib->get_headers();
        $header   = $headers[0];
        
        $this->encoding_key = $file_lib->get_encoding_info();
        //$this->encoding_key = $uplib->detect_encoding();
        
        $spread_lib = new fSpreadheet_ana();
        $spread_lib->set_file_header($header);
        $spread_lib->analyseHeader($colsExpect);
        if ( $error->Got(READONLY) ) {
            $type= $file_lib->get_type();
            echo 'File-Type: '.$type.' Header: '.print_r($header,1)."<br>";
            $error->set( $FUNCNAME, 1, "File-Header analysis failed.");
            return;
        }
        
        $headerColPos = $spread_lib->getHeaderCol2Pos();
        if (!$headerColPos['substance'] and !$headerColPos['substance ID']) {
            
            echo 'Header: '.print_r($header,1)."<br>";
            $error->set( $FUNCNAME, 2, 'File-Header: need at least "substance" or "substance ID".');
            return;
        }
        if ($headerColPos['substance ID']) {
            $this->useIDs = 1;
            if ($headerColPos['device']) {
                $error->set( $FUNCNAME, 3, 'File-Header: if "substance ID" is given, you must also give "device ID".');
                return;
            }
        }
        
        $this->head_col_block_has=0;
        if(array_key_exists('block',$headerColPos)) {
            $this->head_col_block_has=1;
        }
        
        if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
            
            debugOut("head_col_block_has:".$this->head_col_block_has, $FUNCNAME, 2);

        }
        
        $this->_info_out('Encoding', $this->encoding_key);
        if ( $this->options['use_step_nr'] ) {
            $this->_info_out('Use original Step_nr?', 'yes');
        }
        
        $this->tab_len=0;
        $this->sum = array();
        $this->sum['steps_inserted']= 0;
        $this->sum['substToCreate'] = 0;
        
        $this->sum['step_nr']=1;
        $this->sum['main_step_nr']=1;
        if ($this->options['old_steps_keep']) {
            if (!$this->options['use_step_nr']) {
                $this->sum['main_step_nr'] = $this->pra_info['MAIN_STEP_NR.max'] + 1;
                $this->sum['step_nr'] = $this->pra_info['STEP_NR.max'] + 1;
            }
        }
        
        
        echo "<table bgcolor=#EFEFEF cellspacing=1>\n";
        echo "<tr bgcolor=#D0D0D0>";
        echo "<td><B>step</B></td>";
        echo "<td><B>".$colmapp["NAME"]."</B></td>";
        echo "<td><B>".$colmapp["ABSTRACT_SUBST"]."</B></td>";
        echo "<td><B>".$colmapp["DEVICE"]."</B></td>";
        echo "<td><B>".$colmapp["QUANTITY"]."</B></td>";
        echo "<td><B>".$colmapp["H_UNIT"]."</B></td>";
        echo "<td><B>".$colmapp["NOTES"]."</B></td>";
        echo "<td><B>".$colmapp["MANDATORY_VALUES"]."</B></td>";
        echo "<td>&nbsp;</td></tr>";
        $tmp_empty_str = "<font color=#808000>[empty field]</font>";
        $this->_tmp_empty_str=$tmp_empty_str;
        
        if ( $go==2 ) {
            $this->assoclib = new  fAssocUpdate();
            $this->assoclib->setObj( $sql,  "ABSTRACT_PROTO_STEP", $proto_id );
        }

        while( $file_lib->read_line(0) )  {
            $this->_row_one($sql, $sql2, $file_lib, $spread_lib);
        }
        
        echo "</table>";
        
        $file_lib->close_file();
        
        $fail_num = $this->tab_len - $this->sum['steps_inserted'];
        echo "<br><B>".$this->sum['steps_inserted']."</B> valid steps.<br>";
        if ($fail_num) echo "<B>$fail_num</B> failed.<br>";
        if ( $this->sum['substToCreate'] ) echo "<B>".$this->sum['substToCreate']."</B> substances to be created.<br>";
        
        
    }
    
    function delsteps( &$sqlo ) {
        $assoclib = new  fAssocUpdate();
        $assoclib->setObj( $sqlo,  "ABSTRACT_PROTO_STEP", $this->proto_id );
        $assoclib->delMore( $sqlo );
    }
    
    function form1() {
        
        $proto_id = $this->proto_id;
        
        $initarr   = array();
        $initarr["title"]       = "Import protocol steps";
        $initarr["submittitle"] = "Upload file"; // title of submit button
        $initarr["tabwidth"]    = "AUTO";   // table-width: AUTO
        $initarr["ENCTYPE"]     = "multipart/form-data";
        $initarr["action"]    = "p.php"; 
        
        $hiddenarr = NULL; // hidden form variables
        $hiddenarr["id"]      = $proto_id; // is a required input parameter
        $hiddenarr["MAX_FILE_SIZE"] = $_SESSION['globals']["F.ASCI_TABLE.IMPORT.UPLOAD_MAX_SIZE"];
        $hiddenarr["mod"] = $this->scriptid;
        
        
        $formobj = new formc($initarr, $hiddenarr, 0);
        
        $fieldx = array ( // form-field definition
            "title"   => "Protocol file",
            "name"  => "userfile",
            "namex" => TRUE,
            "object"=> "file",
            "val"    => '',
            
        );
        $formobj->fieldOut( $fieldx ); // output the form-field
        
        $fieldx = array ( 
            "title"  => "Keep old steps?",
            "name"   => "old_steps_keep",
            "object" => "checkbox",
            "val"    => '',
            "notes"  => 'Keep old steps? Otherwise delete them! see help below'  
        );
        $formobj->fieldOut( $fieldx ); // output the form-field
        
        $fieldx = array ( // form-field definition
            "title"   => "Create unknown substances?",
            "name"  => "subst_create",
            "object"=> "checkbox",
            "val"    => '',
            "notes" => "Create substances, if not found in database"
            
        );
        $formobj->fieldOut( $fieldx ); // output the form-field
        
        
            $fieldx = array ( 
                "title"  => "Import original step_nr?",
                "name"   => "use_step_nr",
                "object" => "checkbox",
                "val"    => '',
                "notes"  => "Use original step numbers?"
                
            );
            $formobj->fieldOut( $fieldx ); 
        
        
        $formobj->close( TRUE );
        
        
    }
    
    function form2($tmpfile) {
        
        $proto_id = $this->proto_id;
        
        $initarr   = array();
        $initarr["title"]       = "Import protocol steps";
        $initarr["submittitle"] = "Import now!"; // title of submit button
        $initarr["tabwidth"]    = "AUTO";   // table-width: AUTO
        $initarr["action"]      = "p.php"; 
        
        
        $hiddenarr = NULL; // hidden form variables
        $hiddenarr["id"]           = $proto_id; // is a required input parameter
        $hiddenarr["userfile"]     = $tmpfile;
       
        $hiddenarr["short_file_name"] = $this->newName;
        $hiddenarr["mod"]          = $this->scriptid;
        
        
        foreach($this->options as $key => $val) {
            $hiddenarr["parx[".$key."]"]          = $val;
        }
        
        $formobj = new formc($initarr, $hiddenarr, 1);

        $formobj->close( TRUE );
        
        if ($this->options['old_steps_keep']) {
            echO 'INFO: Keep old steps? YES<br>';
        }
        
        
    }
    
    function help() {
        ?>
     	<br><br>
     	<?
    	htmlInfoBox( "Short help", "", "open", "HELP" );
    	?>
    	<ul>
    	<LI>Version: 06.07.2021</LI>
    	<LI>Support import of steps, when old steps on protocol exist. Do not touch the old ones.</LI>
     	
     	<br>
     	<li><a href="p.php?mod=<?php echo $this->scriptid.'&id='.$this->proto_id ; ?>&action=download"><b>Download "Import Template"</b></a></li>
    	<li>Tip: you can <b>export</b> an existing protocol as Excel-file: <b>protocol &gt; functions &gt; export steps</b></li>
    	<br>
     	<LI>sub-steps are identified by at least <b>3 space-characters</b> at the beginning of "name"</LI>
    	<LI><b>substance</b>: the import searches material (template) in the database (identified by name or ID)</LI>
    	<LI><b>device</b>: the import searches equipment in the database (identified by name or ID)</LI>
    	<LI><b>measurement units</b>: the import searches defined measurement units in the database (controlled vocabulary)</LI>
    	<LI><b>flags</b>: Optional: flags for the user on protocol edit: s: give a material; q: quantity; n: notes, d: equipment/device</LI>
    	<LI><b>block</b>: Optional: Identifies a super block-name</LI>
    	</ul>
    	<br>
    	<b>Options:</b>
    	<ul>
    	<li>If you want to MERGE new steps with old steps:
    	  <ul>
    	  <li>Use Option: "Keep old steps" and "Import original step_nr" to keep/update old STEP_NRs and insert new STEP_NRs</li>
    	  </ul>
    	</li>
    	</ul>
    	<br>
    	example file:
     	<?
     	echo "<table bgcolor=#EFEFEF cellspacing=1>\n";
    	echo "<tr bgcolor=#D0D0D0>";
    	echo "<td>name</td>";
    	echo "<td>substance</td>";
    	echo "<td>device</td>";
    	echo "<td>quantity</td>";
    	echo "<td>measurement unit</td>";
    	echo "<td>notes</td>";
    	echo "<td>flags</td>";
    	echo "</tr>";
    	
    	echo "<tr>";
    	echo "<td>blocking</td>";
    	echo "<td>Ethylenglykol</td>";
    	echo "<td></td>";
    	echo "<td>15</td>";
    	echo "<td>min</td>";
    	echo "<td>used only for assay</td>";
    	echo "<td>sn</td>";
    	echo "</tr>";
    	
    	echo "<tr>";
    	echo "<td>&nbsp;&nbsp;&nbsp;remove blocking solution</td>";
    	echo "<td>4567</td>";
    	echo "<td></td>";
    	echo "<td>2</td>";
    	echo "<td>min</td>";
    	echo "<td>&nbsp;</td>";
    	echo "<td>q</td>";
    	echo "</tr>";
    	
    	echo "<tr>";
    	echo "<td>&nbsp;&nbsp;&nbsp;denaturation</td>";
    	echo "<td>&nbsp;</td>";
    	echo "<td>ThermoCycler X5</td>";
    	echo "<td>3</td>";
    	echo "<td>min</td>";
    	echo "<td>&nbsp;</td>";
    	echo "<td></td>";
    	echo "</tr>";
    	
    	echo "<tr>";
    	echo "<td>finish</td>";
    	echo "<td>&nbsp;</td>";
    	echo "<td>&nbsp;</td>";
    	echo "<td>&nbsp;</td>";
    	echo "<td>check in</td>";
    	echo "<td>q</td>";
    	echo "</tr>";
    	
     	echo "</table>";
     	
     	
    	
    	htmlInfoBox( "", "", "close" );
    }
    
    function moveFile($userfile, $userfile_name, $userfile_size) {
    	
        $this->newName = $userfile_name;
        $tmpfile = $this->uploadLib->mvUpload2Tmp( $this->tmp_sub_dir, $this->newName, $userfile, $userfile_name, $userfile_size );
    	return ($tmpfile);
    }
    
    function getTempFile() {
        $tmpfile = $this->uploadLib->getFileFromTmp($this->tmp_sub_dir, $this->newName);
    	return ($tmpfile);
    }
    
    function set_short_file_name($short_file_name) {
        $this->newName = $short_file_name;
    }
    
    /**
     * export ...
     * @param object $sql
     * @param object $sql2
     */
    function download_template() {
        
        
        $tmp_filename = "protocol.import_template.xlsx";
        
        $exportObj = new f_exportDataFile_C('xlsx', $this->tmp_sub_dir, $tmp_filename);
    
        $stepOpt=array();
        $raw_proto_lib = new  oAbsProtoStepsExp($stepOpt);
        $head1 = $raw_proto_lib->tableStart();
        
        $headerx = array( $head1 );
        $exportObj->outputStart( $headerx );
    
        $exportObj->close();
        echo $exportObj->getDownloadText();
        
        
    }

}

class o_ABSTRACT_PROTO_import_XPL extends gPlugin {
	
	
	function register() {
		$tablename	= 'ABSTRACT_PROTO';
		$title		= 'Import protocol (abstract) step list';
		
		$this->infoarr			 = NULL;
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'obj'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_id']   = $_REQUEST['id'];
		$this->infoarr['checkid']  = 1;
		$this->infoarr["help_url"] = "o.ABSTRACT_PROTO.import.html";
		
		$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('write'), 'obj'=>array('read', 'write') )
			);
	}
	

	
	function startMain() {
		global $error;
		$FUNCNAME='startMain';
		
		$tablename = "ABSTRACT_PROTO";
		
		$sqlo = &$this->_sqlo;
		$sqlo2= &$this->_plug_regSql2();
		
		$proto_id = $_REQUEST['id'];
		$go       = $_REQUEST['go'];
		
		$action   = $_REQUEST['action'];
		$short_file_name = $_REQUEST['short_file_name'];
		$options   = $_REQUEST['parx'];
		
		echo "<ul>";
		if (empty($go)) $go = 0;
		
		$mainLib = new oAbstract_protoImpC($sqlo, $proto_id, $this->_mod, $options);
		
		if ($action=='download') {
		    $mainLib->download_template();
		    return;
		    
		}
		
		$mainLib->GoInfo($go);
		
		$sqls= "select count(1) from ABSTRACT_PROTO_STEP where ABSTRACT_PROTO_ID=".$proto_id;
		$sqlo->query($sqls);
		$sqlo->ReadRow();
		$numsteps=$sqlo->RowData[0];
		if ( $numsteps>0 and !$go) {
		    echo "<br>";
		    $this->_html->messageBox2( "warning", "Already <B>$numsteps</B> steps defined. They will be deleted!" );
		    echo "<br>";
		}
		
		if ( !$go  ) {
		    $mainLib->form1();
		    $mainLib->help();
		    return;
		}
		
		if ( $go < 2 ) {
		    
		    $userfile_size = $_FILES['userfile']['size'];
		    $userfile  = $_FILES['userfile']['tmp_name'];
		    $userfile_name = $_FILES['userfile']['name'];
		    //$userfile_type = $_FILES['userfile']['type'];
		    
		    $tmpfile = $mainLib->moveFile( $userfile, $userfile_name, $userfile_size );
		    if ($error->Got(READONLY))  {
		        return;
		    }
		    
		    $mainLib->form2($tmpfile);
		    
		}
		
		if ( $go == 2 ) {
		    
		    $mainLib->set_short_file_name($short_file_name);
		    
		    $tmpfile = $mainLib->getTempFile();
		    if ($error->Got(READONLY))  {
		        return;
		    }
		    
		    if (!$options['old_steps_keep']) {
		        $this->_html->messageBox2( "warning", "Already <B>$numsteps</B> steps defined. They will be deleted!" );
    		    $mainLib->delsteps($sqlo);
    		    if ($error->Got(READONLY))  {
    		        return;
    		    }
		    }
		}
		
		$mainLib->import_file( $sqlo, $sqlo2, $tmpfile, $go );
		if ($error->Got(READONLY))  {
		    return;
		}
		
		if (  $go == 2 ) {
		    
		    echo "<br><br>";
		    
		    cMsgbox::showBox("ok", "Done.");
		    
		    unlink ($tmpfile);
		}
		
		
	}
}
