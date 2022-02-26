<?php
/**
 * help methods for frame.left.nav.php
 * @package frame.left.func.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/www/pionir/frame.left.func.inc 59 2018-11-21 09:04:09Z $
 */
class frameLc {

	/**
	 * 
	 * @param unknown $headtxt
	 * @param string $opt
	 * "addlink" = TEXT
	   "back" 0|1
	 */
	static function header3out( $headtxt, $opt=NULL ) {
		$HeadTxtColor = "#555566"; //#CCCCCC
		
		echo '<table  class="yLFramHead3">'; // 
		// echo "<tr><td colspan=2><img src=\"0.gif\" height=1 width=249></td></tr>\n";
		echo '<tr><td><img src="images/ic.frmmode.gif" hspace=5>';
	    echo "<B><font color=".$HeadTxtColor.">".$headtxt."</font></B></td>";
		echo "<td align=right>&nbsp;";
		if ($opt["addlink"]!="") echo $opt["addlink"]."&nbsp;";
		if ($opt["back"])
			echo "<a class=yhea1 href=\"frame.left.nav.php?mode=\"><img src=\"res/img/x.svg\" border=0 TITLE=\"back to history\"></a>&nbsp";
	    echo "</td></tr>\n";
		//echo "<tr><td colspan=2><img src=\"0.gif\" height=1 width=249></td></tr>\n"; // placeholder horizontal
	    echo "</tr></table>\n";
	}
	
	static function header2open() {
	
		echo '<table class="yLFramHead2">';
	    echo '<tr><td><img src=0.gif height=2 width=1>';
	    echo '</td></tr>';
	
	}

}
