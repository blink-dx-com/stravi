<?php 
require_once 'o.proj.profile.inc';
// 
/**
 * manage DOC class "user_view"
 * @package o.LINK.list_user.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 * 

 * 
 * @var array DOC_config_STRUCT
 *  't': tablename
 *  'cols' : col config [
 *     COL_NAME => 0,1
 *  ]

 *  'sort' : [
 *     {'col':'', 'dir':ASC,DESC }
 *   ]
 *  'filt' : f.sql_sel2.inc:sql_filter_STRUCT
 *  'opt':
 *     'showsets' : number of entries
 * 

 */
class oLINK_list_user {
    
    const PROJ_STD_NAME='misc';
    const DOC_CLASS='user_view';
    
    static function get_STD_folder($sqlo) {
        $projProfObj = new profile_funcs();
        $actionxc   = "CREATE";
        $projBookId = $projProfObj->getProjBySyn( $sqlo, self::PROJ_STD_NAME, $actionxc );
        return  $projBookId;
    }
    
    static function doc_load($sqlo, $docid) {
        global $varcol;
        
        $feat = glob_ObjDataGet( $sqlo, 'LINK', $docid, array('DATAX', 'EXTRA_OBJ_ID'));
        
        if (!$feat['EXTRA_OBJ_ID']) {
            throw new Exception('DOC:'.$docid.' not valid for user_view: needs class "'.self::DOC_CLASS.'".');
        }
        $classname  = $varcol->obj_id_to_class_name( $feat['EXTRA_OBJ_ID'] );
        if ($classname!=self::DOC_CLASS) {
            throw new Exception('DOC:'.$docid.' not valid for user_view: needs class "'.self::DOC_CLASS.'" (49).');
        }
        
        if ($feat['DATAX']==NULL) return;
        $json_arr = json_decode($feat['DATAX'], TRUE);
        return $json_arr;
    }
}
