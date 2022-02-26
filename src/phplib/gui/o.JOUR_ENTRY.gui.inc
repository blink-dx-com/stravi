<?php
require_once ('o.PROJ.paths.inc');
require_once ("o.SATTACH.subs.inc");
require_once ("glob.image.inc");
require_once ('f.objview.inc');
require_once ("f.text_html.inc");
/**
 * lab journal support
 * COLUMNS: JOUR_ENTRY: JOUR_ENTRY_ID, START_PAGE, LINK_ID
 * @package o.JOUR_ENTRY.gui.inc
 * @swreq   UREQ:0001068: o.JOUR_ENTRY > labor journal : HOME 
 * @swreq   UREQ:0000691: db.JOUR_ENTRY > code support for KEY and JOUR_H_EXP
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class oEXPlabjourC {
    
    /**
     * 
     * @var array 
     *   "searchstr"    : searchstring
	     "search_keys   : string only keys AND connected
     */
    private $search_arr=array();

    function __construct() {
    	
    	
    	$this->parx		   = NULL;
    	$this->imgInline   = 0; 
    	$this->wrapwidth   = 80;
    	$this->tablename   = "JOUR_ENTRY";
    	$this->tablenice   = "lab journal entry";
    	$this->colorblue     ="#CCDDEE";
    	$this->HeadBackColor = "#F8EFF8";
    	$this->projPathObj = new oPROJpathC ();
    	$this->satObj      = new cSattachSubs();
    	
    	// for security, because this is a new table (year:2009)
    	$this->expAttachHas = glob_table_exists('JOUR_H_EXP');
    
    }
    
    function setPrefs() {
    	$this->parx = unserialize($_SESSION["userGlob"]["o.EXP.labjourList"]);
    	if ( $this->parx["imgInline"]>0 ) $this->setImgInline(1);
    }
    
    function setParx($parx) {
    	$this->parx = $parx;
    	if ( $this->parx["imgInline"]>0 ) $this->setImgInline(1);
    }
    
    function set_search_arr($search_arr) {
        $this->search_arr = $search_arr;
    }
    
    function getKeyBookID() {
    	return ($this->parx['KEY.docid']);
    }
    
    function setImgInline( $flag ) {
    	$this->imgInline = $flag;
    }
    
    /**
     * search first match of string in text
     * @param string $text
     * @param string $searchstr
     */
    private static function txt_search_format( $text, $searchstr ) {
        
        $OUT_LENGTH_MAX=200;
        $searchstr  = trim($searchstr);
        $search_len = strlen($searchstr);
        
        if (!$search_len) {
            $out = htmlspecialchars(substr($text, 0, $OUT_LENGTH_MAX));
            return $out;
        }
        
        $pos = stripos($text, $searchstr);
        if ($pos===false) {
            // nothing found
            $out = htmlspecialchars(substr($text, 0, $OUT_LENGTH_MAX));
            return $out;
        }
        
        $pos_start = intval($pos-$OUT_LENGTH_MAX*0.5);
        if($pos_start<0) $pos_start=0;
        $MAX_POS = $pos_start + $OUT_LENGTH_MAX;
        
        $PRE_len = $pos-$pos_start;
        $out = htmlspecialchars(substr($text,$pos_start, $PRE_len));
        
        $out .= '<b>'.htmlspecialchars(substr($text,$pos,$search_len)).'</b>';
        
        $POS_NEXT = $pos+$search_len;
        $NEXT_LEN = $MAX_POS - $POS_NEXT;
        $out .= htmlspecialchars(substr($text,$POS_NEXT, $NEXT_LEN));
        
        // echo "DDD: $searchstr| 0:".$pos_start." (len:$PRE_len) 1:".$pos. " (len:$search_len) <br>";
        
        return $out;
    }
    
    /**
     * show all attachments
     * @param object $sql
     * @param int $id
     */
    function ShowAttach( &$sql,$id ) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
    	$tablename = $this->tablename;
    	$tmp_table = "SATTACH";
    	$sql->query("SELECT COUNT(1) FROM ".$tmp_table." WHERE TABLE_NAME='".$tablename."' AND OBJ_ID=".$id); // calculate number of values
    	$sql->ReadRow();
    	$cnt = $sql->RowData[0];
    	if (!$cnt) { // do not show ???
    	    //echo '<span class="yGgray">None.</span>';
    	    //echo "<br>\n";
    		return;
    	} 
    	
    	
    	
    	//$icon      = "images/icon.".$tmp_table.".gif";
    	//$tmpout    = "";
    	$dummy_arr = NULL;
    	$maxoridim = array(350, 250);
    	$sql->query("SELECT NAME, REL_ID, MIME_TYPE, NOTES, XDATE FROM ".$tmp_table." WHERE TABLE_NAME='".$tablename."' AND ".
    	    " OBJ_ID=".$sql->addQuotes($id). " order by REL_ID"); 
    	while ($sql->ReadRow()) {
    	    
    	    $i_name = $sql->RowData[0];
    		$relid  = $sql->RowData[1];
    		$mime   = $sql->RowData[2];
    		$notes  = $sql->RowData[3];
    		$upload_date = $sql->RowData[4];
    		
    		echo "<a href=\"obj.sattach.down.php?t=".$this->tablename."&id=".$id."&rel_id=".$relid.
    		   "\" class=xgray><img src=\"images/icon.LINK.gif\" border=0> ". htmlspecialchars($i_name)."</a>";
    		if (!$this->imgInline) {
    		    if ($notes!=NULL) {
    		        if(strlen($notes)>100) {
    		            $notes_out = htmlspecialchars(substr($notes,0,100).' ...');
    		        } else {
    		            $notes_out = htmlspecialchars($notes);
    		        }
    		        echo ' '.$notes_out;
    		    }
    		}
    		echo "<br>\n";
    		
    		if ($this->imgInline AND substr($mime,0,6)=="image/" ) {
    		    echo '<div class="container" style="display: flex; flex-grow: 3;">';
    		    echo '  <div style="width: 50%;">';
    			$img_serv_path = $this->satObj->getDocumentPath($this->tablename, $id, $relid);
    			$upload_date=str_replace(':', '_', $upload_date); // create cache file, containing the upload time ...
    			$cache_file="SATTACH.".$id.".".$relid. $upload_date;
    			
    			imgVarioShow($img_serv_path, $cache_file, $dummy_arr, $maxoridim);
    			echo '  </div>'."\n";
    			echo '  <div style="flex-grow: 1;">';
    			echo htmlspecialchars($notes);
    			echo '  </div>'."\n";
    			echo '</div>'."\n";
    			// $tmpout  .= "<img src=\"obj.sattach.down.php?t=".$this->tablename."&id=".$id."&rel_id=".$sql->RowData[1]."\" style=\"max-width:400px\"><br>\n";
    		}
    	}
    	// echo '<p />'."\n";
    	return;
    }
    
    /**
     * show keys
     */
    function _shKeys($keyval) {
        echo '<img src="images/ic.colKey.gif" TITLE="keys"> '. htmlspecialchars($keyval)."\n";
    	// echo	'<br>'."\n";
    }
    
    function _shRow($icon, $iconTitle, $text) {
    	echo '<img src="images/'.$icon.'" TITLE="'.$iconTitle.'"> '. $text .'<br>'."\n";
    }
    
    function _getExpNum(&$sqlo, $objid) {
    	if ( !$this->expAttachHas ) return; 
    	$sqlsel = 'count(1) from JOUR_H_EXP where JOUR_ENTRY_ID='.$objid;
    	$sqlo->Quesel($sqlsel);
    	$sqlo->ReadRow();
    	$cntobj = $sqlo->RowData[0];
    	return ($cntobj);
    }
    
    function getProjInfo( &$sql, $JOUR_ENTRY_ID ) {
    	
    	$infox3 = " ";
    	
    	list($projidx, $projexists) = $this->projPathObj->searchProject ( $sql, $JOUR_ENTRY_ID, $this->tablename );
    	if ($projidx) {
    		$infox3 .= "<img src=\"images/ic.proj_calm.gif\"> " . $this->projPathObj->showPathSlim($sql, $projidx, "edit.tmpl.php?t=PROJ&id=")."<br>\n";
    	} else {
    		//
    	}
    	return $infox3;
    }
    
    function _getDocInfo( &$sql, $docid) {
    	if ( !$docid or !is_numeric($docid) ) return;
    	
    	$sqls = "select LINK_ID, Name from LINK where LINK_ID=".$docid;
    	$sql->query($sqls);
    	$sql->ReadRow();
    	$linkname = $sql->RowData[1];
    	return ($linkname);
    }
    
    /**
     * get device-IDs from S_OBJLINK
     * @param  $sqlo
     * @return array of device ID
     */
    function getDeviceInfo(&$sqlo, $JOUR_ENTRY_ID) {
    	$objLinkLib = new oS_OBJLINK_subs();
    	$childArr   = $objLinkLib->getChiObjByChiTab($sqlo, 'JOUR_ENTRY', $JOUR_ENTRY_ID, 'CHIP_READER');
    	return $childArr;
    }
    
    function setCss() {
    	echo "<style type=\"text/css\">\n";
    	echo ".xwrap {  font-family:  monospace;  } \n"; // Courier,
    	echo "</style>\n";
    }
    
    /**
     * 
     * @param object $sql
     * @param array $thisData  expects: x.JOUR_ENTRY_ID, x.NAME, x.EXTRA_OBJ_ID, x.LINK_ID, x.START_PAGE_ID, 
    				   			x.CCT_ACCESS_ID, x.NOTES, x.EXEC_DATE EXEC_DATE, x.KEYS
     * @param array $option   "info" => [""], 
    				  		    "onePage" -- the output is inside the SINGLE experiment page, show more links
     */
    function oneEntryOut( &$sql, &$thisData, $option = NULL ) {
    	
    	$objid      = $thisData["JOUR_ENTRY_ID"];
    	$accinfo    = access_data_getai( $sql, $thisData["CCT_ACCESS_ID"], 4 );
    	
    	$projinfo   = $this->getProjInfo( $sql, $objid );
    	$docid 	    = $thisData["LINK_ID"];
    
    	$docinfo  =  $this->_getDocInfo($sql, $docid);
    	if ($docinfo=="")  $docinfo = $docid;
    	else {
    		$docinfo = $docinfo;
    		if ($option["info"] =="onePage" ) {
    			$docinfo = "<a href=\"edit.tmpl.php?t=LINK&id=".$docid."\">".$docinfo."</a>";
    		}
    	}
    	$expnum = $this->_getExpNum($sql, $objid);
    	
    	$EXEC_DATE = $thisData["EXEC_DATE"]; // $accinfo["crea_date"]
    	
    	echo "<tr valign=top bgcolor=#EFEFEF>";
    	echo "<td>";
    	
    	// echo "<img src=\"0.gif\" height=2 width=1><br>";
    	$tmpExpName = "<a href=\"edit.tmpl.php?t=".$this->tablename."&id=".$objid."&xmode=labview\"><img src=\"images/arrow.but.gif\" border=0> ".
    				$thisData["NAME"]."</a>";
    	if ($option["info"] =="onePage" ) $tmpExpName = "<img src=\"images/arrow.but.gif\" border=0> ".$thisData["NAME"]; // no Link ... 
    	echo "<b>$tmpExpName</b>";
    	
    	if ($thisData["IMPACT"]) {
    	    
    	    $impact_use = min($thisData["IMPACT"],10);
    	    $tmp_cnt = $impact_use*0.5;
    	    echo '&nbsp;&nbsp;';
    	    for ($i=0; $i<$tmp_cnt;$i++) {
    	        echo '<img src="res/img/star.yellow.svg" title="Impact: '.$impact_use.'" width="25">';
    	    }
    	}
    	echo "<br>\n"; //NEW:
    	echo "<img src=\"images/ic.cread.gif\" TITLE=\"experiment date\"> ". $EXEC_DATE.
    		  "&nbsp;&nbsp;<img src=\"images/ic.usergr2.gif\"  TITLE=\"creator\"> ".$accinfo["owner"];
    	echo " &nbsp;&nbsp;<img src=\"images/ic.booklogo.gif\" TITLE=\"Book ID:".$docid."\"> ".$docinfo;
    	if ($thisData["START_PAGE"]) {
    		 echo " <img src=\"images/ic.pagenumb.gif\" TITLE=\"Start page\"> ".$thisData["START_PAGE"];
    	}
    	if ($thisData["IMPACT"]) {
    	    echo ' &nbsp;&nbsp;<span style="color:gray;">Impact:</span> '.$thisData["IMPACT"];
    	}
    	echo "<br>\n";
    	//echo "</td></tr>";
    	//echo "<tr valign=top bgcolor=#EFEFEF><td>"; //NEW: colspan=2
    	echo $projinfo;
    	$this->ShowAttach( $sql, $objid );
    	
    	if ($expnum) $this->_shRow('ic.exp.calm.gif', 'experiments', $expnum. ' (number of experiments)');
    	if ($thisData["KEYS"]!=NULL) $this->_shKeys($thisData["KEYS"]);
    	echo "</td></tr>";
    	
    	echo "<tr><td>"; // NEW: colspan=2
    	
    	if ( $this->parx["wrap"] AND strlen($thisData["NOTES"]) ) {
            // TBD: who needs this ???
    	    // echo "<pre>";
    		$lines = explode("\n", $thisData["NOTES"]);
    		$new_str='';
    		foreach($lines as $line) { 
    		    $new_str  .= wordwrap($line, $this->wrapwidth, "\n");
    		}
    		f_text_html::ptxt_markdown_html($sql, $new_str);
    		// echo "</pre>";
    		
    	} else {
    		//echo "<pre>";
    	    f_text_html::ptxt_markdown_html($sql, $thisData["NOTES"]); // old: fObjViewC::notes_out()
    		//echo "</pre>";
    	}
    	
    	echo "</td>";
    	echo "</tr>\n";
    }
    
    /**
     * show one short entry with search MARKED keywords
     * @param object $sql
     * @param array $thisData
     * @param array $option
     */
    function oneEntrySlim( object &$sql, array &$thisData, $option = NULL ) {
        
        $objid      = $thisData["JOUR_ENTRY_ID"];
        
        echo "<tr valign=top bgcolor=#EFEFEF>";
        echo "<td>";
        
        // echo "<img src=\"0.gif\" height=2 width=1><br>";
        $tmpExpName = "<a href=\"edit.tmpl.php?t=".$this->tablename."&id=".$objid."&xmode=labview\"><img src=\"images/arrow.but.gif\" border=0> ".
        $thisData["NAME"]."</a>";
        if ($option["info"] =="onePage" ) $tmpExpName = "<img src=\"images/arrow.but.gif\" border=0> ".$thisData["NAME"]; // no Link ...
        echo "<b>$tmpExpName</b>";
        
        if ($thisData["IMPACT"]) {
            
            $impact_use = min($thisData["IMPACT"],10);
            $tmp_cnt = $impact_use*0.5;
            echo '&nbsp;&nbsp;';
            for ($i=0; $i<$tmp_cnt;$i++) {
                echo '<img src="res/img/star.yellow.svg" title="Impact: '.$impact_use.'" width="25">';
            }
        }
        echo "</td>";
        echo "</tr>\n";
        
        echo "<tr><td>";
        
        $text_piece_formatted = self::txt_search_format( $thisData["NOTES"], $this->search_arr['searchstr'] );
        echo $text_piece_formatted;
        // show search keywords
        echo "</td></tr>";
    }
    
    function startTable () {
    	$colorblue = $this->colorblue;
    	echo "<table cellpadding=0 cellspacing=1 border=0 bgcolor=".$colorblue." width=800><tr><td bgcolor=#FFFFFF>";
    	echo "<table cellpadding=4 cellspacing=0 border=0 width=100%>\n";
    }
    
    function stopTable () {
    	echo "</table>\n";
    	echo "</td></tr></table>\n";
    }
    
    /**
     * LABBOOK : show related JOUR_ENTRYs
    	give LINK_ID and search for experiments
     * @param object $sql
     * @param object $linkid
     */
    static function labBookTrack (object &$sql, int $linkid) { 
    	$tablename='JOUR_ENTRY';
    	
    	$MAX_SHOW=30;
    	
    	$ordercond = " order by x.JOUR_ENTRY_ID desc";
    	$sqlFrom   = "from ".$tablename. " x";
    	$sqlsWhere = "x.LINK_ID=".$linkid;
    	
    
    	$sqlsCount = "select count(1) ".$sqlFrom ." where ". $sqlsWhere;
    	$sql->query($sqlsCount);
    	$sql->ReadRow();
    	$countexp = $sql->RowData[0];
    	echo '<span style="color:gray;">Contains LabJournal-entries</span>:';	
    	
    	if ($countexp>0) {
    		$condExp_url= urlencode($sqlsWhere);
    		echo "<a href=\"view.tmpl.php?t=".$tablename."&condclean=1&tableSCond=".$condExp_url."\">";
    		echo " <B>$countexp</B></a>";			
    		echo "<blockquote>\n";
    		
    		$sqls 	   = "select x.JOUR_ENTRY_ID, x.name ".$sqlFrom.
    		  " where ".$sqlsWhere.$ordercond;
    		$sql->query($sqls);
    		$cnt= 0;
    		while ( $sql->ReadRow() ) {
    		    if ($cnt > $MAX_SHOW) {
    				echo "... more ...<br>\n";
    				break;
    			}
    			
    			$tmp_id   = $sql->RowData[0];
    			$tmp_name = $sql->RowData[1];
    			echo "<img src=\"images/icon.JOUR_ENTRY.gif\" border=0> <a href=\"edit.tmpl.php?t=".$tablename."&id=$tmp_id\">$tmp_name</a><br>\n";
    			
    			$cnt++;
    		}
    		echo "</blockquote>\n";
    		
    	} else echo " <font color=gray>none</font> <br>\n";
    	
    }

}
