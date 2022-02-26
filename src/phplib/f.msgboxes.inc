<?php


/**
 * show message boxes
 * @package f.msgboxes.inc
 * @swreq UREQ:GLOBAL
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/phplib/f.msgboxes.inc 59 2018-11-21 09:04:09Z $
 */
class cMsgbox {

	/**
	 * show slim message box
	 * @param string $type : "ok", "error", "warning", "info"
	 * @param string $text : the message text
	 * @param string $moretext : optional more text in extra line
	 * @param string $open : '':default; 'open', 'close'
	 */
	static function showBox($type,  $text	 =NULL, $moretext=NULL, $open=NULL ) {
		
	  $optx=array();
	  $optx["framecol"] = "#BFF588";
	  $optx["icon"]     = "ic.ok.gif";
	  
	  if ($type == "error") {
	  	  $optx["framecol"] = "#FF3333";
	  	  $optx["icon"]     = "ic.err40.gif";
	  } 
	  if ($type == "warning") {
	  	$optx["framecol"] = "#F2E346";
	  	$optx["icon"]     = "ic.warn40.gif";
	  }
	  if ($type == "info") {
	      $optx["framecol"] = "#3333FF";
	      $optx["icon"]     = "ic.info40.png";
	  } 
	  
	  $closeTxt = "</table></td></tr></table>\n";
	  
	  if ($open!="close") {
	    ?>
	    <table cellpadding="1" cellspacing="0" border="0" width="100%" bgcolor="<?echo $optx["framecol"]?>">
		<tr><td>
		<table cellpadding="5" cellspacing="0" border="0" width="100%" bgcolor="#ffffff">
		 <tr><td width="40" valign="top" align="center">
			<img src="<?echo $_SESSION['s_sessVars']["loginURL"]?>/images/<?echo $optx["icon"]?>" width="37" height="40"/>
		  </td>
		  <td ><?echo $text?></td>
		 </tr>
		 <?
	   }
	   
	   if ($moretext!="") {
		 	echo "<tr><td>".$moretext."</td></tr>\n";
	   }
	   
	   if ($open!="open") echo $closeTxt;
	  
	}
	
	/**
	 *  bool printAll (string origin = "", int error_id = 0)
	 * prints all or the specified errors to the screen (only nice description)
	   removes the shown errors from the error-stack.
	   returnes wether an error was printed to the screen or not.
	 * @return boolean
	 */
	 function printAllEasyWarn( ) {
		global $error;
		
		$origin=0;
		$error_id=0;
	    $error_stack = $error->getAll($origin, $error_id);
	
	    if (count($error_stack) == 0)
	      return false; // don't print anything if no error in stack.
		
		htmlInfoBox( "Warnings", "", "open", "WARN" ); 
	    echo '<table border="0" cellspacing="1" cellpading=0>';
	    foreach($error_stack as $single_error)
	        $single_error->printTRs();
	    echo "</table>";
		htmlInfoBox( "", "", "close" ); 
		
	    return true;
  }

}
