<?php
require_once('insert.inc');
require_once 'glob.obj.touch.inc';
require_once ("glob.obj.update.inc");

/**
 * add/link/copy PROJECT to/from a project
 * @package obj.proj.manage.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 ERRORS: manage_proj
 1 you are not allowed to read the project in clipboard
 2 you are not allowed to write to the selected project
 3 project does not exist
 4 no right to delete
 5 Project not deletable because it is not empty.
 6 No role right to detete a project!
 */
class oProjManageC {
	
    function __construct($proj_id=NULL) {
    	if ($proj_id) {
    		$this->setProjId($proj_id);
    	}
    }
    	
    /**
     * @todo : check insert rights here ...
     * @param int $proj_id
     */
    function setProjId($proj_id) {
    	$this->proj_id = $proj_id;
    }
    
    function proj_new( &$sql ) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
    	// create a new SUB-project
    	
    	$proj_id = $this->proj_id;
    	$argu=array();
        $argu['PROJ_ID']    ='0';
        $argu['PRO_PROJ_ID']=$proj_id;
        $argu['NAME']       ='NEW_PROJECT';
      
        $prim_id = insert_row($sql, 'PROJ', $argu);
    	
        if ( !$prim_id ) {	
            $error->set( $FUNCNAME, 1, 'insert of new '.tablename_nice2('PROJ').' failed!' );
            return;
           
        } 
        return $prim_id;
    }
    
    /**
     * paste $proj_id in new MOTHER-proj $paste_id
     * @param object $sql
     * @param int    $paste_id -- ID of PROJ
     */
    function proj_paste( &$sql, $paste_id ) {	
    	global $error;
    	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
    	
    	$proj_id = $this->proj_id;		
        $o_rights = access_check($sql, "PROJ", $paste_id);
        	
        if ( !$o_rights["insert"] )  {
            $error->set( $FUNCNAME, 1, 'No insert right on MOVING-project!');
            return;
        }
            
        if ($paste_id == $proj_id) {
            $error->set( $FUNCNAME, 2, 'You tried to move a project to itself.' );
            return;
        }
        
        $args=array(
            'vals'=>array(
                'PRO_PROJ_ID'=>$proj_id
            )
        );
        $UpdateLib = new globObjUpdate();
        $up_opt    = array("noRightCheck"=>1);
        $UpdateLib->update_meta( $sql, "PROJ", $paste_id, $args, $up_opt );

		$actarr = array (
		    'd'=> array('x'=>array('fea'=>NULL) ),
		    'a'=> array('key'=>'mod')
		);
		$touch_lib = new globObjTouch();
		$touch_lib->touch($sql, 'PROJ', $proj_id, $actarr);
		
		if ($error->Got(READONLY))  {
            $error->set( $FUNCNAME, 5, 'Error on update of Proj:'.$paste_id );
            return;
        } else {
            //No output ...  echo "Moved project.<br>";
        }
        
        
    }
    
    function pj_pasteAsNew (  &$sql,       $proj_to_paste_id ) {
    # descr:  - paste a project from the clipboard as a new project (without its contents!)
    #		  - mother-project: $new_mother_proj_id
    #			
    # return: id of new project or void on error
    # errors
    #        manage_proj   1 you are not allowed to read the project in clipboard
    #                      2 you are not allowed to write to the selected project
    #                      3 project does not exist
    #   	 varcols     ...
    #        db_access     2 error in query
    	global $varcol, $error;
    	$FUNCNAME= "pj_pasteAsNew";
    	
    	$new_mother_proj_id = $this->proj_id;		
        $varcol             = & Varcols::get();
        $error              = & ErrorHandler::get();
    	
        $allowed_to_read_paste_proj = access_reading_allowed($sql, 'PROJ', $proj_to_paste_id);
        if ($error->got(READONLY)) return;
        $o_rights = access_check($sql, "PROJ", $new_mother_proj_id);
        if ($error->got(READONLY)) return;
    
        if (!$allowed_to_read_paste_proj) {
            $error->set($FUNCNAME, 1, 'You are not allowed to read the project you want to duplicate.');
            return;
        }
    
        if ($o_rights['write']) {
            do {
                $everything_fine = false;
                $old_autocommit = $sql->SetAutoCommit(false);
    	  
                $varcol_dbh = & $varcol->replace_db_handle($sql); // put varcols into same transaction
    	  
                $sql->query('SELECT name, notes, extra_obj_id FROM proj WHERE proj_id = '.$proj_to_paste_id);
                if ($error->got(READONLY)) break;
                if ($sql->ReadArray()) {
                    $paste_proj_name  = $sql->RowData["NAME"];
                    $paste_proj_notes = $sql->RowData["NOTES"];
                    $extra_obj_id     = $sql->RowData["EXTRA_OBJ_ID"];
                } else {
                    $error->set($FUNCNAME, 3, 'The project you want to duplicate does not exist.');
                    break;
                }
    
                if (!empty($extra_obj_id)) {
                    $extra_obj  = $varcol->select_by_id($extra_obj_id);
                    if ($error->got(READONLY)) break;
    	  
                    $new_extra_ob_id = $varcol->insert($extra_obj['extra_class_id'], $extra_obj['values']); // duplicate extra-obj
                    if ($error->got(READONLY)) break;
                } else {
                    $new_extra_ob_id = "";
                }  
    
                $argu = array();
                $argu['PROJ_ID']      = 0;
                $argu['PRO_PROJ_ID']  = $new_mother_proj_id;
                $argu['NAME']         = $paste_proj_name;
                $argu['EXTRA_OBJ_ID'] = $new_extra_ob_id;
                $argu['NOTES']        = $paste_proj_notes;
    
                $new_proj_id = insert_row($sql, "PROJ", $argu);
                if ($error->got(READONLY)) break;
    
                $everything_fine = true; // we are allowed to do a commit
            } while (false);
            $sql->SetAutoCommit($old_autocommit); // restore original state
            $varcol->replace_db_handle($varcol_dbh); // restore original state
            if ($everything_fine) {
                if ($old_autocommit) // commit only, if autocommit was switched on
                    $sql->commit();
                return $new_proj_id;
            } else {
                $sql->rollback();
                return;
            }
        } else {// no right to write
            $error->set($FUNCNAME, 2, 'You are not allowed to write to the selected project.');
            return;
        }
        // this point is never reached
    }

}