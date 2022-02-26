<?php
/**
 * ProtoImporter: import protocol steps to experiments, concrete_subst or other
 * @package obj.exp.imp_sample.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param   $go 
  	0
  	1 : get file
  	2 : get proto
  	3 : update
  	$parx["action"] : 
  	  'update' - update protocol
  	  'create' - create full protocol
    $parx["expunexact"] 
    $parx["aprotoid"] 
    $parx["refExpId"]    // REFERENCE-object : only go=1
    $parx["filename"]
	$tablename   = ("EXP"), "CONCRETE_PROTO", "W_WAFER", "CONCRETE_SUBST"
	
	FORMAT:
   - Header: 
    EXP	STEP_NR:[SUBST, QUANT, NOTES, INACT] - can be repeated  
   
   - where: 
      SUBST: name or [ID]
      QUANT: number
      NOTES: string (no TABs!) 
      INACT: 0|1
       
   - example:
   # experiment data
   # for Multi-Protocol-Object: need comment-line:  #Multi-PRA 
   #Multi-PRA-IDs   456         889
  	EXP		    1:SUBST		1:QUANT 2:NOTES
    typer4		DNA_samp6   2.3     jamei isset
    typer5		DNA_samp7   5.6     hossa
    typer6		DNA_samp8   7.4     karamba

 * @version $Header: trunk/src/www/pionir/obj.exp.imp_sample.php 59 2018-11-21 09:04:09Z $
 */
session_start(); 


require_once ('reqnormal.inc');
require_once ('func_form.inc');
require_once ('class.filex.inc');
require_once ("f.objview.inc");	
require_once ('o.PROTO.subs.inc');
require_once ('subs/obj.exp.imp_sample.inc');

class import_GUI {
	
	function formshow(&$sqlo, $parx) {
	
		global  $tablename;
		$initarr   = NULL;
		$initarr["action"]      = $_SERVER['PHP_SELF'];
		$initarr["title"]       = "1. Import parameter file";
		$initarr["submittitle"] = "Next &gt;&gt;";
		$initarr["tabwidth"]    = "AUTO";
		$initarr["ENCTYPE"]     = "multipart/form-data";
		$initarr["dblink"]      = 1;
	
		$hiddenarr = NULL;
		$hiddenarr["tablename"]     = $tablename;
	
		$formobj = new formc($initarr, $hiddenarr, 0);
	
		$fieldx = array ("title" => "Parameter file", "name"  => "userfile", "namex" => TRUE,
				"colspan"=>2, "val"   => "", "notes" => "CSV file", "object" => "file" );
		$formobj->fieldOut( $fieldx );
	
		if ($parx['action']=='create') {
			$fieldx = array (
					"title" => "Protocol (abstract)", 
					"name"  => "aprotoid",
					"val"   => "", 
					"notes" => "", 
					"object" => "dblink",
					"inits" => array( 'table'=>'ABSTRCAT_PROTO', 'getObjName'=>1, 'sqlo'=>&$sqlo, 'pos' =>'0', 'projlink'=> 1),
				);
			$formobj->fieldOut( $fieldx );
		}
		
		if ($tablename=="EXP" OR $tablename=="W_WAFER" OR $tablename=="CONCRETE_SUBST" OR $tablename=="CHIP_READER") {
			$fieldx = array ("title" => "name-parts", "name"  => "expunexact",
					"val"   => "", "notes" => "search for parts of name of object", "object" => "checkbox" );
			$formobj->fieldOut( $fieldx );
		}
		
		
		
		$formobj->close( TRUE );
	
	}
	 
	function form1($parx, $protoarr) {
		global  $tablename;
		$savehid=array();
		$savehid[] = "expunexact";
		$savehid[] = "refExpId" ;
	    $savehid[] = "filename" ;
	    $savehid[] = "action" ;
	
		
		$initarr   = NULL;
		$initarr["action"]      = $_SERVER['PHP_SELF'];
		$initarr["title"]       = "Select protocol (abstract)";
		$initarr["submittitle"] = "Next &gt;&gt;";
		$initarr["tabwidth"]    = "AUTO";

		$hiddenarr = $this->_hiddenSave($savehid, $parx);
		$hiddenarr["tablename"]     = $tablename;

		$formobj = new formc($initarr, $hiddenarr, 1);

		$fieldx = array ("title" => "Select protocol", "name"  => "aprotoid", "inits" => $protoarr,
				"val"   => $parx["aprotoid"], "notes" => "", "object" => "select" );
		$formobj->fieldOut( $fieldx );

		$formobj->close( TRUE );
		
	}
		
	function _hiddenSave($savehid, &$parx) {
		$hiddenarr = NULL;
		if (sizeof($savehid)) {
			foreach( $savehid as $key=>$val) $hiddenarr["parx[".$val."]"] = $parx[$val];
			reset($savehid);
		}
			return ($hiddenarr);
	}
	
	function form2($parx) {
		global  $tablename;
		
		$savehid[] = "expunexact";
		$savehid[] = "refExpId" ;
		$savehid[] = "filename" ;
		$savehid[] = "aprotoid";
		
		
		$initarr   = NULL;
		$initarr["action"]      = $_SERVER['PHP_SELF'];
		$initarr["title"]       = "Prepare database update";
		$initarr["goNext"]	    = "3";
		$initarr["submittitle"] = "Update now";
	    $initarr["tabwidth"]    = "AUTO";
		
		$hiddenarr = $this->_hiddenSave($savehid, $parx);
		$hiddenarr["tablename"]     = $tablename;
		
		$formobj = new formc($initarr, $hiddenarr, 1);
		$formobj->close( TRUE );
	}
	
	function help($tablename, $tablenice, $action='') {
	
		echo "<br>";
		htmlInfoBox( "Help", "", "open", "HELP" );
		?>
	    <ul>                    
	    <li> imports PROTOCOL-steps parameters from an Excel-File.</li>
		<li> one goal is to import the experiment => sample mapping
	    <li> checks, if NAME or ID of the object exist in database</li>
	    <li> all protocols must have the same 'protocol (abstract)'</li>
		<li> Format:</li>
	    
	    <pre>
	- Comment: # Some comment at beginning ...    
	- Header:
	    <?
	    $first_col = $tablename.'_ID';
	    if ($action=='create') {
			$first_col = $tablename;
		}
	    echo $first_col;
	    ?>	STEP_NR:[SUBST, QUANT, NOTES, INACT] - can be repeated<?
	    
		  echo "\n\n- where:\n";
		  echo "      ".$tablename.":     name or [ID] of a ".$tablenice."\n";
		  ?>
	      <?echo $tablename?>_ID:  explicitly an ID of an <?echo $tablenice."\n"?>
	      STEP_NR: step_nr of a protocol step 
	      SUBST:   name or ID of a substance (concrete); give brackets around the ID; e.g. <font color=blue>[73748]</font></li>
	      SUBST_ID:explicitly an ID of a substance (concrete)
	      QUANT:   number
	      NOTES:   string (no TABs!) 
	      INACT:   0|1  
	- Special values:
		  [NULL]: is an empty ID-value for EXP_ID of SUBST_ID (TBD: not yet implemented!)
		  </pre>
	- example 1: mixed names and IDs for experiments and substances<br>
	  	<TABLE CELLSPACING=0 COLS=4 RULES=GROUPS BORDER=1>
		<COLGROUP><COL WIDTH=47><COL WIDTH=84><COL WIDTH=66><COL WIDTH=74><COL WIDTH=66></COLGROUP>
		<TBODY>
			<TR>
				<TD WIDTH=47 HEIGHT=17 ALIGN=LEFT>EXP</TD>
				<TD WIDTH=84 HEIGHT=17 ALIGN=LEFT>    1:SUBST</TD>
				<TD WIDTH=66 HEIGHT=17 ALIGN=LEFT>1:QUANT</TD>
				<TD WIDTH=74 HEIGHT=17 ALIGN=LEFT> 2:NOTES</TD>
				<TD WIDTH=74 HEIGHT=17 ALIGN=LEFT> 4:QUANT</TD>
			</TR>
			<TR>
				<TD WIDTH=47 HEIGHT=17 ALIGN=LEFT>typer4</TD>
				<TD WIDTH=84 HEIGHT=17 ALIGN=LEFT>DNA_samp6</TD>
				<TD WIDTH=66 HEIGHT=17 >2.3</TD>
				<TD WIDTH=74 HEIGHT=17 ALIGN=LEFT>jamei isset</TD>
				<TD WIDTH=66 HEIGHT=17 >2.99</TD>
			</TR>
			<TR>
				<TD WIDTH=47 HEIGHT=17 ALIGN=LEFT>typer5</TD>
				<TD WIDTH=84 HEIGHT=17 ALIGN=LEFT>DNA_samp7</TD>
				<TD WIDTH=66 HEIGHT=17  >5.6</TD>
				<TD WIDTH=74 HEIGHT=17 ALIGN=LEFT>hossa</TD>
				<TD WIDTH=66 HEIGHT=17 >2.9</TD>
			</TR>
			<TR>
				<TD WIDTH=47 HEIGHT=17 ALIGN=LEFT>[3487]</TD>
				<TD WIDTH=84 HEIGHT=17 ALIGN=LEFT>DNA_samp7</TD>
				<TD WIDTH=66 HEIGHT=17  >5.6</TD>
				<TD WIDTH=74 HEIGHT=17 ALIGN=LEFT>hossa</TD>
				<TD WIDTH=66 HEIGHT=17 >7.9</TD>
			</TR>
			<TR>
				<TD WIDTH=47 HEIGHT=17 ALIGN=LEFT>typer6</TD>
				<TD WIDTH=84 HEIGHT=17 ALIGN=LEFT>[234]</TD>
				<TD WIDTH=66 HEIGHT=17  >7.4</TD>
				<TD WIDTH=74 HEIGHT=17 ALIGN=LEFT>karamba</TD>
				<TD WIDTH=66 HEIGHT=17 >2.123</TD>
			</TR>
		</TBODY>
	</TABLE>
	<br><br>
	
	- example 2: just with IDs<br>
	  	<TABLE CELLSPACING=0 COLS=4 RULES=GROUPS BORDER=1>
		<COLGROUP><COL WIDTH=47><COL WIDTH=84><COL WIDTH=66><COL WIDTH=74></COLGROUP>
		<TBODY>
			<TR>
				<TD WIDTH=47 HEIGHT=17 ALIGN=LEFT><?echo $tablename?>_ID</TD>
				<TD WIDTH=84 HEIGHT=17 ALIGN=LEFT>3:SUBST_ID</TD>
				<TD WIDTH=66 HEIGHT=17 ALIGN=LEFT>4:QUANT</TD>
				<TD WIDTH=74 HEIGHT=17 ALIGN=LEFT>4:INACT</TD>
			</TR>
			<TR>
				<TD WIDTH=47 HEIGHT=17 ALIGN=LEFT>236</TD>
				<TD WIDTH=84 HEIGHT=17 ALIGN=LEFT>93921</TD>
				<TD WIDTH=66 HEIGHT=17 >2.3</TD>
				<TD WIDTH=74 HEIGHT=17 ALIGN=LEFT>0</TD>
			</TR>
			<TR>
				<TD WIDTH=47 HEIGHT=17 ALIGN=LEFT>237</TD>
				<TD WIDTH=84 HEIGHT=17 ALIGN=LEFT></TD>
				<TD WIDTH=66 HEIGHT=17  ></TD>
				<TD WIDTH=74 HEIGHT=17 ALIGN=LEFT>1</TD>
			</TR>
			<TR>
				<TD WIDTH=47 HEIGHT=17 ALIGN=LEFT>239</TD>
				<TD WIDTH=84 HEIGHT=17 ALIGN=LEFT>93945</TD>
				<TD WIDTH=66 HEIGHT=17  >5.6</TD>
				<TD WIDTH=74 HEIGHT=17 ALIGN=LEFT>0</TD>
			</TR>
		</TBODY>
	</TABLE>
	<br><br>
	- example 3: Multi-protocol-templates: support more than ONE protocol per Object (e.g. MAC)<br>
	  	<TABLE CELLSPACING=0 COLS=4 RULES=GROUPS BORDER=1>
		<COLGROUP><COL WIDTH=47><COL WIDTH=84><COL WIDTH=66><COL WIDTH=74></COLGROUP>
		<TBODY>
		    <TR>
				<TD WIDTH=47 HEIGHT=17 ALIGN=LEFT>#Multi-PRA-IDs</TD>
				<TD WIDTH=84 HEIGHT=17 ALIGN=LEFT>455</TD>
				<TD WIDTH=66 HEIGHT=17 ALIGN=LEFT>455</TD>
				<TD WIDTH=74 HEIGHT=17 ALIGN=LEFT>677</TD>
			</TR>
			<TR>
				<TD WIDTH=47 HEIGHT=17 ALIGN=LEFT><?echo $tablename?>_ID</TD>
				<TD WIDTH=84 HEIGHT=17 ALIGN=LEFT>3:SUBST_ID</TD>
				<TD WIDTH=66 HEIGHT=17 ALIGN=LEFT>3:SUBST_ID</TD>
				<TD WIDTH=74 HEIGHT=17 ALIGN=LEFT>4:INACT</TD>
			</TR>
			<TR>
				<TD WIDTH=47 HEIGHT=17 ALIGN=LEFT>236</TD>
				<TD WIDTH=84 HEIGHT=17 ALIGN=LEFT>93921</TD>
				<TD WIDTH=66 HEIGHT=17 >554</TD>
				<TD WIDTH=74 HEIGHT=17 ALIGN=LEFT>0</TD>
			</TR>
			
		</TBODY>
	</TABLE>
	
	<br>
	    <li> the tool supports protocol import for these objects:<ul>
			<li> <b><a href="<?echo $_SERVER['PHP_SELF']."?tablename=EXP";?>">experiment</a></B> </li>
			<li> <B><a href="<?echo $_SERVER['PHP_SELF']."?tablename=CONCRETE_PROTO";?>">protocol</a></b> </li>
			<li> <B><a href="<?echo $_SERVER['PHP_SELF']."?tablename=CONCRETE_SUBST";?>">substance</a></b> </li>
			<li> <B><a href="<?echo $_SERVER['PHP_SELF']."?tablename=CHIP_READER";?>">device</a></b></li> 
			<li> <B><a href="<?echo $_SERVER['PHP_SELF']."?tablename=W_WAFER";?>">array-batch</a></b></li>
			</ul>
		</li>
	
	    <?
	    htmlInfoBox( "", "", "close");
	    
	}   
	
}

// ---------------------------------------------------------------------------------------
$FUNCNAME='MAIN';
$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] );
// $sql2  = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot(); 
$varcol = & Varcols::get(); 


$tablename=$_REQUEST['tablename'];
$parx=$_REQUEST['parx'];
$go=$_REQUEST['go'];

if ( $tablename == "" ) $tablename  = "EXP";

// -----------------------------------------
$mainLib = new gObjMapSample($tablename, $go);

$tabMetaInf = gObjImp_static::destTabData();

$infox=array();
$infox["nicobject"] = tablename_nice2($tablename);
$infox["niceProto"] = $infox["nicobject"] ." protocol";
if ($tablename=="CONCRETE_PROTO") { 
    $infox["niceProto"] = $infox["nicobject"];
}


$title       = 'ProtoImporter-Update: Import protocol step data (on existing protocols) from Excel file';
$title_sh    = 'ProtoImporter-Update';
if ($parx['action']=='create') {
	$title       = 'Proto-Import-Creator: Import protocol step data from Excel file and create protocols';
	$title_sh    = 'Proto-Import-Creator';
}

$infoarr=array();
$infoarr['help_url'] = 'o.exp.imp_sample.html';
$infoarr["title"] = $title;
$infoarr["title_sh"] = $title_sh;
$infoarr["form_type"]= "list";
$infoarr["obj_name"] = $tablename;


$pagelib = new gHtmlHead();
$pagelib->_PageHead ( $infoarr["title"],  $infoarr );
?>
<style type="text/css">
        tr.t1  { font-size: 0.8em; }   
</style>
<?  
$pagelib->_startBody($sql, $infoarr);

$guilib = new import_GUI();

$isAllowed = 0;
foreach( $tabMetaInf as $key=>$val) {
	if ($key == $tablename) $isAllowed = 1;
}


if ( !$isAllowed ) {
	htmlFoot("Error", "Table $tablename not allowed for this tool.");
}

gHtmlMisc::func_hist("obj.exp.imp_sample", $title_sh,  'obj.exp.imp_sample.php?tablename='.$tablename );   
if ( !$go ) {
 	echo "Get file header format for the current protocol: [<a href=\"obj.concrete_proto.infimp.php?tablename=".$tablename."\">Used steps</a>] \n";
 	echo "[<a href=\"obj.concrete_proto.infimp.php?tablename=".$tablename."&parx[usesteps]=all\">ALL steps</a>]\n";
 	echo '<br />'."\n";
}

echo "<ul>";
$mainLib->GoInfo($go, 'tablename='.$tablename);
  
if ( !$go ) {
    $guilib->formshow($sql, $parx); 
    $guilib->help($tablename, $infox["nicobject"], $parx['action']);
    htmlFoot();
} 

$tmptxt= "exact name matches";  
if ( $parx["expunexact"] ) $tmptxt= "unexact: search parts in exp-name";
this_info("name search method", $tmptxt, "only for experiment names");

$tmptxt = "NAME";

if ( $go == 1 ) {  

   $filename =  gObjImp_static::getFile( $_FILES['userfile'] );
   if ($error->Got(READONLY)) {
         $error->printAll();
         htmlFoot();
   }
   $parx["filename"] = $filename;
   $topt = NULL;
   if ($parx["expunexact"]) $topt["like"] = 1;                                        
   $mainLib->openfile($filename);
   
   list($headerArr, $headerRaw) = $mainLib->fileParseHead2();
   if ($error->Got(READONLY))  {
        htmlFoot();
   }
   $MultiPras_flag = $mainLib->has_MultiPras();
   
   if ($parx['action']!='create') {
       
       
	   $oneValidArr = $mainLib->getOneValidObj($sql, $headerArr, $topt);
	   if (!$oneValidArr[0]) { 
	        htmlErrorBox("Error", "no ".$infox["nicobject"]." in the file matches the database.".
	        ' First object: "'.$oneValidArr[2].'"'); 
	        this_showFirstLines($filename);
	        htmlFoot();
	   } 
	   $parx["refExpId"] = $oneValidArr[0];
	   $tmpExpname       = $oneValidArr[1];
	   
	   debugOut('(388) get PRAs from REF-Obj: '.$parx["refExpId"], $FUNCNAME, 1);
	   this_info("Reference-".$infox["nicobject"], $parx["refExpId"].":".$tmpExpname);
	   
	   $aProtosArr = gObjImp_static::getaProtos($sql, $parx["refExpId"], $tablename);
	    
	   if (!sizeof($aProtosArr)) {
	   	  htmlFoot("Error", "the reference ".$infox["nicobject"]." contains no protocol!");
	   }
	   
	   $aProtoid = key($aProtosArr);
	   $aProtoName  = current($aProtosArr);
	   
	   
   } else {
	   $aProtoid = $parx["aprotoid"];
	   debugOut('(412) get PRA Input-param: '.$aProtoid, $FUNCNAME, 1);
	   $aProtoName = gObjImp_static::getaProtoName($sql, $parx["aprotoid"]);
	   $aProtosArr = array($aProtoid=>$aProtoName);
	   
   }

   
   $parx["aprotoid"] = $aProtoid;
   
   $infox["aProtoName"] = gObjImp_static::getaProtoName($sql, $parx["aprotoid"]);
   if ($MultiPras_flag) {
       this_info("Multi-PRA-Flag", "PRAs are defined in the, special header ...");
       $pra_ids = $mainLib->get_pra_ids();
       $tmparr=array();
       $pra_ids_unique = array_unique($pra_ids);
       
       foreach($pra_ids_unique as $pra_loop) {
           if (!$pra_loop) continue;
           $tmparr[]=fObjViewC::bo_display( $sql, 'ABSTRACT_PROTO', $pra_loop);
       }
       this_info("Multi-PRAs", implode('; ',$tmparr));
   }
   this_info("Abstract protocol", $infox["aProtoName"]." [ID:". $parx["aprotoid"] ."]");
    
   echo "<br>\n";
   $guilib->form1($parx, $aProtosArr);
   htmlFoot();
} 

$filename = $parx["filename"];  

if (!$parx["aprotoid"])  this_errorEnd( "A selection of an protocol is missing" ); 
$infox["aProtoName"] = gObjImp_static::getaProtoName($sql, $parx["aprotoid"]);
this_info("Abstract protocol", $infox["aProtoName"]." [ID:". $parx["aprotoid"] ."]");

$mainLib->openfile($filename);
list($headerArr, $headerRaw) = $mainLib->fileParseHead2();
if ($error->Got(READONLY))  {
	htmlFoot();
}

if ( $go == 2 ) {
	 $guilib->form2($parx);
}


if (!$parx["aprotoid"]) { // can only happen when $go==1
     htmlFoot("Error","Typical abstract protocol missing.");
}
  
echo "<table cellpadding=1 cellspacing=1 border=0>";
echo "<tr bgcolor=#D0D0D0><th>No</th><th>Info</th>";
$cnt = 0;
foreach( $headerArr as $tmparr) { 
	if (!$cnt) {
    	if ( $tmparr['col']!=$tablename )  htmlFoot("Error", "First column in header must be '".$tablename."'");
    } 
    echo "<th>".$headerRaw[$cnt];
	if ( $_SESSION['userGlob']["g.debugLevel"]>0 ) {
		if ($tmparr['isID']) echo " isID";
	}
	echo "</th>";
    $cnt++;
}

echo "</tr>\n";  

$mainLib->doMain($sql, $headerArr, $parx);

htmlFoot();

