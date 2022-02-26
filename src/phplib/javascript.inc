<?php
/**
 * main functions to use java-script in the HTML-pages
 * @package javascript.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */


function js__history_back($referer) {
# descr: does the same like history.back() but it works also in mozilla
#        till now only GET-data is added to the url
# input: referer .. url to which go back

  js__location_replace(js__history_back_get_url($referer));
}

function js__history_back_get_url($referer) {
# descr: does the same like history.back() but it works also in mozilla
#        till now only GET-data is added to the url
# input: referer .. url to which go back
# return: string for the back-link

  return $referer.'?'.js__get_param_to_url();
}

/**
 * converts the values in $_GET (get-parameters) to url-parameters starting with &
 * referer .. url to which go back
 * @param array $overLoadArr other url-params
 * @return string for the back-link
 */
function js__get_param_to_urlArray(&$input) {

  if (!is_array($input)) return;
  
  $url = '';
  reset($input);
  foreach( $input as $key=>$val) {
	if(is_array($val)) {
	  foreach( $val as $akey=>$aval) {
		$url .= '&'.$key.'['.$akey.']='.$aval;
	  }
	} else {
	  $url .= '&'.$key.'='.$val;
	}
  }
  reset($input);
  
  return $url;
}

/**
 * converts the values in $_GET (get-parameters)
 */
function js__get_param_to_url() {
  return js__get_param_to_urlArray($_GET);
}

function js__history_back2() {
	
	if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
		echo "&nbsp;(Automatic Forward stopped due to debug-level)<br>\n";
		return;
	}
	
	echo '<script type="text/javascript">'."\n";
	echo '  history.back();'."\n";
	echo '</script>'."\n";
	
}

/**
 * writes location.replace() to output
 * @return 
 * @param string $url  replace location with this url
 * @param string $link_text[optional] text for the noscript-link
 * @param object $stop[optional]  array(0=>flag, 1=>reason)
 * @param int    $delay[optional] in milliseconds
 */
function js__location_replace( $url, $link_text=NULL, $stop = NULL,	$delay = NULL, $method = NULL) {

		echo '<a href="',$url,'">AutoForward &gt;&gt; '.$link_text.'</a>'."\n";
		
		if ( is_array($stop) and $stop[0]>0 ) {
			echo "&nbsp;(Automatic Forward stopped due to: ".$stop[1].")<br>\n";
			return;
		}
		if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
			echo "&nbsp;(Automatic Forward stopped due to debug-level)<br>\n";
			return;
		}
		
		if ($method == NULL or $method == "GET") {	//use the default method GET
		
			$jsCmd = 'location.replace("'.$url.'");';
			echo '<script type="text/javascript"><!--',"\n";
			if ( $delay>0 ) {
				echo 'function FuncForward() {'."\n";
				echo '   ' .$jsCmd. "\n";
				echo '} '."\n";
				echo 'setTimeout("FuncForward()", '.$delay.');'."\n";
			} else {
				echo $jsCmd."\n";
			}
			echo '//--></script>',"\n";
		}
	
		elseif ($method == "POST") {	//using hidden form
		    
			$scheme = parse_url($url, PHP_URL_SCHEME);
			$host = parse_url($url, PHP_URL_HOST);
			$path = parse_url($url, PHP_URL_PATH);
			$query = parse_url($url, PHP_URL_QUERY);
			$params=NULL;
			parse_str($query, $params);
			$in_fields = '';
			foreach($params as $name => $value) {
				$in_fields .= '<input type="text" name="'.$name.'" value="'.$value.'" />';
			}
			
			$post_url = $scheme.'://'.$host.$path; 
			
			$formhead = '<form id="form" style="display: hidden" action="'.$post_url.'" method="post">';
			$formhead = "document.write('".$formhead;
			$formtail = "</form>');"."\n";
			$formexec = "var form = document.getElementById('form');form.submit();"."\n";
			$jsCmd = $formhead.$in_fields.$formtail.$formexec;
			
			echo '<script type="text/javascript"><!--',"\n";
			if ( $delay>0 ) {
				echo 'function FuncForward() {'."\n";
				echo '   ' .$jsCmd. "\n";
				echo '} '."\n";
				echo 'setTimeout("FuncForward()", '.$delay.');'."\n";
			} else {
				echo $jsCmd."\n";
			}
			echo '//--></script>',"\n";
			
			//echo $post_url;
			//echo '<code>'.$formhead.'</code><br />';
			//echo '<code>'.$in_fields.'</code><br />';
			//echo '<code>'.$formtail.'</code><br />';
			//echo '<code>'.$jsCmd.'</code>';
		}
	  
		else {
			echo "&nbsp;(Automatic Forward stopped due to unknown method ".$method.")<br>\n";
			return;
		}
	}

function js__location_href($url, $link_text="") {
# descr: writes location.href() to output
# input: url       .. href to location with this url
#        link_text .. text for the noscript-link

	if ( $_SESSION["userGlob"]["g.debugLevel"]>1 ) {
		echo "DEBUG: &nbsp;(Automatic Forward stopped due to debug-level)<br>\n";
		echo "Dest-URL: ".htmlspecialchars($url)."<br />\n";
		
		return;
	}
  echo '<script type="text/javascript"><!--',"\n";
  echo 'location.href = "',$url,'";',"\n";
  echo '//--></script>',"\n";
  if ($link_text)
	echo '<noscript><a href="',$url,'">',$link_text,'</a></noscript>',"\n";
}

/**
 * writes the steffens javascript-function openwin
 * @param array $option : "cond" = STRING add fix condition
 */
function js__openwin($option=NULL) {

	
	
	echo '<script type="text/javascript"><!--',"\n";
  // ALL parameters MUST be set, otherwise gives javascript-problems ...
	echo 'function openwin(desttable, element_id, condition, retcolname) {',"\n";
	if ($option['cond']!="") { // set FIX condition
		echo '  condition=\''.$option['cond']."'\n";
	}  
	echo '  url_name = "',$_SESSION['s_sessVars']['loginURL'],'/view.tmpl.php?t="+ desttable +"&cctgobacol="+ retcolname +"&cctgoba="+ element_id + condition;',";\n";
	echo '  InfoWin  = window.open(url_name, desttable +"'.rand(1,10000).'","scrollbars=yes,width=750,height=500,status=yes,resizable=yes");',"\n"; // 
	echo '  InfoWin.focus();',"\n";
	echo '}',"\n";
	echo '//--></script>',"\n";
}


/**
 * writes the javascript-function linkto()
 * - based on DOM-object: see gJS_edit::getObjLnk())
 */
function js__linkto() {
  echo '<script type="text/javascript"><!--',"\n";
  echo 'function linktox(table, input_id) {',"\n";
  echo '  if (document.editform.elements[input_id].value != "") {',"\n";
  echo '    urlname  ="edit.tmpl.php?t="+ table',"\n";
  echo '    urlname +=  "&id=" + document.editform.elements[input_id].value;',"\n";
  echo '    InfoWin  =  window.open(urlname, table, "scrollbars=yes,width=750,height=500,status=yes,resizable=yes")',"\n";
  echo '    InfoWin.focus();',"\n";
  // echo '    location.href = urlname +"&id=" + document.editform.elements[input_id].value;',"\n";
  echo '  }',"\n";
  echo '}',"\n";
  echo '//--></script>',"\n";
}

/**
 * javascript-function inputRemote()
 * - writes object(ID + NAME) back to the DOM-object from calling window 
 * - based on DOM-object: see gJS_edit::getObjLnk()
 * - DEPRICATED: formname "xtraform"
 * javascript:params:
 *  remotepos : ID of DOM-object in page 
 *    - hidden-element of object in form
 *    - the shown HREF-element has ID: [remotepos]shy
 *         must be unique on page !!!
 *  valuex : ID of object
 *  valtext: NAME of object
 */
function js__inputRemote() {
  
  echo '<script type="text/javascript"><!--',"\n";
  echo 'function inputRemote(remotepos, valuex, valtext) {'."\n";
  echo '  valtexto = valuex +":"+ valtext',"\n";
  echo '  if ((valuex == "") || (valtext == "")) valtexto = valuex + valtext;'."\n";
  echo '  document.getElementById(remotepos).value = valuex;'."\n";
  
  echo '  hrefID = remotepos+"shy";'."\n";
  echo '  if ( document.getElementById(hrefID).hasChildNodes() == false) { '."\n";
  echo '	var myText = document.createTextNode(valtexto);'."\n";
  echo '	document.getElementById(hrefID).appendChild(myText);'."\n";
  echo '  } else { '."\n";
	
  echo '     document.getElementById(hrefID).firstChild.nodeValue = valtexto;'."\n"; 
  echo '     document.getElementById(hrefID).style.color = null;'."\n"; // remove style, if exists
  echo '  }'."\n";
  echo '}'."\n";
  echo '//--></script>'."\n";
}

// writes the steffens javascript-function openproj
function js__openproj() {

  
  
  echo '<script type="text/javascript"><!--',"\n";
  echo 'function openproj(desttable, element_id) {',"\n";
  echo '  url_name = "',$_SESSION['s_sessVars']['loginURL'],'/obj.proj.selector.php?tablename="+desttable+"&cctgoba="+element_id;',"\n";
  echo '  InfoWin  = window.open(url_name, desttable, "scrollbars=yes,status=yes,width=650,height=500,resizable=yes");',"\n";
  echo '  InfoWin.focus();',"\n";
  echo '}',"\n";
  echo '//--></script>',"\n";
}

//  writes the steffens javascript-function open_info
function js__open_info() {
   // is now content of main.js
} 


// write all needed javascript functions for the class gJS_edit database-object-link
function js_formAll() {
        
    js__openwin();
    js__linkto();
    js__inputRemote();
    js__openproj();

}		




// support a select-element
function js_formSel() {

?>
	<script type="text/javascript">
	function valueInput( inputField_id ){
		oldVal = document.editform.elements[inputField_id].value;
		input=prompt( "Value or [NULL]", oldVal);

		if ( input != null )  {
				document.editform.elements[inputField_id].value =input;
		}
	}
	<?php // give an alternative value for a <select>  ?>
	
	function valueInSel( fieldID ) { 
		valid = document.editform.elements[fieldID].options.selectedIndex ;
		valu  = document.editform.elements[fieldID].options[valid].text;
		newval= prompt ( 'Value or [NULL]', valu );
		nullval = 0;
	
		if ( newval == "[NULL]" )  {
			nullval = 1;
			newval  = "";
		}
		if ( (newval != null) || nullval ) {
			document.editform.elements[fieldID].options[0].text    = newval;
			document.editform.elements[fieldID].options[0].value   = newval;
			document.editform.elements[fieldID].options[0].selected= true;
		}
	}
	</script>
	<?php
}

// set focus on a field
function js_formFocus($fieldname) {

	?>
	<script type="text/javascript">
	document.editform.elements["<?php echo $fieldname;?>"].focus();
	</script>
	<?php 
}

/**
 * - deny LF in form
 * - have to add <javascript>
 * @param string $newchar in future use this as replacement
 * @example
 *   js_replaceLF();
 *   <textarea onKeyUp="return submitenter(this,event)"> </textarea>
 */
function js_replaceLF($newchar=0) {
	
	?>
	function glob_replaceLF(myfield,e)
	{
		var keycode;
		if (window.event) keycode = window.event.keyCode;
		else if (e) keycode = e.which;
		else return true;
		if (keycode == 13)
		{
			return false;
		}
		else
		return true;
	}
	<?php
}

function js_table_sortable() {
    echo '<script type="text/javascript" src="res/js/jquery-3.5.1.min.js"></script>'."\n";
    echo '<script src="res/js/jquery-ui.1-12.1.js"></script>'."\n"; // jquery-ui for support of TABLE > .sortable
    //echo '<script type="text/javascript" src="res/js_goz/glob.obj.glob.objtab.pref.js?dummy=1"></script>'."\n";
    ?>
    <script>
        $("#xtable tbody").sortable({
            items: "> .sortx",
            appendTo: "parent",
            helper: "clone",
            cancel: ':input'
        })
    </script>
    <?php
}

/**
 * javascript class for a html-form
 * - produces special elements:
 * - dblink : getAll() - describes a database object
 *
 * @example: <pre>
	$jsFormLib = new gJS_edit();
	$fopt = array("urlfull" =>1);
	$answer = $jsFormLib->getAll('TABLE', 'varname', $this_colval, $button_value,  $viewcnt, $fopt);
	echo $answer;
  </pre>
 * @see  file://CCT_QM_doc/89_1002_SDS_code.pdf
 */
class gJS_edit {

    var $butpre='fyq';
    
    function setTable( $tablename, 
    	$formPos	// 0,1,2,...
      ) {
    	$this->table     = $tablename;
    	$this->formpos = $formPos;
    	$this->butid 	 = 'fyq'.$this->formpos;
    	$this->butidSh = $this->butid.'shy';
    }
    
    function pkInput($viewcnt) {
    	echo "<a href=\"javascript:valueInput('".$this->butpre.$viewcnt."')\" >";
    	echo "<IMG src=\"images/input.png\" border=0 TITLE=\"new value\"></a>";
    }
    
    // get ID of field
    function getID($viewcnt) {
    	return ($this->butpre. $viewcnt);
    }
    
    // show the HTML string
    function shSelectAlter($viewcnt) {
    	echo $this->shSelectAlter_str($viewcnt);
    }  
    
    // get the string
    function shSelectAlter_str($viewcnt) {
    	$result = "<a href=\"javascript:valueInSel('".$this->butpre.$viewcnt."')\" >";
    	$result .= "<IMG src=\"images/input.png\" border=0 TITLE=\"new value\"></a>";
    	return $result;
    }
    
    /**
     * get object-link
     * 1. element hidden-element with OBJECT-ID
     * 2. + <a href>-tag with OBJECT-NAME
     * - corresponding javascript-functions:
     *   - inputRemote()
     *   - linktox()
     * @param string $showname, can be NULL
     * @param $opt 
     *   'butxtra' -- e.g. class parameter
     *   'emptyOnNoVal' : 0,1 - hide OBJECT-NAME-tag, if $value is empty
     * 			
     */
    function getObjLnk( $varname, $value, $showname, $opt=NULL ) {
    
    	//$link_dest = "edit.tmpl.php?t=" . $this->table ;
    	$outname   = $showname;
    	if ($outname==NULL) $outname===''; // should not be NULL for javascript !!!
    	if ( $outname==NULL) {
    		 if ( $opt['emptyOnNoVal']<=0 ) $outname='---'; // no data
    		 else $outname=' ';
    	}
    	$outtxt = "";
        
      	$styleOpt=NULL;
      	$styleOpt=' class="yButObjLink" '; // background-color:#D0D0D0; 
      	//if ($value===NULL or $value=='0') {
      		// $styleOpt .='color:#808080;'; // color:#808080; style for NONE-object
      	//}
      
    	$outtxt .= '<input type="hidden" name="'.$varname.'" id="'.$this->butid.'" value="'.$value.'">'."\n";
    	$outtxt .= "<a href=\"javascript:linktox('".$this->table."', '".$this->butid."');\" ". $opt['butxtra']. 
       		' id="'.$this->butidSh.'"'.$styleOpt.'>'. htmlspecialchars($outname) . "</a>\n";
    	return ($outtxt);
    }
    
    /**
     * get HTML-string for the dblink-selection elements
     * @param $opt
     * 
    		"showproj"  = -1 0 [1]
    		"noshDel"   = 0|1 do not show DELETE button
    		'selUseTxt' = STRING - use this link text instead of IMG images/select.gif
    		"urlfull"   = 0|1 use FULL url-paths to the links/images
    		"cond"      = extra condition like "x.EXP_ID>100", you can use "'" inside the string!
    		'liTab'     = use TABLE for LIST selection
    		'liCol'     = use COLUMN as ID for LIST selection
    		
     * @return string
     */
    function getObjSelector($opt = NULL) {
    	
    	$formPos = $this->butid; 
    	$outtxt = ''; // OLD: span forced problems on IE8: <span style="white-space:nowrap;">';
    	$addurl = "";
    	$addCond= "";
    	$tablename = $this->table;
    	if ($tablename=="") return ("INTERN-ERROR: getObjSelector(): not initialised! "); 
    	
        if ($opt["urlfull"])  $addurl  = $_SESSION['s_sessVars']["loginURL"]."/";
    	// replace ' by \' due to javascript
        if ($opt["cond"]!="") $addCond = "&condclean=1&tableSCond=".rawurlencode(str_replace("'","\'",$opt["cond"]));
    	
    	if ( $opt["showproj"] >= 0 ) {
            if ((cct_access_has2($tablename))) {
                $outtxt .=  "<a href=\"javascript:openproj('" .$tablename. "','" .$formPos. "')\" >";
                $outtxt .=  "<IMG src=\"".$addurl."images/ic.proj.bw.png\" border=0 TITLE=\"select from project\"></a> "; // icon.PROJ.gif,  but.proj_sel.gif
            }
        }
    	
    	
        // use other TABLENAME than the default one ?
    	if ( $opt["liTab"]== '' ) {
    		$outtxt .=  "<a href=\"javascript:openwin('".$tablename."', '".$formPos."', '".$addCond."', '')\">";
    	} else {
    		$outtxt .=  "<a href=\"javascript:openwin('".$opt["liTab"]."', '".$formPos."', '".$addCond."', '".$opt["liCol"]."')\">";
    	}
    	
    	// which kind of selection-element: BUTTON or HTML-text ?
    	if ( $opt['selUseTxt']=='' ) {
    	    $outtxt .=  '<IMG src="'.$addurl.'images/but.list2.gif" border="0" TITLE="select from list"></a>'."\n"; // but.list2.gif,  but.list_sel.gif
    	} else {
    		$outtxt .= $opt['selUseTxt'].'</a>'."\n";
    	}
    	
    	
    	// show UNLINK-button ?
    	if ($opt['noshDel']<=0) {
    		$outtxt .=  " <a href=\"javascript:inputRemote( '".$formPos."', '', '---' )\" >";
    		$outtxt .=  "<IMG src=\"".$addurl."images/but.del2.gif\" border=0 TITLE=\"delete link\"></a>"; // but.dellink2.gif
    	}
    	// OLD: $outtxt .= '</span>';
    	return ($outtxt);
    }
    
    /**
     * get JS command to open a list selector
     * @param string $tablename
     * @param int $formPos
     * @param string $addCond OTIONAL
     * @return string
     */
    function getJS_open_list($addCond='') {
        if ($this->table==NULL) {
            sys_error_my( 'table not initialized.' );
        }
        return "openwin('".$this->table."', '".$this->butid."', '".$addCond."', '')";
    }
    
    /**
     * get ObjectLink and ObjectSelector
     */
    function getAll($tablename, $varname, $value, $showname,  $formPos, $opt = NULL) {
    	$this->setTable($tablename, $formPos);
    	$out =  $this->getObjLnk( $varname, $value, $showname, $opt);
    	$out .=  '&nbsp;&nbsp;';
    	$out .=  $this->getObjSelector( $opt );
    	return ($out);
    }
    
    /**
     * for special uses (e.g. in obj.abstract_proto.xmode.edi.php)
     * @param string $table
     * @param string $but_code
     * @param string $dataval
     * @param string $nicename
     */
    static function getLinkSpecial1($table, $but_code, $dataval, $nicename) {
        ?>
      <input type="hidden" id="<?php echo $but_code;?>" value="<?php echo $dataval;?>">
      <a href="javascript:linktox('<?php echo $table;?>', '<?php echo $but_code;?>' );"  id="<?php echo $but_code;?>shy"
        class="yButObjLink" ><?php echo $nicename;?></a>
      <div >
      <a href="javascript:openproj('<?php echo $table;?>','<?php echo $but_code;?>' )" ><IMG src="images/ic.proj.bw.png" border=0 TITLE="select from project"></a> 
      <a href="javascript:openwin('<?php echo $table;?>', '<?php echo $but_code;?>' , '', '')"><IMG src="images/but.list2.gif" border="0" TITLE="select from list"></a>
      <a href="javascript:inputRemote( '<?php echo $but_code;?>' , '', '---' )" ><IMG src="images/but.del2.gif" border=0 TITLE="delete link"></a>
      </div>
      <?php 
    }



}

