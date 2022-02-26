<?php
/**
 * [ProtoQuant] functions
  	- e.g. sample tracking: show protocol steps, where the substance is used
 * @package o.PROTO.quant.inc
 * @swreq UREQ:0001578: o.PROTO > ProtoQuant : Analyse protocol steps 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
    
 * @var array pq_one_config_STRUCT : contains the data for one visualiation configuration

   	   "table"      => 'EXP' or 'CONCRETE_SUBST' => destination table	
   	   "abs_obj_id" => ID of main abstract object
   	   'version'    => 1.0  -- version of config
       'format' : ['html'], 'csv', "img"
	   'select' : [OPTIONAL] the table select structure 
	       'filt' : array 
	       OLD: obj_usr_select_STRUCT -- -- see glob.objtab.searchform.inc
       ------------
      "objects":  {OBJ_STRUCT} -- index => $so_id, starts with 1
          
        	   
        [ID:1]
        table:  EXP
        obj_id: [OPTIONAL]
        from: ID: 
              -1    -- start object
              0,1,2 identifies the origin object
        src:  -- describes the data source from the source object
            NULL -- start object
            array( 
              'ty'  : 'vals', 'ass', 'objtrc' (object tracking), 'objtrc2': tracking special
              'col'=> 'CART_ID', -- column name of the mother object OR identifier for 'ty' == 'objtrc2'
              't'  => foreign table (if ty== ass or objtrc )   
              "step" => [OPTIONAL] step-nr: needed for protocol
              "abs_id"  [OPTIONAL] ID of the ABSTRACT_OBJECT which is behind THIS object, e.g. ABSTRACT_SUBST or ABSTRACT_PROTO
              "depend_id" [OPTIONAL] needed for 'ty' = 'objtrc2'
        "steps": [OPTIONAL] => array(STEPNR=>"qnsd") - define exported details of steps : STEP_COLTYPE_STRUCT
              these key-codes are translated for the protocol-output in function fProtoQ_one_obj::get_step_outarr()
    			q : quantity
    			n : notes
    			s : name of SUC
    			d : name of device
    			a : name of SUA
    			S : ID of SUC
    			D : ID of device
    			X : follow the SUC protocol => creates sub_struct: "SUB:STEP_NR" => SUB_STRUCT + field (PARENT-SUC)
        
        cols: array of {
          'name': 'BARCODE', 
               or 'vals:BARCODE' 
               or 'x.CBATCH'       -- extra  column defined by code extension
               --- future ---
               or 'cx.class_name'  -- name of TARGET extra class (need also 'CX_name':NAME)
               or 'ca.{NAME}'  -- name of ONE extra attribute
               or 'v.{NAME}'  -- name of VARIO key
          'col_nice' [OPTIONAL] : nice name of column
          'nice' : [OPTIONAL] 0,1 => get nice name of object
          'opt' : [OPTIONAL] 'optional output name'
          'ty'  : [OPTIONAL]  ['col], 'ass', 'objtrc' -- do object tracking to data['fkt']
          't'   : [OPTIONAL] tablename of derived data
          'fkt' : [OPTIONAL]  'BCBATCH_H_SOC'  -- foreign table
          'pk2_val' : [OPTIONAL]  value of second primary key (on 'ty'=='ass')
          'notes' [OPTIONAL]  'Used in Cart-Batch'
           }
          
        {OBJ_STRUCT:END}  
        --------------------------------- 
        
        [2]
        table:  CART
        obj_id: [OPTIONAL]
        from: ID:1
        src: ['vals'][CART_ID] -- describes the data source from the source object
        cols: array of {  'origin_column': 'vals:BARCODE', 'colout': 'optional output name' }
        
        [3]
        table: CONCRETE_PROTO
        from: ID:1
        data_src: 'ty'=>"ass", 't'=>'CS_HAS_PR', 'col'=>'CONCRETE_PROTO_ID', 'abs_id'=> ABSTRACT_PROT_ID ???
        cols: 
        'apid'   => 34
        "steps"  => array() see STEP_COLTYPE_STRUCT
        
        [4]
        table: CONCRETE_SUBST
        from: ID:3
        src: 'ty'=>'ass', 't'=>'CS_HAS_PR', 'col'=>'CONCRETE_SUBST_ID', 'step'=>3, 'abs_id'=> SUA_ID
        cols: array of {
          'origin_column': 'vals:NAME', 
          'colout': 'optional output name'
           }
           

        [5]
        table: EXP
        from: ID:3
        src:   
            [ty]  => objtrc
            [objtrc_x] = 'pk'=>'CONCRETE_PROTO_ID', 't'=>'CONCRETE_PROTO_STEP, 'sea_col'=>CONCRETE_SUBST_ID
            [col] => CONCRETE_PROTO_ID
            [t]   => EXP_HAS_PROTO
            [col_nice] => Related Cart-Batch
        cols:
        each object stores its data in OBJCACHE
          'steps'
          'vals'
          'obj'
        
       
                
     ---- OLD ----
     "children" : array of "ID" => {SUB_STRUCT} (without children)
		
    	   {SUB_STRUCT} --- index = "APID:STEP"
    		"obj_id" => optional : abstract ID of abstract object, e.g. SUA-ID
    		"apid"     => ID of protocol for details, 
    		"steps"    => array(STEPNR=>"qnsd") - define exported details of steps
    			q : quantity
    			n : notes
    			s : name of SUC
    			d : name of device
    			S : ID of SUC
    			D : ID of device
    			X : follow the SUC protocol => creates sub_struct: "SUB:STEP_NR" => SUB_STRUCT + field (PARENT-SUC)
    		
       	   {SUB_STRUCT:END}      	  
            
 */
require_once 'f.sql_query.inc';
require_once ("f.visu_list.inc"); 
require_once 'class.filex.inc';
require_once ("lev1/f.exportDataFile.inc");
require_once 'o.S_VARIO.mod.inc';
require_once 'o.S_VARIO.subs.inc';
require_once ("f.objview.inc");
require_once 'o.proj.profile.inc';
require_once 'o.PROJ.subs.inc';
// require_once 'gui/glob.objtab.searchform.inc';
require_once 'gui/glob.objtab.filter_GUI.inc';

require_once ("o.PROTO.quant_help.inc");

/**
 * DOCUMENT related methods
 */
class fQuant_doc {
    
    private static $VARIO_KEY_PARAMS='o.PROT.quant.params';
    private static $PROFILE_DIR_NAME='Protoquant';
    
    /**
     * get analysis parameters from document
     * @param object $sql
     * @param int $docid
     * @return  array pq_one_config_STRUCT $params
     */
    static function get_ana_params($sqlo, $docid) {
        
        $vario_read   = new oS_VARIO_sub('LINK');
        $params_ser = $vario_read->getValByKey($sqlo, $docid, self::$VARIO_KEY_PARAMS);
        
        if (strlen($params_ser)) {
            $params = json_decode($params_ser, 1); // TRY JSON-decoding
            if (!is_array($params)) {
                $params = unserialize($params_ser);  // try PHP-decoding
            }
        }
        
        return $params;
    }
    
    /**
     * save analysis parameters in document
     * save params in file:
     * @param object $sql
     * @param int $docid
     * @param array pq_one_config_STRUCT $params
     * @return
     */
    static function save_ana_params($sqlo, $docid, $params) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $variolib  = new oS_VARIO_mod();
        
        $variolib->setObject($sqlo, 'LINK', $docid);
        if ($error->Got(READONLY))  {
            $error->set( $FUNCNAME, 1, 'Error on save parameters.' );
            return;
        }
 
        $params_ser_json = json_encode($params);
        
        $variolib->updateKeyVal($sqlo, self::$VARIO_KEY_PARAMS, $params_ser_json);
    }
    
    /**
     * get all PROTOQUANT docs of user profile project
     * @param object $sqlo
     */
    static function get_docs_of_profile($sqlo) {
        $proj_lib = new profile_funcs();
        $profile_proj_id  = $proj_lib->getProj($sqlo,  self::$PROFILE_DIR_NAME, "CREATE" );

        $docarr = array();
        $sql_after = cProjSubs::getTableSQLInSort( $profile_proj_id, 'LINK' );
        $sqlsel = "LINK_ID, NAME from ".$sql_after;
        $sqlo->Quesel($sqlsel);
        while ( $sqlo->ReadRow() ) {
            $docarr[] = $sqlo->RowData[0];
        }
        return $docarr;
    }
}

class fQuant_obj_ABS {
    
    protected $protoquant_lib;
    protected $sqlAfter;
    
    function set_table($tablename) {   
        $this->tablename=$tablename;
    }
    
    function init_quant_loop( oProtoQuantC &$pquant_obj, &$sqlo ) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $this->protoquant_lib = &$pquant_obj;
        
        $tablename=$this->tablename;
        
        $base_table = $pquant_obj->get_globset_baseval('table');
        $select_arr = $pquant_obj->get_globset_baseval('select');
        if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
            $tmp = $pquant_obj->get_globset();
            echo "DDD:$FUNCNAME (231): tablename:$tablename select_arr:".print_r($select_arr,1)."<br>\n";
            echo "DDD2:globset: ".print_r($tmp,1)."<br>";  
        }
        
        if(is_array($select_arr) and $base_table==$tablename) {  
            
            $sea_help       = new g_objtab_filter_GUI($tablename, $select_arr, '');
            
            $sea_help->create_SQL($sqlo);
            $this->sqlAfterNoOrd = $sea_help->get_sqlAfter();
            $this->sqlAfter      = $sea_help->get_sqlAfterSort();
            
            $sql_nice = $sea_help->get_cond_nice($select_arr);
            
            $sqlsel = 'count(1) from '.$this->sqlAfterNoOrd;
            $sqlo->Quesel($sqlsel);
            $sqlo->ReadRow();
            $cnt = $sqlo->RowData[0];
            echo 'Condition set. Selected objects: <b>'.$cnt.'</b> <span style="color:gray">Condition: '.$sql_nice.'</span><br>'."\n";
            
            if (!$cnt) {
                $error->set( $FUNCNAME, 10, 'No objects selected in condition.' );
                return;
            }
            
        } else {
            
            $utilLib = new fSqlQueryC($tablename);
            $sqlopt=array();
            $sqlopt["order"] = 1;
            $this->sqlAfter       = $utilLib->get_sql_after( $sqlopt);
            $this->sqlAfterNoOrd  = $utilLib->get_sql_after( );
            
            if ($utilLib->get_sql_info()==NULL ) {
                $error->set( $FUNCNAME, 20, 'No objects selected in condition.' );
                return;
            }
        }
    }
    
    /**
     * @return array
     * COL_DEF_STRUCT
        'name' => 'Sensiflow_value',   # column KEY
        'col_nice'=> 'Sensiflow value of Cart',  # nice name of column
        'nice' => 1,                 # allows NICE-name checkbox on feature selection
        'ty'     => 'objtrc',        // type of column: 'fea', 'obtrc', 'objtrc2' 
        'dty'    => destination table action type => for the object finding : 'fea', 'obtrc', 'objtrc2' 
        'fk_col' => 'SOCKET_ID',     // on object tracking: look for this column in BCBATCH_H_SOC
        't'      => 'BCBATCH',       // the destination table
        'fkt'    => 'BCBATCH_H_SOC', // the table for the object tracking
        'notes'  => 'Used in Cart-Batch'  # notes for column
     */
    function get_columns() { }
    function get_objtrc2_obj() {}
    function get_objtrc2_data() {}
    function getAbsProtos($sqlo) {}
}

/**
 * manage one OBJ_STRUCT
 * @author skube
 *
 */
class oPQ_OBJ_STRUCT {
    
    private $OBJ_STRUCT;
    
    /**
     * 
     * @param string $tablename
     * @param int $src_os_id
     * @param array $data_src
     *   'type':'features', 
     *   'col'=>'CART_ID'
     */
    function create($tablename, $src_os_id, $data_src) {
        
        $result = array(
            'table'=> $tablename,
            'from' => $src_os_id,
            'src' => $data_src,
            'cols' => array()
        );
        
        $this->OBJ_STRUCT = $result;
    }
    
    function get_structure() {
        return $this->OBJ_STRUCT;
    }
}

/**
 * manage the sub structure {SUB_STRUCT}
 * @author skube
 
 */
class oProtoQ_substr {
    
    function set_SUB_STRUCT( $SUB_STRUCT ) {
        $this->SUB_STRUCT = $SUB_STRUCT;
    }
    
    /**
     * - get a_proto_params
     - order steps by MAIN_STEP_NR
     RETURN: $this->aProtoInfo[A_PROTO_ID] =
     array("NAME", "features"=> array(STEP_NR=> array(features_per_step)) )
     TBD: is $this->globset set ? => get only defined steps
     * @param object $sql
     * @param int $aprotoid
     * @return array("NAME"=>$name, "features"=> $steparr)
     */
    function get_aproto_infos( &$sql, $aprotoid) {
        // glob_printr($this->aProtoInfo, "thisAprotoInfo");
        
        $sqls = "select NAME from ABSTRACT_PROTO where ABSTRACT_PROTO_ID=".$aprotoid;
        $sql->query($sqls);
        $sql->ReadRow();
        $name = $sql->RowData[0];
        $steparr = NULL;
        
        if ( $this->SUB_STRUCT["src"]['abs_id'] == $aprotoid ) {
            $steparr = oProtoQuant_help::_getAbsStepsSel($sql, $aprotoid,  $this->SUB_STRUCT["steps"] );
        } else {
            $steparr = oProtoQuant_help::_getAbsStepsNorm($sql, $aprotoid);
        }
        
        if ( !empty($steparr)) {
            foreach( $steparr as $num=>$val) {
                if ($val["H_UNIT_ID"]) {
                    $sqlsel = "NAME from H_UNIT where H_UNIT_ID=".$val["H_UNIT_ID"];
                    $sql->Quesel($sqlsel);
                    $sql->ReadRow();
                    $h_name = $sql->RowData[0];
                    $steparr[$num]["H_UNIT"] = $h_name;
                }
                if ($val["ABSTRACT_SUBST_ID"]) {
                    $sqlsel = "NAME from ABSTRACT_SUBST where ABSTRACT_SUBST_ID=".$val["ABSTRACT_SUBST_ID"];
                    $sql->Quesel($sqlsel);
                    $sql->ReadRow();
                    $su_name = $sql->RowData[0];
                    $steparr[$num]["ABSTRACT_SUBST"] = $su_name;
                }
            }
            reset ($steparr);
        }
        
        
        return array("NAME"=>$name, "features"=> $steparr);
    }
    
    function step_no_get() {
        $steps = $this->SUB_STRUCT['steps'];
        
        if (empty($steps)) return 0;
        $cnt=0;
        foreach($steps as $val) {
            $cnt = $cnt + strlen($val);
        }
        
        return $cnt;
    }
    
    function col_no_get() {
        $cols = $this->SUB_STRUCT['cols'];
        if (empty($cols)) return 0;
        $cnt = sizeof($cols);
        return $cnt;
    }
    
    /**
     * get name for the protocol header
     * @param object $sqlo
     * @return string
     */
    function get_obj_name($sqlo) {

        $abs_id = $this->SUB_STRUCT['src']['abs_id'];
        $obj_nice='?';
        
        if ($abs_id) { 
            $obj_nice = obj_nice_name ( $sqlo, 'ABSTRACT_PROTO',  $abs_id ).' ['.$abs_id.']';
        }

        return $obj_nice;
    }
}

/**
  main CLASS
  TBD:    - design a function: setObjCnt() -- define the expected number of objects
  Variables:	   
   $id   (subst_id)
 */
class oProtoQuantC {

/**
 * 
 * @var array $parx
 *   'docid'    : analysis parameters
 *   "format"   : ["html"], "csv", "img"
     "aprotoid"   : selected protocol (abstract)
     "aprotoDiff" : 0, 1 : ignore different abstract_protos   
     "aprotoMeas" : 0, 1 : show only kind 'Measurement'
 */
var $parx;

var $colNames;   /* DEPRECATED: only used on image; may be not needed anymore
       object-detail columns in table 
       array[NUM] => array( "nice" => "Nicename", "table" => TABLENAME ) 
       */
var $tmp_abstractProtos;  // array[] = proto_id

var $defAbsProtID;	  // the used abstract_proto; get-method: getSelectedApid()
var $cxParams;		  // data for "xParams"
var $aProtoInfo;	  /* aProtoInfo_STRUCT : saves the TARGET-abstract_proto_info
     INIT: getAprotoInfos()
	    is array of A_PROTO_ID => array(
	       "NAME"=>      $name, 
	       "features"=> array( ... step array ...
	        	NUM => array("H_UNIT"=>, "TYPE"=>(s,d,n,..), "ABSTRACT_SUBST"=>, ...
	        )
		)
		*/
var $aProtoOther;	 // array[ABS_PROTO_ID] = array ("name"=>NAME, "isMeasure"=>0,1)
private $exportObj;  // CSV export object
private $cStepArr;  /* array: data of Concrete-Protocol-Steps
		[STEP_NR] => array(
			"QUANTITY"=>, 
			"CONCRETE_SUBST_ID" => ID 
			"CONCRETE_SUBST"    => NAME, 
			'DEV'    => NAME, ...
			'DEV_ID' => ID of device
			) 
	 	*/

protected $globset;     // @var pq_one_config_STRUCT $globset set in set_docid(), initPure()
private $img_lib = NULL;
protected $MAX_OBJ_ANA; // max number of objects to be SHOWN
protected $_head_arr = array(); // header array used by "img"

/**
 * @var array
 * "table"=> correlates with $globset['table']
 * "single"=>1,2
 */
protected $quanType  = array(); 

function __construct() { 
	//global $error;
	//$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	$this->globset = array();
	
	$this->MAX_OBJ_ANA = 500;
	
	$this->debug  = $_SESSION["userGlob"]["g.debugLevel"];
	
	$this->xParamsDef = array(
			"CHIP_READER" =>  array("nice"=>"ChipReader", "table"=>"CHIP_READER")
		);
	
	$this->sub_struct_obj = new oProtoQ_substr();
	
}



// ABSTRACT CLASS
function get_apids_of_abs( &$sqlo, $abs_id ) {}

/**
 * show all
 */
function _show_STRUCT() {
    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
    echo "DEBUGX:".$FUNCNAME.':globset: objects:<br>';
    
    foreach($this->globset as $key=>$val) {
        if ($key=='objects') continue;
        if (is_array($val)) $valout=print_r($val,1);
        else $valout=$val;
        echo $key.': '.$valout."<br>\n";
    }
    
    foreach($this->globset['objects'] as $soid =>$row) {
        echo $soid. ':' . print_r($row,1)."<br>";
    }
    
}

/**
 * set config and return content
 * @param object $sqlo
 * @param int $docid
 * @param int $is_empty 0,1
 * @return array pq_one_config_STRUCT
 */
function set_docid(&$sqlo, $docid, $is_empty=0) {
    global $error;
    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
    
	if ($docid>0) {

		$this->globset = $this->get_ana_params($sqlo, $docid);
		if (empty($this->globset) and !$is_empty) {
		    // check, if doc is empty
		    $error->set( $FUNCNAME, 1, 'Config-doc (ID:'.$docid.') is empty!' );
		    return;
		}
		
		if(empty($this->quanType)) $this->quanType=array();
		$this->quanType["table"] = $this->globset['table'];
	}
	$this->parx["docid"] = $docid;
	
	return $this->globset;
}

private function _show_log($text) {
    echo "-INFO: ".$text."<br>";
}

//TEMPLATE:  function getAbsProtos( &$sql ) {} 

function get_xParamsDef() {
	return ($this->xParamsDef);
}
function get_parx() {
    return ($this->parx);
}
function get_globset() {
    return $this->globset;
}
function get_globset_baseval($key) {
    return $this->globset[$key];
}

/**
 * set MAIN features
 * @param array $main_features
 *    'version'
 *    'abs_obj_id' : ID of abstract object ...
 *    'table'
 */
function set_globset_main($main_features) {
    
    foreach($main_features as $key=>$val) {
        $this->globset[$key] = $val;
    }
    $this->globset['version'] = 1.0;
    
}

/**
 * get analysis parameters from document
 * @param object $sql
 * @param int $docid
 * @return  array pq_one_config_STRUCT $params
 */
static function get_ana_params($sqlo, $docid) {
    return fQuant_doc::get_ana_params($sqlo, $docid);
}

/**
 * save analysis parameters in document
 * save params in file:
 * @param object $sql
 * @param int $docid
 * @param array pq_one_config_STRUCT $params
 * @return 
 */
function save_ana_params($sqlo, $docid, $params) { 
    $this->_show_log('save config params.');  
    fQuant_doc::save_ana_params($sqlo, $docid, $params);
}

/**
 * save ALL parameters
 * @param object $sqlo
 */
function save_globset($sqlo) {
    $this->save_ana_params( $sqlo, $this->parx["docid"], $this->globset );
}

/**
 * user has write access to document
 * @param object $sqlo
 * @return number
 */
function has_write_access($sqlo) {
    
    $doc_id = $this->parx["docid"];
    $has_access=0;
    $o_rights = access_check($sqlo, 'LINK', $doc_id);
   
    if ( $o_rights["insert"] ) $has_access=1;
    return $has_access;
}

/**
 * init any Protoquant object
 * @param object $sql
 * @param string $table "CONTACT", "CONCRETE_PROTO"
 * @param int $single
 *   1 - single
 *   2 - list
 * @param int $objid : can be 0
 * @param array $pageinf : 
 *    "title", 
 *    "info"
 * @param array $parx
 *   'docid'
 *   "format"
 *   "aprotoid"
 *
 */
function initPure( &$sql, $table, $single, $objid, $pageinf, $parx	) {
	$this->parx  = $parx;
	
	if ($parx["docid"]>0) {
		$this->set_docid($sql, $parx["docid"]);
	}
	
	if ($this->parx["format"]=="") {
		$this->parx["format"]="html";
	}
	
	$this->pageinf  = &$pageinf;
	$this->scriptid = 'obj.concrete_proto.quant';
	$this->objid = $objid;
	
	$this->quanType = array( "table"=>$table, "single"=>$single );
	$this->title = "[ProtoQuant] - ".$pageinf["title"];
	
	if ($this->parx['format']=="img") {
		require_once ("o.PROTO.quant.img.inc");
		$this->img_lib = new oProtoQuant_imgC();
	}
	if ($this->parx['format']=="img2") {
		require_once ("o.PROTO.quant.img2.inc");
		$this->img_lib2 = new oProtoQuant_img2C();
	}
	
	$this->format = $this->parx["format"];
	$this->setFormat($this->format);
	
	$objName = "";
	if ( $this->quanType["single"]==1 ) {
	    $objName = obj_nice_name( $sql, $table, $this->objid );
	}
	
	if ($this->parx["format"]=='csv') {
	    $tablenice = tablename_nice2($table);
	    if ($this->quanType["single"]==1) $fileRaw = $objName;
	    else $fileRaw = date("Y-m-d").".". $tablenice;
	    $fileName_base = fileC::objname2filename( $fileRaw );
	    $fileName = $fileName_base.".proto";
	    $this->fileName = $fileName;
	}
	
}

function get_run_url() {
    $q_table = $this->quanType["table"];
    switch($q_table) {
        case 'EXP':
            $url= "obj.exp.prot_comp2.php?d"; // plus dummy param
            break;
        case 'CONCRETE_SUBST':
            $url= 'p.php?mod=DEF/o.CONCRETE_SUBST.pq'; //OLD: obj.concrete_subst.track3.php
            break;
    }
    return $url;
}


// inner HEAD table
function head_table($sql) {

    $tablename = $this->quanType["table"];

    echo '<table cellpadding=0 cellspacing=0 border=0><tr><td valign=top>'."\n";
    echo "<img src=\"images/f.protoquant.steine2.png\"></td><td>\n";
    echo '&nbsp;&nbsp;&nbsp;</td><td valign=top style="padding-top:6px;">'."\n";
    
    // $config_url = $_SERVER['PHP_SELF'].'?d';
    // if ($this->objid) $config_url .= '&id='. $this->objid;
    // if ($this->parx['docid']) $config_url .= '&parx[docid]='. $this->parx['docid'];
    
    $run_url = $this->get_run_url();
    $conf_url= 'obj.protocol.quant.php?action=';
    echo '<a href="'.$run_url.'"><img src="res/img/play-circle.svg" title="RUN!"> Run</a> &nbsp;&nbsp;';
    // echo "[<a href=\"".$tmp_url."crea_config&q_table=".$tablename."\">NEW configuration</a>] ";
    echo '<a href="'.$conf_url.'home&q_table='.$tablename.'"><img src="res/img/settings.svg" title="config"> Config home</a>  &nbsp;&nbsp;';
    echo '<a href="'.$conf_url.'sel_config&q_table='.$tablename.'"><img src="res/img/align-justify.svg" title="Select a config"> Select a configuration</a> ';
    // echo "[<a href=\"".$tmp_url."autoselect&q_table=".$tablename."\">AutoSelect</a>] ";
   
    // $this->settingLink($tablename); --- MODIFY config
    
    if ($this->parx["docid"]) {
        
        echo '<p style="margin-top:2px; margin-bottom:2px;"><a href="'.$run_url.'&parx[format]=html'.
            "\"><img src=\"images/but.list2.gif\" border=0> Web-format</a> &nbsp;&nbsp;&nbsp;";
        echo "<a href=\"".$run_url."&parx[format]=csv".
            "\"><img src=\"images/ic.EXCEL2.gif\" border=0> Excel-format</a> &nbsp;&nbsp;&nbsp;\n";
        
        
        //$graph_url= 'p.php?mod=DEF/o.CONCRETE_SUBST.protquant.img&docid='. $this->parx['docid'];
        //$graph_url=$config_url."&parx[format]=img";
        //         $graph_url='';
        //         echo "&nbsp;&nbsp;&nbsp;<a href=\"".$graph_url.
        //         "\"><img src=\"images/ic.24.diagram.png\" border=0 title='TBD'> Bargraph (FUTURE)</a>\n";
        
        //$graph_url = $config_url."&parx[format]=img2";
        //$graph_url='';
        // echo "&nbsp;&nbsp;&nbsp;<a href=\"".$graph_url."\">";
        echo "<img src=\"images/ic.24.diagram.png\" border=0 title='under construction'> Bargraph Interactive (FUTURE)"; // </a>
        echo '</p>'."\n";
    }
    if ($this->pageinf["info"]!=NULL) echo '<br><span style="color:gray;"><b>Info:</b> '. $this->pageinf["info"]."</span>\n";
    echo '<p style="margin-top:2px; margin-bottom:2px;" ><span style="color:gray;">Config-File: ';
    if ($this->parx['docid']) {
        $objLinkLib = new fObjViewC();
        $html_tmp = $objLinkLib->bo_display( $sql, 'LINK', $this->parx['docid'] );
        echo '<b>'.$html_tmp.'</b>';
        if ($this->globset['abs_obj_id']) {
            $abs_table=NULL;
            if ($tablename=='CONCRETE_SUBST') $abs_table='ABSTRACT_SUBST';
            if ($tablename=='EXP') $abs_table='EXP_TMPL';
            
            if ($abs_table) {
                $html_tmp = $objLinkLib->bo_display( $sql, $abs_table, $this->globset['abs_obj_id'] );
                echo ' &nbsp;&nbsp; <span style="color:gray;">Defined for</span> '.$html_tmp;
            }
        }
        
    } else {
        echo 'None';
    }
    echo '</p>'."\n";
    echo "</span>\n";
    echo "</td><tr></table>\n";
    
    echo "<ul>\n";
}

/**
 * init HTML-page or CSV page, not suitable for IMG
 * @param object $sql
 * @param string $table "CONTACT", "CONCRETE_PROTO"
 * @param int $single
 *      1 - single
		2 - list
 * @param int $objid : can be 0
 * @param array $pageinf : array("title", "info")
 * @param array $parx
 *   'docid'
 *   "format"
 *   "aprotoid"
 *   
 */
function initPage( &$sql, $table, $single, $objid, $pageinf, $parx	) {
	
	$this->initPure( $sql, $table, $single, $objid, $pageinf, $parx);
	
	$tablename = $this->quanType["table"];
	$tablenice = tablename_nice2($tablename);
	
	$infoarr			 = array();
	$infoarr["title"]    = $this->title;
	$infoarr["title_sh"] = "[ProtoQuant]";
	$infoarr["obj_name"] = $this->quanType["table"];
	$infoarr["help_url"] = 'o.CONCRETE_PROTO.quant.html';
	$infoarr["design"] = 'slim';
	
	if ($this->parx["format"]=="img2") {
		$infoarr['jsFile'] = f_html_diagram::get_js_script();
	}
	
	
	if ( $this->quanType["single"]==1 ) {
		$infoarr["obj_id"]   = $this->objid;
		$infoarr["form_type"]= "obj"; 
	} else {
	    $infoarr["form_type"] = "list";
	    if ( is_array($this->globset['select']) ) {
	       // build SQL in other way 
	       
	    } else {
    		$infoarr["obj_cnt"]   = 1;
	    }
	}
	
	$pagelib = new gHtmlHead();
	$pagelib->startPage($sql, $infoarr);
	
    
	$this->head_table($sql);


	if ( $this->quanType["single"] == 2 ) {
		//
		// check TABLE selection
		//
		$listVisuObj = new visu_listC();
		$copt = array("doCount"=>1);
		list ($stopFlag, $stopReason)= $listVisuObj->checkSelection( $sql, $tablename, $copt );
		if ( $stopFlag<0 ) {
			$errmesg = $stopReason." Please select elements of '".$tablenice."'!";
			if ($this->parx["format"]=="html") {
				htmlFoot("Attention", $errmesg);
			} else {
				echo "# Attention: ".$errmesg."\n";
				exit;
			}
		}		
	}
}

function setFormat($format) {

	$this->format = $format;

	$this->startline = "<tr bgcolor=#EFEFEF><td nowrap>";
	$this->startlineHead= "<tr bgcolor=#E0E0FF><td>";
	$this->separator  = "</td><td nowrap>"; //"\t";
	$this->separColor = "</td><td bgcolor=#D0D0EF>"; //"\t";
	$this->endline    = "</td></tr>\n";
	
	if ($format=="csv") {
		$this->startline = "";
		$this->startlineHead= "";
		$this->separator  = "\t"; //"\t";
		$this->separColor = "\t"; //"\t";
		$this->endline    = "\n";
	} 
}

function setAbstractProtos( &$abstractProtos ) {
	// FUNCTION: set abstract_protos from extern
	$this->tmp_abstractProtos = $abstractProtos;
	
}

function get_AbstractProtos(  ) {
    return $this->tmp_abstractProtos;
}




/**
 * - get a_proto_params
   - order steps by MAIN_STEP_NR
   RETURN: aProtoInfo_STRUCT
   TBD: is $this->globset set ? => get only defined steps
 * @param object $sql
 * @param int $aprotoid
 */
function getAprotoInfos( &$sqlo, $aprotoid) {
	// glob_printr($this->aProtoInfo, "thisAprotoInfo");
    $proto_info = $this->sub_struct_obj->get_aproto_infos($sqlo, $aprotoid);
    $this->aProtoInfo[$aprotoid] = $proto_info; // array("NAME"=>$name, "features"=> $steparr);
}


/**
 * OUTPUT: $dataarr
 * @param array  $a_proto_feat
 * @param string $abstractColumn: key from abstractCols_STRUCT
 * @param array  $dataarr
 */
private function _build_data_row( &$a_proto_feat, $abstractColumn, &$dataarr ) {
    
    foreach( $a_proto_feat as $valarr) {
        
       
        $valout = "";
        do {
            if ($abstractColumn=="NAME") {
                $valout = $valarr[$abstractColumn];
                break;
            }
            
            if ($abstractColumn=="TYPE") {
                // create nice name of the column type ...
                $typex  = $valarr[$abstractColumn];
                $valout = $this->_col_head_map[$typex];
                break;
            }
            
            if ($valarr["TYPE"]=="s" AND $abstractColumn=="ABSTRACT_SUBST" ) {
                $valout = $valarr[$abstractColumn];
                break;
            }
            if ($valarr["TYPE"]=="q" AND $abstractColumn=="H_UNIT" ) {
                $valout = $valarr[$abstractColumn];
                break;
            }
        } while(0);
        
        $dataarr[] = $valout;
    }
   
}

private function _prep_export($sql) {
    global $error;
    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
    
    $ana_opt=array();
    $ana_opt["aprotoDiff"] = $this->parx["aprotoDiff"];
    
    if (!is_array($this->globset['objects']) ) {
        $error->set($FUNCNAME, 1, "No object defs found.");
    }
        
    // init sub struct
    foreach($this->globset['objects'] as $obj_row) {
        $this->sub_struct_obj->set_SUB_STRUCT($obj_row);
        if ($obj_row['table']== 'CONCRETE_PROTO') {
            $apid = $obj_row['src']['abs_id'];
            $this->getAprotoInfos($sql, $apid );
        }
    }
    

    $this->_obj_out_lib  = new fProtoQ_one_obj($this->aProtoInfo, $this->globset, $ana_opt);
    
    
}

/**
 * start TABLE header; 	
 * show abstract_proto_steps
 * @param object $sql
 * @param boolean $show_header
 */
function tabHeader2( &$sql, $show_header=TRUE ) {
    global $error;
    $FUNCNAME= __CLASS__.':'.__FUNCTION__;

    $this->_head_arr = array();
	
    if ( !is_array($this->globset['objects']) ) {
        $error->set( $FUNCNAME, 1, "No object-defs found.");
		$this->show_header=TRUE;
		return;
	}
	
	$this->_prep_export($sql);
	if ($error->Got(READONLY))  {
	    $error->set( $FUNCNAME, 1, 'Error on header preparation.' );
	    return;
	}
	
	if ($show_header == TRUE) $this->show_header = FALSE;
	

	if ( $show_header == TRUE ){
		
		if ( $this->format == "csv" ) {
			$this->exportObj = new f_exportDataFile_C('xlsx', $this->scriptid, $this->fileName);
			
			$headerx = array (array('# Export of datasets'));
			if (!empty( $this->_file_headinfo)) {
			    foreach($this->_file_headinfo as $row) {
			        $headerx[] = array($row); 
			    }
			}
			
			$this->exportObj->outputStart( $headerx );
		}
		
// 		if ( $this->format == "img" ) {
// 			return;
// 		}
		
		if ( $this->format == "img2" ) {
			return;
		}
		
    	if ( $this->format == "html" ) {
    		echo "<table cellpadding=1 cellspacing=1><tr><td>";
    		echo "<table cellpadding=1 cellspacing=1 border=0>\n";
    	}
    	
	    
    	$dataarr=array();
	    
    	// FIRST row
	    foreach($this->globset['objects'] as $so_id => $proto_row) {
	        
	        $this->sub_struct_obj->set_SUB_STRUCT($proto_row);
	        $loop_table = $proto_row['table'];
	        $from = $proto_row['from'];
	        
	        if (!empty($proto_row['cols'])) {
	            
	            $table_nice_1 = tablename_nice2($loop_table);
	            $tmp_str =  '#'.$so_id.':'.$from.':'.$table_nice_1.' ID'; // <Info row object>
	            if ($this->format=='html') $tmp_str ='<b>'.$tmp_str.'</b>';
	            $dataarr[] = $tmp_str;
	                
	            // object columns
	            foreach($proto_row['cols'] as $col_row) {
	                $col_raw   = $col_row['name'];
	                if ($col_row['col_nice']) {
	                    $col_nice = $col_row['col_nice'];
	                } else {
    	                $tmp_feats = colFeaturesGet2( $loop_table, $col_raw );
    	                $col_nice  = $tmp_feats['NICE_NAME'];
	                }
	                if ($col_row['nice']) $col_nice .= '.nice';
	                $dataarr[] = $col_nice;
	                
	            }
	        }
	        
	        $step_cnt        = $this->sub_struct_obj->step_no_get();
	        
	        if ($step_cnt) {
	            // protocol steps 
    	        $first_cell_data = $this->sub_struct_obj->get_obj_name($sql);
    	        $dataarr[] = $first_cell_data; // first col
    	        for ($i=0;$i<$step_cnt; $i++) {
    	            $val_tmp='';
    	            $dataarr[] = $val_tmp;
    	        }
	        }
	        
	    }
		
		
	    if ( $this->format == "img" ) {
	        $this->_head_arr[]=$dataarr;
	    } else {
    	    $this->_linout($sql, $dataarr, 1);
	    }
	    
    	// abstractCols_STRUCT
    	$abstractCols = array("NAME"=>"name", "ABSTRACT_SUBST"=>"subst", "H_UNIT"=>"unit", "TYPE"=>"type");
    	$this->_col_head_map = oProtoQuant_help::get_types_map(); 
    	
    	//
    	// for each protocol-step-FEATURE : one line ...
    	//
    	$line_cnt=0;
    	foreach( $abstractCols as $abstractColumn=>$nicecol) {
    	
    	    $dataarr=array();

            
		    foreach($this->globset['objects'] as $proto_row) {
		        
		        $this->sub_struct_obj->set_SUB_STRUCT($proto_row);
		        $step_cnt        = $this->sub_struct_obj->step_no_get();
		        $feature_col_num = $this->sub_struct_obj->col_no_get();
		        $loop_table = $proto_row['table'];
		        
		        if ($feature_col_num) {
		            
		            $tmp_obj_html='';
		            if (!$line_cnt) {
		                if ($proto_row['src']['abs_id']) {
		                    $tmp_obj_html = oProtoQuant_help::get_abs_obj_html($sql, $loop_table, $proto_row['src']['abs_id']);
		                }
		            }
		            
		            $dataarr[] = $tmp_obj_html; // object info
    		        for ( $i=0;$i<$feature_col_num; $i++ ) {
    		            $dataarr[] = ' '; // spacer for feature column
    		        }
		        }
		        
		        if ($step_cnt) {
		            
		            $colout    = $this->parx["format"]=="html" ? '<span style="color:gray">'.$nicecol.' =&gt;</span>' : $nicecol;
		            $dataarr[] = $colout; // Info -column

    		        
    		        $loop_apid         = $proto_row['src']['abs_id'];
    		        $loop_AprotoInfo   = &$this->aProtoInfo[$loop_apid];
    		        $loop_a_proto_feat = &$loop_AprotoInfo["features"];
  
    		        $this->_build_data_row($loop_a_proto_feat, $abstractColumn, $dataarr);
		        }
		   }
    		
		   if ( $this->format == "img" ) {
		       $this->_head_arr[]=$dataarr;
		   } else {
		       $this->_linout($sql, $dataarr, 1);
		   }
    	   
    	   $line_cnt++;
    		
    	}
    	
    }
}



private function _imgColData( &$sql, $dataarr) {
	// FUNCTION: collect data for the bar-image
			
	
	$colkey   = 0; // the first cell contains the object-ID
	$nicename = $this->_getObjNice( $sql, $dataarr[$colkey] );
	
	$this->img_lib->add_datarow($dataarr, $nicename);
	
}

private function _img2ColData( &$sql, $dataarr) {
	// FUNCTION: collect data for the bar-image
		
	$pos_datasep = sizeof( $this->colNames );
	$colkey = 0;
	$nicename = $this->_getObjNice( $sql, $dataarr[$colkey] );

	$this->img_lib2->add_datarow($dataarr, $pos_datasep, $nicename);

}

/**
 * 
 * @param object $sql
 * @param string $colkey : absolute pos in outarr
 * @param string $val
 * @return string
 */
private function _getObjNice( &$sql, $objid ) {
	
    $table = $this->globset['table'];
    
    if ( $table AND $objid ) {
		
        $objxname = obj_nice_name( $sql, $table, $objid );
        if ($objxname=="")  $objxname="ID:".$objid;
		$valx = $objxname;
		return $valx;
	}
	
}

private function _obj_sub_data(&$sqlo, $val, $this_col_def) {
    
    $valx = $val;
    $tabx = $this_col_def["table"];
    
    $pkname  = PrimNameGet2($tabx);
    $colname = $this_col_def["colname"];
    
    if (strstr($colname,'.')!='') {
        $col_arr   = explode(".",$colname);
        $sub_table = $col_arr[0];
        $sub_col   = $col_arr[1];
        
        if ($sub_table=='CCT_ACCESS') {
            $val_accid = glob_elementDataGet( $sqlo, $tabx, $pkname, $val, 'CCT_ACCESS_ID');
            $valx      = glob_elementDataGet( $sqlo, 'CCT_ACCESS', 'CCT_ACCESS_ID', $val_accid, $sub_col );
        }
        
    } else {
    
        $obj_value = glob_elementDataGet( $sqlo, $tabx, $pkname, $val, $colname);
        $valx    = $obj_value;
    }
    
    if ($this_col_def['fk_table']!='') {
        $valx = obj_nice_name ( $sqlo, $this_col_def['fk_table'], $obj_value );
        if ($valx=="" AND $obj_value)  $valx = "ID:".$val;
    }
    
    if ( $this->format=="html" ) {
        if ($colname=='NAME') {
            $valx = "<a href='./edit.tmpl.php?t=".$tabx."&id=" . $val . "'>" .$obj_value. "</a>";
        }
    }
    
    return $valx;
    
}

/**
 * OUTPUT one line: html or csv OR img
 * @param object $sql
 * @param array $dataarr
 * @param number $head  // flag: == 1 it is a header-line
 */
function _linout(&$sql, $dataarr, $head = 0 ) {

		
	if ( $this->format == "img" ) {
		$this->_imgColData($sql, $dataarr);  // draw for one object
		return;
	}
	if ( $this->format == "img2" ) {
		$this->_img2ColData($sql, $dataarr);  // draw for one object
		return;
	}
	
	$linearr=array(); // needed for CSV
	
	if (count($dataarr)>0){
	
		if ( $this->format=="html" ) {
	    	if ($head==0) echo $this->startline;
	    	if ($head==1) echo $this->startlineHead;
		}
	    $cnt=0;
	    

		// $pos_datasep = sizeof( $this->colNames )-1;
	    //echo "DDDXY(1101): ".print_r($this->colNames,1)."<br>";
		
    	foreach( $dataarr as $val) {
    	    
    		$valx = $val;
			
    		/* TBD: may be do use later ...
    		$this_col_def = $this->colNames[$key];
    		if ( !$head AND $this_col_def["table"]!="" ) {
			        // nice names
				if ($val!="") {
				    $valx = $this->_obj_sub_data($sql, $val, $this_col_def);	
				}
			}
			*/
			
			if ( $this->format=="csv" ) {
				$linearr[] = $valx;
			} else {
				echo $valx;
			}
			
			
    		//if ($this->format == "html" AND $cnt==$pos_datasep) {
    		//	echo $this->separColor . "&nbsp;";
    		//}
    		
			if ( $this->format=="csv" ) {
				// nothing
			} else {
				echo $this->separator;
			}
    		
    		$cnt++;
    	}
    	
    	if ( $this->format=="csv" ) {
    		$this->exportObj->oneRow($linearr);
    	} else {
    		echo $this->endline;
    	}
    	
    }
	
}	

/**
 * show Protocol parameters (one ROW)
 * @param object $sql
 * @param int $obj_id : if root-element is EXP: this is EXP_ID
 * @param number $showNoMatching
 */
function show_one_row( &$sqlo, $obj_id_in, $showNoMatching=0 ) {
    global $error;
    // $FUNCNAME= __CLASS__.':'.__FUNCTION__;

    $dataarr   = array();
    $this->_obj_out_lib->init($obj_id_in);
    
    // init sub struct
    
    if ( $this->format == "img" ) {
        
        // take only QUANTITY values
        
        $cnt=0;
        foreach($this->globset['objects'] as $so_id => $obj_row) {
            
            $this->_obj_out_lib->set_data($sqlo, $obj_row, $so_id);
            if ($error->Got(READONLY))  {
                // $this->_set_error_text('Problem on Object-Set.');
                $error->reset();
            } else {
                $this->_obj_out_lib->ana_features( $sqlo );
                $this->_obj_out_lib->ana_protocol($sqlo );
            }
            
            // $this->_obj_out_lib->get_cache($so_id);
            
            $dataarr_tmp =  $this->_obj_out_lib->get_fea_outarr($sqlo);
            
            // just take from teh FIRST OBJECT: the first colunmn ==OBJECT array
            if (!$cnt) $dataarr = array_merge($dataarr, array($dataarr_tmp[0]) ); 
            
            if ($obj_row['steps']) {
                $dataarr_tmp0 =  $this->_obj_out_lib->get_step_outarr($sqlo);
                
                foreach($obj_row['steps'] as $step=>$types) {
                    $step_types = str_split($types);
                    // get only the QUANTITIES
                    $dataarr_tmp=array();
                    $i=0;
                    foreach($step_types as $type) {
                        if ($type=='q') $dataarr_tmp[]=$dataarr_tmp0[$i+1];
                        $i++;
                    }
                }
                // echo "DDD: so_id:$so_id STEPS: ".print_r($obj_row['steps'],1)." DATA_IN:".print_r($dataarr_tmp0,1)."<br>";
                
                
                $dataarr = array_merge($dataarr, $dataarr_tmp);
            }
            
            
            $cnt++;
        }
    } else {
    
        $cnt=0;
        foreach($this->globset['objects'] as $so_id => $obj_row) {
            
            $this->_obj_out_lib->set_data($sqlo, $obj_row, $so_id);
            if ($error->Got(READONLY))  {
                // $this->_set_error_text('Problem on Object-Set.');
                $error->reset();
            } else {
                $this->_obj_out_lib->ana_features( $sqlo );
                $this->_obj_out_lib->ana_protocol($sqlo );
            }
            
            // $this->_obj_out_lib->get_cache($so_id);
       
            $dataarr_tmp =  $this->_obj_out_lib->get_fea_outarr($sqlo);
            $dataarr = array_merge($dataarr, $dataarr_tmp);
            
            if ($obj_row['steps']) {
                $dataarr_tmp =  $this->_obj_out_lib->get_step_outarr($sqlo);
                $dataarr = array_merge($dataarr, $dataarr_tmp);
            }
            
            
            $cnt++;
        }
    }
    
	$this->_linout($sqlo, $dataarr);
}

private function _img_build_axis() {
    $axis_info=array();
    
    $step_info_arr = $this->_head_arr[4]; // fourth ROW : get type
    
    $i=0;
    foreach($step_info_arr as $type) {
        if ($type=='Quantity') {
            $name = $this->_head_arr[1][$i];
            $axis_info[]=array('NAME'=>$name, 'TYPE'=>'q');
        }
        $i++;
    }
    return $axis_info;
}


function tabClose() {

	if ( $this->format == "html" ) {
		echo "</table>";
		echo "</td></tr></table>";
	}
	if ( $this->format == "img" ) {
	    $axis_info = $this->_img_build_axis();
	    $this->img_lib->img_init($axis_info, $this->title);
		$this->img_lib->show_img();
		return;
	}
	if ( $this->format == "img2" ) {
		$this->img_lib2->img_init($this->defAbsProtID, $this->aProtoInfo, $this->title);
		$this->img_lib2->show_img();
		return;
	}
	if ( $this->format == "csv" ) {
		$this->exportObj->close();
		$text = $this->exportObj->getDownloadText();
		echo "<br><br>".$text."<br>";
	}
}

/**
 * table close on Error
 */
function tabCloseOnErr() {
	
	if ( $this->format == "html" ) {
		echo "</table>";
		echo "</td></tr></table>";
	}
	if ( $this->format == "csv" ) {
		$this->exportObj->close();
	}
	if ( $this->format == "img" ) {
		$this->img_lib = NULL;
		return;
	}
	if ( $this->format == "img2" ) {
		$this->img_lib2=NULL;
		
		return;
	}
}

function print_error($text) {
    
    if ( $this->format == "html" ) {
        echo 'ERROR'.$text."<br>";
    }
    if ( $this->format == "csv" ) {
        echo 'ERROR'.$text."<br>";
    }
    if ( $this->format == "img" ) {
        
        $this->img_lib->error_image($text);
        return;
    }
    if ( $this->format == "img2" ) {
        $this->img_lib2=NULL; //TBD:
        
        return;
    }
}

/**
 * set link in ProtoQuant
 */
function settingLink($table) {
    
	// $filex =  basename($_SERVER['SCRIPT_FILENAME']);
    // 	if ( $this->quanType["single"] == 1 ) {
    // 		$urlParam = "?id=".$this->objid;
    // 	} else {
    // 		$urlParam = "";
    // 	}
    // 	$quantParams = "";
    // 	if ( $this->parx["aprotoid"] ) {
    // 		$quantParams = "&apid=".$this->parx["aprotoid"]."&go=1";
    // 	}
    // 	$param2=NULL;
    // 	if ($this->parx['docid']) $param2 = '&docid='. $this->parx['docid'];
	
	echo ' [<a href="obj.protocol.quant.php?q_table='.$table.'">Modify configuration</a>] ';

	

}

function selectInfo(&$sql) {

	$info=array();
	$this->_file_headinfo=array();
	
	if ( $this->parx["aprotoMeas"]>0 ) $info["aprotoMeas"] = " Show only 'Measurement'-protocols";
	if ( $this->parx["aprotoDiff"]>0 ) $info["aprotoDiff"] = " Allow different abstract protos";

	switch ( $this->format ) {
		case "html":
		    
		    if (empty($info)) return;
		    
			$txtpre    = "&nbsp;|&nbsp;";
			$txtafter  = "";
			$infoAfter = "<br>\n";
			foreach( $info as $val) {
			    echo $txtpre . $val. $txtafter;
			}
			echo $infoAfter;
			
			break;
		case "csv":
		    
		    $info[] = "Selected Config-DOC-ID: ".$this->parx['docid'];
			$txtpre = "# ";

			foreach( $info as $val) {
			    $this->_file_headinfo[] =  $txtpre . $val;
			}
			break;
	}
	
	
}

/**
 * change the object of a sub structure ...
 * @param string $so_id
 * @param int $obj_id
 */
function sub_struct_ch_obj($sqlo, $so_id, $obj_id) {
    global $error;
    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
    
    if (!is_array($this->globset["objects"][$so_id])) {
        $error->set( $FUNCNAME, 1, 'SUB_STRUCT with SO_ID '.$so_id. ' not found.' );
        return;
    }
    
    $pointer = &$this->globset["objects"][$so_id];
    
    /*
    if ($pointer['table']!='ABSTRACT_SUBST') {
        $error->set( $FUNCNAME, 2, 'This method is only supported for table=ABSTRACT_SUBST.' );
        return;
    }
    */
    
    $pointer['src']['abs_id'] = $obj_id;
    unset($pointer['steps']); // remove old steps
    
}

/**
 * DELETE one sub_struct
 * @param string $key
 */
function sub_struct_unset($key) {
    unset($this->globset["objects"][$key]);
}

function get_sub_struct($so_id) {
    return $this->globset["objects"][$so_id];
}

/**
 * get MAX ID of OBJ_STRUCTs
 */
function get_sub_maxid() {
    $keys = array_keys($this->globset["objects"]);
    return max($keys);
}

function set_sub_struct($so_id, $OBJ_STRUCT) {
    
    if (!is_array($this->globset["objects"])) {
        $this->globset["objects"]=array();
    }
    $this->globset["objects"][$so_id] = $OBJ_STRUCT;
}

/**
 * search for a OBJ_STRUCT with following features
 * @param string $table
 * @param int $mo_id
 * @param array $src
 * @return $so_id
 */
function sub_struct_sea( $table, $mo_id, $src) {
    
    if (!is_array($this->globset["objects"])) {
        return 0;
    }
    $found=0;
    foreach($this->globset["objects"] as $so_id => $sub) {
        
        if ($sub['table']==$table and $sub['from']==$mo_id) {
            // extra search 
            $is_equal=1;
            foreach($src as $key=>$val) {
                if ($sub['src'][$key]!=$val) {
                    $is_equal=0;
                    break;
                }
            }
            if ( $is_equal ) {
                $found = 1;
                break;
            }
        }
    }
    if ($found) return $so_id;
    else return 0;
}

}
