<?php
/**
 * manage a HTML-form
 * @package func_form.in
 * @swreq common requirement
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @link  file://CCT_QM_doc/89_1002_SDS_code.pdf#pck:func_form
 
 */  
class formc {
    
    const KEY_COLOR  =  "#606060"; //  "#46648C";
    const KEY_OPT_COLOR="#A0A0A0";


var $go;  // 0,1,2, ...  
var $colspan;
var $hiddenarr;
/**
 *  info array
 * @var $htmltab
 * "col0width"
   "col1width"
   "col2width"
   "cellspace"
 */
var $htmltab; 
            

/**
 * - init the HTML-form
 * - set <form> tag
 * - start form table 
 * - manage hidden-fields
 * - variable-array: parx[]
 * 
 * @param $initarr <pre>
    "action"  = URL 
    "ENCTYPE" = "", "multipart/form-data"
    "title"
    "submittitle"
    ... OPTIONAL ... 
    "FORMXTRA" : extra text in <form>-tag
    "formname" : alternative form name
    "goNext"   : 1,2,3, ...
    "goBack"   : 0,1,2, ...
    'no_html'  : [0], 1 : do NOT show the HTML form ?
    "colwidth" : array of three columns
    "tabwidth" : width e.g. "80%" or "AUTO"
    "tabnowrap": 0|1
	"tabBgColor": #C0FFC0 backgolor of header
	"tabBgHdImg": image of header image
    "fsize"    : global field size (e.g. 40)
	"cellBgColor": cell back color
	"dblink"   : 1 => include special javascript scripts ...
	"colspan"  : set number of table columns
	"js_do_data_check"  : 0,1  -- can add javascript code to do soem checks; e.g. REQ-flag
  </pre>
 * @param &$hiddenarr
 *  - hidden fields  ( "name" => "value" ) 
 *  - do not give $go here !!!, will be given automatically at the end
 * @param int $go, the current go-value 0,1,2, ...
 */

function __construct( &$initarr,  &$hiddenarr, $go ) {

    $this->intern_arr = array(); // internal array of columns, used for submit check ... array('var'=>$tmpname, 'req'=>$fieldx["req"]);
    $this->initarr   = $initarr;
    $this->hiddenarr = $hiddenarr; 
    $this->go        = $go;   
    
    $this->colspan   = 3;
    $this->htmltab["col0width"] = "20%";
    $this->htmltab["col1width"] = "30%";
    $this->htmltab["col2width"] = "60%";
    $this->htmltab["cellspace"] = 0;
	
	// if ($initarr["cellspace"]!="") $this->htmltab["cellspace"] = $initarr["cellspace"];
	if ($initarr["colspan"]>0)  $this->colspan   = $initarr["colspan"];
	
    if ($this->initarr["tabnowrap"]) $this->htmltab["NOWRAP"]=" NOWRAP";

    if (is_array($initarr["colwidth"])) {
        $this->htmltab["col0width"] = $initarr["colwidth"][0];
        $this->htmltab["col1width"] = $initarr["colwidth"][1];
        $this->htmltab["col2width"] = $initarr["colwidth"][2];
    }
    
	$tmpcolarr = $this->getColors();
    $this->htmltab["bgcolor"]   = $tmpcolarr["bgcolor"];
	if ($initarr["tabBgColor"]!="" ) $this->htmltab["bgcolor"]   =  $initarr["tabBgColor"];
	
	$this->htmltab["bgcolor2"] = "#FFFFFF";       // #EBF2FA | #EFEFEF;
	if ($initarr["tab2BgColor"]!="" ) $this->htmltab["bgcolor2"]   =  $initarr["tab2BgColor"];
	
    $this->htmltab["keycolor"]  = $tmpcolarr["keycolor"];   
    $this->htmltab["spaceKey"]  = "<td width=1%><img width=10 height=1></td>";  
    
    $headtext = $initarr["title"];
    $twidth   = "width=100%";    
    if ( $initarr["tabwidth"]!=""     ) $twidth   = "width=\"".$initarr["tabwidth"]."\""; 
    if ( $initarr["tabwidth"]=="AUTO" ) $twidth   = "";
    
    if ($initarr["class"] != "") $class = $initarr["class"];
    else $class = "ttabf";
    
    $color1   = $this->htmltab["bgcolor"];
    $color2   = $this->htmltab["bgcolor2"];

    $this->htmltab["cellBgColor"] = $initarr["cellBgColor"];
    
    // call it editform, because it is needed for backcalls !!! 
    $this->formname='editform';
    if ($initarr["formname"]) {
        $this->formname=$initarr["formname"];
    }
	
    if ( !$initarr["no_html"] ) {
    
    	if ($initarr["dblink"]>0) { // put before form
    		require_once ('javascript.inc'); 
    		js_formAll();
    		$this->jsFormLib = new gJS_edit();
    	}
    	$this->spaceGif = $_SESSION['s_sessVars']["loginURL"]."/0.gif";

    	$this->start_form();
    	
        echo '<style type="text/css">'."\n";
    	echo ' table.ttabf {  } '."\n"; // Future ...
    	echo ' table.yFormiTab{ padding-top:5px; } '."\n";
    	echo '</style>'."\n";
        print "<table class=\"".$class."\" border=0 cellspacing=1 cellpadding=0 bgcolor=".$color1." ".$twidth."><tr><td>";
        print "  <table id=\"form_head_row\" border=0 cellspacing=0 cellpadding=2 bgcolor=".$color1." width=100% valign=top><tr><td style='padding:5px;'>\n";
        print '    <div style="color:'.$this->htmltab["keycolor"].'; font-weight:bold; font-size:1.2em;">'.$headtext.'</div></td>'."\n";
        print "    </tr></table>\n";
        print '  <table class="yFormiTab" border=0  cellpadding=1 bgcolor='.$color2.' width=100%>'."\n\n"; //OLD: cellspacing='.$this->htmltab["cellspace"].'
    }
}

private function out_hidden_vars() {
    $gotmp = $this->go + 1;
    if ( is_numeric($this->initarr["goNext"])  ) $gotmp = $this->initarr["goNext"];
    echo "<input type=hidden name=\"go\" value=\"".$gotmp."\">\n";
    if (!empty($this->hiddenarr)) {
        foreach( $this->hiddenarr as $idx=>$valx) {
            $valx = str_replace('"', '&quot;', $valx); // replace " by &quot;
            echo "<input type=hidden name=\"". $idx ."\" value=\"".$valx."\">\n";
        }
        echo "\n";
    }
    echo "\n";
}

/**
 * PURE open of form
 * - can be called by user, if __construct( $initarr['no_html']=1 )
 * 
 */
function start_form() {
    $tmpform = $this->initarrarr["FORMXTRA"];
    if ( $this->initarr["ENCTYPE"] != "" ) $tmpform .= " ENCTYPE=\"".$this->initarr["ENCTYPE"]."\"";
    echo "<form method=\"post\" name=\"".$this->formname."\" id =\"id_".$this->formname."\" action=\"". $this->initarr["action"] ."\" ".$tmpform.">\n"; 
}
function close_form() {
    $this->out_hidden_vars();
    echo "</form>\n";
}

static function encodeValue($s) {
    return htmlentities($s, ENT_QUOTES);
}

function getColors() {
	return array(
	    "bgcolor"   => "#E0E0E0",  // "#CAD5E3"
	    "keycolor"  => "#404040"   // "#46648C"
	); 
				// changing bgcolor
				// 
				/* hell:      #CAD5E3 
					dark-blau: #46648C 
					gelb:      #F8FB13
					dark-gelb: #FDC311  
								*/
}

/**
 * get a HTML-<select> field-string
 * @param  $varname  : FORM-variable-name
 * @param  array $sel_array 
 *    simple:  key => nice_name
 *    complex: array( array(key,nice_name) )
 * @param  $preselected : can be: 
 *   NULL -OR-  
 *   STRING -OR- 
 *   when $option["multiple"] => array ("keyword1" => 1, "keyword2" => 1, ...) 
 * @param  array $option
 *  "selecttext" if key ist set, take the text, 
 *     default: " --- select ---"
 *     if NULL => do not show the text.
	"selbody"   string : extra text inside <select> e.g. javascript
	"id"        HTML-tag id
	"req" => [0],1 input required ?
	"field.type" : type of $sel_array
	   ['simple'], array of ID => TEXT
	    'complex'  array of array(ID,NAME)
 */
static function selectFget( $varname, $sel_array, $preselected, $option=array(), $attStr=NULL) {
	
	$tmpissarr   = 0;
	$found = 0;
	if (is_array($preselected)) $tmpissarr   = 1;
    $selecttext = " --- select ---";
    
    if (!is_array($option)) $option=array();
    
    if ( array_key_exists("selecttext", $option) ) $selecttext = $option["selecttext"];
    
    $tmpID = NULL;
    if ($option['id']!=NULL) $tmpID = ' id="'.$option['id'].'" ';
    if ($option["req"]>0) $requiredTag=' required';
    else $requiredTag=NULL;
    
	$tmptxt  = "<select name=\"".$varname."\" " . $tmpID . $option["selbody"] ." ".$attStr.$requiredTag.">\n";
	if ($selecttext) $tmptxt .= "<option value=\"\">".$selecttext."</option>\n";	
	
	if (!empty($sel_array)) {
	    
		foreach( $sel_array as $tmp_id=>$row) {
		    
		    if ($option["field.type"]=='complex') {
		        $id  =$row[0];
		        $text=$row[1];
		    } else {
		        $id  =$tmp_id;
		        $text=$row;
		    }
		    
			$tmpsel = "";
			if ( $preselected!==NULL and $preselected == $id ) {
				$tmpsel = "selected";
				$found=1;
			}
			if ( $tmpissarr AND $preselected[$id] )   $tmpsel = "selected";
			$tmptxt .= "<option value=\"". $id ."\" ".$tmpsel."> ".$text."</option>\n";
		}
		
	}
	if (!$found AND $preselected!="")  
		$tmptxt .= "<option value=\"". $id ."\" selected> ".$preselected."</option>\n";
	$tmptxt .= "</select>\n"; 
	return ($tmptxt);
} 

/**
 * show radiobuttons
 * @param string $varname
 * @param string $initval
 * @param array $radioVals
 * @param string $radioSeperator
 */
static function radioArrOut($varname, $initval, $radioVals, $radioSeperator) {
	
	foreach( $radioVals as $id=>$text) {
			$tmpsel = "";
			if ( $initval == $id ) $tmpsel = "checked";
			echo "<input name=\"".$varname."\" type=radio value=\"". $id ."\" ".$tmpsel."> ".$text."\n".$radioSeperator;	
	}
}

/**
 * show one value field
 * @param  array $fieldx field_STRUCT 
 */
function _valueField(&$fieldx) {
	$fsize    = "";
    if ( $this->initarr["fsize"] !="" ) $fsize = " size=".$this->initarr["fsize"];
    if ( $fieldx ["fsize"] !="" ) $fsize = " size=".$fieldx["fsize"];
    
    $tmpname = "parx[". $fieldx["name"] ."]";
    if ( $fieldx["namex"] == TRUE )  $tmpname = $fieldx["name"] ;
    if ( $fieldx["req"]>0) $requiredTag=' required';
    else $requiredTag=NULL;
    
    if ($fieldx["name"]) {
        $this->intern_arr[]=array('var'=>$tmpname, 'req'=>$fieldx["req"], 'nice'=>$fieldx["title"]);
    }
    
	if ($fieldx["view"]!=1) {
    
		switch ( $fieldx["object"] ) {  
			
			case "button":    
				echo "<input type=button name='dummy' value='".$fieldx["val"]."' ";
				if ( $fieldx["inits"]["varset"] !="" ) {
					if ( $fieldx["namex"] == TRUE ) $tmpelemname = $fieldx["inits"]["varset"];
					else    $tmpelemname = "parx[".$fieldx["inits"]["varset"]."]";
					
					echo "onclick=\"document.".$this->formname.".elements['".$tmpelemname."'].value='".$fieldx["inits"]["value"]."';".
						" document.".$this->formname.".submit();\"";
				} elseif ( $fieldx["inits"]["func"] !="" ) {
					echo $fieldx["inits"]["func"]; // can be onClick="" ...
				} 
				echo ">";
				break; 
			
			case "checkbox":
				$tmpinit = 1; // default value
				if ($fieldx["inits"]!="") $tmpinit = $fieldx["inits"];
				
				$tmpopt = "";
				if ( $fieldx["val"] == $tmpinit ) $tmpopt = " checked";
				// changed on 2018-09-27: hidden field as default value, if checkbox not checked
				echo "<input type=hidden   name=\"".$tmpname."\" value=\"\">";
				echo "<input type=checkbox name=\"".$tmpname."\" value=\"".$tmpinit."\" $tmpopt>";
				if ( $fieldx["optx"]!="") echo "&nbsp;".$fieldx["optx"]; 
				break;
			
			case "dblink":
				// function formc() must be called with "dblink" = 1 !!!
				$opt    = NULL;
				if ($fieldx['req']>0) $this->initarr['js_do_data_check'] = 1;
				$inipoi = &$fieldx["inits"];
				if ( $inipoi["urlfull"] ) $opt["urlfull"] = 1;
				if ( $inipoi["noshDel"] ) $opt["noshDel"] = 1;
				if ( $inipoi["cond"]!=NULL )    $opt["cond"] = $inipoi["cond"];
				echo "\n";
				if ( $inipoi['getObjName']>0  ) {
					if ($inipoi['sqlo']!=NULL and $fieldx["val"]!=NULL ) {
						$fieldx["inits"]["objname"] = obj_nice_name ( $inipoi['sqlo'], $inipoi['table'], $fieldx["val"] );
					} else {
						$fieldx["inits"]["objname"] = NULL;
					}
				}
				$answer = $this->jsFormLib->getAll( $fieldx["inits"]["table"], $tmpname, $fieldx["val"],
						$fieldx["inits"]["objname"], (int)$fieldx["inits"]["pos"], $opt );
				echo '<span style="white-space: nowrap;">'.$answer.'</span>';
				break;
			
			case "file":
				echo "<input type=\"file\" name=\"".$tmpname."\" value=\"".$fieldx["val"]."\" ".$fsize.$requiredTag.">";
				break; 
			
			case "info2":
				// NO input field, only the value
				$tmpOut = $fieldx["val"];
				if ($fieldx["inits"]!="") $tmpOut = $fieldx["inits"];
				echo $tmpOut;
				if ($fieldx["name"]!="" ) echo "<input type=\"hidden\" name=\"".$tmpname."\" value=\"".$fieldx["val"]."\">";
				break;      
			
			case "radio":
				$tmpinit = 1;
				$do_it   = 1;
				if ($fieldx["inits"]===NULL) {
				    $do_it=0; // bad input config
				}
				if ($do_it) {
    				if ( !is_array($fieldx["inits"]) ) {
    					$tmpinit = $fieldx["inits"];
    					$tmpopt = "";
    					if ( $fieldx["val"] == $tmpinit ) $tmpopt = " checked";
    					echo "<input type=radio name=\"".$tmpname."\" value=\"".$tmpinit."\" $tmpopt>";
    				} else {
    					$tmpSep = "&nbsp;";
    					if (  $fieldx["optx"]["rowbr"]==1 ) $tmpSep = "<br>\n";
    					$this->radioArrOut($tmpname, $fieldx["val"], $fieldx["inits"], $tmpSep);
    				}
				}
				break;    
			
			case "select":
				$selopt = array();
				if ( array_key_exists("selecttext", $fieldx) ) $selopt["selecttext"] = $fieldx["selecttext"];
				if ($fieldx["req"]>0) $selopt['req']=1;
				$rettxt = $this->selectFget( $tmpname, $fieldx["inits"], $fieldx["val"], $selopt, $fieldx["attStr"]);
				echo $rettxt;
				break;
			
			case "password":
				echo "<input type=\"password\" name=\"".$tmpname."\" value=\"".$fieldx["val"]."\" ".$fsize.$requiredTag.">";
				break;	
			
			case "textarea":
			
				$tmpsize = "";
				$tmprows  = 5; 
				$tmpcols = $fieldx["inits"]["cols"];
				if ( $fieldx["inits"]["rows"]>0 ) $tmprows = $fieldx["inits"]["rows"];
				if ( $tmpcols   ) $tmpsize .= " cols=\"$tmpcols\"";
				
				$tmpsize .= " rows=\"$tmprows\"";
				
				echo "<textarea name=\"".$tmpname."\" ".$tmpsize.$requiredTag.">".$fieldx["val"]."</textarea>\n";
				break;
				
			case "date": //jquery datepicker
				echo "<input class=\"datepicker\" type=\"text\" name=\"".$tmpname."\" value=\"".$fieldx["val"]."\" ".
					$fsize.$requiredTag." ".$fieldx["attStr"].">";
				break;
				
			default:   // "text" !!!
			    echo '<input type="text" name="'.$tmpname.'" value="'.self::encodeValue($fieldx["val"]).'" '.$fsize.$requiredTag.' '.$fieldx["attStr"].'>';
				break;
		}
		
	} else {
		switch ( $fieldx["object"] ) {  
			case "select":
				$valx = $fieldx["val"];
				if (isset($fieldx["inits"])) {
					echo $fieldx["inits"][$valx];
				} else echo $valx;
				break;
			case "textarea":
				echo "<pre>".$fieldx["val"]."</pre>";
				break;
				
			case "dblink":	
				$inipoi = &$fieldx["inits"];
				
				if ( $inipoi['getObjName']>0  ) {
					if ($inipoi['sqlo']!=NULL and $fieldx["val"]!=NULL ) {
						$inipoi["objname"] = obj_nice_name ( $inipoi['sqlo'], $inipoi['table'], $fieldx["val"] );
					} else {
						$inipoi["objname"] = NULL;
					}
				}
				$tmpname   = $inipoi["objname"];
				$pionirUrl = "";
				if ($tmpname=="" AND $fieldx["val"]!="")  $tmpname = "[".$fieldx["val"]."]";
				if ( $inipoi["urlfull"] ) $pionirUrl = $_SESSION['s_sessVars']["loginURL"]."/";
				if ( $fieldx["val"]!="") echo "<a href=\"".$pionirUrl."edit.tmpl.php?t=".$inipoi["table"]."&id=".
					$fieldx["val"]."\">".htmlspecialchars($tmpname)."</a>";
				break;
			default:   // "text" !!!
				echo $fieldx["val"];
				break;
		}
		if ($fieldx["hidden"]>0) {
			echo '<input type=hidden name="'.$tmpname.'" value="'.$fieldx["val"].'">'."\n"; 
		}
	}
}

/**
 * only title and value
 */
function slimField( &$fieldx ) {
	
	
    $keycolor = self::KEY_COLOR;
	$titleStyle = " font-weight:bold; color:".$keycolor.";";
	echo "<td NOWRAP align=right  valign=top>";
    echo "<font style='".$titleStyle."'>".$fieldx["title"]."</font>"; 
    if ($fieldx["req"]>0) echo " <img src=\"".$_SESSION['s_sessVars']["loginURL"]."/images/redstar.gif\"  title=\"required\">";
    
    echo "</td>";
    echo '<td>';
    $this->_valueField($fieldx);
    echo '</td>';
	
}

/**
 * open row for free content, many columns
 * - do not forget to set "colspan" in the constructor !
 * set table colspan
 * @param $opts
 *   "rowid"
 *   'style':''
 */
function row_open($opts=array()) {
    echo "<tr valign=top";
    if ($opts["rowid"])  echo ' id="'.$opts["rowid"].'"';
    if ($opts["style"])  echo ' style="'.$opts["style"].'"';
    echo ">";
}
function row_close() {
    echo "</tr>\n";
}

/**
 * full row, ONE cell
 */
function freeRow_open() {
	echo "<tr valign=top>";
    echo "<td colspan=".$this->colspan.">";
}
function freeRow_close() {
	echo "</td></tr>\n"; 
}


/**
 * check data for one field
 *   
 * @param array $one_field_arr
 *   'maxlen' : max length of string
 *   'req'
 *   'dtype':
 *      DATEM
 * @return array($ok, $info)
* 		$ok>0 : ok
 */

static function check_field( $one_field_arr, $val ) {
    
    $ok  = 0;
    $info= NULL;
    $val = trim( $val );
    
    if  ($one_field_arr['req']>0 ) {
        if ($val===NULL or $val==='') {
            $ok   =-1;
            $info ='no value given';
            return array($ok, $info);
        }
    }
    
    $fieldType = $one_field_arr['dtype'];
    switch ($fieldType) {
        case "DATE1":
            if ($val==NULL) return array(1, NULL);
            if ( !preg_match( '/^([0-9]{4})-([0-9]{2})-([0-9]{2})$/', $val ) ) {
                $ok   =-2;
                $info ='bad format; need: "YYYY-MM-DD"';
            } else $ok   = 1;
            break;
        case "DATEM": // just Year and month
            if ($val==NULL) return array(1, NULL);
            $val_tmp = $val.'-01';
            
            if ( !preg_match( '/^([0-9]{4})-([0-9]{2})-([0-9]{2})/', $val_tmp ) ) {
                $ok   = -2;
                $info ='bad format; need: "YYYY-MM"';
            } else $ok   = 1;
            break;
        case "STRING":
            $ok   = 1;
            if ($one_field_arr['maxlen']>0 and strlen($val)>$one_field_arr['maxlen']) {
                $ok   =-3;
                $info ='string too long. Max-length:'.$one_field_arr['maxlen'];
            }
            break;
        default:
            $ok  = 1;
            break;
    }
    return array($ok, $info);
}

/**
 * manage one row (one field) of the form
 * 
 * @param array $fieldx field_STRUCT  <pre>
	  title => "Name"	// title of field
	, name  => "nachname" // varname 
	, namex => TRUE       // other namespace than parx[name], take pure name
	, val   => "345"      // initial value of the field
	, inits => $optarr    // optional data-array e.g. for select-fields
	, req   => 0,1        // required? -- changed on 2021-03-13
	, errtxt=> ""         // text of an error message, column appears RED
	, dtype  => string    // OPT: data type
	   INT
       FLOAT
       DATE1   "YYYY-MM-DD"
       STRING
	, notes => "yajaja"   // OPT: notes
	, fsize => "40"       // field size [40] 
	, vmin  => "3"        // OPT: minimum characters allowed
	, vmax  => "50"       // OPT: maximal characters allowed
	, colspan=>"2"        // OPT: span field over next column; default:1 
					         use this for long fileds, e.g. textarea 
	, backcolor=>"#efedfe"// backcolor
	, optional            // 0|1 silent backcolor
	, addobj => "text"	  // add more html-text behind the INPUT-object (e.g. more input fields)
	, view   => [0]|1     // show field in VIEW-mode (no edit possible)
	, hidden => [0]|1     // save field as hidden field (on "view"=1)
	, attStr => disabled="disabled"		  // additional attribute string
	, rowid  => "xf34"    // HTML-tag: id: for this row
	, rowStyle=> "DISPLAY: none" // HTML-css-sytle of the row
	, optx   => ...		  // other options ...
	, object => the input-field type ...
		text    : text-field 
		checkbox: needs "inits"
					 "optx" => optional text behind checkbox 
		radio   : needs ["inits"] = SIZE
				=1 => contains VALUE
				>1 => array of VALUE=>TEXT for an radio field
				give $fieldx["optx"] => array ("rowbr"=>1) line break for radio elements
		select  : needs "inits"= array of key=>value pairs
		textarea: rows=$fieldx["init"]["rows"],
					cols=$fieldx["init"]["cols"]  
		file   : recommended: "name"=>"userfile", "namex"=>TRUE (deny parx-name)
			on upload, the file-info is stored in $_FILES['userfile']
			you have to add  initarr["ENCTYPE"]="multipart/form-data" in 
			the constructor formc() !!!
		hr     : horizontal line (like "space")
		space  : a white space row (like "hr")
		info   : takes only "title" for infotext, full span
		info2  : implement as HIDDEN, shows title, value as HTML-formated text and notes
				   if "inits" !="" => show as HTML-formated text
		password : password style
		button : interactive button 
			take "inits" for javascript text:
			"varset" => "action"  -> the name of a parx[] variable
			"value" => "actlink"
	    submit    : SUBMIT-field 
	           'title'=> submit text
		dblink : special partisan link // in formc() need option "dblink" => 1
			"inits" =
			   ["table"],
			   ["objname"]
			   ["pos"]       = 0,1,2...
			   ["projlink"]  = 0|1
			   ["urlfull"]   = 0|1 (optional) write full URL of images
			OPTIONAL: 
				'getObjName']= 0,1 : get object-name of object, need 'sqlo'
				'sqlo'		 = pointer to sqlo-object ( $sqlo )
				'noshDel'    = 0,1
				'cond'		 = condtion for table

      example: $fieldx = array ("title" => "", "object" => "" );
   </pre>
 */
function fieldOut( $fieldx ) { 
    
    $firstspace = "&nbsp;&nbsp;&nbsp; ";
	$bgcolor  = ""; 
    if ($this->htmltab["cellBgColor"]!="") $bgcolor  = " bgcolor=\"".$this->htmltab["cellBgColor"]."\"";
    $keycolor = self::KEY_COLOR;
    
    switch ($fieldx["object"]) {
                           
        case "info":
            echo "<tr valign=top bgcolor=".$this->htmltab["bgcolor"].">"; // #C0C0C0,<font color=".$keycolor.">
            echo "<td colspan=".$this->colspan." valign=top>".$firstspace."<font color=".$keycolor."><B>".$fieldx["title"]."</B></font>";
            echo "</td></tr>\n";  
            return;
        case "hr":    
            echo "<tr bgcolor=".$this->htmltab["bgcolor"].">";
            echo "<td colspan=".$this->colspan."><img src=\"".$this->spaceGif."\" height=1 width=1>";
            echo "</td></tr>\n";  
            return;
        case "space":   
            echo "<tr".$bgcolor.">";
            echo "<td colspan=".$this->colspan."><img src=\"".$this->spaceGif."\" height=10 width=1>";
            echo "</td></tr>\n";  
            return;
        case  "submit":
            echo "<tr".$bgcolor.">";
            echo "<td></td>\n";
            echo "<td>";
            echo '<input class=yButton type=submit value="'.$fieldx["title"].'">'. "\n"; 
            echo "</td>\n";
            echo "<td></td>\n";
            echo "</tr>\n";
            return;
    }
	
    
    $titleStyle = "";
    
    if ($fieldx["optional"]) {
        // $bgcolor  = " bgcolor=\"#EFEFEF\"";
        $keycolor = self::KEY_OPT_COLOR;
    } else {
		$titleStyle .= " font-weight:bold;";
	}
	if ($fieldx["backcolor"]!="") {
        $bgcolor  = " bgcolor=\"".$fieldx["backcolor"]."\"";
    }
    if ($fieldx["errtxt"] !="" ) {
    	$keycolor='#FF0000';
    	// $bgcolor=" bgcolor=\"#FFDFDF\"";
    } 
	$titleStyle .= " color:".$keycolor.";"; 
	
	$rowidStr=NULL;
	if ($fieldx["rowid"]!=NULL) {
		$rowidStr = ' id="'.$fieldx['rowid'].'" ';
	}
	
	$rowStyleStr=NULL;
	if ($fieldx["rowStyle"]!=NULL) {
		$rowStyleStr = ' style="'.$fieldx['rowStyle'].'" ';
	}
	
    echo "<tr " . $bgcolor . $rowidStr . $rowStyleStr. " valign=top>";
    echo '<td NOWRAP align=right  valign=top width="'.$this->htmltab["col0width"].'" style="padding-left:5px; padding-right:10px;">';
    echo "<span style='".$titleStyle."'>".$fieldx["title"]."</span>";
    if ($fieldx["req"]>0) echo ' <img src="'.$_SESSION['s_sessVars']["loginURL"].'/images/redstar.gif" title="required">';
    
    echo "</td>";
    // echo $this->htmltab["spaceKey"];
    $colspanHtml="";
    if ( $fieldx["colspan"]==2 ) $colspanHtml=" colspan=2";
    echo "<td".$colspanHtml.$this->htmltab["NOWRAP"].">";
    
    $this->_valueField($fieldx);
    
    if ( $fieldx["colspan"]!=2 ) {
		if ( $fieldx["addobj"] != "") echo $fieldx["addobj"];
        echo "</td>";
        echo "<td width=".$this->htmltab["col2width"].$this->htmltab["NOWRAP"].">";
    }
    
    // notes-field
    if ($fieldx["errtxt"]!="") {
        echo "[<font color=red>".$fieldx["errtxt"]."</font>] ";
    }
    echo "<font color=gray>".$fieldx["notes"]."</font></td>";
    echo "  </tr>\n"; 

} 

static function editViewBut(
	$modenow,		// ["view"], "edit"
	$edit_possible, // 0|1
	$seturl,  		// go to ... e.g. "waferfunc.php?id=92939"
	$formvar		// e.g. "viewmode"
	) {
	// FUNCTION: show edit/view buttons
	
	
	$pioPath     = $_SESSION['s_sessVars']["loginURL"]."/";
	// $urlenc = urlencode($backurl);
	// $sessvarname = 'o.'.$tablename.'.editmode';
	// $_SESSION['s_sessVars'][$sessvarname]
	// $seturl = $pioPath."sessVars_set.php?variable=".$sessvarname."&backurl=".$urlenc."&val=";
	
	if ( $modenow == 'edit') {
		$but_view = $pioPath."images/but.view.gif";
		$but_edit = $pioPath."images/but.edit.ac.gif";	
	} else {
		$but_view = $pioPath."images/but.view.ac.gif";
		$but_edit = $pioPath."images/but.edit.gif";
	}
	
	if ( !$edit_possible )  $but_edit = $pioPath."images/but.edit.no.gif";
	$outstr  = "<a href=\"".$seturl."&".$formvar."=view\" >";

	$outstr .= '<img src="'.$but_view.'" TITLE="view" border=0></a>';
	if ( $edit_possible ) {
			$outstr .= "<a href=\"".$seturl."&".$formvar."=edit\" >".'<img src="'.$but_edit.'" TITLE="edit" border=0></a>';
	} else  $outstr .= '<img src="'.$but_edit.'" TITLE="edit">';
		
	return ($outstr); 
}

function addHiddenParx( $parx ) {
	foreach( $parx as $key=>$val) {
		$this->hiddenarr["parx[".$key."]"] = $val;
	}
}
function addHidden( $key, $val ) {
    $this->hiddenarr[$key] = $val;
}
  
// show whole array
function fieldArrOut( $fieldArr ) {
	foreach( $fieldArr as $valarr) {
		$this->fieldOut(  $valarr );
	}
} 



/**
 * CLOSE TABLE + FORM
 * - set submit buttons 
 * - set hidden fields
 * - close form
 * - close table 
 * class-input: $this->initarr["tabBgHdImg"]
 * 
 * @param $allowFlag,     
 *  - [TRUE] - submit button ACTIVE
 *  - FALSE  - submit button incative
 * @param $option = NULL  
 *  - ["noSubmitButton"]  0|1 do not show any submit button
 *  - ["noRow"]      0|1 do not show any BOTTOM row (including the buttons) !
 *  - ["addObjects"] = "TEXT" TEXT contains HTML text like extra buttons 
 *  - ["noBackButton"] = 0|1
 */
function close( $allowFlag, $option = NULL   ) {
    
     $tabBotMore = "";
	 if ($this->initarr["tabBgHdImg"]!="" ) $tabBotMore .= " background=\"".$this->initarr["tabBgHdImg"]."\"";
	 
     echo "\n";    
     
     $this->out_hidden_vars();
     
     if ($this->initarr['js_do_data_check']) {
         // build a JS data list for goz_check_form();
         $xt='let goz_form_data= ['."\n";
         foreach( $this->intern_arr as $row) {
             $xt .= '  {var: "'.$row['var'].'", nice:"'.$row['nice'].'"';
             if ($row['req']>0) $xt .= ', req:1';
             $xt .= '},'."\n";
         }
         $xt .='];'."\n";
         echo '<script>'."\n";
         echo $xt."\n";
         echo '</script>'."\n";  
     }

     if ( !$option["noRow"] ) {  // show BOTTOM submit-row ?
       
        //OLD: $newbgcol= $this->htmltab["bgcolor"] ; //  "#FDC311"  
        
        // Y-distance row
        /*
        echo "<tr bgcolor=\"".$newbgcol."\" height=5>";
        echo '<td width="'.$this->htmltab["col0width"].' style="padding-right:10px;"><img width=1 height=1></td>'; // 1. col
        // echo $this->htmltab["spaceKey"];
        echo "<td width=".$this->htmltab["col1width"]."><img width=1 height=1></td>"; // 2. col
        
        $last_col_span = $this->colspan - 3;
        echo "<td colspan=".$last_col_span."><img width=1 height=1></td>";                                       // 3. col
        echo "</tr>\n";
        */
        

        echo '<tr id="form_submit_row" '.$tabBotMore.' style="padding-top:10px;">';  // bgcolor="'.$newbgcol.'"
        echo '<td nowrap style="padding-right:10px;">';     // 1. col

        if ( ($this->go>0) AND (!$option["noBackButton"])) {
            $goBack = $this->go - 1; 
            if ( $this->initarr["goBack"]!="" ) $goBack = $this->initarr["goBack"];
            echo "&nbsp; <input class='yButton actbutton_calm' type=button name='dummy' value='&lt;&lt; Back' ".
               "onclick=\"document.".$this->formname.".go.value=".$goBack."; document.".$this->formname.".submit();\">\n";
        }
        echo "&nbsp;</td>";
        //echo $this->htmltab["spaceKey"];
        
        $submit_col_span = $this->colspan - 1;
        
        echo '<td nowrap colspan='.$submit_col_span.' style="padding-right:10px;">';    // 2. col
       

        if ($option["noSubmitButton"] != "1") { // show submit-button info ???

            if ( $allowFlag != FALSE ) {
                $submittitle = "Submit";
                if ( $this->initarr["submittitle"]!="" ) $submittitle = $this->initarr["submittitle"];
                echo "<input class='yButton' type=submit value=\"".$submittitle."\" ";
                if ($this->initarr['js_do_data_check']) echo " onclick='return goz_check_form(\"".$this->formname."\");'";
                echo ">\n"; 
            } else {
                // echo "<font color=red>Submit is not allowed</font>";
            }

        } else echo "&nbsp;";     // no submit button, even no error
		
		if ($option["addObjects"]!="" ) {
			echo "&nbsp;".$option["addObjects"];
		}
        echo "</td>"; 

        
        echo "</tr>\n";
     }
     
     echo "  </table>\n";
     echo "</td></tr></table>\n";   
     echo "\n";
     echo "</form>\n";
}

}

/**
 * pure <FORM> methods, no GUI components
 * @author skube
 *
 */
class formPure {
    
    function __construct(  &$initarr, &$hiddenarr, $go   ) {

        $this->initarr   = $initarr;
        $this->hiddenarr = $hiddenarr;
        $this->go        = $go;   
        
        $this->formname='editform';
        if ($initarr["formname"]) {
            $this->formname=$initarr["formname"];
        }
    }
    
    function start() {
        
        $form_extra_text = NULL;
        if ( $this->initarr["ENCTYPE"] != "" ) $form_extra_text .= " ENCTYPE=\"".$this->initarr["ENCTYPE"]."\"";
        echo "<form method=\"post\" name=\"".$this->formname."\" action=\"". $this->initarr["action"] ."\" ".$form_extra_text.">\n";
       
    }
    
    function close() {
        
        $gotmp = $this->go + 1;
        if ( $this->initarr["goNext"]!="" ) $gotmp = $this->initarr["goNext"];
        echo "<input type=hidden name=\"go\" value=\"".$gotmp."\">\n";
        
        if (sizeof($this->hiddenarr)) {
            foreach( $this->hiddenarr as $idx=>$valx) {
                $valx = str_replace('"', '&quot;', $valx); // replace " by &quot;
                echo "<input type=hidden name=\"". $idx ."\" value=\"".$valx."\">\n";
            }
            reset($this->hiddenarr);
            echo "\n"; 
        }
        echo "</form>\n";
    }
}

/**
 * class for important functions, supporting a html-page with a FORM
 * - support for the page-status-chart (graphics like:  * + * * Preparation phase)
 * @example <pre>

	$goArray   = array( "0"=>"Give file", 1=>"Prepare Import", 2=>"Import and Create protocols" );
	$extratext = '[<a href="'.$_SERVER['PHP_SELF'].'">Start again</a>]';
	$formPageLib = new FormPageC();
	$formPageLib->init( $goArray, $extratext );
	$formPageLib->goInfo( $go ); 
 * </pre>
 */
class FormPageC {
	
	/**
	 * contains the state-info
	 * @var array $goArray
	 *    stateid => status text
	 *      where stateid: corresponds usually the value of variable $go, can be a VARIUS NumberFormatter
	 *           e.g. 2,5,10,20
	 */
	var $goArray;

/**
 * init GUI-component
 * @param array $goArray : see $this->goArray 
 * @param string $extratext :show extra text after the status-chart
 */
function init($goArray, $extratext="") {
	$this->goArray     = $goArray;
	$this->goExtratext = $extratext;
	
	$arrkeyx = array_keys($this->goArray);
	$this->goNumbers = array_flip($arrkeyx);
	
}

/**
 * - show status headline for a page with several status (wizard)
 * - show title of each status in title of status-POINT-image
 */
function goInfo($go) {

	if ( !$go ) $go = 0;
	
	$stepnr = $this->goNumbers[$go]+1;
	
	$addurl  = $_SESSION['s_sessVars']["loginURL"]."/";
	foreach( $this->goArray as $key=>$val) {
		$icon = "ic.point.ina.gif";
		if ( $key == $go ) $icon = "ic.point.act.gif";
		$title_loop = htmlspecialchars($val);
		echo '<img src="'.$addurl.'images/'.$icon.'" title="'.$title_loop.'">';
	}
	
	
	echo "&nbsp;&nbsp;&nbsp;<B><font size=+1 color=#606060>";
	
	echo $stepnr .". ".$this->goArray[$go];
	echo "</font></b>";
	if ( $go>0 AND $this->goExtratext!="" )echo "&nbsp;&nbsp;". $this->goExtratext."";
	echo "<br><br>\n";
}

private static function get_info2_css() {
    // from https://jsfiddle.net/jermartin77/vmc3e/
    return '
    .y_wiz_progress {
        box-sizing: border-box;
    }
    
    .y_wiz_progress {
        padding: 0;
        list-style-type: none;
        font-family: arial;
        // font-size: 12px;
        clear: both;
        line-height: 1em;
        margin: 0 -1px;
        text-align: center;
    }
    
    .y_wiz_progress li {
        float: left;
        padding: 10px 30px 10px 40px;
        background: #CCC;
        color: #fff;
        position: relative;
        margin: 0 1px;
    }
    
    .y_wiz_progress li:before {
        content: "";
        border-left: 18px solid #fff;
        border-top: 18px solid transparent;
        border-bottom: 18px solid transparent;
        position: absolute;
        top: 0;
        left: 0;
        
    }
    .y_wiz_progress li:after {
        content: "";
        border-left: 18px solid #CCC;
        border-top: 18px solid transparent;
        border-bottom: 18px solid transparent;
        position: absolute;
        top: 0;
        left: 100%;
        z-index: 20;
    }
    
    .y_wiz_progress li.active {
        background: #3366cc;
    }
    
    .y_wiz_progress li.active:after {
        border-left-color: #3366cc;
    }
    .y_wiz_progress li a {
       color: #fff;
    }
    ';
}

/**
 * show status headline for a page with several status (wizard)
 * @param $go
 * @param array $status_links
 *    KEY => URL
 *  
 */
function goInfo2($go, $status_links=array() ) {
	
	
	// $goMax = sizeof($this->goArray);

	if ( !$go ) $go = 0;
	
	
	echo '<style type="text/css">'."\n";
	echo self::get_info2_css();
    echo '</style>'."\n";
	
    
    echo '<ul class="y_wiz_progress">'."\n";
	
	
	$i=0;
	
	//$lastWasAct=0;
	foreach( $this->goArray as $key=>$val) {
		
		//$in_selected_xmode = false;
		$active_str='';
		
		if ( $go==$key ) {
			//$in_selected_xmode = true;
			$active_str=' class="active"';
		}
  
	    $val_html = $val;
	    if (isset($status_links[$key])) {
	        $val_html='<a href="'.$status_links[$key].'">'.$val.'</a>';
	    }
	    echo '<li'.$active_str.'>'.$val_html.'</li>'."\n";

		$i++;
	} 

		
	if ( $go>0 AND $this->goExtratext!="" )
		echo "<li>&nbsp;&nbsp;". $this->goExtratext."</li>";
	
	echo '</ul>'."\n";
	echo '<br><p />'."\n";
	
}

}
