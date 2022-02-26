<?php
require_once ("o.SATTACH.mod.inc"); 
require_once ('glob.obj.conabs.inc');
require_once ('o.S_VARIO.subs.inc');

/**
 * manage SATTACH uploads / updates
 * @package o.SATTACH.upload.inc
 * @author Steffen Kube
 */
class oSATTACH_modWork{
	
	/**
	 * init object
	 * @param object $sqlo
	 * @param string $tablename
	 * @param int $id
	 * @param int $go
	 */
	function __construct(&$sqlo, $tablename, $id, $go) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$this->tablename=$tablename;
		$this->objid  = $id;
		$this->go	  = $go;
		$this->editAllow = 1;
		$this->iniArgu= NULL;
		$this->doStop = 0;
		
		$this->satObj  = new cSattachSubs();
		
		$this->infox=NULL;
		// new columns exists ? ARCHIVE, XDATE, DB_USER_ID
		$newColsExist = glob_column_exists('SATTACH', 'ARCHIVE');
		$this->infox['newColsExist'] = $newColsExist;
		
		$t_rights = tableAccessCheck( $sqlo, $tablename );
		if ( $t_rights["write"] != 1 ) {
			$answer = getRawTableAccMsg( $tablename, 'write' );
			$error->set( $FUNCNAME, 1, $answer );
			return;
		}

		$o_rights = access_check($sqlo, $tablename, $id);
		if ( !$o_rights["write"])  {
			$error->set( $FUNCNAME, 1, 'no "write" access on object.' );
			return;
		}
	}
	
	/**
	 * update attachment
	 * TBD: check used other place ?
	 * TBD: check, if ARCHIVE flag is set
	 */
	private function update(&$sqlo, $rel_id, $file_info, $parx) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$tablename  = $this->tablename;
		$objid      = $this->objid;
		
		$atmodLib = new oSattachMod();
		$atmodLib->setObj($tablename, $objid);
		
		// unset forbidden KEYs for update
		$notAllowedKeys=array('DB_USER_ID', 'XDATE', 'XESIG_ID');
		foreach ($notAllowedKeys as $badKey) {
			unset( $parx[$badKey] );
		}

		if ($file_info['size']>0) {
		    debugOut('Upload attachment ...', $FUNCNAME, 1);
		    $userfile = $file_info['tmp_name'];
		    $destAttachFile = $this->satObj->getDocumentPath($tablename, $objid, $rel_id);
		    if ( !copy( $userfile, $destAttachFile ) ) {
		        $error->set( $FUNCNAME, 102, "Failed to copy uploaded file." );
		        return; 
		    }
		    $parx["XDATE"]      = date_unix2datestr(time(),1);
		}
		
		// is NAME reflected?
		if (array_key_exists('NAME', $parx)) {
			if ($parx['NAME']==NULL) {
				$error->set( $FUNCNAME, 9, 'Parameter "Name" missing.' );
				return;
			}
		}
		
		if ($parx['NAME']!=NULL) { 
			// only check, if name is given ...
			$nameexists = $atmodLib->checkNameExists($sqlo, $parx['NAME'], $rel_id);
			if ($nameexists) {
				$error->set( $FUNCNAME, 8, 'Attachment with name "'.$parx['NAME'].'" already exists.' );
				return;
			}
		}
		
		$rel_id = $atmodLib->update($sqlo, $rel_id, $parx);
		if ($error->Got(READONLY))  {
			return;
		}
		
		echo "<font color=green>... updated</font><br>";
	}
	
	function checkNameExistsSubPos($sqlo, $sub_pos, $name) {
	    return cSattachSubs::checkNameExistsSubPos($sqlo, $this->tablename, $this->objid, $sub_pos, $name);
	}
	
	
	// 
	/**
	 * do insert attachment
	 * @param object $sql
	 * @param array $file_info - uploaded file
	 *  ['tmp_name']; 
	    ['name'];
	    ['size'];
		['type'];
	   @param $parx array -- SATTACH features
	   @param array $options
	      'name_check' : [1], -1
	 */
	function insert_att(&$sql, $file_info, $parx, $options=array() ) {  
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		// $go 		= $this->go;
		$tablename	= $this->tablename;
		$id 		= $this->objid;
		

		debugOut('Create attachment ...', $FUNCNAME, 1);
	    $userfile = $file_info['tmp_name']; 
	    $userfile_name = $file_info['name'];
	    $userfile_size = $file_info['size'];
		$userfile_type = $file_info['type'];
		
		if ($userfile==NULL or !file_exists($userfile)) {
			$error->set( $FUNCNAME, 1, 'Upload failed' );
			return;
		}
		
		$FH = fopen($userfile, 'r');
	    if ( !$FH ) {
		    $error->set( $FUNCNAME, 2, 'Upload failed' );
			return;
	    }
		fclose($FH);
		if ($userfile_size<=0) {
			$error->set( $FUNCNAME, 3, 'Uploaded file is empty' );
			return;
		}
		
		
		$atmodLib = new oSattachMod();
		$opt_arg_keys=array(
		    'NOTES',
		    'SUB_POS_STR'
		);
		
		$argu = array();
		$argu["NAME"]      = $userfile_name;
		$argu["MIME_TYPE"] = $userfile_type;
		$argu["KEY"]       = $parx['KEY'];
		$argu["ARCHIVE"]   = $parx['ARCHIVE'];
		// OPTIONAL columns
		foreach($opt_arg_keys as $opt_col) {
		    if (array_key_exists($opt_col, $parx)) $argu[$opt_col] = $parx[$opt_col];
		}

		$atmodLib->setObj($tablename, $id);
		
		// for attachments with SUB_POS_STR: an other NAME check method will be used before ...
		$do_check = 1;
		if ($options['name_check']<0) $do_check = 0;
		if ($do_check) {
    		$nameexists = $atmodLib->checkNameExists($sql, $userfile_name);
    		if ($nameexists) {
    			$error->set( $FUNCNAME, 4, 'Attachment with name "'.$userfile_name.'" already exists.' );
    			return;
    		}
		}
		$rel_id     = $atmodLib->insertAtt( $sql, $argu );
		
		$attach_key = $argu["KEY"];
		
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 101, 'Attachment creation failed.' );
			return;
		}
		
		$destAttachFile = $this->satObj->getDocumentPath($tablename, $id, $rel_id);
		if ( !copy( $userfile, $destAttachFile ) ) {
			$error->set( $FUNCNAME, 102, "Failed to copy uploaded file." );
			return;
			
		}
		
		// check after action
		$plugin_info = array();
		if ($attach_key!=NULL) {
			
			$plugin_info = $this->checkForPlugin($sql, $attach_key);
			if ($error->Got(READONLY))  {
				$error->set( $FUNCNAME, 103, 'Attachment-Plugin analysis failed.' );
				return;
			}
			
			if ($plugin_info['plug_name']!=NULL) {
				$this->start_plugin($sql, $plugin_info, $rel_id);
			}
		}
	}
	
	
	/**
	 * check, if after-upload plugin exists
	 * ABS_OBJECT:VARIO: attachment.plugin.{ATTACH_KEY} = NAME_OF_PLUGIN
	 * - if plugin found for this ATTACH_KEY: return plugin-info
	 * @param object $sqlo
	 * @param string $attach_key : check for this attachment key
	 * @return array $plugin_info
	 *   ('plug_name'=>$plugin_name, 'file'=>$filename)
	 */
	function checkForPlugin(&$sqlo, $attach_key) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
		$helpConCAbs = new gConcAbsC();
		$abs_info    = $helpConCAbs->getAbsFrConc($sqlo, $this->tablename, $this->objid);
		if ($abs_info['id']<=0) return array();
		
		
		$abstract_table = $abs_info['t'];
		$varioLib    = new oS_VARIO_sub($abstract_table);
		$key         = 'attachment.plugin'; // $attach_key
		$plugin_name = $varioLib->getValByKey( $sqlo, $abs_info['id'], $key );
		
		
		if ($plugin_name==NULL) return array('info'=>'no plugin for object.');
		if ($plugin_name!=$attach_key) return array('info'=>'no plugin for this ATTACH_KEY.');
		
		// search it in LAB
		$filename = $_SESSION['s_sessVars']['AppLabLibDir'].'/obj/'.$abstract_table.'/ATPL_'.$plugin_name.'.inc';
		if (!file_exists($filename)) {
			$error->set( $FUNCNAME, 1, 'Plugin-file for plugin "'.$plugin_name.'" not found. File: '.$filename );
			return array();
		}
		
		return array('plug_name'=>$plugin_name, 'file'=>$filename);
	}
	
	/**
	 * check, if a plugin is connected with ABS_WORKLIST
	 * check for VARIO:worklistPlugin = NAME
	 * - check, if plugin is correct
	 * @param $sqlo
	 */
	function start_plugin(&$sqlo, $plugin_info, $pos) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
		
		$filename    = $plugin_info['file'];
		$plugin_name = $plugin_info['plug_name'];
	
		require_once('o.SATTACH.plugin_m.inc');
		
		// $filename: e.g. www/lab/obj/ABSTRACT_SUBST/ATPL_BTRTaq.inc
		// $plugin_name: e.g. BTRTaq
		require_once($filename); 
		$class_name = 'ATPL_'.$plugin_name;
		if (!class_exists($class_name)) {
			$error->set( $FUNCNAME, 2, 'Plugin "'.$plugin_name.'": Class "'.$class_name.'" not found.' );
			return;
		}
	
		$plugin_obj = new $class_name();
		$plugin_obj->__set_obj($this->tablename, $this->objid);
		$plugin_obj->start($sqlo, $pos);
		
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 1, 'Error on Attachment-Plugin "'.$plugin_name.'"' );
			return;
		}
		
		$this->doStop = 1; // stop to show message
		
		
		echo "<br>\n";
		// cMsgbox::showBox("ok", "Attachment plugin was executed."); 
		debugOut("Attachment plugin was executed.", $FUNCNAME, 0);
		echo "<br>\n"; 
	}
	
	
	/**
	 * manage insert/update of attachment
	 * @param object $sql
	 * @param array $file_info
	 * @param array $parx : SATTACH column features
	 * @param int $rel_id : can be NULL on action=insert
	 * @param string $action: insert or update
	 * @return -
	 */
	public function manageActions(&$sql, $file_info, $parx, $rel_id, $action) {
		global $error;
		// $FUNCNAME= __CLASS__.':'.__FUNCTION__;

		if ($action=="insert") {
			$this->insert_att($sql, $file_info, $parx);
			return;
		} 
		
		if ($action=="edit") {
		    $this->update($sql, $rel_id, $file_info, $parx);
			if ($error->Got(READONLY))  {
				return array('do'=>'showform', 'argu'=>$parx);
			}
			return;
		}
		
	}
	
	public function get_doStop() {
		return $this->doStop;
	}
}
