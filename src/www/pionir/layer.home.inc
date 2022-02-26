<?php
require_once ("f.textOut.inc");
require_once ("o.DB_USER.subs2.inc");
require_once 'subs/glob.obj.table_overview.inc';

/**
 * layer for home.php
 * @package layer.home.inc
 * @namespace core::gui::homeLayer
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/www/pionir/layer.home.inc 59 2018-11-21 09:04:09Z $
 */
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
	$textoutObj = new textOutC();
	
	echo '<br /><div style="padding-left:9px; margin-left:9px;">';
	
	$homeFuncObj->home_showHead("home");
	echo '<br /><br />'."\n";
	
	
	
	
	echo '<table border=0 cellpadding=10><tr valign=top>';
	echo '<td width=30%>'."\n";
	
//	$home_proj_id = oDB_USER_sub2::userHomeProjGet( $sql );
// 	$flist=array();
// 	$flist[] = array("ty"=>"head", "txt"=>'My personal area', "lay"=>"1" );
// 	if ($home_proj_id) $flist[] = array("ty"=>"lnk", "txt"=>'my folder',  "href"=>'edit.tmpl.php?t=PROJ&id='.$home_proj_id,
// 		"iicon"=>'icon.PROJ.gif', 'li'=>'br');
// 	else  $flist[] = array("ty"=>"lnk", "txt"=>'my folder (not exists)',  "href"=>'', "iicon"=>'icon.PROJ.gif', 'li'=>'br' );
// 	$flist[] = array("ty"=>"lnk", "txt"=>'my account',  "href"=>'obj.db_user.settings.php', "iicon"=>'icon.DB_USER.gif', 'li'=>'br');
// 	$flist[] = array("ty"=>"lnk", "txt"=>'my '.tablename_nice2('JOUR_ENTRY').' Report',  "href"=>'obj.jour_entry.list.php', "iicon"=>'icon.JOUR_ENTRY.gif', 'li'=>'br');
// 	$flist[] = array("ty"=>"headclose"); 
// 	$textoutObj->linksOut($flist);
// 	echo "<br>\n";
	
	
//     $flist=array();
// 	$flist[] = array("ty"=>"head", "txt"=>'Theme park overview', "lay"=>"1" );
// 	$flist[] = array("ty"=>"headclose");  
// 	$textoutObj->linksOut($flist);
	
	echo '<span style="font-weight:bold; font-size:1.2em;">Theme park overview</span>'."\n";
	
	//$labfirst=0;
	echo '<ul>'."\n";
	$nicearr_poi = $homeFuncObj->nicearr;
	foreach( $nicearr_poi as $key=>$valarr) {
	
		$parent_html = NULL;
		$niceName = $valarr["nice"];
		$parent   = $valarr["parent"];
		$type     = $valarr["ty"];
		if ($type=='hr') {
		    echo "<br>\n";
		    continue;
		}
		
		if ($key=='home') {
		    continue;
		}
		
		if ($parent!=NULL) {
			$parent_html = $homeFuncObj->nicearr[$parent]["nice"] .' &gt; ';
		}
		if ($valarr["dir"]=="lab") {
			//$labfirst=1;
			if (!file_exists("../".$_SESSION['globals']['lab_path']) ) continue;
		} 
		echo '<li >'.$parent_html.'<a href="home.php?layer='.$key.'">'.$niceName.'</a>'."</li>\n"; // style="font-size:1.2em;"
		
		
	}
	
	echo '</ul>'."\n";
	echo '<br>'."\n";
	
	$flist=array();
	$flist[] = array("ty"=>"head", "txt"=>'System area' );
	$flist[] = array("ty"=>"lnk", "txt"=>'Root '.tablename_nice2('PROJ'),  "href"=>'edit.tmpl.php?t=PROJ&id=NULL', "iicon"=>'icon.PROJ.gif', 'li'=>'br');
	$flist[] = array("ty"=>"lnk", "txt"=>'Users',  "href"=>'view.tmpl.php?t=DB_USER', "iicon"=>'icon.DB_USER.gif', 'li'=>'br');
	$flist[] = array("ty"=>"lnk", "txt"=>'User groups',  "href"=>'view.tmpl.php?t=USER_GROUP', "iicon"=>'icon.USER_GROUP.gif', 'li'=>'br');
	$flist[] = array("ty"=>"lnk", "txt"=>'Roles',  "href"=>'view.tmpl.php?t=ROLE', "iicon"=>'icon.ROLE.gif', 'li'=>'br');
	
	
	$flist[] = array("ty"=>"headclose"); 
	$textoutObj->linksOut($flist);
	echo '<hr style="width:300px; text-align: left; margin: 0 auto 0 0;">';
	
	echo '</td><td>&nbsp;</td>';
	echo '<td width=70%>'."\n";
	
	$flist=array();
	$flist[] = array("ty"=>"head", "txt"=>'Available Business object types', "lay"=>"1" );
	$flist[] = array("ty"=>"headclose");
	$textoutObj->linksOut($flist);
	
	$helpLib = new glob_obj_table_overview();
	$helpLib->showDet($sql, $sql2);
	
	/*
	$messopt=array('width'=>300);
	htmlInfoBox( 'Short help', "", "open", "CALM", $messopt );
	?>
	This is the HOME of the "theme park".<br />
	The HOME-page shows links to your personal area, other "theme parks" and other areas of the application.
	<br><br>	
	A theme park presents important links to tools, which are relevant for a specific working area of your lab.
	<?php
	htmlInfoBox( "", "", "close" );
	
	echo '<br>';
	echo '<img src="images/layerimg.home.jpg" width=300>';
	*/
	
	echo '</td></tr></table>'."\n";
	
	
	echo "</div>\n";
	
	
}

}