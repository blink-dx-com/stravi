<?php
require_once ("f.assocUpdate.inc");

/**
 * paste ASSOC table elements to destination object[s]
 * @package glob.obj.assocpaste.inc
 
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class assocPasteC {
   var $assoc_name;
   var $idname;
   var $src_id;
   var $tablename;
   
   const TABLES_DENY=array(
       'CS_HAS_PR',
       'EXP_HAS_PROTO'
   );

function __construct() {
	$this->manilib = new  fAssocUpdate();
}
	

function _insert( &$sqlo, &$argu ) {
	global $error;
	
	$retVal		   = 0;
	// $args 		   = $sql->RowData;
	$this->manilib->insert( $sqlo, $argu );
	return;
}

function check1($tablename, $assocTabs) {
	
	
	$nice_name  = tablename_nice2($tablename);
	
	if (!sizeof($assocTabs) ) htmlFoot('ERROR', 'Object type "'.$nice_name.' has no associated elements!"');

	$tmpassocs="";
	$tmpkomma="";
	foreach( $assocTabs as $tmparr) {
		$niceassoc = $tmparr[1];
		$tmpassocs = $tmpassocs.$tmpkomma.$niceassoc;
		$tmpkomma=", ";
	}
	
	
	if ($tablename=="ABSTRACT_PROTO")  htmlInfoBox( "Warning", "For protocol (abstract) it is not usual to use this function.<br>".
	     "Please use an other function to paste steps!", "", "WARN");
	
	if (count($_SESSION['s_clipboard']) == 0) htmlFoot('INFO', 'No object '.$nice_name.' (or associated elements: '.$tmpassocs.') in clipboard.');

	$parx=array();
	$found = "";
	$th    = $_SESSION['s_clipboard'][0];
	$tmp_tabname = $th["tab"];
	do {
		$parx["src_id"] = $th["ida"];

		if ( $tmp_tabname == $tablename ) {
			if ( $parx["src_id"] <= 0 ) htmlFoot('ERROR', 'No object for '.$tablename.' in clipboard!');
			break;
		}
		
		foreach( $assocTabs as $tmparr) {
			$assoctmp  = $tmparr[0];
			$niceassoc = $tmparr[1];
			if ($tmp_tabname == $assoctmp) {
				$found = $assoctmp;
				break;
			}
		}
	
		
		if ($found == "") htmlFoot("ERROR",
			"Clipboard: no object or no associated elements found in clipboard!");
		
		$parx["assoc_name"] = $found;
		$parx["assocclip"]  = 1; // take elements from clipboard
		$go = 1; // one step ahead

	} while (0);
	
	return array($parx, $go);
}

function get_assoc_tables_allowed($sql, $tablename) {
   
    $metatab_RAW = get_assoc_tables2( $sql,$tablename );
   
    if (empty($metatab_RAW)) return;

    $metatab=array();
    foreach($metatab_RAW as $row) {
        $tab=$row[0];
        if ( !in_array($tab,self::TABLES_DENY)) {
            $metatab[]=$row;
        }
    }
    return $metatab;
}

function form( &$sql, $tablename, $id, $inf1, $inf2, $go, $parx, 
	$opt=NULL // "showDelflag"
			  // "singleObj"    0 | [1] is a single object form ???
	) {

	
	
	$tmptitle = "Select assoc table";
	if ( !$go ) {
		$sumbitTitel = "Prepare &gt;&gt;";
	} else {
		$tmptitle = "Prepare paste";
		$sumbitTitel = "Update now!";
	}
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = $tmptitle;
	$initarr["submittitle"] = $sumbitTitel;
	$initarr["tabwidth"]    = "AUTO";
	$initarr["tabnowrap"]   = 1;
	$hiddenarr = NULL;
	$hiddenarr["tablename"]  	   = $tablename;
	$hiddenarr["id"]  		 	   = $id;
	$hiddenarr["parx[src_id]"]     = $parx["src_id"];
	$hiddenarr["parx[mode]"]       = $parx["mode"];
	
	if ( $go ) {
		$hiddenarr["parx[assoc_name]"] = $parx["assoc_name"];
		$hiddenarr["parx[assocclip]"]  = $parx["assocclip"];
	}

	$metatab = $this->get_assoc_tables_allowed($sql, $tablename);
	
	if ( !sizeof($metatab) ) {
	    htmlErrorBox("Error", "table contains no associated lists");
	    return;
	}

	$formobj = new formc($initarr, $hiddenarr, $go);

	
	if ($parx["mode"]=="single") {
		$fieldx = array ( "title" => "From", "object" => "info2", "val" => $inf1[0], "notes" => $inf1[1] );
		$formobj->fieldOut( $fieldx );
	
		$fieldx = array ( "title" => "To"  , "object" => "info2", "val" => $inf2[0], "notes" => $inf2[1] );
		$formobj->fieldOut( $fieldx );
	} else {
		$fieldx = array ( "title" => "From", "object" => "info2", "val" => $inf1[0] );
		$formobj->fieldOut( $fieldx );
		
		$fieldx = array ( "title" => "insert elements"  , "object" => "info2", "val" => $inf1[1] );
		$formobj->fieldOut( $fieldx );
	}
	
	$tmpstr = "all from object";
	if ($parx["assocclip"]) $tmpstr = "clipboard";
	$fieldx = array ( "title" => "Elements from", "object" => "info2", "val" => $tmpstr, "notes" => "" );
	$formobj->fieldOut( $fieldx );

	if (!$go) {
	    
	    $initstab=array();
		foreach( $metatab as $dummy=>$tmptab) {
			$initstab[$tmptab[0]] = $tmptab[1];
		}
		
		$firsttab = key($initstab);
		

		$fieldx = array ( "title" => "Select", "name"  => "assoc_name",
				"object" => "select",
				"val"    => $firsttab, "inits" => $initstab,
				"notes"  => "select the associated list" );
		$formobj->fieldOut( $fieldx );
	} else {
		if ( $opt["showDelflag"] ) {
			$fieldx = array ( "title" => "Delete old elements?", "name" => "delflag",
				"object" => "checkbox", "val" => "", "notes" => "" );
			$formobj->fieldOut( $fieldx );
		}
	}
	$formobj->close( TRUE );
}

function srcObjInfo( &$sql, $parx ) {
	
	
	if ( !$parx["assocclip"] ) {
		$sql->query('SELECT COUNT(1) FROM '.$this->assoc_name.' WHERE '. $this->idname.' = '.
				$sql->addquotes($this->src_id));
		$sql->ReadRow();
		$srcNum_steps=$sql->RowData[0];
	} else {
		$srcNum_steps = sizeof($_SESSION['s_clipboard']);
	}
	
	return ($srcNum_steps);
}

function destObjInfo( &$sql, $dest_id) {
	// return: array ("id", "name", "access", "steps")
	
	global $error;
	
	$infoy = NULL;
	$assoc_name = $this->assoc_name;
	$idname     = $this->idname;
	$tablename  = $this->tablename;
	$src_id     = $this->src_id;
	
	$destNice      = obj_nice_name( $sql, $tablename, $dest_id);
	$infoy["id"]   = $dest_id;
	$infoy["name"] = $destNice;
	
	$o_rights      = access_check( $sql, $tablename, $dest_id);
	if ( $o_rights['insert'] ) $infoy["access"] = 1;
	
	$sql->query('SELECT COUNT(1) FROM '.$assoc_name.' WHERE '.$idname.' = '.$sql->addquotes($dest_id));
	$sql->ReadRow();
	$infoy["steps"]=$sql->RowData[0];
	
	if ( $src_id==$dest_id   ) {
		$error->set("destObjInfo", 1, 'Source and destination object must be different');
	}
	
	return ($infoy);
}

function oneObjInsert( &$sql, &$sql2, $parx, $infox, $dest_id) {
	global  $error;
	
	$assoc_name = $this->assoc_name;
	$idname     = $this->idname;
	$src_id     = $this->src_id;

	
	$this->manilib->setObj( $sql, $assoc_name, $dest_id );
	
	reset ($_SESSION['s_clipboard']);
	
	if ( $parx["delflag"]==1 ) {
		$retval = $this->manilib->delMore( $sql );
	    // echo 'Existing ',$infox["destNum_steps"],' elements on destination object deleted.<br>';
	}
	
	
	if ( $parx["assocclip"]!=1 ) {
		// take ALL elements from object
		$sql->query('SELECT * FROM '.$assoc_name.' WHERE '.$idname.' = '.$src_id);
		$cnt_elem = 0;
		while ($sql->ReadArray()) {
			$args = $sql->RowData;
			unset( $argu[$idname] ); // remove PK
			$this->_insert( $sql2, $args );
			if ($error->got(READONLY)) {
				$error->set("oneObjInsert", 1, "element No $cnt_elem");
				if (!$opt["onErrCont"]) break;	
			}
			$cnt_elem++;
		}
	} else {
		// copy elements from clip board
		$cnt_elem = 0;
		$pk_arr	  = primary_keys_get2($assoc_name);
	
		foreach( $_SESSION['s_clipboard'] as $dummy=>$tmptab) {
	
			$idarr[0]=$tmptab["ida"];
			$idarr[1]=$tmptab["idb"];
			$idarr[2]=$tmptab["idc"];
	
			$select_str  = primary_select_str( $pk_arr, $idarr,  $idname, $src_id  ); // from
			$tmpsql = 'SELECT * FROM '.$assoc_name.' WHERE '.$select_str;
			$sql->query($tmpsql);
			$sql->ReadArray();
			$args = $sql->RowData;
			unset( $argu[$idname] ); // remove PK

			$this->_insert( $sql2, $args );

			if ($error->got(READONLY)) {
				$error->set("oneObjInsert", 1, "element No $cnt_elem");
				if (!$opt["onErrCont"]) break;
			}
			$cnt_elem++;
		}
		reset ($_SESSION['s_clipboard']);
	}
	return ($cnt_elem);
}

}