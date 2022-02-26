<?php
/**
 * export abstract protocol steps as HTML or CSV
 * @namespace core::obj::
 * @package obj.abstract_proto.print.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param long $id 
 *   $parx["raw"] = 1 : raw output
 *   $parx["ids"] = 1 : with subst ID
 *   $parx["step_nr"] = 1
 *   $go : 0: params, 1: export
 */
 
session_start(); 


require_once ('reqnormal.inc');
require_once ("gui/o.PROTO.stepout1.inc");
require_once ("class.filex.inc");
require_once ("export/o.ABSTRACT_PROTO.steps.inc");
require_once ('lev1/f.exportDataFile.inc');
// require_once ("down_up_load.inc");

class oAbsProtoPrintC {
    
    const SCRIPT_NAME = 'obj.abstract_proto.print';
    
    function __construct($id, $parx) {
        
    	$this->id = $id;
    	$this->parx = $parx;
    }
    
    function init( &$sql ) {
    	
    	$id = $this->id;
    	$this->access_data = access_data_get( $sql, "ABSTRACT_PROTO", "ABSTRACT_PROTO_ID", $id);
    	$sqls= "select name, notes, h_proto_kind_id from ABSTRACT_PROTO where ABSTRACT_PROTO_ID=".$id;
    	$sql->query("$sqls");
    	$sql->ReadRow();
    	
    	$this->parms["proto_name"]=$sql->RowData[0];
    	$this->parms["notes"]	  =$sql->RowData[1];
    	$this->parms["h_proto"]   =$sql->RowData[2];
    }
    
    function formshow() {
    	require_once ('func_form.inc');
    	$parx=$this->parx;
    	$initarr   = NULL;
    	$initarr["action"]      = $_SERVER['PHP_SELF'];
    	$initarr["title"]       = "Set options";
    	$initarr["submittitle"] = "Export";
    	$initarr["tabwidth"]    = "AUTO";
    
    	$hiddenarr = NULL;
    	$hiddenarr["id"]     = $this->id;
    	$hiddenarr["parx[raw]"]     = $this->parx['raw'];
    
    	$formobj = new formc($initarr, $hiddenarr, 0);
    
    	$fieldx = array ( 
    		"title" => "with IDs", 
    		"name"  => "ids",
    		"object"=> "checkbox",
    		"val"   => $parx["ids"], 
    		"notes" => "export/show IDs of substances and devices?"
    		 );
    	$formobj->fieldOut( $fieldx );
    	
    	$fieldx = array (
    	    "title" => "with Step_nr",
    	    "name"  => "step_nr",
    	    "object"=> "checkbox",
    	    "val"   => $parx["step_nr"],
    	    "notes" => "ewith Step_Nr ?"
    	);
    	$formobj->fieldOut( $fieldx );
    
    	$formobj->close( TRUE );
    	
    }
    
    function preInfo ( &$sql ) {
    	
    	$access_data = &$this->access_data;
    	$accarr = array(
    			"Creator"=>$access_data['owner'],
    			"Crea-Date"=>$access_data["crea_date"]
    				);
    	if ($access_data["mod_date"]!="") { 
    		$accarr["Modifier"] =$access_data["modifier"];
    		$accarr["Modi-Date"]=$access_data["mod_date"];
    		if ($accarr["modifier"]=="") $accarr["Modifier"]= $accarr["Creator"];
    	}
    	
    	foreach( $accarr as $key=>$val) {
    		echo "<font color=gray>".$key.":</font> ". $val. " &nbsp;&nbsp;";
    	}
    	reset ($accarr); 
    	
    	echo "<br>";
    	if ( $this->parms["notes"] != "" ) {
    		echo "<font color=gray>Notes:</font><pre>";
    		echo  htmlspecialchars ($this->parms["notes"]);
    		echo "</pre>";
    	}
    
    }
    
    
    /**
     * export ...
     * @param object $sql
     * @param object $sql2
     */
    function rawoutMeta( &$sql, &$sql2) {
    	
    	$tmpname = fileC::objname2filename( $this->parms["proto_name"] );	
    	$tmp_filename = "protocol.".$tmpname.".xlsx";

    	$exportObj = new f_exportDataFile_C('xlsx', self::SCRIPT_NAME, $tmp_filename);

    	$id = $this->id;
    	$p_options=array();
    	$proto_step_c = new  protostep( $sql, $sql2, $id, 0, 0, 0, $p_options  );
    	$step_arrayX  = $proto_step_c->getStepArray();
    	$step_count = sizeof( $step_arrayX );
    	if (!$step_count) {
    	    return;
    	}
    	
    	
    	
    	$stepOpt=array(
    	    'ids'=>$this->parx['ids'], 
    	    'step_nr'=>$this->parx['step_nr'], 
    	);
    	$raw_proto_lib = new  oAbsProtoStepsExp($stepOpt);
    	$raw_proto_lib->setProto( $id );
    	$head1 = $raw_proto_lib->tableStart();
    	
    	
    	$headerx = array( $head1 );
        $exportObj->outputStart( $headerx );
        
        $i=0;
        while ( $i< $step_count ) {
            
            $step_nr = $step_arrayX[$i];
            $row = $raw_proto_lib->outStepAbsRaw( $sql, $step_nr );
            
            $exportObj->oneRow( $row );
            
            $i++;
        }
        

        $exportObj->close();
        echo $exportObj->getDownloadText();
    	
    	
    }
    
    function stepsHtml( &$sql, &$sql2 ) {
    	$id = $this->id;
    	
    	$sqls= "select count(*) from ABSTRACT_PROTO_STEP where ABSTRACT_PROTO_ID=".$id;
    	$sql->query("$sqls");
    	$sql->ReadRow();
    	$steps=$sql->RowData[0];
    	if ( !$steps ) {
    		echo "<font color=gray>... No steps defined.</font>";
    		return;
    	} 
    	
    	$options=array();
    	$options["out_type"]="printer";
    	$options["absSubstIdShow"] = 1;
    	if ($this->parx['step_nr']) $options['showIntStepNo'] = 1;
    	
    	$proto_step_c = new  protostep( $sql, $sql2, $id, 0, 0, 0, $options  );
    	$proto_step_c->tableOut($sql);	
    	
    }

}
		
/***********************************************************/


$sql = logon2( $_SERVER['PHP_SELF'] );
$sql2 = logon2( $_SERVER['PHP_SELF'] );
global $error;
$error = & ErrorHandler::get();

$id=$_REQUEST['id'];
$parx=$_REQUEST['parx'];
$go = $_REQUEST['go'];

$showHtml = 1;
if ( $parx["raw"]>0 ) $showHtml = 0;


$title ="export steps as html or excel";
$tablename="ABSTRACT_PROTO";
$infoarr = array(); 
$infoarr["title"] = $title;
$infoarr["title_sh"] = 'export steps';
$infoarr["form_type"]= "obj";
$infoarr["obj_name"] = $tablename;
$infoarr["obj_id"]   = $id;
$infoarr["checkid"]  = 1;
$mainLib = new  oAbsProtoPrintC($id, $parx);


$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);
echo "<ul>\n";


$o_rights = access_check( $sql, "ABSTRACT_PROTO", $id );	
if ( !$o_rights["read"] ){
	echo "<B>WARNING:</B> You do not have read permission on this object!<br>\n";
	return -1;
}

if (!$go) {
	echo '<ul>';
	$mainLib->formshow();
	echo '</ul>';
	$pagelib->htmlFoot();
}

$mainLib->init( $sql );

if (!$showHtml) {
	$mainLib->rawoutMeta($sql, $sql2);
} else {

    $mainLib->preInfo($sql);
    $mainLib->stepsHtml( $sql, $sql2 );
}

echo "</ul>\n";
$pagelib->htmlFoot();

