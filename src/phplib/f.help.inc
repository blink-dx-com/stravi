<?php

/**
 * help-doc functions; get HTML-strings of online-documentation
 * @package f.help.inc
 * @swreq   GLOBAL
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/phplib/f.help.inc 59 2018-11-21 09:04:09Z $
 */
class fHelpC {
	
/**
 * get the string for the help-URL
 * @param string $robo_url
 * @param string $location : ['pionir'], 'lab', 'wiki' 
 * @return string
 */
static function get_help_url($robo_url, $location=NULL) {
	
	$loginUrl  = $_SESSION["s_sessVars"]["loginURL"];
	
	$help_base = $loginUrl.'/help/robo/';
	switch ($location) {
		case 'lab':
			$help_base = $loginUrl.'/../lab/help/robo/';
			break;
		case 'wiki':
			$help_base = $_SESSION['globals']['app.helpUrls']['wiki'].'/';
			break;
	}
	
	$hrefer = $help_base.$robo_url;
	
	return $hrefer;
}

/**
 * get help link+text
 * @param $robo_url html-file-name in "robo doc directory"
 * @param string $useText the shown text
 * @param array $option  
 * 		"object"   => ["text"] | "icon" -- if set, show help-icon
 * 		'location' => key of alternativ location
 *          	      ['pionir'], 'lab', 'wiki' 
 * @return string
 */
static function link_show ($robo_url, $useText, $option  = NULL ) {
	
	
	$outtext = "";
	if ($robo_url == '') return ($useText);
	
	$hrefer = self::get_help_url($robo_url, $option['location']);
	
	if ( $option["object"]=="icon" ) {
		$infoobj = "<img src=\"images/help.but.gif\" border=0 TITLE=\"".$useText."\">";
	} else {
		$infoobj = $useText;
	}
 
	$outtext = "<a href=\"".$hrefer."\" target=_help>".$infoobj."</a>";   
	return ($outtext);
}



/**
 * check for a help page
 * @param  $helppage : filename without path
 * @param  $labflag  : ["auto"], "lab", "pionir"
 */
static function htmlCheckHelp( 
	$helppage, 			
	$labflag = "auto"   
	) {
	
	
	$robolink =  "";
	
	if ( $_SESSION['globals']["lab_path"]!="" ) {
		$helpPageRobo = "../".$_SESSION['globals']["lab_path"]."/help/robo/".$helppage; 
		if ( file_exists( $helpPageRobo ) ) {  
			$robolinkL = $helpPageRobo;
			$robolink  = $robolinkL;
		}
	}
	if ($labflag=="lab")  return $robolinkL;
	
	$helpPageRobo = "help/robo/".$helppage;                                   
	if ( file_exists( $helpPageRobo ) ) {  
		$robolinkP = $helpPageRobo;
		$robolink  = $robolinkP;
	}
	if ($labflag=="pionir")  return $robolinkP; 
	
	return $robolink;
} 

/**
 * - produce a helpfile-URL (rawurlencode)
   - replace % by "_", because % is no valid file-character 
   - check if a help entry exists
   example: 
    $tablename = "H_EXP_RAW_DESC"
	$entryName = "ic [cellcount]"
	$fullLink  = o.H_EXP_RAW_DESC.ic_20_5Bcellcount_5D.html	
 * @param  $tablename 
 * @param  $entryName
 */
static function getTableEntryHelp($tablename, $entryName) {
	
	$entryCoded = str_replace("%", "_", rawurlencode($entryName));
	$fileUrlShort = "o.".$tablename.".".$entryCoded.".html";
	$fullLink = self::htmlCheckHelp( $fileUrlShort );
	return ($fullLink);
}

}
