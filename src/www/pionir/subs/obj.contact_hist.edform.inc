<?php

require_once ('f.prefvals.inc');
require_once ('func_form.inc');
require_once ("visufuncs.inc");
require_once ("f.objview.inc");	
require_once('f.objview.inc');


/**
 * manage edit form for CONTACT_HIST
 * @package obj.contact_hist.edform.inc
 * @swreq UREQ:
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com) 
 * @version $Header: trunk/src/www/pionir/subs/obj.contact_hist.edform.inc 59 2018-11-21 09:04:09Z $
 */
class oCONTACT_HIST_edformC {
	
	
function __construct () {
	
}





/**
 * one row
 * @param unknown_type $text
 * @param unknown_type $field
 * @param array $params 'req' : 0,1
 * @return unknown_type
 */
function _formLine($text, $field, $params=NULL) {
	
	$keyOpt = NULL;
	if ($params['req']>0) {
		$keyOpt = ' <img src="images/redstar.gif">';
	
	}
	echo "<tr valign=top>";
	echo "<td align=right><b>$text</b>".$keyOpt."&nbsp;</td>";
	echo "<td>$field";
	echo "</td></tr>\n";
}

function _formLineOpen($text) {
	echo "<tr valign=top>";
	echo "<td align=right><b>$text</b>&nbsp;</td>";
}

function _formLineClose() {
	echo "</tr>\n";
}

function _showRow($params) {
	echo "<tr valign=top>";
	echo "<td align=right><b>".$params[0]."</b>&nbsp;</td>";
	echo "<td>".$params[1];
	echo "</td></tr>\n";
}
function show_infoarrOp($key) {
	echo "<tr valign=top>";
	echo "<td align=right><b>".$key."</b>&nbsp;</td>";
}


function _addHidden( $hiddenarr ) {
	foreach( $hiddenarr as $key=>$val) {
		echo '<input type=hidden name="'.$key.'" value="'.$val.'">'."\n"; 
	}
	reset ($hiddenarr); 
}


private function _infoBoxOpen($title) {
	$topt = array("color"=> "#E0EFFF", "fcolor"=>"#404080" );
	htmlInfoBox( "&nbsp;".$title, "", "open", "INFO",  $topt );
}

private function _infoBoxClose() {
	htmlInfoBox("" , "", "close" );
}


/**
 * show edit form
 * @param  $sql
 * @param  $desturl
 * @param  $parx
 * @param  $formopt
 *   'action' : 
		['insert']
		'update' : $parx['KEY'] not allowed to change
	 'allowKEY': 0,1 needed if action==update for obj.chip_reader.lognew_li.php
 *   'DEVID' : ID of device
 *   'A_DEVID' : ID of abstract device
 * @return unknown_type
 */
function edform( &$sql, $desturl, $parx=NULL, $hiddenarr=NULL, $formopt=NULL ) {
	
	// get new possible columns
	
	$hiddenarr['tabArgu[go]']=1;
	
	if ($formopt['action'] == NULL ) $action='insert';
	else  $action = $formopt['action'];
	
	
	if ($action=='insert') {
		$title = 'Add new entry';
		$submitBut = 'Add entry';
		$textareaHeight = 5;
		
	} else {
		$hiddenarr['tabArgu[POS]']=$parx['POS'];
		$title = 'Update entry';
		$submitBut = 'Update';
		$textareaHeight = 10;
		
	}
	
	$this->_infoBoxOpen($title);
	echo "<table cellpadding=1 cellspacing=0 border=0 bgcolor=#E0EFFF>";
	
	echo "<form style=\"display:inline;\" method=\"post\" ".
			"name=\"editform\"  action=\"".$desturl."\" >\n";
	$this->_formLine("",  "<input type=submit value=\"".$submitBut."\" class=\"yButton\">") ;
	
	$rowparam=array('req'=>1);
	
	$dateText = '<input type=text name="tabArgu[EVENT_DATE]" size=18 value="'.$parx['EVENT_DATE'].'">';
	

	$text_temp = $dateText;
	if ($action!='insert') {
		$userx = $parx['DB_USER_ID'];
		if ($parx['DB_USER_ID']) {
			$userx= DB_userC::getFullname( $sql, $parx['DB_USER_ID'] );
		}
		$text_temp .= ' Last editor: '.$userx. '; Last edit date:'.$parx['MOD_DATE'];
	}
	$this->_formLine( "Event date", $text_temp, $rowparam);
	$this->_formLine( "Short description",'<input type=text name="tabArgu[SUBJECT]" size=50 value="'.$parx['SUBJECT'].'"> ', $rowparam);
	$this->_formLine( "notes", '<textarea  name="tabArgu[NOTES]" cols=60 rows='.$textareaHeight.'>'.$parx['NOTES'].'</textarea>');
	
	if ($hiddenarr!=NULL) {
		$this->_addHidden( $hiddenarr );
	}
	
	echo "</form>\n";
	
	echo "</table>\n";
	$this->_infoBoxClose();
	
	
	
	
}



/**
 * show only the log entry
 */
function viewEntry(&$sqlo, $objid, $pos) {
	$sqlsel = '* from CONTACT_HIST where CONTACT_ID='.$objid.' and POS='.$pos;
	$sqlo->Quesel($sqlsel);
	$sqlo->ReadArray();
	$params = $sqlo->RowData;
	
	$tabobj = new visufuncs();
	$objLinkLib = new fObjViewC();

	
	$NOTES_tmp = htmlspecialchars($params['NOTES']);
	$NOTES_tmp = str_replace("\n", "<br>\n", $NOTES_tmp);
	
	$this->_infoBoxOpen('Show one service log entry');
	echo "<table cellpadding=1 cellspacing=0 border=0 bgcolor=#E0EFFF>";

	$this->_showRow ( array( 'key',$params['KEY'] . '&nbsp;&nbsp;&nbsp;&nbsp;<B>Last editor:</B> '.$params['USERX']) );
	$this->_showRow ( array( 'short description',$params['NAME'] ) );
	$this->_showRow ( array( 'start time', $params['XDATE']. 
		         ' &nbsp;&nbsp;&nbsp; <b>end time:</b> '.$params['XDATE2'] ) );
	$this->_showRow ( array( 'notes', $NOTES_tmp ) );
	
	echo "</table>\n";
	$this->_infoBoxClose();
	
	
	
}

}
