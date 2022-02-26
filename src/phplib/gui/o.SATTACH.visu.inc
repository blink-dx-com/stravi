<?php
require_once ('f.msgboxes.inc');
require_once ('gui/o.SATTACH.imgshow.inc');

/**
 * visualization of attachment list and othe
 * @package o.SATTACH.visu.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/phplib/gui/o.SATTACH.visu.inc 59 2018-11-21 09:04:09Z $
 */
class oSATTACH_visu {
	
	function set_object($tablename, $objid) {
		$this->tablename = $tablename;
		$this->objid = $objid;
	}
	
	/**
	 * show attachment list
	 * @param object $sqlo
	 * @param array $options
	 *  'filter' : -- extra SQL-filter : f.sql_sel2.inc::sql_filter_STRUCT
	 *     [ {'col': , 'con': 'val' } ]
	 *  'img.thumbnail' : 0,1 -- if is image: show thumbnail
	 * 	'MAX_SHOW'
	 *  'show_count' show header line with "elements: XXX"
	 *  'show_count_add_text' : extra text after "elements: XXX"
	 * @return string $html_str
	 */
	function show_attachments(&$sqlo, $options=array() ) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
		$MAX_SHOW  = 20; // max shown attachments
		if ($options['MAX_SHOW']>0) $MAX_SHOW  = $options['MAX_SHOW'];
		
		if (!$this->objid) {
			sys_error_my( 'Class not initialized.' );
		}
		
		$id = $this->objid;
		$tablename = $this->tablename;
		
		$tmp_table = "SATTACH";
		$sql_main = "FROM ".$tmp_table." WHERE TABLE_NAME='".$tablename."' AND OBJ_ID=".$sqlo->addQuotes($id);
		
		if (!empty($options['filter']) and is_array($options['filter']) ) {
		    
		    $filter_sql='';
		    $tmp_and='';
		    foreach ($options['filter'] as $row) {
		        
		        if (!is_array($row)) { 
		            echo "<br>";
		            cMsgbox::showBox("error", "Input Error: Bad Filter-Format. (oSATTACH_visu)"); 
		            echo "<br>";
		            return;
		        }
		        
		        $col_code = $row['col'];
		        $val = $row['val'];
		        
		        $cond='=';
		        if ($row['con']) {
		            $cond=' '.$row['con'].' ';
		        }
		        $filter_sql .= $tmp_and.'('.$col_code . $cond . $sqlo->addQuotes($val).')';
		        $tmp_and= ' AND ';
		    }
		    $sql_main .= ' AND '.$filter_sql;
		}
		
		$sqlo->Quesel("COUNT(1) ". $sql_main ); // calculate number of values
		$sqlo->ReadRow();
		$cnt = $sqlo->RowData[0];
	
		$icon      = "images/icon.".$tmp_table.".gif";
		$tmpout    = '';
		
		if ($options['show_count']>0) {
			$tmpout    .= '<a href="obj.sattach.view.php?t='.$tablename.'&id='.$id.'">';
			$tmpout    .= 'elements: <B>'.$cnt.'</B></A>';
		}
		if ($options['show_count_add_text']!=NULL) {
			$tmpout    .= $options['show_count_add_text'];
		}
		if ($options['show_count']>0) {
			$tmpout    .= "<br>\n";
		}
		
		$MIME_IMG='image/';
		$MIME_IMG_len = strlen($MIME_IMG);
		
		$sqlo->Quesel("NAME, REL_ID, MIME_TYPE ".$sql_main. " order by REL_ID"); 
		$shCnt=0;
		$tmp_br = '';
		while ($sqlo->ReadRow()) {
			// show HTML-Download-link to attachment
			if ($shCnt>=$MAX_SHOW) {
				$tmpout  .= "<br>\n&nbsp;... more ...";
				break;
			}
			
			$att_name = $sqlo->RowData[0];
			$rel_id   = $sqlo->RowData[1];
			$mime     = $sqlo->RowData[2];
			
			$tmpout  .= $tmp_br . "\n".'<a href="obj.sattach.down.php?t='.$tablename.'&id='.$id.'&rel_id='.$rel_id.'">'.
			 			'<img src="images/icon.LINK.gif" border=0> '. htmlspecialchars($att_name).'</a>';
			if ($options['img.thumbnail']) {
			    
			    $mime_sub = substr($mime,0, $MIME_IMG_len);
			    if ( $_SESSION["userGlob"]["g.debugLevel"]>1 ) {
			        debugOut('(114) mime_sub:'.$mime_sub, $FUNCNAME);
			    }
			    
			    if ( $mime_sub == $MIME_IMG ) {
			        $attachLib = new oSATTACH_imgshow($tablename, $id);
		            $htmlTmp = $attachLib->getImgTag($rel_id);
		            $tmpout .= "<br>\n".$htmlTmp;
			    }
			}
			$tmp_br = '<br>'."\n";
			$shCnt++;
		}
	
		echo $tmpout;
	}
	
}
