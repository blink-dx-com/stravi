<?php
/**
 * JOUR_ENTRY main editor form
 * @author Steffen Kube
 * @module obj.jour_entry.ed.inc
 */


require_once ("o.PROJ.addelems.inc");
require_once ("o.proj.profile.inc");
require_once ("func_form.inc");
require_once ("javascript.inc");
require_once ("class.history.inc");
require_once ("date_funcs.inc");
require_once ("f.help.inc");
require_once 'f.clipboard.inc';
require_once 'f.msgboxes.inc';

require_once ('o.LINK.keydoc.inc');
require_once ('gui/o.JOUR_ENTRY.gui.inc');
require_once ('o.JOUR_ENTRY.subs2.inc');
require_once ('o.JOUR_ENTRY.mod.inc');

/**
 * JOUR_ENTRY main editor form
 * @param $parx
 *    KEYs_old
 *    KEYs_new
 *
 */
class oEXPlabjour1 {
    
    private $params;

    function __construct(
    	$id, 	// can be NULL, if CREATE
    	$action // update, create
    	) {
    	global $varcol, $error;
    	$varcol = &Varcols::get();
    	
    	$this->params = NULL;
    	$this->id	  = $id;
    	$this->parx   = NULL; // e.g. "PROJ_ID"
    	$this->tablename   = "JOUR_ENTRY";
    	$this->tableNice   = tablename_nice2($this->tablename);
    	$this->dateStrLen  = 
    	$this->action 	   = $action;
    	$this->datestamp   = date_unix2datestr( time(),2 );
    	$this->dateStringLength = strlen("2000-01-01");
    	$this->bookclassid = $varcol->class_name_to_id( "LINK", "labbook" );
    	
    	$this->LabJourLib = new oEXPlabjourC();
    	$this->LabJourLib->setPrefs();
    	$error->reset();
    }
    
    /**
     * get default lab journal project
     */
    private function _get_profile_proj($sqlo) {
        // $hist_obj = new historyc();
        // $lastProj = $hist_obj->last_bo_get('PROJ');
        $projProfObj = new profile_funcs ();
        $actionxc   = "CREATE";
        $proj_id = $projProfObj->getProjBySyn( $sqlo, "lab-journal-collection", $actionxc );
        return $proj_id;
    }
    
    function setParamsNew(&$sql, $params, $parx) {
        
        if ( !$this->parx["PROJ_ID"] ) {
            $use_proj_id = $this->_get_profile_proj($sql);
            $this->parx["PROJ_ID"] = $use_proj_id;
        }
        $this->params["EXEC_DATE"] = date_unix2datestr(time(), 2 );
        $this->params["IMPACT"]    = $this->parx["IMPACT"];
        
    }
    
    function setParams(&$sql, $params, $parx, $go) {
    
    	$this->parx   = $parx;
    	$this->params = $params;

    	if ( $go<1 ) {	// initial settings
    			
    		// nothing
    		
    	} else {
    	    
    	    $keys_all = $parx['KEYs_old'];
    	    if (!empty($keys_all)) {
    	        $keys_all = array_map('trim', $keys_all);
    	    }
    	    
    	    $keys_new_arr = array();
    	    $this->params["KEYS"] = NULL;
    	    if ( !empty($parx['KEYs_new']) ) {
    	        $keys_new_arr = explode(',',$parx['KEYs_new'] );
    	        $keys_new_arr = array_map('trim', $keys_new_arr);
    	    }
    
    	    if ( !empty($keys_all) ) {
    		    if (!empty($keys_new_arr)) {
    		        foreach($keys_new_arr as $new_key) {
    		            if ( in_array($new_key,$keys_all) ) {
    		                // nix
    		            } else {
    		                $keys_all[]=$new_key;
    		            }
    		        }
    		    }
    		} else {
    		    $keys_all = $keys_new_arr;
    		}
    		
    		$this->params["KEYS"]  = implode(', ', $keys_all);
    	}
    
    }
    
    
    /**
     * create new entry
     * @param  object $sql
     * @global $this->params["NAME"], $this->params["EXEC_DATE"]
     * 
     * @return -
     */
    function createEntry(&$sql) {
    	global $error;
    	$FUNCNAME='createEntry';
    	
    	$errAnswer = $this->checkParams();
    	if ($errAnswer!="") {
    		$error->set("createEntry", 2, "Parameter check failed: ".$errAnswer);
    		return;
    	} 
    	
    	if ($this->params["NAME"]==NULL) {
    		$this->params["NAME"] = substr($this->params["EXEC_DATE"],0,$this->dateStringLength) . " " . $this->parx["NAME"];
    	}
    	
    	$args  = array ("vals"=> $this->params );
    	// $insopt= array("types" => array( "EXEC_DATE" => "DATE1") );
    	$modlib = new oJOUR_ENTRY_mod();
    	$newid  = $modlib->insert( $sql, $args);
    	$this->id = $newid;
    	if ( !$this->id OR $error->Got(READONLY)) {
    		$error->set($FUNCNAME, 1, "Error during entry creation");
    		return;
    	}
    	echo "<font color=green>Entry ".$this->id." created.</font><br>\n";
    	$this->action = "update"; // transform action
    	
    	// add to project
    	if ($this->parx["PROJ_ID"]) {
    		$hist_obj = new historyc();
    		$hist_obj->historycheck("PROJ", $this->parx["PROJ_ID"]); // bring to history
    		
    		$projLib = new oProjAddElem( $sql, $this->parx["PROJ_ID"] );
    		if ($error->got(READONLY)) {
    			$error->set($FUNCNAME, 7, "Copy to folder ".$this->parx["PROJ_ID"]." failed.");
    	    	return;
    	    }
    		$projLib->addObj( $sql,  $this->tablename, $this->id ); 
        }
    	
    	return ( $this->id );
    }
    
    function isValidDate(
    	$dateStr // "2006-10-01"
    	) {
    	
    	$year  = substr($dateStr,0,4);
    	$month = substr($dateStr,5,2);
    	$day   = substr($dateStr,8,2);
    	$answer = checkdate ( (int)$month, (int)$day, (int)$year );
    	return ($answer);
    
    }
    
    function checkParams() {
    	$MAXLEN_KEYS=255;
    	
    	if ( !$this->isValidDate($this->params["EXEC_DATE"]) ) {
    		$answer = "Please give a valid DATE!";
    		return ($answer);
    	}
    	
    	if ( $this->parx["NAME"]=="" ) {
    		$answer = "Please give an entry name!";
    		return ($answer);
    	}
    	
    	if ( strlen($this->params["KEYS"]) > $MAXLEN_KEYS) {
    		$answer = "KEY text is too long (max '.$MAXLEN_KEYS.' chars)!";
    		return ($answer);
    	}
    	
    	return;
    }
    
    function updateEntry(&$sqlo) {
    	global $error;
    	
    	$errAnswer = $this->checkParams();
    	if ($errAnswer!="") {
    		$error->set("updateEntry", 1, "Parameter check failed: ".$errAnswer);
    		return;
    	} 
    	
    	$this->params["NAME"] = substr($this->params["EXEC_DATE"],0,$this->dateStringLength) . " " . $this->parx["NAME"];
    	$this->params["JOUR_ENTRY_ID"] = $this->id;
    
    	$modlib = new oJOUR_ENTRY_mod();
    	$modlib->set_obj($sqlo, $this->id);
    	$modlib->update($sqlo, $this->params);
    	
    	
    	if ( $error->Got(READONLY)) {
    		$error->set("updateEntry", 2, "Error during entry update");
    		return;
    	}
    	echo "<font color=green>Entry ".$this->id." updated.</font><br>\n";
    
    	return;
    }
    
    private function _row_open(string $key) {
        echo "<tr><td align=right>".$key."</td><td>\n";
    }
    private function _row_close() {
        echo "</td></tr>\n";
    }
    
    /**
     * 
     * @param string $key
     * @param string $button
     * @param array $opt
     *   "type" 
     *       "[SPACE]
     *       "submit"
     *       "NOTES" 
    	 "required" => 0,1
     */
    private function _rowout(string $key, $button, $opt=NULL	) {
    	
        if ( $opt["type"] == "NOTES" ) {
            echo "<tr><td colspan=2>".$button."</td></tr>\n";
            return;
        }
        
    	$keyOut = $key;
    	$requiredIcon = "";
    	if ($key=="[SPACE]") {
    		echo "<tr><td><img src=0.gif width=1 height=5></td></tr>\n";
    		return;
    	} 
    	
    	if ( $opt["required"] == "1" ) {
    		$requiredIcon = " <img src=\"images/redstar.gif\" TITLE=\"required\">";
    		$keyOut = "<b>".$key."</b>"; 
    	}
    	
    	if ($key!="") $keyOut = $keyOut;
    	if ( $opt["type"] == "submit" ) {
    		// $tmpcolor = " bgcolor=#D0D0FF";
    		$keyOut = "<img src=0.gif width=1 height=35>";
    	}
    	echo '<tr valign=top><td style="text-align: right; padding-right:10px;">'.$keyOut.$requiredIcon."</td><td>$button</td>";
    	echo "</tr>\n";
    }
    
    /**
     *  get lab books from "bookmark" project
    - if not exists, create a "bookmark" project
     */
    function getFromProjProfile( &$sql ) {
    	
        global $varcol;
        $varcol = & Varcols::get();
        
        $BOOK_CLASS = 'labbook';
        $table      = "LINK";
        
    	$projProfObj = new profile_funcs ();
    	$actionxc   = "CREATE";
    	$projBookId = $projProfObj->getProjBySyn( $sql, "bookmarks", $actionxc );
    	$this->projBookId = $projBookId;
    	
    	if (!$projBookId) return;
    	
    	
    	$sqlsel  = "p.PRIM_KEY, l.NAME, l.EXTRA_OBJ_ID from PROJ_HAS_ELEM p JOIN LINK l ON p.PRIM_KEY=l.LINK_ID ".
    		     "where p.PROJ_ID=".$projBookId." AND p.TABLE_NAME='".$table."'";
    	$sql->Quesel($sqlsel);
    	
    	$destarr = array();
    	$xo_tmp  = array();
    	while ( $sql->ReadRow() ) {
    		$tmpid   = $sql->RowData[0];
    		$tmpname = $sql->RowData[1];
    		$destarr[$tmpid] = $tmpname;
    		
    		$xo_tmp[] = $sql->RowData[2];
    	}
    	
    	// check for class, delete bad documents
    	$ids = array_keys($destarr);
    	foreach($ids as $i => $tmp_id) {
    	  
    	    $xo = $xo_tmp[$i];
    	    if (!$xo) {
    	        unset($destarr[$tmp_id]);
    	        continue;
    	    }
    	    
    	    $classname  = $varcol->obj_id_to_class_name( $xo );
    	    if ($classname != $BOOK_CLASS) {
    	        unset($destarr[$tmp_id]);
    	        continue;
    	    }
    	}
    	
    	$now_book_id = $this->params["LINK_ID"];
    	
    	if (empty($destarr) and !$now_book_id) return;
    	
    	if ($now_book_id and !$destarr[$now_book_id]) {
    	    $destarr[$now_book_id] = obj_nice_name ( $sql, $table, $now_book_id );
    	}
    	
    
    	$tmptext = formc::selectFget( "params[LINK_ID]", $destarr, $this->params["LINK_ID"] ); 
    	return ($tmptext);
    }	
    
    function _showNewBookSel( &$journbooktxt, $bookSelect) {
    		// RETURN: $journbooktxt
    	$journbooktxt .= " &nbsp;&nbsp;&nbsp;<a href=\"edit.tmpl.php?t=PROJ&id=".$this->projBookId."\" target=_new >".
    		"<img src=\"images/but.heart.png\" title=\"Journal books in your favorites\" border=0></A>";
    	$url = "obj.jour_entry.docnew.php";
    	if ( $bookSelect == "" ) {
    		// TBD: create a lab book automatically
    	} else {
    		//
    	}
    	$journbooktxt .= "&nbsp;<a href=\"".$url."\" target=_new >".
    		"<img src=\"images/but.40.new.png\" title=\"new Journal book\" height=20></A>";
    }
    
    private static function _field_IMPACT($IMPACT) {
        
        if (!glob_column_exists('JOUR_ENTRY', 'IMPACT')) return;
       
        $data_init=array(
            
            '1'=>'1 - low',
            '2'=>'2',
            '3'=>'3',
            '4'=>'4',
            '5'=>'5',
            '6'=>'6',
            '7'=>'7',
            '8'=>'8',
            '9'=>'9',
            '10'=>'10 - high',
        );
        
        $selopt   = array(
            'selecttext'=> '- level -'
        );
        $seltext = formc::selectFget( 'params[IMPACT]', $data_init, $IMPACT, $selopt);
        return $seltext;
    }
    
    private function _multi_select($data) {
        
        $usefield=array();
        foreach( $data as $val) {
            $usefield[$val]=$val;
        }
        
        $selopt   = array(
            'selbody'=>' multiple size=5',
            'selecttext'=> '-- multiselect --' 
        );
        $seltext2 = formc::selectFget( 'parx[KEYs_old][]', $usefield, NULL, $selopt);
        $textKey  = '<span class="yGgray">Keywords:</span><br>'.$seltext2;
        return $textKey;
    }
    
    private function _multi_select2( &$possible_keys, $keys_old_str) {
        $keys_old_arr = explode(",", $keys_old_str); 
        $keys_old_arr = array_map('trim', $keys_old_arr);  // remove white spaces

        $seltext2='<div class="x_container">'."\n";
        $seltext2.='<select class="x_form-control" name="parx[KEYs_old][]" id="choices-multiple-remove-button" placeholder="Keywords" multiple>';
        
        foreach( $possible_keys as $val) {
            $val=trim($val);
            if ($val==='') continue;
            $tmpsel='';
            if (in_array($val, $keys_old_arr)) {
                $tmpsel=' selected';
            }
            $seltext2 .= '<option value="'.$val.'"'.$tmpsel.'>'.$val.'</option>'."\n";
        }
        // now OLD keys are NOT in $possible_keys
        foreach($keys_old_arr as $val) {
            if ($val==='') continue;
            $tmpsel='';
            if (!in_array($val, $possible_keys)) {
                $seltext2 .= '<option value="'.$val.'" selected>'.$val.'</option>'."\n";
            }
        }
        $seltext2 .='</select>';
        $seltext2 .='</div>'."\n";
  
        return $seltext2;
    }
    
    private function _getKEY_from_doc( &$sqlo, $keybookID, $val) {
        $textKey = ' <font color=gray>No KEY-dictionary set</font>';
        $DocKeyLib = new oLINK_keydoc();
        $DocKeyLib->open($sqlo, $keybookID);
        
        if (!$DocKeyLib->hasKeys()) {
            $textKey = ' <font color=gray>KEY-dictionary [DOC-ID:'.$keybookID.'] is empty</font>';
            return $textKey;
        }

        $doc_keys = $DocKeyLib->getKeys();
        $textKey = self::_multi_select($doc_keys);
        
        return $textKey;
    }
    
    
    /**
     * get specials for column KEY
     * @return 
     * @param object $sqlo
     */
    private function _getKEY_txt( &$sqlo, $val) {
    	$text    ='<input type=text name="parx[KEYs_new]" size=50 value="'.$val.'" placeholder="Komma separated keywords">';
    	//$comment = ' <span class="yGgray">Komma separated keywords</span>';
    	return $text;
    }
 
    
    function _get_keys_box($sql, $key_str) {
        
        // FURURE ...
        //         $keybookID = $this->LabJourLib->getKeyBookID();
        //         if ($keybookID) {
        //             $textKey = $this->_getKEY_from_doc( $sql, $keybookID, $this->params["KEYS"]);
        //             return $textKey;
        //         }
        
        $key_arr = oJOUR_ENTRY_subs::get_distinct_keys($sql);
        $textKey='';
       
        if( !empty($key_arr) or $key_str!=NULL) {
            $textKey = self::_multi_select2($key_arr, $key_str);
        }
        return $textKey;
    }
    
    private function _sh_attachments($sql) {
        
        echo '<span class="yGgray">Attachments:</span>';
        if ($this->id) {
            echo ' &nbsp;<a href="obj.sattach.view.php?t=JOUR_ENTRY&id='.$this->id.
                '" title="edit attachments"><img src="images/but.edit.gif"></a>';
        }
        echo '<br>'."\n";
        
        if ($this->id) {
            $this->LabJourLib->ShowAttach($sql, $this->id);
        }
    }
    
    function _js_css_load() {
        $jsFiles  = array('res/choices/scripts/choices.min.js');
        foreach($jsFiles as $cfile) {
            echo '<script src="'.$cfile.'"></script>'."\n";
        }
        
        $cssFiles  = array(
            // 'res/choices/styles/base.min.css',
            'res/choices/styles/choices.css',
        );
        foreach($cssFiles as $cfile) {
            echo '<link rel="stylesheet" href="'.$cfile.'" />'."\n";
        }
        
        ?>
        <style>
        
        .x_container {
          display: block;
          margin: auto;
          width: 100%;
          padding: 0px;
        }
        
        .x_form-control {
          display: block;
          width: 100%;
          background-color: #f9f9f9;
          padding: 3px;
          border: 1px solid #ddd;
          border-radius: 2.5px;
          font-size: 12px;
          appearance: none;
          margin-bottom: 4px;
        }
        
        </style>
        <?php
        
    }
    
    function showform( &$sql, $addinfo=NULL) {
    
    	$nheight    = 30;
    	$submitText = "Update";
    	if ($this->action=="create") $submitText = "&nbsp;&nbsp;CREATE&nbsp;&nbsp;";
    	
    	if ($this->action=="create") {
    		$lastProjName = " ------ ";
    		if ( $this->parx["PROJ_ID"] ) {
    			$lastProjName = obj_nice_name ( $sql, "PROJ", $this->parx["PROJ_ID"]);
    		}
    	}
    	
    	$this->_js_css_load();
    	
    	js_formAll();
    	
    	$bookSelect = $this->getFromProjProfile( $sql );
    
    	
    	echo "<form style=\"display:inline;\" method=\"post\"  name=\"editform\"  action=\"".$_SERVER['PHP_SELF']."\" >\n";
    	echo '<div style="border: 1px solid #DFDFDF;" >'; //TBD: border
    	echo '<div class="yParent"><!-- div.PARENT -->'; //div.PARENT
    	echo '<div style="padding:20px;">';
    	echo '<input type=submit class="yButton" value="'.$submitText.'">'."\n";
    	echo '</div>'."\n";
    	echo '<div>';
    	echo "<table cellpadding=1 cellspacing=0 border=0 width=100% bgcolor=#FFFFFF>";
    	
    	
    // 	$optrow = array ("type" => "submit");
    // 	$submit_button = '<input type=submit  class="yButton" value="'.$submitText.'">'."\n";
    // 	$this->_rowout("",$submit_button, $optrow);
    	
    	// $this->_rowout("[SPACE]","");
    	
    	if ($this->action=="create") {
    	    
    	    $requiredIcon = " <img src=\"images/redstar.gif\" TITLE=\"required\">";
    	    $this->_row_open("<b>Folder</b> ".$requiredIcon);
    	    
    		$proopt = array ("showproj" => 1);
    		$jsFormLib = new gJS_edit();
    		$answer = $jsFormLib->getAll( "PROJ","parx[PROJ_ID]", $this->parx["PROJ_ID"], $lastProjName, 1, $proopt );
    		echo $answer;
    		
    		$this->_row_close();
    		
    	} else {
    		$this->_rowout( "Folder", $addinfo	);
    	}
    	
    	$impact_field = self::_field_IMPACT($this->params["IMPACT"]);
    	
    	$optrow = array ("required" => 1);
    	$this->_rowout( "Date",		 "<input type=text name='params[EXEC_DATE]' size=16 value='".$this->params["EXEC_DATE"]."'>".
    	    '&nbsp;&nbsp;<span style="color:gray">YYYY-MM-DD hh:mm</span>'."\n &nbsp;&nbsp;&nbsp;<b>Impact: </b>" . $impact_field, $optrow);
    	
    	$optrow = array ("required" => 1);				
    	$this->_rowout( "Entry Name",
    	    "<input type=text name='parx[NAME]' size=50 value='".$this->parx["NAME"].
    	    "'> \n", $optrow); // <font color=gray>Date will be automatically added.</font>
    	
    	
    	$bookIDBox = $bookSelect;
    	// if ($bookIDBox=="") $bookIDBox="<input type=text name='params[LINK_ID]' size=6 value='".$this->params["LINK_ID"]."'>";
    	if ($bookIDBox=="") {
    	    $bookIDBox='<span class="yGgray">not available</span> &nbsp;';
    	}
    	
    	$journbooktxt = $bookIDBox . " Start-Page: <input type=text name='params[START_PAGE]'  size=6 value='".
    		$this->params["START_PAGE"]."'>\n";
    	if ( $this->projBookId ) {
    		 $this->_showNewBookSel( $journbooktxt, $bookSelect);	
    	}
    	if ( $bookSelect == "" ) $journbooktxt .= "&nbsp;&nbsp;&nbsp;".
    		"<I> See help for: Journal-Book</I>";
    	
    	$this->_rowout( "Journal-Book", $journbooktxt );
    	
    	$keytext = $this->_get_keys_box($sql, $this->params["KEYS"]);
    	$this->_rowout( "Current keywords", $keytext);
    	$keytext = $this->_getKEY_txt($sql, $this->parx["KEYs_new"])."\n";
    	$this->_rowout( "New Keywords", $keytext);
    	
    	echo "</table>\n";
    	echo '  </div>'."\n";
    	
    	// OLD keyword box
    	// echo '<div style="padding-left:10px;"><!-- select:MULTIPLE -->'."\n"; // Key multiple ..
    	// get SELECT BOX
    	// echo $this->_get_keys_box($sql);
    	// echo '  </div>'."\n";
    	
    	echo '<div style="padding-left:10px;">'."\n"; // attachments
    	
    	$this->_sh_attachments($sql);
    	
    	
    	echo '  </div>'."\n";
    	echo '</div><!-- div.PARENT:END -->'."\n";
    	
    	$optrow = array ("type" => "NOTES");
    	$input ="<textarea name='params[NOTES]' rows=".$nheight." style=\"width:100%\">" .
    	     $this->params["NOTES"]."</textarea>";  
    	echo $input."\n";
    	//$this->_rowout( "Notes", $input, $optrow);
    
    	// $optrow = array ("type" => "submit");
    	//$this->_rowout("",$submit_button , $optrow); 
    	
    	
    	echo "</div>\n";
    	echo "<input type=hidden name='go' value='1'>\n";
    	echo "<input type=hidden name='action' value='".$this->action."'>\n";
    	echo "<input type=hidden name='id' value='".$this->id."'>\n";
    	echo "<input type=hidden name='t'  value='".$this->tablename."'>\n";
    	//echo "<input type=submit value=\"".$submitText."\">\n";
    	
    	echo "</form>";
    	
    	?>
    	<script>
    	var multipleCancelButton = new Choices(
    	    '#choices-multiple-remove-button',
    	    {
    	        removeItemButton: true,
    	    }
    	);
    	</script>
    	<?php
    	
    }
    
    public function paste_objects($sqlo) {
        $clip_lib = new clipboardC();
        $objects = $clip_lib->getAll();
        if(empty($objects)) {
            cMsgbox::showBox("warning", "No objects in clipboard.");
            echo '<br>';
            return 0; 
        }
        
        $notes = $this->params['NOTES'];
        $notes .= "\n\n";
        $num_obj = 0;
        foreach($objects as $row) {
            
            $obj_link='db:'. globTabGetSHORT($row['t']).':'.$row['id'];
            
            // check link already exists
            if ( strstr($notes, $obj_link)!=NULL ) continue;
          
            $num_obj++;
            $notes .= '  * '.$obj_link."\n";
        }
        
        if(!$num_obj) {
            cMsgbox::showBox("warning", "No NEW objects added.");
            echo '<br>';
            return 0;
        }
        
        
        $this->params['NOTES'] = $notes;
        
        $modlib = new oJOUR_ENTRY_mod();
        $modlib->set_obj($sqlo, $this->id);
        $modlib->update($sqlo, $this->params);
        
        return $num_obj;
    }
    
    function Hints() {
        
    	echo "<br><br><br>";
    	htmlInfoBox( "Short help", "", "open", "CALM" );
    	?>
    	The text format is <a href="https://www.markdownguide.org/basic-syntax/" target=new>MARKDOWN</a>; Syntax:<br>
    	<br>
    	<ul>
    	#  Header 1  <br>
    	## Header 2  <br>
    	**BOLD** text <br>
    	&nbsp;&nbsp;* LIST element (two SPACES in front) <br>
    	force a LINEBREAK: put one or more SPACES at the end of the line ...<br>
    	<br>
    	Table:<br><pre>
| Versuch | Ölmenge   | Blasenbildung | 
|--------:|:---------:|:--------------|                          
|1        |1000µl   |test         |
</pre>
    	
    	</ul>
    	<br>
    	<a name="bookdocs"></a><b>Journal-Book: </b> Copy 'JournalBook'-documents in your '.profile/bookmarks' folder!
    	<br>
    	<b>Object-Links: </b> Produce an object-link in the entry-comment; example:<ul>
    	<br>
    	- db:EXP:134  &nbsp;&nbsp;&nbsp;&nbsp;(Link to Experiment ID:134)<br>
    	- db:MAC:8904  &nbsp;&nbsp;&nbsp;&nbsp;(Link to Material ID:8904)<br>
    	<?
    	echo " - ";
    	$linktext = fHelpC::link_show("single_sheet_edit.html#notesfield", "... more formatting help");	
    	echo $linktext;
    	echo "<br>";
    	echo '<br>';
    	echo '</ul>';
    	echo '<b>KEY-dictionary</b>: this dictionary is saved in a "document"-object: '.
    		'notes-field: komma separated text. Set the dictionary-document in the "'.
    		$this->tableNice.
    		'" preferences'."<br>\n";
    	
    	
    	htmlInfoBox( "", "", "close" );
    }

}