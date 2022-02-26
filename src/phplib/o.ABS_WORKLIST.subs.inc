<?php
require_once 'o.ABS_WORKLIST.subs2.inc';
require_once 'o.S_VARIO.subs.inc';

/**
 * common analysis methods for ABS_WORKLIST
 * 
 * @package o.ABS_WORKLIST.subs.inc
 * @swreq   UREQ:0000932 worklist management
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class oABS_WORKLIST_subs {
    
    const table='ABS_WORKLIST';
    const GUI_TYPE_VARIO_KEY='GUI-Type';
    const MAC_PROTO_VARIO_MAA='MAA.ID';
    const MAC_PROTO_VARIO_PRA='PRA.ID';
    const GUI_TYPE_MAC_PROTO='MAC-protocol';
	
	function __construct($objid=NULL) {
		if ($objid) $this->setObj($objid);
	}
	
	function setObj($objid) {
		$this->objid = $objid;
	}
	
	function get_gui_type($sqlo) {
	    $vario_lib = new oS_VARIO_sub(self::table);
	    $gui_type  = $vario_lib->getValByKey( $sqlo, $this->objid, self::GUI_TYPE_VARIO_KEY );
	    return $gui_type;
	}
	
	/**
	 * get MAC-protocol features
	 * @param object $sqlo
	 * @return array
	 *    'sua'=> ABSTRACT_SUBST_ID
	      'pra'=> array of ABSTRACT_PROTO_ID
	 *  @var  JSON-struct for table: S_VARIO_DESC (tool DEF/root/install/g.install.TABLE_OBJs)
 [
    {
        "vals": {
            "TABLE_NAME": "ABS_WORKLIST",
            "KEY": "PRA.ID",
            "EDITABLE": 1,
            "NICE":"PRA.ID",
            "NOTES": "ID of protocol template"
        }
    },
    {
        "vals": {
            "TABLE_NAME": "ABS_WORKLIST",
            "KEY": "MAA.ID",
            "EDITABLE": 1,
            "NICE":"MAA.ID",
            "NOTES": "ID of material template"
        }
    },
    {
        "vals": {
            "TABLE_NAME": "ABS_WORKLIST",
            "KEY": "GUI-Type",
            "EDITABLE": 1,
            "NICE":"GUI-Type",
            "NOTES": "[standard] or MAC-protocol"
        }
    }
]
	 */
	function get_MAC_proto_features($sqlo) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    $vario_lib = new oS_VARIO_sub(self::table);
	    $found=0;
	    $sua  = intval($vario_lib->getValByKey( $sqlo, $this->objid, self::MAC_PROTO_VARIO_MAA ));
	    if ($sua>0) {
	        if (gObject_exists ($sqlo, 'ABSTRACT_SUBST', $sua) ) $found=1;
	    }
	    if (!$found) {
	        $error->set( $FUNCNAME, 1, 'ABS_WORKLIST:'.$this->objid.' need valid ID in VARIO:'.self::MAC_PROTO_VARIO_MAA );
	        return;
	    }
	   
	    
	    $found=0;
	    $pra_str  = trim($vario_lib->getValByKey( $sqlo, $this->objid, self::MAC_PROTO_VARIO_PRA ));
	    $pra_str_arr = explode(',',$pra_str);
	    $pra_arr=array();
	    if (!empty($pra_str_arr)) {
	        foreach($pra_str_arr as $pra) {
	            $pra=intval($pra);
        	    if ($pra>0) {
        	        if (gObject_exists ($sqlo, 'ABSTRACT_PROTO', $pra) ) {
        	            $found=1;
        	            $pra_arr[]=$pra;
        	        } else {
        	            $error->set( $FUNCNAME, 1, 'ABS_WORKLIST:'.$this->objid.' VARIO:'.self::MAC_PROTO_VARIO_PRA. ': PRA:'.$pra.' not found.' );
        	            return;
        	        }
        	    }
	        }
	    }
	    if (!$found) {
	        $error->set( $FUNCNAME, 1, 'ABS_WORKLIST:'.$this->objid.' need valid ID in VARIO:'.self::MAC_PROTO_VARIO_PRA );
	        return;
	    }
	    $out=array(
	        'sua'=>$sua,
	        'pra'=>$pra_arr
	    );
	    return $out;
	}
	
	/**
	 * check, if a plugin is connected with ABS_WORKLIST
	 * check for VARIO:worklistPlugin = NAME
	 * - check, if plugin is correct
	 * @param $sqlo
	 * @return string class_name OR NULL
	 */
	function get_wl_plugin(&$sqlo) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
		$varioLib    = new oS_VARIO_sub('ABS_WORKLIST');
		$key         = 'worklistPlugin';
		$plugin_name = $varioLib->getValByKey( $sqlo, $this->objid, $key );
		
		if ($plugin_name==NULL) return NULL;
		
		// search it in LAB
		$filename = $_SESSION['s_sessVars']['AppLabLibDir'].'/obj/ABS_WORKLIST/plug.'.$plugin_name.'.inc';
		if (!file_exists($filename)) {
			$error->set( $FUNCNAME, 1, 'Plugin-file for plugin "'.$plugin_name.'" not found. File: '.$filename );
			return;
		}
		
		require_once($filename); // load plugin
		$class_name = $plugin_name.'_WRKPLG';
		if (!class_exists($class_name)) {
			$error->set( $FUNCNAME, 2, 'Plugin "'.$plugin_name.'": Class "'.$class_name.'" not found.' );
			return;
		}
		
		return $class_name;
	}
	
	function getColOrder(&$sqlo) {
		if (!$this->objid) sys_error_my(__CLASS__.': not initialized.');
		
		$sqlsel = "XPOS from ABS_WORKLIST_ENTRY ".
			" where ABS_WORKLIST_ID=".$this->objid.' order by XPOS';
		$sqlo->Quesel($sqlsel);
		while ( $sqlo->ReadRow() ) {
		    $xpos    = $sqlo->RowData[0];
		    $colOrder[]=$xpos;
		}
		return $colOrder;
	}
	
	/** 
	 * get planned attributes per column
	 * @return array[POS] array('title', 't'=>$table, 'typ'=>, 'note'=>)
	 */
	function getNameAttribs(&$sqlo) {
		if (!$this->objid) sys_error_my(__CLASS__.': not initialized.');
		
		$absEntries=NULL;
		
		$sqlsel = "XPOS, TABLE_NAME, TYPE, SH_NOTES, AUTO_COLLECT, OPTIONS from ABS_WORKLIST_ENTRY ".
			" where ABS_WORKLIST_ID=".$this->objid.' order by XPOS';
		$sqlo->Quesel($sqlsel);
		while ( $sqlo->ReadRow() ) {
		    $xpos    = $sqlo->RowData[0];
		    $table   = $sqlo->RowData[1];
		    $TYPE    = $sqlo->RowData[2];
		    $NOTES   = $sqlo->RowData[3];
		    $OPTIONS = $sqlo->RowData[5];
		    
		    $title = NULL;
			if ($OPTIONS!=NULL) {
		    	$colOptArr = oABS_WORKLIST_subs2::option2arr($OPTIONS);
		    	$title     = $colOptArr['title'];
		    }
			if ($title==NULL) {
				if ($table!=NULL) {
					$title   = tablename_nice2($table);
				}
			}
			if ($title == NULL) $title   = '??? COL:'.$xpos;
			
		    $oneRowArr=array('t'=>$table, 'title'=>$title, 'typ'=>$TYPE, 'note'=>$NOTES );
		    
		    $absEntries[$xpos]= $oneRowArr;
		}
		return $absEntries;
	}
	
	/**
	 * get XCOL-ID where TABLE_NAME=$tablename
	 * @param $sqlo
	 * @param $tablename
	 * @return int  XCOL-ID
	 */
	function getColByTablename(&$sqlo, $tablename) {
		if (!$this->objid) sys_error_my(__CLASS__.': not initialized.');
		
		$sqlsel = "XPOS from ABS_WORKLIST_ENTRY ".
			" where ABS_WORKLIST_ID=".$this->objid." and TYPE='OBJ' and TABLE_NAME=".$sqlo->addQuotes($tablename);
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$xpos = $sqlo->RowData[0];
		return $xpos;
	}
}
