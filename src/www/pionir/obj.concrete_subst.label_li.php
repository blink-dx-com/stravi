<?php
/**
 * - create labels for a selection of substances
 * - this tool can easily be extended for other label-A4-page-formats ($parx['labsheet'])
 * 
 * @package obj.concrete_subst.label_li.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq UREQ:0001434: o.CONCRETE_SUBST > Produktionslabel erstellen 
 * @param $go: 0,1
 * @param $parx:
			colStart
		  	rowStart
			numcopy
 * @param $parx['mode']
 * 		   [1] - only name
			2  - Code128-barcode + ID
			3  - name + ID
			4  - ID + name + ID
			5  =>'Kleines QC-Label'
 * @param $parx['labsheet'] <pre> 
 * 	["CLEAR-238"],
 * 	"CCT-4x20" - 4x20
 *  "L4778"    - 4x12
 * 	'CSV'
 * </pre>
 */



session_start(); 


require_once ("reqnormal.inc"); // includes all normal *.inc files
require_once ('glob.objtab.page.inc');
// require_once ("f.labelPrintx.inc"); 
require_once ('f.workdir.inc');
require_once ('barcode.code128.inc');
require_once ("down_up_load.inc");

/**
 * BASE class 
 * @author steffen
 *
 */
class globListLabelB {
	
	function __construct( $tablename, $parx, $go, $sqlafter ) {
		global $error;
		
		$this->go = $go;
		$this->tablename=$tablename;
		$this->sqlafter=$sqlafter;
		
		if ($parx["mode"] == "")      $parx["mode"] = 1;
		if ($parx["labsheet"] == "")  $parx["labsheet"] = "CLEAR-238";
		// if ($parx["mode"] == 3) $parx["labsheet"] = "CCT-4x20"; // other format ...
		
		$this->parx = $parx;
	}
	
	function printInit() {} // abstract
	/**
	 * print one label
	 * @param $info_arr additional parameters; e.g. EXPIRY_DATE
	 */
	function labelout( $tmpid, $name_cpy, $info_arr) {} // abstract
	function printEnd() {} // abstract
	
	/**
	 * print labels of all substances
	 */
	function printit(&$sqlo, &$sqlo2) {
		global  $error;
		
		$tablename = $this->tablename;
		$primary_key = PrimNameGet2($tablename);
		$namecol = "NAME";
		
		
		$this->printInit();
		
		$sqlsLoop = "SELECT x.".$primary_key.", x.".$namecol.", x.EXPIRY_DATE FROM ".$this->sqlafter;
		$sqlo->query($sqlsLoop);
		$copy_MAX = $this->parx["numcopy"];
		if ($copy_MAX<=0) $copy_MAX=1;
			 
		while ( $sqlo->ReadRow() ) {
		
			$info_arr = array();
			$tmpid = $sqlo->RowData[0];
			$name  = $sqlo->RowData[1];
			$expiry_date  = substr($sqlo->RowData[2],0,10);
			$info_arr['EXPIRY_DATE']=$expiry_date;
			
			$copycnt = 0;
			while ($copycnt <  $copy_MAX) {
				$name_cpy = $name;
				if ($copycnt>0)  $name_cpy .= " (".chr(97-1+$copycnt).")";
				$this->labelout( $tmpid, $name_cpy, $info_arr);
				$copycnt++;
			}
			
		}
		
		$this->printEnd();
	}
}

/**
 * create PDF label sheets
 * @author steffen
 *
 */
class globListLabelPdf extends globListLabelB {
	
	function __construct( $tablename, $parx, $go, $sqlafter ) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		
		parent::__construct( $tablename, $parx, $go, $sqlafter );
		
		// $this->label_obj = new fLabelPrintC( $this->parx["labsheet"] ); // "Z92"
		$error->set( $FUNCNAME, 1, "PDF creation currently not supported.." );
		return;
		
		$workdirObj = new workDir();
		if ($this->parx["mode"] == "2") {
			$this->tmpdir = $workdirObj->getWorkDir ( "obj.concrete_subst.label_li" );
			$params = array(
					"charExpect" => 6, 
					"IMG_Y_LEN"  => 40,
					"txt_noZero" => 1,
					"txt_font"   => 10
					);
			$this->barObj = new barcodeCode128($params); 
		}	
	}

	/**
	 * get coordinates for one label
	 * @param $objid
	 * @param $name
	 * @return array($textarr, $imgarr)
	 */
	function _getcoords( $objid, $name, $infoarr ) {
		global $error;
		$FUNCNAME= "_getcoords";
		$imgarr = NULL;
		$textarr= NULL;
		
		switch ( $this->parx["mode"] ) {
			case 1:
				$pos1 = strpos($name, "-");
				if ($pos1>0) $pos2 = strpos($name, "-", $pos1+1);
				if ($pos2>0) $pos3 = strpos($name, "-", $pos2+1);
				if ($pos3>0) {
					$nameArr = array( substr($name,0,$pos3 ), substr($name,$pos3 ) );
					
				} else $nameArr = array($name);
				$textarr[] = array( "x"=>15, "y"=>17, "f"=>5, "txt"=>$nameArr[0] );
				$textarr[] = array( "x"=>15, "y"=>27, "f"=>5, "txt"=>$nameArr[1] );
				break;
			case 2:
				$filename = $this->tmpdir."/".$objid.".png";
				
				$objidName = str_pad( $objid, 6, "0", STR_PAD_LEFT );
				$baropt = array("pretxt"=>"SID:");
				$this->barObj->buildImage($objidName, $baropt); 
				if ($error->Got(READONLY))  {
					 $error->set( $FUNCNAME, 1, "Error on buildImage($objid) occurred." );
					 return;
				}
				$this->barObj->saveImagePng( $filename );
				$imgdim = $this->barObj->getImgDim();
				$this->barObj->destroyx();
				
				$imgarr[] = array( "img"=>$filename, "x"=>2, "y"=>40, "xlen"=>$imgdim[0]*0.8, "ylen"=>$imgdim[1]*0.8 );
				break;
				
			case 3: # name + ID
				$textarr[] = array( "x"=>10, "y"=>17, "f"=>5, "txt"=>$name );
				$textarr[] = array( "x"=>10, "y"=>27, "f"=>5, "txt"=>'ID:'.$objid );
				
				break;
				
			case 5: 
				// @swreq UREQ:0001434:SUBREQ 002:   Kleines QC-Label: ID + Expiry-date + NAME
				$MAX_NAME_LENGTH=20;
				$param=array();
				$EXPIRY_DATE=$infoarr['EXPIRY_DATE'];
				if ($EXPIRY_DATE==NULL) { // must be set !!!
					$param[2]='FEHLT !!!!!!!';
					$param[3]='--- NICHT MOEGLICH';
					$param[4]='--- NICHT MOEGLICH';
				} else {
					if (strlen($name)>$MAX_NAME_LENGTH) {
						$name = substr($name,0,$MAX_NAME_LENGTH).'.';
					}
					$param[2]=$EXPIRY_DATE;
					$param[3]='';
					$param[4]=$name;
				}
				$textarr[] = array( "x"=>15, "y"=>17, "f"=>5, "txt"=>'QC-Material DB-ID: '. $objid );
				$textarr[] = array( "x"=>15, "y"=>27, "f"=>5, "txt"=>'Verf.Dat: '.$param[2] );
				$textarr[] = array( "x"=>15, "y"=>37, "f"=>5, "txt"=>'Lagerb.: ' .$param[3] );
				$textarr[] = array( "x"=>15, "y"=>47, "f"=>5, "txt"=>'Name: '    .$param[4] );
				break;
		}
		
		
		return( array($textarr, $imgarr) );
	}


	// one label
	function labelout( $objid, $name, $infoarr ) {
		
		$this->label_obj->labelStart();
		
		$coordarr = $this->_getcoords( $objid, $name, $infoarr );
		if ($coordarr[0]) $this->label_obj->addTextArr($coordarr[0]);
		if ($coordarr[1]) $this->label_obj->addImgArr ($coordarr[1]);
		$this->label_obj->labelEnd();
		
	}
	
	function printInit() {
		$info_arr=NULL;
		if ( $_SESSION['userGlob']["g.debugLevel"] > 0 ) {
			$info_arr["show_frame"]=1;
		}
		$rowtmp = 0;
		if ($this->parx["rowStart"]>0) $rowtmp = $this->parx["rowStart"]-1;
		$this->label_obj->_setStartCol( $this->parx["colStart"], $rowtmp );
		$this->label_obj->fullinit( $info_arr );
	}
	
	function printEnd() {
		global $error;
		
		$show_pdfcode=0;
		if ( $_SESSION['userGlob']["g.debugLevel"] > 1 OR $error->Got(READONLY) ) {
	    	$show_pdfcode=1;
	    }	
		if ( $this->parx["output"]=="file"  ) {
			set_mime_type ("text/plain");
			print ( $this->label_obj->pdf_getstr() );
		} else {
			$this->label_obj->pdf_out ($show_pdfcode);
		}
	}

}

class globListLabelCSV extends globListLabelB {
	
	function __construct( $tablename, $parx, $go, $sqlafter ) {
		global $error;
		
		parent::__construct( $tablename, $parx, $go, $sqlafter );
		
		$this->separator =",";
		$this->lineend   ="\n";
			
	}
	
	// print CSV header
	function printInit() {
		set_mime_type ("text/tab-separated-values", 
			date('Y-m-d').'_label.'.tablename_nice2($this->tablename).'.txt'  );
		// echo "ID\tNAME\n";
	}
	
	// one label
	function labelout( $id, $name, $info_arr) {
		switch ( $this->parx["mode"] ) {
			
			case 4:
				/* 
				 * $out[0] (ID)
				 * $out[1] (name)
				 * $out[2] (first char chars till 2.stripe; e.g. 001-01)
				 * $out[3] (after 2.stripe e.g. 034-456)
				 */
				$nameArr=explode('-',$name);
				$outx=NULL;
				$outx[0]=$id;
				$outx[1]=$name;
				$outx[2]=$nameArr[0].'-'.$nameArr[1].'-';
				$outx[3]=$nameArr[2].'-'.$nameArr[3];
				echo implode($this->separator, $outx) . $this->lineend;
		}
	}
	
	function printEnd() {
		
	}
}



/**
 * manage GUI funcs for label export
 * @author steffen
 *
 */
class globListLabelGui {
	function __construct() {
		$this->pageFormat=array( 
			"CLEAR-238"=>"CLEAR-238 - 7x17", 
			"CCT-4x20"=>"CCT - 4x20",
			"L4778"   =>"L4778 - 4x12",
			"CSV"=>"CSV-file!" 
		);
	}
	
	function form1() {
		require_once ('func_form.inc');
		
		
		$initarr   = NULL;
		$initarr["action"]      = $_SERVER['PHP_SELF'];
		$initarr["title"]       = "Settings";
		$initarr["submittitle"] = "Submit";
		$initarr["tabwidth"]    = "AUTO";
	
		$hiddenarr = NULL;
		$hiddenarr["tablename"] = $this->tablename;
	
		$formobj = new formc($initarr, $hiddenarr, 0);
	
		$selarr=array(0=>1, 1=>2, 2=>3, 3=>4, 4=>5, 5=>6, 6=>7 );
		$fieldx = array ( 
			"title" => "StartColumn", 
			"name"  => "colStart",
			"object"=> "select",
			'optional'=>1,
			"val"   => 0, 
			"inits" => $selarr,
			"notes" => "Optional start-column of A4-page"
			 );
		$formobj->fieldOut( $fieldx );
		
		
		$fieldx = array ( 
			"title" => "StartRow", 
			"name"  => "rowStart",
			"object"=> "text",
			"val"   => "1", 
			'optional'=>1,
	  		"fsize"  => 2,
			"notes" => "Optional start-row of A4-page (1-17)"
			 );
		$formobj->fieldOut( $fieldx );
		
		$fieldx = array ( 
			"title" => "Copies", 
			"name"  => "numcopy",
			"object"=> "text",
			"val"   => "1", 
	  		"fsize"  => 2,
			"notes" => "Number of copies (aliquots)"
			 );
		$formobj->fieldOut( $fieldx );
		
		$selarr=array(
			"1"=>"[name]", 
			"2"=>"Barcode (Code128)", 
			"3"=>"name + ID", 
			 5=>'Kleines QC-Label (nutze Format 4x12)',
			 4=>'CCT-sample (CSV)' 
			);
		$fieldx = array ( 
			"title" => "Layout", 
			"name"  => "mode",
			"object"=> "select",
			"val"   => 0, 
			"inits" => $selarr,
			"notes" => "Label mode"
			 );
		$formobj->fieldOut( $fieldx );
	
		
		$selarr = $this->pageFormat;
		$fieldx = array ( 
			"title" => "PageFormat", 
			"name"  => "labsheet",
			"object"=> "select",
			"val"   => $this->parx['labsheet'], 
			"inits" => $selarr,
			"notes" => "output format (A4-paper or CSV)"
			 );
		$formobj->fieldOut( $fieldx );
		
		/*
		$selarr=array(0=>"normal", "file"=>"as file" );
		$fieldx = array ( 
			"title" => "Output", 
			"name"  => "output",
			"object"=> "select",
			"val"   => "0", 
	  		"inits" => $selarr,
			"notes" => "Save as file ?"
			 );
		$formobj->fieldOut( $fieldx );
		*/
	
		$formobj->close( TRUE );
		
		echo "<br>\n";
		$this->help();
	}

	function help() {
		htmlInfoBox( "Short help", "", "open", "HELP" );
		?>
		<ul>
			<li>Version: 2011-06-25</li>
			<li>This tool creates labels as:</li>
			<ul>
			  <li> PDF-file for A4-pages</li>
			  <li> CSV-file</li>
			</ul>
			<br />
			
			<li><b>Layout:</b></li>
			<ul>
			<li>Kleines QC-Label</li>
			<li>CCT-sample: XXX-LL-PPP-GG-a</li>
			</ul>
			
			<li><b>PageFormat:</b> Unterst&uuml;tzt verschiedene A4-Formate</li>

			
		</ul>
		<?
		htmlInfoBox( "", "", "close" );
		
	}
} 

// --------------------------------------------------- 
global $error, $varcol;

$error = & ErrorHandler::get();
$sqlo  = logon2( $_SERVER['PHP_SELF'] ); // URL-link for the first db-login; e.g. for object ($_SERVER['PHP_SELF']."?id=".$id)
$sqlo2 = logon2( );
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();

$go = $_REQUEST["go"];
$parx = $_REQUEST["parx"];

$tablename			 = "CONCRETE_SUBST";

$title       		 = "create labels for substances";
$infoarr=NULL;
$infoarr["title"]    = $title;
$infoarr["title_sh"] = 'create labels';
$infoarr["form_type"]= "list";
$infoarr["sql_obj"]  = &$sqlo;
$infoarr["obj_name"] = $tablename;
$infoarr["obj_cnt"]  = 1;          // show number of objects

$headopt = NULL;		

$mainObj = new gObjTabPage($sqlo, $tablename );
if ( $go<=0 ) {
	$mainObj->showHead($sqlo, $infoarr, $headopt);
}
$mainObj->initCheck($sqlo);
$sqlafter = $mainObj->getSqlAfter();


if ($go) {
    if ($parx['labsheet']=='CSV') {
        $scriptlib = new globListLabelCSV( $tablename, $parx, $go, $sqlafter );
    } else {
        $scriptlib = new globListLabelPdf( $tablename, $parx, $go, $sqlafter );
    }
    if ($error->Got(READONLY))  {
    	$error->printAll();
    }
}
$guiLib = new globListLabelGui();

if ( !$go ) {
	
	echo "<ul>\n";
	$guiLib->form1();
	$mainObj->htmlFoot();
}

$scriptlib->printit($sqlo, $sqlo2);
$error->printAll();
