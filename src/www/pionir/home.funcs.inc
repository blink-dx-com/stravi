<?php
/**
 * functions for home.php
 * - used by other home-layers
 * - call this from ./pionir path
 * $Header: trunk/src/www/pionir/home.funcs.inc 59 2018-11-21 09:04:09Z $
 * @package home.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */


class fHomeFunc {

    
/* format: KEY = array(
 "nice"   => name,
 "dir"    =>"lab"
 'menu'   => 0,1 -- if 1: show entry in the main menu of home.php
 "parent" => key of parent layer
 "icon"   => icon url on images
 "shortsvg" => array('t'=>text, 'c'=>color)
 "ty" type: type of entry
    ['def'], default: normal layer
    'hr'     - SPACER
 )
 */
var $nicearr; 

function __construct () {
    
    $layer_config = array(
        "home"   => array("nice"=>"home", 'menu'=>1),
        "my"     => array("nice"=>"my world", 'menu'=>1),
        "_sp1"   => array('menu'=>1, 'ty'=>'hr'), // SPACER
    );
	
	$labInclude = "../".$_SESSION['globals']['lab_path']."/config_lab/home.inc";
	if ( file_exists($labInclude) ) {
  		include($labInclude);
  	}
  	
  	$this->nicearr  = &$layer_config;
  	
  	// "exp"  => array("nice"=>"Experiment structure", 'menu'=>0)  
  	// $this->nicearr["device"] = array("nice"=>"Equipment", 'menu'=>1); 
  	// $this->nicearr["_example"] = array("nice"=>"___Example");

  	//OLD: "array"=> array("nice"=>"Array"),
  	//OLD: "subst"=> array("nice"=>"Substances"),
  	

}

function get_theme_nice($key) {
    return  $this->nicearr[$key]["nice"];
}

function home_title($text, $options=NULL) {
    $color_style = '';
    if ($options['bg.style']=='dark') {
        $color_style = 'color:white;';
    }
    echo '<span style="font-size:1.6em;'.$color_style.'">'.$text."</span></B>";
}
	
/**
 * show theme park title
 * @param string $key, key of $this->nicearr
 * @param string $parent, parent-key of  $this->nicearr
 * @param array $options
 *  'bg.style' = [bright], dark
 */
function home_showHead($key, $parent=NULL, $options=NULL) {
    $color_style = '';
	if ($options['bg.style']=='dark') {
	    $color_style = 'color:white;';
	}
	
	$delimiter = ' <span style="color:gray;">&gt;</span> ';
	
	$themeTxt = 'Theme parks';
	if ($key!='home') echo '<B><a href="home.php?layer=home" style="'.$color_style.'">'.$themeTxt.'</a>  '.$delimiter;
	else echo "<B>".$themeTxt.$delimiter;
	if ($parent!=NULL) {
	    echo '<a href="home.php?layer='.$parent.'" style="'.$color_style.'">'.$this->nicearr[$parent]["nice"].'</a> '.$delimiter;
	}
	$this->home_title($this->nicearr[$key]["nice"], $options);	
}

// show links of sub-theme-parks
function subThemes($themeKeys) {
	foreach( $themeKeys as $theme) {
		echo '&nbsp;&nbsp;&nbsp;+  <a href="home.php?layer='.$theme.'">'.$this->nicearr[$theme]["nice"].'</a> <br />'."\n";
	}
}


function showObjectSelection($tableList) {
	echo "<form style=\"display:inline;\">\n";
	echo "<span style='color:black;'>\n";
	echo "<select name=t size=1 onChange=\"Go(this.form.t.options[this.form.t.options.selectedIndex].value)\">\n";
	echo "<option value=\"---\">--- select object list ---\n";

	$nicename=array();
	foreach( $tableList as $tab_name) {
		$nicename[tablename_nice2($tab_name)] = $tab_name;
	}
	
	
	ksort ($nicename);
	reset ($nicename);
	foreach( $nicename as $key=>$val) {
		echo '<option value="'.$val.'">'.$key."\n";
	}
	
	
	echo '</select></span></form>'."\n";
	
}

function selMainObjects( $showAdvanced, $backdark=0 ) {
	$thisstyle = "";
	if ( $backdark ) {
		$thisstyle = "style=\"background-color: #202030; color:#EFEFEF;\"";
	}
	echo "<form style=\"display:inline;\">\n";
	echo "<select $thisstyle name=\"t\" size=1 onChange=\"Go(this.form.t.options[this.form.t.options.selectedIndex].value)\">\n";
	echo "<option value=\"---\">--- select object list ---</option>";
  
	$tab = array (
		"ABSTRACT_PROTO",		
		"ABSTRACT_SUBST",		
		"CHIP_READER",
	    "CONTAINER",
		"CONCRETE_ARRAY",
		"CONCRETE_PROTO",			
		"CONCRETE_SUBST",
		"CONTACT",				
		"DB_USER",
		"EXP",
		"EXP_TMPL",
		"IMG",
		"JOUR_ENTRY",				
		"LINK",
		"PROJ",
		"SOCKET",
		"USER_GROUP"
		);
	
	
	$nicename=array();
	foreach( $tab as $tab_name) {
	    if (glob_table_exists($tab_name)) {
		  $nicename[tablename_nice2($tab_name)] = $tab_name;
	    }
	}
	ksort ($nicename);
	
	foreach( $nicename as $key=>$val) {
	  echo '<option value="'.$val.'">'.$key."</option>\n";
	}

  
	echo "</select>\n";
	echo "</form>\n";
}

}