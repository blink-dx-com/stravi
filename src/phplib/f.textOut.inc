<?php
/**
 * sub function for HTML text formatting, e.g. link lists 
 * @namespace core::gui
 * @package f.textOut.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/phplib/f.textOut.inc 59 2018-11-21 09:04:09Z $
 * @example <pre>
 
 * $flist = array();
 * $flist[] = array("ty"=>"head", "txt"=>"Header Text"  );
 * $flist[] = array("ty"=>"lnk",  "txt"=>"Service Log Suche" , 
		"href"=> "p.php?mod=DEF/o.CHIP_READER.logSearch&projid=".$devProj_id, 
		"icon"=>"images/but.search.gif", "li"=>"br" );
    $flist[] = array("ty"=>"br"); 
	$flist[] = array("ty"=>"headclose");
	
	
	$textoutObj = new textOutC();
	$textoutObj->linksOut($flist);
 * </pre>
 */
class textOutC {
	var $listLay;
	
	/**
	 * 
	 * @param array  $opt
	 *    "imgFullPath" = 0,1
	 *    'font-size' : ['STD'], 'large'
	 */
    function __construct($opt=NULL) {
    	
    	$this->listLay=0;
    	$this->opt = $opt;
    	$this->pioPath = "";
    	if ($opt["imgFullPath"]>0) {
    		$this->pioPath = $_SESSION["s_sessVars"]["loginURL"]."/";
    	} 
    	
    	$this->icon_default=NULL; // default icon
    	
    	$this->separator_ini   = "<li>";
    	$this->separator_close = "</li>";
    }
    
    function set_option($key, $val) {
        
    }
    
    /**
     * show text/link list
     * @param array $textlist
    		 ["ty"] : 
    		   "head", 
    		      options: 
    		         ["spaceic"]: 
    		 			[0],
    		 			>0 : number of space pixels; if no icon given: set empty icon on front of each line	  
    		 		 "li_std" : standard LI for next items ["li"], "br", "icli"
    		 		 "lay" : currently not implemented: BLOCK level: 1,2,..
     		   "lnk", 
    		   "headclose",
    		   "br"  : lineBreak 
     		   "raw" : print only "txt"
    		 		 
    		 		
    		 ["txt"]  : the text
    		 ["href"] : the reference text
    		 ["notes"]: notes behind link
    		 ["icon"] : icon-url
    		 ["iicon"]: icon-name on "images/"
    		 ["iheight"] : icon-height [OPT]
    		 ["icon_opt"] : extra HTML text in IMG tag [OPT]
    		 ["icbord"] : 0|1
    		 ["target"] : name
    		 ["li"]   : set LIST-separator, can also be set by "li_std" ...
    		    ["li"], "br", "icli"
    		 "txtInTab": 0|1 : show <td>icon</td> and <td>text</td> in TABLE
     * @param array $option  "ulbr" => 0|1 extra line breaks
    				   "ulclass" = css-class-name
     */
    function linksOut(&$textlist, $option   = NULL) {
    	
    	
    	$ulclass = ' class="yul2"';
    	if ( $option["ulclass"]!="") $ulclass = " class=\"".$option["ulclass"]."\"";
    	
    	if ($this->opt['font-size']=='large') {
    	    echo '<span class="yGlarger">'."\n";
    	}
    	
    	foreach( $textlist as $key => $valarr ) {
    	
    		$out     = "";
    		$imgfile = "";
    		$sep     = $this->separator_ini;
    		$sepClose= $this->separator_close;
    		
    		if ($valarr["li"]!=NULL) {
        		if ($valarr["li"]=="br") {
        			$sep = "";
        			$sepClose = "<br>";
        		} 
        		if ($valarr["li"]=="icli") {
        			$sep = "";
        			$sepClose = "<br>";
        			$imgfile  = "<img src=\"".$this->pioPath."images/i13._li.gif\" border=0>&nbsp;";
        		} 
    		} else {
    		    if ($this->separator=="icli") {
    		        $imgfile  = "<img src=\"".$this->pioPath."images/i13._li.gif\" border=0>&nbsp;";
    		    }
    		}
    		$tmpType = $valarr["ty"];
    		
    		switch ($tmpType) {
    			case "head":
    				$out = "<B>".$valarr["txt"]."</B>";
    				if ($valarr["notes"]!=NULL) $out .= ' '.$valarr["notes"];
    				$out .= "<UL".$ulclass.">\n"; //  class=\"ul2\"
    				if ($option["ulbr"]) echo "<br>\n";
    				
    				if ( $valarr["spaceic"]>0) {
    					$this->icon_default='<img src="0.gif" width='.$valarr["spaceic"].' height=1>';
    				}
    				if ($valarr["li_std"]!=NULL) {
    				    // set standard 
    				    if ($valarr["li_std"]=="li") {
    				        $this->separator = "li";
    				        $this->separator_ini   = "<li>";
    				        $this->separator_close = "</li>";
    				    }
    				    if ($valarr["li_std"]=="br") {
    				        $this->separator = "br";
    				        $this->separator_ini   = "";
    				        $this->separator_close = "<br>";
    				    }
    				    if ($valarr["li_std"]=="icli") {
    				        $this->separator = "icli";
    				        $this->separator_ini = "";
    				        $this->separator_close = "<br>";
    				        $imgfile  = "<img src=\"".$this->pioPath."images/i13._li.gif\" border=0>&nbsp;";
    				    } 
    				    
    				} 
    				
    				break;
    			case "lnk":
    				$out =  $sep;
    				$topts = "";
    				$tmpbord = "0";
    				$tmp_img_extra = NULL;
    				if ($valarr["icbord"]) $tmpbord ="1";
    				if ($valarr["icon_opt"]!=NULL) $tmp_img_extra= ' '.$valarr["icon_opt"];
    				if ($valarr["iheight"]!=NULL)  $tmp_img_extra= ' height="'.$valarr["iheight"].'"';
    				
    				if ($valarr["target"]!="") $topts = " target=\"".$valarr["target"]."\"";
    				if ($valarr["icon"]!="")   $imgfile = "<img src=\"".$valarr["icon"]."\" border=".
    					$tmpbord . $tmp_img_extra . ">&nbsp;";
    				if ($valarr["iicon"]!="")  $imgfile = "<img src=\"images/".$valarr["iicon"]."\" border=".$tmpbord . $tmp_img_extra.">&nbsp;";
    				
    				if ($imgfile==NULL and $this->icon_default!=NULL) {
    					$imgfile = $this->icon_default. '&nbsp;';
    				}
    				
    				if ( $valarr["txtInTab"]>0 ) {
    					$out .= "<table cellpadding=0 cellspacing=0 border=0><tr><td>".$imgfile."</td>".
    							"<td>".$valarr["txt"]."</td></tr></table>".$sepClose."\n";
    					break;
    				}
    				
    				// if ($this->opt['font-size']=='large') $topts.=' class="yGlarger"';
    				
    				if ($valarr["href"] ) {
    					$out .= "<a href=\"".$valarr["href"]."\"".$topts.">".$imgfile.$valarr["txt"]."</a> ".$valarr["notes"].$sepClose."\n";
    				} else {
    					$out .= $imgfile.$valarr["txt"]." ".$valarr["notes"].$sepClose."\n";
    				}
    				
    				break;
    			case "headclose":
    				$out = "</UL>\n";
    				$this->icon_default=NULL;
    				break;
    			case "br":
    				$out = "<BR />\n";
    				break;
    			case "raw":
    				$out = $valarr["txt"];
    				break;
    			default:
    				$out = "<li>".$valarr["txt"]." ".$valarr["notes"]."</li>\n";
    		}
    		
    		
    		
    		echo $out;
    	}
    	
    	if ($this->opt['font-size']=='large') {
    	    echo  '</span>'."\n";
    	}
    
    }

}