<?php
/**
 * message boxes for html pages
 * more HTML-methods see func_head2.inc !!!
 * @package func_head.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/phplib/func_head.inc 59 2018-11-21 09:04:09Z $
 */


require_once('globals.inc');
require_once('object.subs.inc');
require_once('javascript.inc');
require_once('func_head2.inc');

/**
 * warning/errorbox
 * @param string $warnlevel "Warning", "Error", other
 * @param string $headtext e.g. "This is a warning box!"
 * @param string $message
 */
function htmlErrorBox( $warnlevel, $headtext,  $message = NULL  ) {

	//print "<table border=0 cellspacing=1 cellpadding=0 bgcolor=#800000><tr><td>";
    print "  <table border=0 cellspacing=0 cellpadding=5 bgcolor=#FFD0D0><tr><td>";
    
    if ($message !="") $headtext="<B>".$headtext."</B>";
    print "<font color=red><B>".$warnlevel.":&nbsp;</B></font>".$headtext."\n";
    if ($message!="") print "<br><br>\n".$message;   
    print "  </td></tr></table>\n";
    //print "</td></tr></table>\n";
}

/**
 * show SHORT/LONG error from Error stack
 * @example:
 * $updateError = $error->getAll();
 * htmlErrorClassBox('Update Error, $updateError);
 * 
 * @param string $title
 * @param string $INFOMODE
 * @param object $error_stack
 */
function htmlErrorClassBox($title, $INFOMODE, &$error_stack) {
    
    $opt=array('morehead'=>' &nbsp;[<a href="javascript:goz_error_box()" style="color:white">more</a>]');
    htmlInfoBox( $title, "", "open", $INFOMODE, $opt );
    
    echo '<div id="x_error_short">'."\n";
    foreach($error_stack as $single_error) {
        echo $single_error->text."<br>"."\n";
    }
    echo '</div>'."\n";
    
    echo '<div id="x_error_long" style="display:none;">'."\n";
    echo '<table>'."\n";
    echo '<tr><th>Code</th><th>Err-No</th><th>Message</th></tr>'."\n";
    foreach($error_stack as $single_error) {
        
            $tmp = $single_error->getAsArray();
            $tmp_id = $tmp['id'];
            if ($tmp['warn']) $tmp_id = 'WARN:'.$tmp_id;
            echo '<tr valign=top>';
            echo '<td>',$tmp['ori'],'</td>';
            echo '<td>',$tmp_id,'</td>';
            echo '<td align="left">',$tmp['text'],'</td>';
            echo '</tr>',"\n";
            
    }
    echo '</table>'."\n";
    echo '</div>'."\n";
        
    htmlInfoBox( "", "", "close", "" );
}

/**
 * MESSAGE BOX warning/errorbox
 * @param string $headtext  "This is a info box!"
 * @param string $message  "Message
 * @param string $outputMode
     ""        open box and put message direct
    "open"    open box  (keep box open)
    "close"   close box (ignore $headtext)
 * @param string $INFOMODE   "HELP", "INFO", "CALM", "ERROR", "WARN", "WARNRED", "FORM"
 * @param array $options  
       "width" : "100%", ["AUTO"], ...
       "nohead" [0] | 1   suppress head
       "morehead" more headtext, but not BOLD
       "icon"
       "color"   backcolor of text-message e.g. #EFEFEF
       "fcolor"  frame color e.g. #D0D0D0
       "vspace"  1: give vertical space around box
 */
function htmlInfoBox(  $headtext, $message= NULL, $outputMode = NULL,  $INFOMODE= NULL,$options= NULL) {

    $color1= "#336699"; // framecolor
    $color2= "#EFEFFF"; // backcolor of text-message
	$color3= "#FFFFFF"; // head-text color
    $icon  = "";
    
    switch ($INFOMODE) {
    	case "HELP":
            $color1= "#336699";
            $color2= "#F8F8F8"; 
            $icon  = "ic.docu15.gif";
            break; 
        case "ERROR":  // calm outfit, but not a help
            $color1= "#FF0000";   // #336699
            $color2= "#FFD0D0";
            break;
        case "CALM":
            $color1= "#A0A0A0";   
            $color2= "#F8F8F8";
            break;
    	case "INFO":
            $color1= "#3399FF";
            $color2= "#EFEFFF";
            break;
    	case "WARN": // WARNING (yellow)
            $color1= "#000000";
            $color2= "#E8EB05";
    		$color3= "#E8EB05";
    		$icon  = "i13_warning.gif";
            break;
    	case "WARNRED": // WARNING (red)
            $color1= "#FF0000";
            $color2= "#FFD0D0";
    		$icon  = "i13_warning.gif";
            break;
        case "FORM": // FORM: blue
            $color1= "#CAD5E3";
            $color2= "#F8F8F8"; 
            $color3= "#46648C";
            break;
    }
	
	if ($options["color"]!="") {
		$color1= "#000000";
        $color2= $options["color"];
	}
	if ($options["fcolor"]!="") {
		$color1= $options["fcolor"];
	} 
	
	if ($options["icon"]!="") $icon=$options["icon"];

    if ( $outputMode!="close" ) {                        
        $twidth="";
        if ($options["width"]!="")   {
            $twidth="max-width: ".$options["width"]."; ";
			  
		}
        if ($options["width"]=="AUTO") {
			$twidth="";
        }   
		if ($options["vspace"]>0) echo "<br>\n"; 
		
		$x_msg_style='';
		if ($twidth!=NULL) {
		    $x_msg_style='style="'.$twidth.'"';
		}
		print '<div class="x-msg" '.$x_msg_style.'>'."\n";
		
		if ($options["nohead"] != 1) {
		    print '<div class="x-msg-header" style="background: '.$color1.'">';
		    if ($icon!="") echo '&nbsp;<img src="'.$_SESSION['s_sessVars']["loginURL"].'/images/'.$icon.'">&nbsp; ';
		    print $headtext.
		      ' '.$options["morehead"].
		      '</div>'."\n";
		}
		print '<div class="x-msg-container" style="border-color: '.$color1.'">'."\n";

//         print "<table border=0 cellspacing=1 cellpadding=0 bgcolor=". $color1 .' '.$twidth."><tr><td>";
//         if ($options["nohead"] != 1) {
//             print '  <table style="border:0px; margin: 3px 3px 3px 10px; background-color:'.$color1.' width: 100%;"><tr><td>';  
//             if ($icon!="") echo '&nbsp;<img src="'.$_SESSION['s_sessVars']["loginURL"].'/images/'.$icon.'">&nbsp; ';
//             print '    <span style="color:'.$color3.'"><B>'.$headtext.'</B> '.$options["morehead"].'</span></td>';
//             print "    </tr></table>\n";
//         }
//         print "  <table border=0 cellspacing=0 cellpadding=5 bgcolor=$color2 width=100%><tr><td>";
        if ($message!="") print $message;   
    }
    if ($outputMode=="" OR $outputMode=="close") { 
        print '</div>'."\n";
        print '</div>'."\n";
//         print "</td></tr></table>\n";
//         print "</td></tr></table>\n"; 
        
		if ($options["vspace"]>0) echo "<br>\n";
    } 
}
     

function htmlCheckHelp( 
	$helppage, 
	$labflag = "auto"  // ["auto"], "lab"
	) {
// FUNCTION: check for a help page
       $robolink =  "";
	   
	   
       $helpPageRobo = "../lab/help/robo/".$helppage; 
       if ( file_exists( $helpPageRobo ) ) {  
            $robolink =  $helpPageRobo;
       }
	   if ($labflag=="lab")  return $robolink;
	   
       $helpPageRobo = "help/robo/".$helppage;                                   
       if ( file_exists( $helpPageRobo ) ) {  
            $robolink =  $helpPageRobo;
       }
       return $robolink;
}   

// update left frame of HTML-window
function htmlShowHistory ($table=NULL, $id=NULL) {
 
    $destHref = $_SESSION['s_sessVars']["loginURL"]."/frame.left.nav.php"; // do absolute link
    
    if ($_SESSION['s_sessVars']["frameL.mode"] != "")  return; // no refresh, if mode !!!
    
    echo '<script language="JavaScript"><!--',"\n";
    echo ' if (parent.left != null) {';
    echo '   parent.left.location.replace(\''.$destHref.'\');';
    echo ' }';
    echo '//--></script>'."\n";
}





function htmlObjIcon($tablename, $relative=0) {
	
	
	if ($relative) {
		$pathNFS = $_SESSION['s_sessVars']["loginPATH"]."/";
		$pathWEB = $_SESSION['s_sessVars']["loginURL"]."/";
	} else {
		$pathNFS = "";
		$pathWEB = "";
	}
	
	$icon  = file_exists($pathNFS."images/icon.".$tablename.".gif") ? 
				$pathWEB."images/icon.".$tablename.".gif":
				$pathWEB."images/icon.UNKNOWN.gif";
	return ($icon); 
}

/**
 * writes the html-footer and ends the script.
 * -if both parameters are given, they are printed through info_out
 * -if only one parameter is given it is printd using print
 * @param string $red_text
 *  - if not given: print <hr>
 * @param string $text
 * @param string $textopt
 * @param string $posttext text after MESSAGEBOX; e.g. <hr>
 */
function htmlFoot( $red_text="",       $text="", $textopt="", $posttext="") {
    
    if (!empty($text)) {
	    htmlErrorBox($red_text, $text, $textopt);
    } else {
        if ($red_text=='') $red_text='<br><hr>'."\n";
	    print ($red_text);
    }
  
  echo $posttext;
  die("\n</body>\n</html>");
}

// handle exception
// TBD: write this error to error log
function htmlFootExc( $e ) {
    $errmess = $e->getMessage( );
    $trace_string = $e->getTraceAsString();
    $trace_string = str_replace("\n", "<br>", $trace_string);
    htmlFoot('ERROR', $errmess.': '.$trace_string);
}
