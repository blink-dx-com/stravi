<?php


/**
 * HTML tab view (Reiter)
 * @package f.rider.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/phplib/f.rider.inc 59 2018-11-21 09:04:09Z $
 */
class fRiderC {

	var $bgcolor;
	
	function __construct() {
		$this->bgcolor = "#3366cc";
	}
	
	/**
	 * 
	 * @param array $xmodes   array[KEY] = array( text, url)
	 * @param string $selKey
	 * @param string $title
	 * @param string $options
	 *  "bgcolor"  
		"noTable" 0,1 : do not show the outer table
		"backurl" -- optional back-text
		"addcell"
	 */
	function riderShow( &$xmodes, 	$selKey, $title, $options= NULL ) {

		$bgcolor = "#3366cc";
		if ($options["bgcolor"] !="") $bgcolor=$options["bgcolor"];
		$this->bgcolor = $bgcolor;
	
		$cnt     = 0;
		
		echo '<table style="border-bottom: 2px solid '.$bgcolor.';"  cellpadding=0 cellspacing=0><tr bgcolor='.$bgcolor.'>'."\n";
		if ($options["backurl"]!="") {
			echo "<td bgcolor=#EFEFEF>&nbsp;<a href=\"".$options["backurl"]."\">&lt;&lt;</a>&nbsp;</td>";
	 		$cnt++;
		}
		echo "<td bgcolor=#FFFFFF>&nbsp;<b><font color=gray>".$title." </font></b></td>";
	    echo "<td bgcolor=white>&nbsp;&nbsp;&nbsp;&nbsp;</td>";

	      
	   foreach( $xmodes as $key=>$tmparr) {
	 
	   	 $xtext  = $tmparr[0];
		 $tmpurl = $tmparr[1];
		 
	     $in_selected_xmode = false;
	 
		 do {
			if (($selKey=="") AND !$cnt) {
			 	$in_selected_xmode = true;
				break;
			}
			if ($selKey == $key) $in_selected_xmode = true;
	 	 } while (0);
		 
	     if ($in_selected_xmode) {
	       echo "<td bgcolor=".$bgcolor.">&nbsp;<B><a href=\"".$tmpurl."\"><font color=white>".
				   $xtext."</font></a></B>&nbsp;</td>";
	     } else {
	       echo "<td bgcolor=#EFEFEF>&nbsp;<a href=\"".$tmpurl."\">".$xtext."</a>&nbsp;</td>";
	     }
	     echo "<td bgcolor=white>&nbsp;&nbsp;</td>";
	     $cnt++;
	   } 
	   
	   
	   if ($options["addcell"]!="") {
		   echo "<td  bgcolor=#FFFFFF>&nbsp;".$options["addcell"]."</td>";
	   } 
	   
	   echo "</TR>";
	   echo "</table>\n";
	
	}
	
	function showNotes($notes) {
		echo "<font color=".$this->bgcolor.">".$notes."</font><br>\n";
	}

}
