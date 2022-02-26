<?php 
/**
 * manage the Config doc
 * @package o.LINK.TTC.inc
 * @swreq REQ:41 BEADS > REF-Beads > Target-Type CHECKER
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @created 2021-06-14
 */

require_once ('object.info.inc');
require_once 'o.PROTO.steps.inc';
require_once 'o.ABSTRACT_SUBST.proto.inc';
require_once 'o.CONCRETE_SUBST.proto.inc';
require_once 'o.S_OBJLINK.mod.inc';
require_once 'o.S_OBJLINK.subs.inc';
require_once ("glob.obj.update.inc");

require_once 'o.CONCRETE_SUBST.TTC.inc';

// some static functions for ONE_VERS_PROTO_CONF_struct
class o_LINK_TTC_one_vers {
    
    static function IND_of_STEP_STRUCT(&$step_arr, $step_nr) {
        foreach($step_arr as $ind=>$row) {
            if ($row['st']==$step_nr) {
                return $ind;
            }
        }
    }
}


class o_LINK_TTC {
    
    const S_OBJLINK_REF_key='SUA.REF';  // DOC => linked SUA
    const S_OBJ_MAC_REF_key='SUC.REF';
    const SUC_VARIO_VERSION_key='SUA.REF.version';
    const DOC_CLASS_REF='SUA_REF_config';
    const TOOL_NAME='Tschimtarga';
    const CONFIG_NAME_USER='Standard-Configuration';
    
    /**
     * 
     * @var array
     * 0=>{
          'version'=>1 
          'valid'=>0,1,
          'data': {  =>   ONE_VERS_PROTO_CONF_struct
            {SUA-ID} : {    ONE_SUA_PROTO_CONF_struct
              PROTO_NO => {  VERS_PROTO_PRA_STRUCT
                'ap'=>433,
                'steps'=> [   VERS_PROTO_PRA_STEPS_STRUCT
                   array(  
                       'st'=>$step_nr, 
                       'q'=>
                       'sua'=> 
                          ID or 
                          array of SUAs
                       'sua_opt'=> 0,1 if set to 1: SUA is OPTIONAL ...
                       'sua_link'=> 0,1 [OPTIONAL] array of ONE_VERS_PROTO_CONF_struct
                       ) 
                ]
             }
            }
          }
        }
     */
    private $config_all; // 
    
    function  __construct($sqlo, $doc_id) {
        global $varcol;
        $varcol = & Varcols::get();
        
        $this->doc_id = $doc_id;
        $this->sua_linked_id = $this->get_linked_SUA($sqlo);
        $this->link_class_id = $varcol->class_name_to_id( 'LINK', self::DOC_CLASS_REF );
        $this->curr_version = NULL;
        
        $this->reload($sqlo);
 
        $version = $this->get_released_version_no();
        if ( $version===NULL and !empty($this->config_all) ) {
            // get first version
            $first_row = current($this->config_all);
            $version = $first_row['version'];
        }
        $this->curr_version = $version;
    }
    
    static function get_data_of_STEP_STRUCT(&$step_arr, $step_nr) {
        foreach($step_arr as $row) {
            if ($row['st']==$step_nr) {
                return $row;
            }
        }
    }
    
    function show_data_struct() {
        
        echo "SHOW_VERSIONS: <br>";
        foreach($this->config_all as $row) {
            echo "- Version:".$row['version'].' Valid:'.$row['valid']." <br>";
        }
        echo " <br>";
        
        $version = $this->current_version();
        $data = $this->get_data_of_version($version);
        echo "SHOW_VERSION_DATA: version:$version: <pre>";
        print_r($data);
        echo '</pre>';
        
        
    }
    
    function reload($sqlo)  {
        $objInfoLib = new objInfoC();
        $this->doc_features = $objInfoLib->getParamsRaw ( $sqlo, 'LINK', $this->doc_id);
        
        $this->config_all=array();;
        if ($this->doc_features['vals']['DATAX']!=NULL) {
            $this->config_all = json_decode($this->doc_features['vals']['DATAX'], TRUE);
        }
    }
    
    private function get_linked_SUA($sqlo) {
        $ref_id=0;
        $ref_arr = oS_OBJLINK_subs::getChiObjByTabOrKey($sqlo, 'LINK', $this->doc_id, 'ABSTRACT_SUBST', self::S_OBJLINK_REF_key);
        if(!empty($ref_arr)) {
            $ref_id=current($ref_arr);
        }
        return $ref_id;
    }
    
    function get_SUA() {
        return $this->sua_linked_id;
    }
    function set_sua($sqlo, $sua_id) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $tablename='LINK';
        $objlink_lib = new oS_OBJLINK_mod();
        $objlink_lib->setObj($sqlo, $tablename, $this->doc_id);
        if ($this->sua_linked_id) {
            $objlink_lib->removeLink_by_key($sqlo, self::S_OBJLINK_REF_key);
        }
        $objlink_lib->addLink($sqlo, 'ABSTRACT_SUBST', $sua_id, self::S_OBJLINK_REF_key);
        if ($error->Got(READONLY))  {
            $error->set( $FUNCNAME, 5, 'Error on adding REF-MAA to this config.' );
            return;
        }
        $this->sua_linked_id = $sua_id;
        
        
    }
    
    function current_version() {
        return $this->curr_version;
    }
    
    function get_data_of_version($version) {
        foreach($this->config_all as $row) {
            if ($version==$row['version']) {
                return $row['data'];
            }
        }
    }
    
    function get_released_version_no() {
        foreach($this->config_all as $row) {
            if ($row['valid']==1) {
                //$version=$row['version'];
                return $row['version'];
            }
        }
    }
    function last_version() {
        $ind_arr=array_column($this->config_all, 'version');
        $Last_val = end($ind_arr);
        return $Last_val;
    }
    
    // get ONE_VERS_PROTO_CONF_struct
    function get_released_version_data() {
        foreach($this->config_all as $row) {
            if ($row['valid']==1) {
                //$version=$row['version'];
                return $row['data'];
            }
        }
    }
    
    
    private function _index_of_version($version) {
        $ind_arr = array_column($this->config_all, 'version' );
        $index = array_search( $version, $ind_arr);
        return $index;
    }
    
    /**
     * copy old data to new version
     */
    function create_new_version() {
        $last_vers=0;
        $old_data=array();
        if (empty($this->config_all)) {
            $this->config_all=array();
        } else {
            foreach($this->config_all as $index=>$row) {
                $last_vers=max($last_vers, $row['version']);
            }
            $old_data = $this->get_data_of_version($last_vers);
        }
        $new_version=$last_vers+1;
        
        $valid_flag=0;
        if (!$last_vers) {
            $valid_flag=1;
        }
        
        // copy the data ...
        $this->config_all[]=array(
            'version'=> $new_version,
            'valid'=>   $valid_flag,
            'data' =>   $old_data
        );

        $this->curr_version = $new_version;
    }
    
    // set ONE_VERS_PROTO_CONF_STRUCT
    function set_one_proto_def(&$data) {
        $ind = $this->_index_of_version($this->curr_version);
        $this->config_all[$ind]['data'] = $data;
    }
    
    function has_config() {
    }
    
    function save_config($sqlo) {
        // update REF-Bead version
        $args=array(
            'vals'=>array(
                'DATAX'=> json_encode($this->config_all)
            )
        );
        $UpdateLib = new globObjUpdate();
        $UpdateLib->update_meta( $sqlo, 'LINK', $this->doc_id, $args );
    }
    
    // return array of array('version'=>, 'valid'=> )
    function get_versions_short() {
        $versions=array();
        if(empty($this->config_all)) return $versions;
        foreach($this->config_all as $row) {
            $one_vers = array('version'=>$row['version'], 'valid'=>$row['valid'] );
            $versions[]=$one_vers;
        }
        return $versions;
    }
    
    // set valid version
    function set_valid_version($version) {
        $found=0;
        foreach($this->config_all as $index=>$row) {
            $valid_tmp=0;
            if($version==$row['version']) {
                $valid_tmp=1;
                $found=1;
            }
            $this->config_all[$index]['valid']= $valid_tmp;
        }
        if(!$found) throw new Exception('Version '.$version.' not found.');
    }
    function set_edit_version($edit_vers) {
        $this->curr_version = $edit_vers;
    }
     
    /**
     * get SQL-command for object selection
     * @param object $sqlo
     * @param int $doc_id
     * @return string
     *   OBJ_ID from ...
     */
    static function get_linked_SUC_SQL($sqlo, $doc_id) {
        // S_OBJ_MAC_REF_key;
        $sql_cmd = oS_OBJLINK_subs::sql_getMobyChiTab($sqlo, 'CONCRETE_SUBST', 'LINK', $doc_id);
        
        return $sql_cmd;
    }
}


