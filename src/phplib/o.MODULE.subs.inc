<?php
require_once ("f.assocUpdate.inc");

/**
 * analyse one MODULE
 * @package o.MODULE.subs.inc
 * @author  Steffen Kube (steffen@blinx-dx.com)
 */ 
class oMODULE_one {
    
    const TYPE_MISC    =1; // MISC config
    const TYPE_TRIGGER =2; // workflow
    const TYPE_PLUGIN  =3; // GUI-Plugin config 
    
    const keyFamDefs=array(
        'flow.def' =>array('notes'=>'Workflow transition graph', 'value'=>''),
        'state.allowGrp'=>array('notes'=>'allow only these groups, to set this new status;', 'value'=>'group-IDs, separated by komma'),
        'state.project' =>array('notes'=>'link object to this project', 'value'=>'project-ID'),
        'state.unlink'  =>array('notes'=>'unlink object from this project', 'value'=>'project-ID'),
        'state.emailuser' =>array('notes'=>'send email to user', 'value'=>'nicknames, komma separated or "OBJ.creator"'),
        'state.rights' =>array('notes'=>'give rights to these groups', 'value'=>'group-right mask'),
        'main.group'    =>array('notes'=>'main worklflow group', 'value'=>'ID of user group'),
    );

	/**
	 *
	 * @param string $baseDir point to Partisan-BASE-Dir; e.g. /opt/partisan
	 */
	function __construct($mod_id=0) {
	    $this->mod_id = $mod_id;
	}

	function get_meta_by_key($sqlo, $key) {
	    
	    if (!$this->mod_id) return;
	    
	    $value = glob_elementDataGet2($sqlo, 'MOD_META', array('MXID'=>$this->mod_id, 'KEY'=>$key ), 'VALUE' );
	    return $value;
	}
	
	/**
	 * save one KEY/VAL pair in MOD_META
	 * @param object $sqlo
	 * @param string $key
	 * @param string $val
	 */
	function save_meta_val($sqlo, $key, $val) {
	    
	    if (!$this->mod_id) return;
	    
	    $assoclib = new  fAssocUpdate();
	    $optarr = array('noAccCheck'=>1);
	    $assoclib->setObj( $sqlo, 'MOD_META', $this->mod_id , $optarr);
	    $idarr=array('KEY'=>$key);
	    $argu=array('VALUE'=>$val);
	    $assoclib->updateInsert($sqlo, $argu, $idarr);
	}

    // get family by KEY
	static function get_family_by_key($key) {
	    $keyFamily = $key;
	    $keyfamArr = explode('.',$key);
	    if (is_array($keyfamArr)) {
	        // if is an array, e.g. 'state.released.rights'
	        $keyFamily=$keyfamArr[0].'.'.$keyfamArr[2];
	    }
	    
	    if ($key == 'flow.def')   $keyFamily = $key;
	    if ($key == 'main.group') $keyFamily = $key;
	    
	    return $keyFamily;
	}
	static function get_notes_of_family($family) {
	    return self::keyFamDefs[$family];
	}
}

