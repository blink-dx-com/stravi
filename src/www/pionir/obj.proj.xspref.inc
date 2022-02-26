<?
/**
 * contains function for project object preferences
 * sets global variables: 
 *   - $_SESSION['userGlob']["o.PROJ.sop.EXP"]
 *   - $_SESSION['userGlob']["o.PROJ.sop.CONCRETE_SUBST"]
 *   - $_SESSION['userGlob']["o.PROJ.pref"]
 *   
 * - these global variables are used in: 
 *   - obj.proj.xsubst.inc
 *   - obj.TABLENAME.xfunc.inc => method inProjShowAtt()
 * @package obj.proj.xspref.inc
 * @swreq   UREQ:0001416: o.PROJ > function for project object preferences 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */ 
class o_PROJ_XSPREF  extends o_xspref_ABS {

    function _ProjPrefHead($text, $icon=NULL) {
    	if ($icon!="") $iconTag = "<img src=\"".$icon."\">";
    	echo "<tr><td colspan=3 bgcolor=#EFEFEF>".$iconTag."&nbsp;<B>$text</B></td></tr>";	
    }
    
    function _empty_line() {
        echo "<tr><td colspan=3 bgcolor=#FFFFFF>&nbsp;</td></tr>";
    }
    
    
    function singlePref( &$sql, $info ) {
    	
    	
        $this->_ProjPrefHead("Experiment visualization", "images/icon.EXP.gif");
    	$tmpexp = unserialize($_SESSION['userGlob']["o.PROJ.sop.EXP"]);
        		
    	prefNameShow( "Show sample" );
    	$vartmp = "sop.EXP.sample";
    	$infotmp= $tmpexp["sample"];
        $checked_tmp="";
    	if ($infotmp) $checked_tmp="checked";
    	echo "<td><input type=checkbox name=\"setti[".$vartmp."]\" value=\"1\" ".$checked_tmp."></td>";
    	prefClose ("Show sample information"); 
    
    	
    	prefNameShow( "Show Short-Result" );
    	$vartmp = "sop.EXP.SHORTRES";
    	$infotmp= $tmpexp["SHORTRES"];
        $checked_tmp="";
    	if ($infotmp) $checked_tmp="checked";
    	echo "<td><input type=checkbox name=\"setti[".$vartmp."]\" value=\"1\" ".$checked_tmp."></td>";
    	prefClose ("Show value of Short-Result");
        
    
    	// -----------------------------------------------------
    	
    	$this->_empty_line();
    	$this->_ProjPrefHead("Equipment visualization", "images/icon.CHIP_READER.gif");
    	
    	$seriVar = "o.CHIP_READER.";
    	$tmpsubst = unserialize($_SESSION['userGlob']["o.PROJ.sop.CHIP_READER"]);
    	
    	prefNameShow( "Show location" );
    	$vartmp = "location";
    	$infotmp= $tmpsubst[$vartmp];
    	$checked_tmp="";
    	if ($infotmp) $checked_tmp="checked";
    	echo "<td><input type=checkbox name=\"setti[".$seriVar.".".$vartmp."]\" value=\"1\" $checked_tmp></td>";
    	prefClose ("Show location of equipment?");
    	
    	$this->_empty_line();
    	$this->_ProjPrefHead("Substance visualization", "images/icon.CONCRETE_SUBST.gif");
    	
    	$seriVar = "o.SUC.";
    	$tmpsubst = unserialize($_SESSION['userGlob']["o.PROJ.sop.CONCRETE_SUBST"]);
    	
        prefNameShow( "Show batch" );
    	$vartmp = "batch";
    	$infotmp= $tmpsubst[$vartmp];
    	$checked_tmp="";
    	if ($infotmp) $checked_tmp="checked";
    	echo "<td><input type=checkbox name=\"setti[".$seriVar.".".$vartmp."]\" value=\"1\" $checked_tmp></td>";
    	prefClose ("Show batch attribute?");
    	
    	// --------------------------------
        
    	$this->_empty_line();
    	$this->_ProjPrefHead("Other visualization options");
        
        $seriVar = "o.PROJ.pref";
    	$projPrefs = unserialize($_SESSION['userGlob'][$seriVar]);
    	
    	
    
    	prefNameShow( "Max shown objects" );
    	$vartmp = "maxob";
    	$infotmp= $projPrefs[$vartmp];
    	if ( $infotmp=="" OR $infotmp<10 ) $infotmp="200";
    	echo "<td><input type=text name=\"setti[".$seriVar.".".$vartmp."]\" value=\"".$infotmp."\" size=4></td>";
    	prefClose ("Number of max shown objects (typical: 200)");
    	
    	prefNameShow( "Show notes" );
    	$vartmp = "notes";
    	$infotmp= $projPrefs[$vartmp];
    	echo "<td><input type=text name=\"setti[".$seriVar.".".$vartmp."]\" value=\"".$infotmp."\" size=4></td>";
    	prefClose ("Number of characters of Notes-fields of objects (typical: 20, off: 0)");
    	
    	
    	prefNameShow( "Show owner" );
    	$vartmp = "addinfown";
    	$infotmp= $projPrefs[$vartmp];
    	$checked_tmp="";
    	if ($infotmp) $checked_tmp="checked";
    	echo "<td><input type=checkbox name=\"setti[".$seriVar.".".$vartmp."]\" value=\"1\" $checked_tmp></td>";
    	prefClose ("Show owner of object, <font color=#EEBB00><b>Warning:</b></font> this is more time cosuming");
    	
    	prefNameShow( "Show date" );
    	$vartmp = "addinfdate";
    	$infotmp= $projPrefs[$vartmp];
    	$checked_tmp="";
    	if ($infotmp) $checked_tmp="checked";
    	echo "<td><input type=checkbox name=\"setti[".$seriVar.".".$vartmp."]\" value=\"1\" $checked_tmp></td>";
    	prefClose ("Show date of object, <font color=#EEBB00><b>Warning:</b></font> this is more time cosuming");
    	
    	
    	prefSeparator();
    	
    	prefNameShow( "Slim skin" );
    	$vartmp = "slim";
    	$infotmp= $projPrefs[$vartmp];
    	if ( $infotmp=="1" ) $infotmp="checked";
    	echo "<td><input type=checkbox name=\"setti[".$seriVar.".".$vartmp."]\" value=\"1\" ".$infotmp."></td>";
    	prefClose ("Slim skin with reduced functionality? (reduces also the web-traffic)");
    	
    	prefNameShow( "Lab specific view" );
    	$vartmp = "addlab";
    	$infotmp= $projPrefs[$vartmp];
    	if ( $infotmp=="1" ) $infotmp="checked";
    	echo "<td><input type=checkbox name=\"setti[".$seriVar.".".$vartmp."]\" value=\"1\" ".$infotmp."></td>";
    	prefClose ("Add a lab specific view?");
    }
    
    function _singleSaveSub(&$tmpexp2, &$setti, $flagname) {
    
        $formvar  = "sop.EXP.".$flagname;
    	unset( $tmpexp2[$flagname] );
    	$tmp_val = $setti[$formvar] ;
    	if ( $tmp_val != "" ) $tmpexp2[$flagname] = $tmp_val;
    
    }
    
    function prefSave( &$sql, $info, $setti ) {
        
        
        $tmpexp = unserialize($_SESSION['userGlob']["o.PROJ.sop.EXP"]);
        $tmpexp2 = array(); // create new array
        
        $this->_singleSaveSub($tmpexp2, $setti, "sample");
    	$this->_singleSaveSub($tmpexp2, $setti, "SHORTRES");
        
    	
    	$_SESSION['userGlob']["o.PROJ.sop.EXP"] = serialize($tmpexp2);    
    
    	
    	$tmpSubst = NULL;
    	$seriVar  = "o.SUC.";
    	$flagname = "batch";
    	$tmp_val  = $setti[$seriVar.".".$flagname] ;
    	$tmpSubst[$flagname] = $tmp_val;
    	
        $_SESSION['userGlob']["o.PROJ.sop.CONCRETE_SUBST"] = serialize($tmpSubst);  
        
        $tmpSubst = NULL;
        $seriVar  = "o.CHIP_READER.";
        $flagname = "location";
        $tmp_val  = $setti[$seriVar.".".$flagname] ;
        $tmpSubst[$flagname] = $tmp_val;
        
        $_SESSION['userGlob']["o.PROJ.sop.CHIP_READER"] = serialize($tmpSubst);  
        
       
        
        // --------------------
    
        $seriVar = "o.PROJ.pref";
    	$projPrefs = NULL;
    	
    	
    
        $flagname = "maxob";
    	$tmp_val = $setti[$seriVar.".".$flagname] ;
    	if ($tmp_val==200 OR $tmp_val<=0) $tmp_val="";
    	$projPrefs[$flagname] = $tmp_val;
    	
    	$flagname = "slim";
    	$tmp_val = $setti[$seriVar.".".$flagname] ;
    	$projPrefs[$flagname] = $tmp_val;
    	
    	$flagname = "notes";
    	$tmp_val = $setti[$seriVar.".".$flagname] ;
    	if ( is_numeric($tmp_val) ) $useval = $tmp_val;
    	else $useval = "";
    	$projPrefs[$flagname] = $useval;
    	
    	$flagname = "addinfown";
    	$tmp_val = $setti[$seriVar.".".$flagname] ;
    	if ($tmp_val) $projPrefs[$flagname] = $tmp_val;
    	
    	$flagname = "addinfdate";
    	$tmp_val = $setti[$seriVar.".".$flagname] ;
    	if ($tmp_val) $projPrefs[$flagname] = $tmp_val;
    	
    	$flagname = "addlab";
    	$tmp_val = $setti[$seriVar.".".$flagname] ;
    	$projPrefs[$flagname] = $tmp_val;
    
    	$_SESSION['userGlob'][$seriVar] = serialize($projPrefs);
        
    }
}
