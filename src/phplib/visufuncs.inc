<?php
/**
 * helper methods for HTML visualization
 * - tables
 * - graphic buttons
 * - links
 * @namespace core::gui
 * @package visufuncs.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com) 
 * @version $Header: trunk/src/phplib/visufuncs.inc 59 2018-11-21 09:04:09Z $
 */
class visufuncs { 
	var $options;

    /**
     * show full table
     * @param ARRAY $header can be NULL
     * @param array $tmparr array of values or just the value
     * @param string $opt
     * // "showid" = [0] | 1
      // "title"  = title-text
      // "mode"   = "" , 
      //			 "easy" : gray; bold; two columns
      // "keyTxtAlign" : ["left"], "right"
     */
    static function table_out( &$header, &$tmparr,  $opt=NULL ) {
        
    	$bgcolorCmd = " bgcolor=#B0B0B0";
    	
    	if ($opt["mode"]=="easy") {
    		$optAlign="";
    		if ($opt["keyTxtAlign"] !="" ) $optAlign=" align=".$opt["keyTxtAlign"];
    		
    		echo "<table cellpadding=1 cellspacing=1 border=0>";
    		
    		foreach( $tmparr as $idx => $valarr ) {
    		
                echo "<tr>"; 
                echo "<td nowrap".$optAlign."><font color=gray>".$valarr[0].":</font>&nbsp;</td>";
    			echo "<td><B>".$valarr[1]."</B></td>";
                echo "</tr>\n";
            }
    		echo "</table>\n";
    		return;
    	} 
    	
        echo "<table cellpadding=1 cellspacing=1 border=0" .$bgcolorCmd. ">";
    	if ($opt["title"]!="") {
    		$colspan=2;
    		if (sizeof($header)) $colspan=sizeof($header);
    		echo "<tr><td colspan=$colspan><font color=white><b>".$opt["title"]."</b></font></td></tr>";
    	}
        if (is_array($header)) {
    		echo "<tr bgcolor=\"#D0D0D0\">";
    		foreach( $header as $tmptxt ) {
    			echo "<td><font color=#707070><B>".$tmptxt."</B></font></td>";
    		}  
    		
    		echo "</tr>\n"; 
    	}
                  
    	$color  = '';
        $color1 = "#EFEFEF";  // SUN violett 
        $color2 = "#EFEFFF";
        if ( sizeof ($tmparr)  )  {
            foreach( $tmparr as  $idx=> $valarr ) {
            
                if ($color == $color1)  $color = $color2;
                else $color = $color1;  
                echo "<tr bgcolor=\"".$color."\">"; 
                if ($opt["showid"])  echo "<td>".$idx."</td>";
                
                if ( is_array ( $valarr ) ) {
                    foreach( $valarr as $tmptxt) {
                        echo "<td>".$tmptxt."</td>";
                    }
                } else echo "<td>".$valarr."</td>";
                echo "</tr>\n";
            }
        }
       
        echo "</table>\n";
    }   
    
    
    /**
     *  based on $this->table_head(), $this->table_row, $this->table_close
     * @param array $header
     * @param array $dataarr
     * @param string $opt
     */
    function table_out2( 
    	&$header, 
    	&$dataarr,
    	$opt=NULL 
    	) {
    	
    	$this->table_head($header, $opt);
    	
    	if (sizeof($dataarr)) {
    	    foreach( $dataarr as $valarr ) {
    		 	$this->table_row ($valarr);
    		 }
    		
    	}
    	$this->table_close();
    }
    
    // show key-value pairs
    function table_out3( &$header, &$keyValArr, $opt=NULL ) {
    	$this->table_head($header, $opt);
    	if (sizeof($keyValArr)) {
    		
    	    foreach( $keyValArr as $key=> $val  ) {
    		 	$this->table_row (array($key,$val));
    		}
    	}
    	$this->table_close();
    }
    
    /**
     * start table head
     * @param  $header  // can be NULL
     * @param  array $opt
     *  "title"   = title-text
     *  "title_notes" => notes in title
     *  "colopt"  = <DEPRICATED: use "cssclass"> array[COLID] = text
     *  "headopt" = <DEPRICATED: use "cssclass">  array[COLID] = text -- header TD-options
     *  "headNoShow" = [0], 1 -- do not show header
     *  "borderColor" <DEPRICATED: use "cssclass">   = #EFEFEF
     *  "cellColor"   <DEPRICATED: use "cssclass"> = #F3F3F3
    
     *  "cssclass"     e.g. "datatab"
     *  "colThCss" = array of styles for header
     *    e.g. "short_name" : shorten the name (see def for css:yTable)
     *    
     *  "format  = ["html"] | "csv"
     */
    function table_head( &$header,  $opt=NULL ) {
    
    	$borderColor="#B0B0B0";
    	if ($opt["borderColor"]!="") $borderColor=$opt["borderColor"];
    	
    	$thisclass = 'class="yTable" '; // new standard class
    	if ($opt["cssclass"]!="") {
    		$thisclass = "class=\"".$opt["cssclass"]."\" ";
    	}
    	$this->cellColor = "#F3F3F3";
    	if ( $opt["cellColor"]!="" )  $this->cellColor = $opt["cellColor"];
    	
    	$this->options = $opt;
    	
    	if ($this->options["format"]=="csv") {
    	
    		if ($opt["title"]!="") {
    			echo "# ".$opt["title"]."\n";
    		}
    		if (is_array($header) AND !$opt["headNoShow"]) {
    			$tmptab = "";
    			foreach( $header as $tmptxt ) {
    			
    				echo $tmptab. $tmptxt;
    				$tmptab="\t";
    			}
    			echo "\n"; 
    			
    		}
    		
    	} else {
    	
    		echo "<table cellpadding=1 cellspacing=1 border=0 ".$thisclass."bgcolor=".$borderColor.">\n";
    		
    		echo '<thead>'."\n";
    		if ($opt["title"]!="") {
    			$colspan=2;
    			if (sizeof($header)) $colspan=sizeof($header);
    			echo '<tr><td colspan='.$colspan.' style="padding:3px; color:white; font-weight:bold;">'.$opt["title"];
    			if ($opt["title_notes"]) {
    			    echo ' <span='.$colspan.' style="font-weight:normal;">'.$opt["title_notes"].'</span>';
    			}
    			echo '</td></tr>'."\n";
    		}
    		
    		//if ($opt["noCellBorder"]) echo "<tr><td><table cellpadding=1 cellspacing=0 border=0 width=100% bgcolor=".$borderColor.">\n";
    		
    		if (is_array($header) AND !$opt["headNoShow"]) {
    			
    			echo "<tr bgcolor=\"#D0D0D0\" style=\"color:#707070; font-weight:bold;\">";
    			$i=0;
    			foreach( $header as $tmptxt ) {
    			
    				$tdopt = $opt["headopt"][$i];
    				if ($opt["colThCss"][$i]!="" ) {
    					$tdopt .=" class=\"".$opt["colThCss"][$i]."\"";
    				}
    				echo "<th $tdopt>".$tmptxt."</th>";
    				$i++;
    			} 
    			echo "</tr>\n";
    			
    		}
    		$this->colopt  = $opt["colopt"];
    		
    		echo '</thead>'."\n";
    	} 
    }
    
    /**
     * @param $dataArr
     * @param $opt=NULL	<pre>
     * 	"bgcolor" = "#EEEEEE"
     *  "colspan" = NUMBER
     *  "other"   = variable text per column
     *  "trOpt"   = optional text in TR-tag
     *  "td_style_arr" = array [$i] = TD-option
     *  </pre>
     */
    function table_row( $dataArr, $opt=NULL) {
    	
    	if ($this->options["format"]=="csv") {
    	
    		$i=0;
    		$tmptab = "";
    		foreach( $dataArr as $tmptxt ) {
    			echo $tmptab.$tmptxt;
    			$tmptab="\t";
    			$i++;
    		}
    		echo "\n";
    		
    	} else {
    	    
    	   
    	
    		$thisbgcolor = $this->cellColor;
    		if ($opt["bgcolor"] !="") $thisbgcolor = $opt["bgcolor"];
    		$tmptd = "";
    		if ($opt["colspan"] !="") $tmptd  = " colspan=\"".$opt["colspan"]."\"";
    		if ($opt["other"] !="")   $tmptd .= " ".$opt["other"];
    		
    		echo "<tr bgcolor=".$thisbgcolor.' '.$opt["trOpt"].'>'; 
    		$i=0;
    		foreach( $dataArr as $tmptxt ) {
    			if (is_array($opt["td_style_arr"])) {
    				$tmptd2 = " ".$opt["td_style_arr"][$i];
    			} else $tmptd2=NULL;
    			
    			
    			echo "<td ".$this->colopt[$i].$tmptd.$tmptd2.">".$tmptxt."</td>";
    			$i++;
    		}
    		echo "</tr>\n";
    	}
    }
    
    /**
     * set HTML-option per column <td>
     * @param array $colopt = array(POS=>text)
     */
    function setColOpt($colopt) {
    	$this->colopt = $colopt;
    }
    
    	
    function table_close() {
    	if ($this->options["format"]=="csv") {
    		// nothing ....
    	} else {
    		//if ($this->options["noCellBorder"]) echo "</table></td></tr>\n";
    		echo "</table>\n";
    	}
    }   
    
    //
    // simple table get HTML text as return value ....
    //
    
    function stable_head($header) {
        $out='';
        $out .= "<table>\n"; 
        $out .= '<tr>';
        foreach( $header as $tmptxt ) {
            $out .= "<th>".$tmptxt."</th>";
        }
    
        $out .= "</tr>\n";
        return $out;
    }
    function stable_row($data) {
        $out='';
        $out .= '<tr>';
        foreach( $data as $tmptxt ) {
            $out .= "<td>".$tmptxt."</td>";
        }
        
        $out .= "</tr>\n";
        return $out;
    }
    function stable_close() {
        $out='';
        $out .= "</table>\n"; 
        return $out;
    }
    
    /**
     * show slim header
     * @param string $text
     */
    static function slimhead($text) {
    	echo "<table cellpadding=3 cellspacing=2 border=0 width=600>";
    	echo "<tr bgcolor=#E8E8F8><td><font color=#808080>&nbsp;<b>".$text.":</B></td></tr>\n";
    	echo "</td></tr><tr><td></td></tr></table>\n";
    }
    
    static function linkOut( 	$url, $text,	$active	) {
    	// get link-text
    	if (!$active) $outtext = "<font color=gray>[".$text."]</font>";
    	else $outtext = "[<a href=\"".$url."\">".$text."</a>]";
    	
    	return ($outtext);
    }

}
