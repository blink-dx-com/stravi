<?php
/**
 * example layer
 * @namespace core::gui::homeLayer
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */

require_once ("f.textOut.inc");


class themeCls extends gThemeClsAbs {

function layer_HtmlInit() {
	$cssstr = 
		".xBodyNorm { background-image: url(images/layerimg.home.bg.jpg); ".
		 " background-repeat: no-repeat; } \n";

	echo '<style type="text/css">'."\n";
	echo $cssstr;
	echo '</style>'."\n";
}

function layer_params() {
  $lparams = array ( "bodycss" => "xBodyNorm");
  return ($lparams);
}


function layer_show( &$sql, &$sql2 ) {
	$homeFuncObj = &$this->__homeFuncObj;
	$textoutObj = new textOutC($txtopt);
	
	echo '<br /><div style="padding-left:9px; margin-left:9px;">';
	
	$homeFuncObj->home_showHead("_example");
	echo '<br /><br />'."\n";
	
	echo "This is an example Theme Park";
	
	echo '</div>';
	
}

}