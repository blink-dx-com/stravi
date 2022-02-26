<?php
/**
 * show options for image visu
 * GLOBALS: $_SESSION['userGlob']["o.IMG.thumbnail_params"]
 * @package obj.img.showHuge.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param
    $img_id
	$go			 : save form params
	
    $opx["mode"]  :   [], 
		             "slim" : use a saved parameter set, do not show input fields
	$opx["profile"]: "default", "galery", ...
	$opx["dim"] :
		[0] x-dim
		[1] y-dim
	$opx["profileset"]: set profile
	$opx["img_params"] (display params for ImageMagick
	$opx["img_dim_flag"]   -1 : zoomout
					 0 : leave it
					 1 : zoomin 
					 2 : reset
					 max: max size
	$opx["actionx"] = "print"
	$opx["advanced_show"]  = 0,1
	$opx["set_default"]  = 0|1  - set as default thumbnail
	$opx["take_default"] = 0|1  - take default parameters
	
	$opt[]
	  sizeOri
	  refpoints
	$img_para_arr[]	 "-normalize", "-negate" = 0,1
  
  
*/
session_start(); 


require_once ('reqnormal.inc');
require_once ("glob.image.inc");
require_once ('func_form.inc');
require_once ('f.imgparams.inc');

class oImgShowHuge {

var $opx;

function init( $img_id, &$opx, &$img_para_arr, &$opt ) {

	$this->paramObj = new fImgParams();

	$this->img_id = $img_id;
	$this->opt = $opt;
	$this->opx = $opx;
	$this->img_para_arr = $img_para_arr;
	
	if ($this->opx["profile"]=="") $this->opx["profile"]  = "default";
	// $paramObj->getProfileParams($this->opx["profile"]);
	
	
	$this->dimen[0]=350;
	$this->dimen[1]=250;
	
		
	if ($this->opx["dim"][0] && $this->opx["dim"][1]) {
		$this->dimen[0]=$this->opx["dim"][0];
		$this->dimen[1]=$this->opx["dim"][1];
	}
	
	// produce imaging parameter string
	if (sizeof ($img_para_arr) ) {
		$this->opx["img_params"] = $this->paramObj->paramArray2str( $this->img_para_arr );
	}
	
	do {
	
		if ($this->opx["img_dim_flag"]=="max") {
			$this->opt["sizeOri"] = 1;
			break;
		}
	
		if ($this->opx["img_dim_flag"]==2) {
			$this->dimen[0]=350;
			$this->dimen[1]=250;
			break;
		}
		if ($this->opx["img_dim_flag"]==1) {
			$this->dimen[0]=$this->dimen[0]*2.0;
			$this->dimen[1]=$this->dimen[1]*2.0;
			break;
		}
		if ($this->opx["img_dim_flag"]<0) {
			$this->dimen[0]=$this->dimen[0]*0.5;
			$this->dimen[1]=$this->dimen[1]*0.5;
			break;
		}
	} while (0);
	
	
	if ($this->opx["img_params"]===NULL) $this->opx["img_params"]=" "; // TBD: WHY a SPACE ???
}

function parout ($head, $param, $notes=NULL, 
	$optrow=NULL // "space", "submit"
	) {
	
	
	if ( $this->opx["advanced_show"] ) {
		$tmpcolor="";
		if ( $optrow["submit"] ) $tmpcolor=" bgcolor=#EFE0FF";
		echo "<tr".$tmpcolor.">";
		echo "<td align=right><b>".$head."</b></td><td>&nbsp;".$param." ".$notes."</td>";
	}
	else {
		$spacer = ": ";
		if ( $optrow["submit"] ) $spacer = "";
		echo "<td>".$head.$spacer.$param."</td><td bgcolor=#FFFFFF>&nbsp;</td>";
	}
	
	if ($this->opx["advanced_show"]) echo "</tr>\n";
}

function showForm() {

	$opx = $this->opx;
	
	$thecolor="#EFE0FF"; // #DFDFDF
	echo "<table bgcolor=$thecolor border=0 cellspacing=1 cellpading=0>";
	if ($opx["advanced_show"]) {
		echo "<tr><td><table bgcolor=#EFEFEF border=0 cellpading=2 cellspacing=0>";
		echo "<tr bgcolor=$thecolor><td colspan=2><font color=white><b>Beauty selection</b></font></td></tr>";
	}
	
	echo "<form style=\"display:inline;\" method=\"post\"  name=\"editform\"  action=\"".$_SERVER['PHP_SELF']."\" >\n";
	echo "<input type=hidden name='img_id' value='".$this->img_id."'>\n";
	echo "<input type=hidden name='opx[dim][0]' value='".$this->dimen[0]."'>\n";
	echo "<input type=hidden name='opx[dim][1]' value='".$this->dimen[1]."'>\n";
	echo "<input type=hidden name='go' 		   value='1'>\n";
	echo "<input type=hidden name='opx[take_default]' value=''>\n";

	if ( !$opx["advanced_show"] ) echo "<tr>";
		
	$this->parout( "Zoom","reset".
		"  <input type=radio name=opx[img_dim_flag] value=2> &nbsp;&nbsp; ".
		"--<input type=radio name=opx[img_dim_flag] value=-1> &nbsp;&nbsp;".
		"  <input type=radio name=opx[img_dim_flag] value=0 checked> &nbsp;".
		"  <input type=radio name=opx[img_dim_flag] value=1>++ ".
		"  <input type=radio name=opx[img_dim_flag] value=max>original" );
	
	$this->parout("Normalize","<input type=checkbox name=img_para_arr[\"-normalize\"] value=\"on\" >");
	$this->parout("Invers","<input type=checkbox name=img_para_arr[\"-negate\"] value=\"on\" >");

	if ( $opx["advanced_show"] ) {
	
		$this->parout( "Threshold",	"<input type=text size=6 name=img_para_arr[\"-threshold\"] value=\"\" >", "(number)");
		$this->parout( "Flip",		"<input type=checkbox name=img_para_arr[\"-flip\"] value=\"on\" >");
		$this->parout( "Gamma",		"<input type=text size=6 name=img_para_arr[\"-gamma\"] value=\"\" >", "(value 0.1 ... +3)" );	
		$this->parout( "Rotate",	"<input type=text size=6 name=img_para_arr[\"-rotate\"] value=\"\" >","(value 0..90)" );		
		$this->parout( "Shear",		"<input type=text size=6 name=img_para_arr[\"-shear\"] value=\"\" >".
			" (&lt;x_degrees&gt;x&lt;y_degrees&gt;) degrees=(0..90]");		
		$this->parout( "Colors",	"<input type=text size=6 name=img_para_arr[\"-colorize\"] value=\"\" >", "(value R/G/B) (e.g. 0/0/70)" ); 
		$this->parout( "Show reference spots","<input type=checkbox name=opt[refpoints] value=\"1\" >");
		$this->parout( "Show segmenation image","<input type=checkbox name=opt[segimg] value=\"1\" >");
	
	} 
	
	if ( !$opx["advanced_show"] ) {
		echo "</tr></table>";
		echo "<table bgcolor=#EFEFEF border=0 cellpading=1 cellspacing=1><tr>";
	} else {
		echo "<tr bgcolor=#CFCFCF><td colspan=2><font color=#FFFFFF>Parameter management (save them)</font></td></tr>";
	}
	if ( $opx["advanced_show"] ) {
		$this->parout("Default thumbnail","<input type=checkbox name=opx[set_default] value=\"1\"> (set as default thumbnail for THIS image)");
	}
	
	if ( $opx["advanced_show"] ) {
	
		$feld 		 = $this->paramObj->profiles;
		$preselected = "";
		$tmptext = formc::selectFget("opx[profileset]", $feld, $preselected);
		$this->parout( "save parameters as profile", $tmptext);
	
		$optrow = array("space"=>1);
		$this->parout( "", "", "", $optrow);
	}
	
	$tmpvar = ''.
			  '<input type=button value="SET and SHOW" onClick="submit();">'.
			  '&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;'.
			  '<input type=button value="Show default parameters" onClick="submitDefault();">';
	$optrow = array("submit"=>1);
	$this->parout( "",$tmpvar, "", $optrow);
	
	if ( !$opx["advanced_show"] ) {
		echo "</tr>\n";
	}
	echo "</table>\n";
	if ($opx["advanced_show"]) echo "</td></tr></table>\n";
	
	echo "</form>\n";
	echo "<br><br>\n";
	
	
	if ( $opx["advanced_show"] ) {
		echo "</blockquote>";
		return 0;
	}
}

function save() {
	
	if ( $this->opx["profileset"]!="" ) {
		$this->paramObj->paramsSet( $this->opx["profileset"], $this->opt, $this->img_para_arr, $this->dimen );
		$this->paramObj->saveDefParams();
		$this->opx["profile"] = $this->opx["profileset"];
		if ( $_SESSION["userGlob"]["g.debugLevel"]>1 ) {
			echo "<B>INFO:</B> DEBUG-mode: save_profile:". $this->opx["profileset"] ."<br>\n";
			glob_printr( $this->img_para_arr, "img_para_arr" );
			glob_printr( $this->dimen, "dimen" );
			echo "<br>";
		}
	}
	$this->opx["img_params"] = $this->paramObj->paramArray2str( $this->img_para_arr );
}

function show( &$sql ) {
	
	
	$img_id = $this->img_id;
	
	$profnicetxt = $this->paramObj->profiles[$this->opx["profile"]];
	if ($profnicetxt=="" ) $profnicetxt = $this->opx["profile"];
	
	$options_tmp = array();
	
	if ($this->opx["take_default"]) {
		$this->opx["img_params"] = $_SESSION['userGlob']["o.IMG.thumbnail_params"] ;
		$options_tmp["refresh"] = 1;
	}
	
	
	$showForm=1;
	if ($this->opx["actionx"]=="print") $showForm=0;
	
	if ($this->opx["mode"]=="slim") {
		// show parameters from a saved PROFILE
		$defparams = $this->paramObj->getProfileParams($this->opx["profile"]);
		
		if ( !is_array($defparams) ) {
			echo "<br>";
			htmlInfoBox( "Warning", "Please define profile '".$profnicetxt."'", "", "WARN" );
			echo "<br>";
		} else {
		
			$this->dimen      = $defparams["dim"];
			$this->opx["img_params"] = $this->paramObj->paramArray2str( $defparams["imgarr"] );
			$this->opt		= $defparams["opt"] ;
			$showForm  = 0;
		}
	}
	
	$img_params_enc = urlencode($this->opx["img_params"]);
	$diminfo = " default-dim:".$this->dimen[0]."x".$this->dimen[1];
	$infoStropt=" ";
	if ( sizeof($this->opt)>0 ) {
		$tmpkomma="";
		$infoStropt=" options: ";
		foreach( $this->opt as $key=>$val) {
			$infoStropt .= $tmpkomma . $key ."=". $val;
			$tmpkomma= ", ";
		}
		reset ($this->opt); 
	}
	$img_params_all = $this->opx["img_params"]. $diminfo.$infoStropt;
	
	
	?>
	[<a href="<?echo $_SERVER['PHP_SELF']?>?img_id=<?echo $img_id?>&opx[dim][0]=<?echo $this->dimen[0]?>&opx[dim][1]=<?echo
	$this->dimen[1]?>&opx[img_params]=<?echo $img_params_enc?>&opx[actionx]=print">Printer view</a>]
	
	<? if ( !$this->opx["advanced_show"] ) { ?>
		[<a href="<?echo $_SERVER['PHP_SELF']?>?img_id=<?echo $img_id?>&opx[advanced_show]=1">advanced</a>] 
	<? } else { ?>
		[<a href="<?echo $_SERVER['PHP_SELF']?>?img_id=<?echo $img_id?>">easy</a>] 
	<? } 
	
	
	
	echo "<font color=gray>Profile:</font>". $profnicetxt." ";
	
	
	if ( $showForm ) {
		echo " <font color=gray>Default
		parameters:</font>".$_SESSION['userGlob']["o.IMG.thumbnail_params"];
	}
	echo "<br>";
	echo "<font color=gray>Used Parameters:</font> ";
	echo "<font color=gray>[<b></font>".$img_params_all."<font color=gray></b>]</font><br>\n";
	
	if ( $this->opx["advanced_show"] ) echo "<blockquote>";
	echo "<br>\n";
	
	if ( $showForm ) {
		$this->showForm();
	}
		
	if ($this->opt["refpoints"]) $options_tmp["show_ref_points"] = 1;
	if ($this->opt["segimg"])    $options_tmp["segimg"]  = 1;
	if ($this->opt["sizeOri"])   $options_tmp["sizeOri"] = 1;
	$options_tmp["set_default"] = $this->opx["set_default"];	
	$options_tmp["noImageLogo"] = 1;
	
	$ret = org_img_show ( $sql, $this->img_id, $this->dimen, $this->opx["img_params"], "", $options_tmp );
}

}

////////////////////////////////////////////////////////////

$error = & ErrorHandler::get();
$sql = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();

$img_id = $_REQUEST["img_id"];
$go  = $_REQUEST["go"];
$opx = $_REQUEST["opx"];
$img_para_arr = $_REQUEST["img_para_arr"];
$opt = $_REQUEST["opt"];


$javastr = '
	function submitDefault() 
	{ 
		document.xform.opx["take_default"].value=1;
		document.xform.submit();
	}
';
		

$title="Image beauty farm (only visualization, NO manipulation)";
$infoarr=array();
$infoarr["title_sh"] = "Image beauty farm ";
$infoarr["title"] = $title;
$infoarr["form_type"]= "obj";
$infoarr["obj_name"] = "IMG";
$infoarr["obj_id"]   = $img_id;
$infoarr["show_name"]= 1;
$infoarr["javascript"]= $javastr;

$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);

$scriptLib = new oImgShowHuge();

$scriptLib->init(  $img_id, $opx, $img_para_arr, $opt );


if ( $go ) {
	$scriptLib->save();
}

$scriptLib->show($sql);
	
	
