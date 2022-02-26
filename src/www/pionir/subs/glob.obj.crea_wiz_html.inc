<?php
class htmlZeugC {
    
    private $tablename;
    
    function __construct($tablename) {
        
        $this->tablename = $tablename;
        $this->rowarr = NULL;
        $this->rowarr["bgcolor"][0] = "#EFEFEF"; // "#CAD5E3"
        $this->rowarr["bgcolor"][1] = "#EFEFEF"; // #D0E0FF
        $this->rowarr["bgcolor"][2] = "#EFEFEF";
        $this->rowarr["bgcolor"][3] = "#EFEFEF";
        
        $this->colorKey    = "#336699";
        $this->colorSelect = "#D0E0FF";
        $this->colorSubmit = "#EFEFEF"; // "#6699DD";
        $this->formLineColor = "#D0D0D0";
        $this->colorBack   = "#EFEFEF";
        $this->COLSPAN = 4;
        
        $this->jsFormLib = new gJS_edit();
        
        
        
    }
    
    function javaScriptX() {
        
        echo '<style type="text/css">'."\n";
        echo '.x1  { color: #ffffff; font-weight:bold; }'."\n";
        echo '.x2  { color: #ffffff; font-weight:normal; text-decoration:underline; }'."\n";
        echo '</style>'."\n";
        
        // TBD: use rowtouch() to activate row-->
        ?>
	
	<script language="JavaScript" type="text/JavaScript">
	var lastTag="";
	
	
	function rowtouch( element_id) {
		rowpos = element_id;
		document.editform.selecter[element_id-1].checked = 1;	
		th_snc( "t"+rowpos );				
  	}
    
	function th_snc( tag ) {

		var newColor     = "<?echo $this->colorSelect?>";
		var touchColor   = "<?echo $this->colorBack?>";
		
		if ( lastTag !="" ) 
			document.getElementById(lastTag).style.backgroundColor = touchColor;
		
		if( (document.getElementById) && (document.getElementById( tag )!=null)) {
	  		var myElement = document.getElementById( tag );        
  			if ((myElement.style)&& (myElement.style.backgroundColor!=null)) {            
                document.getElementById(tag).style.backgroundColor = newColor;
  			}
		}  
		lastTag = tag;  
        // parent.unten.location.href="edit.tmpl.php?t=<?echo $tablename?>&id=" + id;
	}

	function x_form_back( go ) {
		document.editform.go.value = go;
		document.editform.submit();
	}

	</script>
	<?
	js_formAll();

}

function form_start_no_table($proj_id, $tablename, $go_next) {
    
    echo "<form name=editform ENCTYPE=\"multipart/form-data\" ACTION=\"".$_SERVER['PHP_SELF']."\" METHOD=POST>\n";
    
    echo '<input type=hidden name="proj_id" value="'.$proj_id.'">'."\n";
    echo '<input type=hidden name="tablename" value="'.$tablename.'">'."\n";
    echo '<input type=hidden name="go" value="'.$go_next.'">'."\n";
 
}


function form_start($proj_id, $tablename, $go_next, $headtext) {
    
    $this->form_start_no_table($proj_id, $tablename, $go_next);
    
    echo "<table border=0 cellspacing=1 cellpadding=0 bgcolor=#D0D0D0><tr><td><!-- FORM-TABLE:START -->\n";
    echo "<table border=0 cellspacing=0 cellpadding=5 bgcolor=#EFEFEF>\n";
    
    $this->headRow($headtext);
    
}

/**
 * show raw table row
 * @param string $pre
 * @param string $title
 * @param string $value
 * @param string $notes
 * @param array $opt
 *    'title.pure' : 0,1 - no bold title
 */
function form_row_raw($pre, $title, $value, $notes=NULL, $opt=array()) {
    $tit_bold=1;
    if ($opt['title.pure']>0) $tit_bold=0;
    
    echo "<tr valign=top>";
    echo "<td>";
    echo "</td>";
    echo "<td>";
    if ($tit_bold) echo   "<b>".$title."</b>";
    else  echo  $title;
    echo "</td>";
    echo "<td>".$value."</td>";
    echo "<td>".$notes."</td>";
    echo "</tr>\n";
}

/**
 * get HIDDEN parameters ...
 */
function get_hidden_params($in_array, $expcept_keys=array() ) {
    
    $hidden=array();
    foreach($in_array as $key => $val) {
        
        if (in_array($key, $expcept_keys)) continue;
        
        if (is_array($val)) {
            foreach($val as $key2 => $val2) {
                $hidden[$key.'['.$key2.']'] = $val2;
            }
        } else {
            $hidden[$key] = $val;
            
        }
    }
    return $hidden;
}

/**
 * add HIDDEN parameters ...
 * do not save NULL params ...
 */
function add_hidden_params($in_array, $expcept_keys=array() ) {
    
    foreach($in_array as $key => $val) {
        
        if (in_array($key, $expcept_keys)) continue;
        
        if (is_array($val)) {
            foreach($val as $key2 => $val2) {
                echo '<input type="hidden" name="'.$key.'['.$key2.']" value="'.$val2.'">'."\n";
            }
        } else {
            if ($val===NULL or $val==='') continue;
            $encoded = htmlentities($val, ENT_QUOTES);
            echo '<input type="hidden" name="'.$key.'" value="'.$encoded.'">'."\n";
        }
    }
}

/**
 * add all request parameters ...
 */
private function add_all_request_params($more_except=array()) {
    $expcept_keys = array('proj_id', 'tablename', 'go');
    if (!empty($more_except)){
        $expcept_keys=array_merge($expcept_keys, $more_except);
    }
    if ( !is_array($_REQUEST['newparams']) ) $_REQUEST['newparams']=array();
    $this->add_hidden_params($_REQUEST, $expcept_keys);
}

// newparams
public static function get_requests_for_URL() {
    
    $urlarr=array();
    foreach($_REQUEST['newparams'] as $key => $val) {
        $urlarr[]='&newparams['.$key.']='.urlencode($val);
    }
    $out=implode('',$urlarr);
    return $out;
}

/**
 * 
 * @param string $name
 * @param string $inputcell
 * @param string $notes
 * @param array $options
 *   'required' : 0,1
 */
function htmlRow(string $name, string $inputcell, $notes, $options=array() ) {

	$colorx = $this->rowarr["bgcolor"][0];
	$fontc  = $this->colorKey;
	
	echo "<tr bgcolor=".$colorx."><td>";
	echo "&nbsp;"; // leave empty
	echo "</td>\n";
	echo "<td align=right>";	
	echo '<span style="color:'.$fontc.'"><b>'.$name.':</b></span>';	
	if ($options['required']) echo ' <img src="images/redstar.gif">';
	echo "</td><td>";
	echo "<input type=hidden name=dummy>";
	echo "<input type=hidden name=dummy>";
	echo $inputcell;
	echo "</td><td>";
	echo "<i>".$notes."</i>";
	echo "</td>";
	echo "</tr>\n";
}

function htmlRowEmpty($text=NULL) {
	// background=\"0w1.gif\" bgcolor=#0FEFEF 
	echo "<tr ><td colspan=".$this->COLSPAN." >";
	
	if ( $text!="") {
		echo "<hr style=\"margin:0px; border:0; border-top: 1px solid  ".$this->formLineColor.";  height: 1px;\">\n";
	} 
	else echo "<img src=0.gif width=1 height=1>";
	
	echo "</td>";
	echo "</tr>\n";
}

function headRow($text) {
	echo "<tr bgcolor=#C0C0C0 ><td colspan=".$this->COLSPAN." >";
	echo "<font class=x1>".$text."</font>";
	echo "</td>";
	echo "</tr>\n";
}

function _linkOut( $url, $text, $active) {
	// get link-text
	if ($active) $outtext = "<font style='color:gray; font-weight:bold;'>".$text."</font>";
	else $outtext = "<a href=\"".$url."\" style='color:gray;'>".$text."</a>";
	
	return ($outtext);
}

// show DB object link selector
function db_obj_link($tabname, $varname, $obj_tmpl_id, $obj_tmp_name, $html_tab_cnt) {
    $fopt = array("noshDel"=>1);
    $answer = $this->jsFormLib->getAll( $tabname, $varname, $obj_tmpl_id, $obj_tmp_name,  $html_tab_cnt, $fopt);
    return $answer;
}


// 
// 
/**
 * manage the first columns of a selection-row
 * @param int $rowpos 1,2,3
 * @param string $creatype "empty", "obj_tmpl", "obj"
 * @param string $text
 */
function form_row_ColFirst( $rowpos,  $creatype,  $text, $formRadios, $selecter  ) {
 
        //$tablename = $this->tablename;
        $sel_name  = $creatype;
        $backColor = $this->rowarr["bgcolor"][$rowpos];
        
        if ($formRadios) {
            
            if  ($selecter == $sel_name) {
                $checkedx  = ' checked';
                $backColor = $this->colorSelect;
                $this->rowposChecked = $rowpos;
            } else {
                $checkedx  = "";
            }
            
            echo "<tr id=t".$rowpos." bgcolor=".$backColor."><td valign=top>";
            echo '<input type="radio" name="selecter" value='.$sel_name.$checkedx;
            echo " onclick=\"th_snc( 't".$rowpos."' )\">";
        } else {
            // no selection of $creatype possible
            echo "<tr id=t".$rowpos." bgcolor=".$backColor."><td valign=top>";
            echo '<input type="hidden" name="selecter" value="'.$selecter.'">'."\n";
        }
        
        echo "</td>\n";
        
        echo "<td valign=top>";
        echo "   <font color=#606000 size=-1><B>".$text."</B></font>";
        echo "</td>";
        echo "<td valign=top>";
}

/**
 * show row for NAME of object (OLD: formRow_Name)
 * GLOBAL: INPUT: $this->formRowShow["name"]
 * @param object $sql
 * @param int $edit_flag
 *    -1 not allowed
 *     1 allowed
 */
function form_row_Name( &$sql, $useName, $edit_flag=1, $notes_text=NULL ) {
    // global $error;
    
    $tablename       = $this->tablename;
    $main_name_col = importantNameGet2($tablename);
    $mostimpcol_nice = columnname_nice2($tablename, $main_name_col);
    
    
    if (($tablename=="LINK") || ($tablename=="IMG")) {
        $notes_text = "If empty, takes name from uploaded file.</I>";
    }
    
    
    echo "<tr bgcolor=". $this->rowarr["bgcolor"][0].">";
    
    ?>
    <td>
    &nbsp;
    </td><td NOWRAP align=right>
	<?
	$tmpcolor = NULL;
    $tmptext  = "";
	if (($tablename=="IMG") || ($tablename=="LINK") || ($tablename=="CONCRETE_SUBST")) {
		$tmpcolor = "#606060";
        $tmptext  = "[optional]"; 
	}
	echo '<span style="color:'.$tmpcolor.';"><B>'.$tmptext." with ".$mostimpcol_nice.":</B></span>";
	
    ?>
   	</td><td>
   	<!-- for modal-dialog purposes -->
   	<input type=hidden name=dummy>
	<input type=hidden name=dummy>
	<?php 
	if ($edit_flag>=0) {
	   echo '<input type="text" name=obj_name value="'.$useName.'" size=40>'."\n";
	} else {
	   echo '<input type="hidden" name=obj_name value="'.$useName.'">'."\n";
	   echo htmlspecialchars($useName)."\n";
	}
	echo '</td><td>'."\n";
	
	if ($notes_text!=NULL) {
	   echo '<span class=yGgray>'.$notes_text.'</span>';
	}
	echo '&nbsp;</td></tr>';
		
	
}

/**
 @param array $advView=array(
				'url'=>$_SERVER['PHP_SELF'].'?tablename='.$tablename.'?proj_id='.$proj_id,
				'textarr'=>array( 'norm'=>'normal' ),
				'val'=> $advancedView
 @param array $options
   'submit.allow' : [1] , -1
   'submit.text' = ...
   'back.button' : 0,1 show Back-Button;  (needs 'back.go')
   'back.go'     : int
*/
function htmlRowSubmit($advView, $options=array() ) {
    
    
	$this->htmlRowEmpty("hr");
	
	$submit_text  = $options['submit.text']!=NULL ? $options['submit.text'] : 'Create';
	$submit_allow = $options['submit.allow'] ?? 1;
	
	echo "<tr bgcolor=".$this->colorSubmit.">";
	echo "<td valign=middle colspan=2>";
	
	if ($options['back.button']) {
	    $back_go=$options['back.go'];
	    if (!$back_go) $back_go=0;
	    echo '<input type=button value="Back" class="yButton" onClick="x_form_back('.$back_go.')">';	
	}
	
	echo "</td><td valign=middle >"; // align=center
	
	if ($submit_allow>0) echo "<input type=submit value=\"".$submit_text."\" class='yButton' >";	
	
	if ($advView!=NULL) {
		echo '&nbsp;&nbsp;&nbsp;';
		$textarr = $advView['textarr'];
		if ( $advView['val']=='norm' ) $active=1;
		else $active=0;
		echo $this->_linkOut( $advView['url'].'norm' , $textarr['norm'], $active );
		echo ' | ';
		if ( $advView['val']=='all' ) $active=1;
		else $active=0;
		echo $this->_linkOut( $advView['url'].'all' , $textarr['all'], $active );
	} 
	echo "</td> ";
	echo "<td>&nbsp;<br>&nbsp;</td> ";
	echo "</tr>\n";
}

public function showPostHead($title, $headNotes=NULL) {
	$hrtag = '<hr style="margin:4px; border:0; width:600px; border-top: 1px solid; text-align: left;">';
	echo '<br /><font color=gray><b>'.$title.'</b></font>';
	echo "&nbsp;&nbsp;".$headNotes;
	echo '<br>'.$hrtag."&nbsp;&nbsp;&nbsp;&nbsp;\n";
}

/**
 * show UPLOAD-file fields
 * @param $projid - comes from $this->projid
 * @param  $opt 
 *   ['noHR'] => 0,1
 *   'sh_upload_flag' : 0,1
 */
public function formUploadFile( $tablename, $projid, $opt=NULL ) {
	$colorKey     = $this->colorKey;
	$tmp_max_file = $_SESSION['globals']["F.IMG.IMPORT.UPLOAD_MAX_SIZE"];
	
	if ( !$opt['noHR'] ) $this->htmlRowEmpty("hr"); // "<hr size=1>"
	?>
	<tr bgcolor=#EFEFEF><td>
		&nbsp;
	</td><td valign=top align=right><font size=-1 color=<?echo $colorKey?>>
		<B>Upload file:</B></font></td>
	<td colspan=2>
	<INPUT TYPE="hidden" name="MAX_FILE_SIZE" value="<? echo $tmp_max_file; ?>">
	<INPUT NAME="userfile" TYPE="file" accept="*.*" size=50><br>
	<span style="color:gray;">Max file size: <B><? echo $tmp_max_file/1E6; ?></B>&nbsp;MBytes.
	<?php 
	if ( $opt['sh_upload_flag']) {
	    echo '<br><INPUT NAME="newpx[upload.need]" TYPE="checkbox" value="-1"> Do NOT upload a file'."\n";
	
	}    ?>
	</span></td>
	</tr>
	<? 
}

/**
 * show asscociated lists
 * deny: VIEWS, INTERN-tables
 * @param object $sql
 */
function showAssocBoxes(&$sql) {
    
    $tablename = $this->tablename;
    
    $assoc_arr = get_assoc_tables($tablename);
    if (empty($assoc_arr)) return;
    
    $assoc_cnt = sizeof ($assoc_arr);
    
    if ( $assoc_cnt ) {
        echo "<font color=gray>copy lists:</font><br>\n";
        foreach( $assoc_arr as $assoc_name=>$infoarr) {
            
            $allow	= 1;
            
            $assoc_nicename	= $infoarr['nice_name'];
            
            //EXAMPLE: if ( ($tablename=="W_WAFER") && ($assoc_name=="W_CHIP_FROM_WAFER") ) $allow=0; // not allowed
            
            if ($infoarr['is_view']>0)   $allow=0;
            if ($infoarr['is_intern']>0) $allow=0;
            
            if ($allow) {
                echo "<input type=checkbox name=deep_copy[".$assoc_name."] value=\"1\"> ".
                    "<font color=gray>".$assoc_nicename."</font><br>";
            }
        }
        
    }
}

/**
 * - show a form to select an other destination project
 * - form-input-params will be send again
 * - set GO variable to the original GO value ...
 */
function selProject(&$sql, $proj_id, $current_go) {

    $initarr   = NULL;
    $initarr["action"]      = $_SERVER['PHP_SELF'];
    $initarr["title"]       = "Select destination folder";
    $initarr["submittitle"] = "Next &gt;&gt;";
    $initarr["tabwidth"]    = "AUTO";
    $initarr["dblink"]      = 1;
    $initarr["goNext"]      = $current_go;
    $initarr["tabnowrap"]    = 1;
    $hiddenarr = NULL;
    
    if ($proj_id) {
        $projPathObj = new oPROJpathC();
        $proj_name   = $projPathObj->getPathText($sql, $proj_id, 0, 4  );
    }
    
    // send old params again 
    if ( sizeof($_REQUEST) ) {
        $except_keys= array('proj_id', 'action', 'go');
        $hiddenarr = $this->get_hidden_params($_REQUEST, $except_keys);

    }
    
    
    $formobj = new formc($initarr, $hiddenarr, 0);
    
    $fieldx = array (
        "title" => "Folder",
        "name"  => "proj_id",
        "object"=> "dblink",
        "namex" => TRUE,
        "val"   => $proj_id,
        "inits" => array(
            "table"  => "PROJ",
            "objname"=> $proj_name,
            "pos"    => 0,
            "projlink"=> 1,
            'noshDel' =>1
        ),
        "notes" => ""
    );
    $formobj->fieldOut( $fieldx );
    
    $formobj->close( TRUE );
}

/**
 * finish form, finish table
 * @param array $opt
 * 'all_req_params' : 0,1
 * 'params_except_newparams' : 0,1 -- do not add hidden newparams
 * 'rowposChecked' : string
 */
function form_finish( $opt=array() ) {
    
    ?>
	<script language="JavaScript" type="text/JavaScript">
	<?
	if (  $this->rowposChecked != "" ) {
	    echo "lastTag=\"t".$this->rowposChecked."\";";
	}
	?>
	</script>
	<?
	
	echo "</table><!-- FORM-TABLE:END -->\n";
	echo " </td></tr></table>\n";
	if ($opt['all_req_params']) {
	    $except_opt=array();
	    if ($opt['params_except_newparams']) $except_opt=array('newparams');
	    $this->add_all_request_params($except_opt);
    }
	echo "</form>\n"; 
}

function form_finish_pure($opt=array() ) {
    
    if ($opt['all_req_params']) {
        $this->add_all_request_params();
    }
    echo "</form>\n"; 
}


}
