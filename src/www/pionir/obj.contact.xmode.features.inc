<?php
/**
 * contact extension
 * SESSION-vars: $_SESSION['s formState']['obj.contact.xedit'] = array('tree'=> infos )
 *    
 * @package obj.contact.xedit.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq UREQ:0000023: o.CONTACT > edit one object
 * @version $Header: trunk/src/www/pionir/obj.contact.xedit.php 59 2018-11-21 09:04:09Z $
 */
require_once ('o.S_OBJLINK.subs.inc');
require_once ("f.objview.inc");
require_once ("f.data_dir.inc");
require_once ("o.PROJ.subs.inc");


function adressprint ( $texter ) {
	echo "$texter<br>";
}

/**
 * overview over sub contacts
 */
class oContactSubOver {
	
	function __construct(&$sqlo, $id) {
		
		$this->contactid = $id;
		
		$this->typeArr = array(
				'company'	=>'ic.contact.company.png',
				'organization' =>'ic.contact.org.png',
				'person' 	=>'ic.contact.person.png',
				'uni'		=>'ic.contact.uni.png'
		);
		
		$treeCommand = $_REQUEST['tabArgu']['tree'];
		if ( is_array($treeCommand) ) {
			$this->_getTreeInfo($sqlo, $treeCommand);
		}
		
		// glob_printr( $_SESSION['s formState']['obj.contact.xedit']['tree'], 'TREE');
		
	}
	
	public function startChildren($sqlo, $editAllow) {
		$id = $this->contactid;
		echo "<img src=\"images/greenball.gif\"> <span style='color:gray;'><B>Sub contacts:</B></span> ".
				"[<a href=\"view.tmpl.php?t=CONTACT&condclean=1&tableSCond=CON_CONTACT_ID=".$id."\">".
				"Select sub-contacts</a>]";
		if ($editAllow) {
		    $proj_id = cProjSubs::getProjByObject($sqlo, 'CONTACT', $id);
		    $crea_link='glob.obj.crea_wiz.php?tablename=CONTACT&newparams[CON_CONTACT_ID]='.$this->contactid.'&proj_id='.$proj_id;
		    echo ' &nbsp;&nbsp;&nbsp;&nbsp;<a href="'.$crea_link.'" title="create sub contact"><img src="images/ic.plus.png" width=20> Create new Sub-Contact</a>';
		}
		echo "<ul>\n";
	}
	
	function endTable() {
		echo "</ul>\n";
	}
		
	function showFoto($contactid) {
		$tablename='CONTACT';
		$pfilename = datadirC::datadir_filename( $tablename, $contactid ,"jpg" );
	    if ( !file_exists($pfilename) ) {
			return;
		}
		
	    $answer = "<br />&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;".
	       "<img src=\"glob.obj.img_show.php?tablename=".$tablename."&primid=".$contactid."&extension=jpg\" height=80><br>\n";
	    return $answer;
	    
	}
	
	private function getParent(&$sqlo, $childid) {
		$sqlsel = "con_contact_id from CONTACT where contact_id=".$childid;
		$ret    = $sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$parentid = $sqlo->RowData[0];
		
		return $parentid;
	}
	
	/**
	 * show UP-Path
	 * @param  $sqlo
	 */
	function showUpPath(&$sqlo) {
		
		echo "<img src=\"images/greenball.gif\"> <span style='color:gray;'><B>Contact path:</B></span> ";
		
		$parentpath=array();
		
		$childid = $this->contactid;
		
		do {
			$parentid = $this->getParent($sqlo, $childid);
			if (!$parentid) break;
			
			if (in_array($parentid,$parentpath)) {
				echo "Circle-Reference detected!";
				break;
			}
			
			$parentpath[] = $parentid;
			
			$childid = $parentid;
			
		} while (1);
		
		if (!sizeof($parentpath)) {
			echo "<br>\n";
			return;
		}
		
		$parentpath = array_reverse($parentpath);
		reset ($parentpath);
		$komma = '';
		
		foreach( $parentpath as $dummy=>$contactid) {
			$nice = obj_nice_name ( $sqlo, 'CONTACT', $contactid ); 
			echo $komma . '<a href="edit.tmpl.php?t=CONTACT&id='.$contactid.'">'.$nice.'</a>';
			$komma = ' &gt; ';
		}
		echo $komma . '<b>THIS</b>';
		echo "<br>\n";
	}
	
	/**
	 * 
	 * @param  $sqlo
	 * @param  $contactid
	 * @return array ('num'=>, 'detail'=>array of contactids)
	 */
	private function _getChildren(&$sqlo, $contactid, $getDetails=0) {
		
	
		$sqlsel = "count(1) from CONTACT where con_contact_id=".$contactid;
		$ret    = $sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$num = $sqlo->RowData[0];
		
		if (!$getDetails or !$num) return array('num'=>$num);
		
		
	}
	
	private function _getTreeInfo(&$sqlo, $treeCommand) {
		
		
		$treeArr = &$_SESSION['s formState']['obj.contact.xedit']['tree'];
		
		foreach( $treeCommand as $subContactid=>$action) {
			if ($action=='expand') {
				// get sub contacts
				$treeArr[$subContactid]='1'; // show !
			}
			if ($action=='collapse') {
				// get sub contacts
				unset($treeArr[$subContactid]); // delete show-flag
			}
		}
		
		
	}
	
	/**
	 * from SQL-data transform to html-table array
	 * @param $sqlarr : all SQL-fields of CONTACT
	 * @return $dataarr
	 */
	function sql2HtmlArr(&$sqlarr) {
		
		$cont_id 		 = $sqlarr["CONTACT_ID"];
		$cont_name 		 = $sqlarr["NAME"];
		$cont_person     = $sqlarr["CONTACT_PERSON"];
		$type            = $sqlarr["TYPE"];
		
		$type_icon = "arrow.but.gif";
		if ($type!=NULL) {
			if ($this->typeArr[$type]!=NULL) $type_icon = $this->typeArr[$type];
		}
		$type_icon_tag = '<img src="images/'.$type_icon.'" border=0 hspace=5>';
		
		$dataarr=array();
		
		$dataarr[0] =		'<a href="edit.tmpl.php?t=CONTACT&id='.$cont_id.'" title="'.$type.'" >' .
				$type_icon_tag . "</a>";
		$dataarr[1] = "<b>".$cont_name."</b>";
		
		$tmpdata=NULL;
		if ($cont_person!="" and $cont_person!=$cont_name) $tmpdata .= " ($cont_person)";
		if ( $sqlarr["EMAIL"]!="" ) $tmpdata .= ", ".$sqlarr["EMAIL"];
		if ( $sqlarr["TEL"]!="" )   $tmpdata .= ", ".$sqlarr["TEL"];
		if ( $sqlarr["MOBILE"]!="" ) $tmpdata .= ", ".$sqlarr["TEL"];
		if ( $sqlarr["NOTES"]!=NULL ) {
			$tmpdata .= ", ";
			$tmpdata .= strlen($sqlarr["NOTES"])>50 ? substr($sqlarr["NOTES"],0,50).' ...' : $sqlarr["NOTES"];
		}
		$tmpdata .= $this->showFoto($cont_id);
		$dataarr[2] = $tmpdata;
		
		return $dataarr;
	}
	
	/**
	 * thwe the html table
	 * @param array $contactarr
	 * @param number $toomany
	 */
	function showHtmlTable(&$sql, &$contactarr, $level, $toomany=0) {
		
		$treeArr = &$_SESSION['s formState']['obj.contact.xedit']['tree'];
		
		echo "<table cellpadding=0 cellspacing=0 border=0>\n";
		reset ($contactarr);
		$cnt=0;
		foreach( $contactarr as $contactid=>$valarr) {
		
		
			$childArr = $this->_getChildren($sql, $contactid);
		
			$spacelen= $level*15;
			$prespace= '<span style="border: 0px solid; padding-left:'.$spacelen.'px; ">';
		
			$tmptreeicon = '<img src="0.gif" width=9 height=9>';
			if ($childArr['num']) {
				if ( $treeArr[$contactid]<1 ) {
					$tmptreeicon = '<a href="edit.tmpl.php?t=CONTACT&id='.$this->contactid .'&tabArgu[tree]['.$contactid.']=expand">'.
							'<img src="images/but.plus.gif"></a>';
				} else {
					$tmptreeicon = '<a href="edit.tmpl.php?t=CONTACT&id='.$this->contactid .'&tabArgu[tree]['.$contactid.']=collapse">'.
							'<img src="images/but.minus.gif"></a>';
				}
			}
		
			echo '<tr>';
			echo '<td valign=top>'.$prespace.'</span>'.$tmptreeicon.'&nbsp;';
			echo $valarr[0].'</td>';
			echo "<td>".$valarr[1].$valarr[2];
		
			echo "</td></tr>\n";
		
			if ($treeArr[$contactid]>0) {
				echo '<tr><td colspan=2>';
				if ($level>10) {
					echo "TOO MANY LEVELS!!!";
				} else {
					$this->show( $sql, $contactid, $level+1 );
				}
				echo '</td></tr>'."\n";
			}
		
			$cnt++;
		}
		
		if ($toomany) {
			echo "<tr><td></td><td><font color=gray>... more contacts ...</font></td></tr>\n";
		}
		
		echo "</table>\n";
	}
		
	/**
	 * show the tree
	 * @param  $sql
	 * @param  $id
	 */
	function show( &$sql, $id=0, $level=0 ) {
		
		if (!$id) $id = $this->contactid;
		
		$sqlsel = "* from CONTACT where con_contact_id=".$id. " order by TYPE, UPPER(NAME)";
		$ret    = $sql->Quesel($sqlsel);
		$cnt    = 0;
		$toomany= 0;
		
		$contactarr = array();
		
		while ( $sql->ReadArray() )  {
			
			$cont_id 		 = $sql->RowData["CONTACT_ID"];
			$dataarr = $this->sql2HtmlArr($sql->RowData);
			
			$contactarr[$cont_id] = $dataarr;
			
			if ($cnt>=100) {
				$toomany=1;
				break;
			}
			
			$cnt++;
		}
		
	    $this->showHtmlTable($sql, $contactarr, $level, $toomany);
	
	}

}

class oContactCommonGui {
	function __construct($id, $editAllow) {
		$this->id = $id;
		$this->editAllow = $editAllow;
		$this->imgtag = "<img src=\"images/greenball.gif\"> <span style='color:gray;'>";
	}
	
	function showLogo() {
		
		$id        = $this->id;
		$editAllow = $this->editAllow;
		$tablename ='CONTACT';
		
		$pfilename = datadirC::datadir_filename( $tablename, $id ,"jpg" );
		if ( file_exists($pfilename) ) {
			$objectlogoExists = 1;
		}
		
		if ( $objectlogoExists OR $editAllow ) {
			echo $this->imgtag. " <b>Contact logo</b></span>";
			if ( $editAllow ) {
				echo "&nbsp;<a href=\"javascript:showField('logoedit')\"><img src=\"images/but.edit.gif\"></a>";
			}
		
			echo "<ul>\n";
			$showLogoInfo = 1;
		}
		
		if ( $objectlogoExists ) {
			echo "<img src=\"glob.obj.img_show.php?tablename=".$tablename."&primid=".$id."&extension=jpg\" hspace=\"10\"><br>\n";
			if ( $editAllow ) {
				echo " <a href=\"glob.obj.imgdel.php?tablename=".$tablename."&id=$id\"><img src=\"images/but9.del.gif\" border=0>".
						" Delete image</A><br>\n";
			}
			echo "<br>\n";
		} else {
			// echo "<font color=gray>[no image]</font><br>\n";
		}
		
		if ( $editAllow ) {
			$tmpbackurlEnc = urlencode("edit.tmpl.php?t=".$tablename."&id=".$id);
		
			echo '<div id="logoedit" style="DISPLAY: none">';
			echo "<form ENCTYPE=\"multipart/form-data\" ACTION=\"data.upload.php?tablename=".$tablename."&ext=jpg&id=".
					$id . "&backurl=".$tmpbackurlEnc. "\" METHOD=POST>";
			?>
				<INPUT TYPE="submit" VALUE="Upload image">
					<INPUT NAME="userfile" TYPE="file" accept="*.*" >
					<INPUT TYPE="hidden" name="MAX_FILE_SIZE" value="100000"> 
				<br>
				(<I>JPG-format, Max file size: 50 kbytes.)</I>
				</form>
				</div>
				<?
		
		   }
		   if ( $showLogoInfo ) echo "</ul>";
	}
	
	public function showRelationEdit(&$sqlo) {
		$id        = $this->id;
		$editAllow = $this->editAllow;
		$table = 'CONTACT';
		
		if ($editAllow) {
			echo $this->imgtag. " <b>Contact relations - add</b></span> ";
			echo "&nbsp;<a href=\"javascript:showField('relaedit')\"><img src=\"images/but.edit.gif\"></a>";
			echo "<ul>";
			
			echo '<div id="relaedit" style="DISPLAY: none">';
			echo "<form ACTION=\"p.php?mod=DEF/oCONTACT.addrel\" METHOD=POST>\n";
			?>
				<INPUT TYPE="submit" VALUE="Add related contact">
				<INPUT NAME="relid" TYPE="text" size=7 value=""> <i>ID of contact</i>
				<INPUT TYPE="hidden" name="id" value="<?php echo $id;?>"> 
				</form>
				</div>
				<?
					
			echo "</ul>";
		}
		
		
    }
    
    public function showRelations(&$sqlo) {

		$id        = $this->id;
		
		$contSubLib = new oContactSubOver($sqlo, $id);

		$linkSub    = new oS_OBJLINK_subs();
		$table      = 'CONTACT';
		
		//$objLinkLib = new fObjViewC();
		
		$objcollect = array();
		
		$objects = $linkSub->getChiObjByTabOrKey($sqlo, $table, $id, $table, 'relation');
		if ( !empty($objects) ) {
			$objcollect = $objects;
		}
		
		$objects = $linkSub->getMoObjsbyTabKey($sqlo, $table, $id, $table, 'relation');
		if ( !empty($objects) ) {
			$objcollect = array_merge($objcollect, $objects);
		}
		
		if ( !empty($objcollect) ) {

			echo $this->imgtag. " <b>Contact relations</b></span> <ul>";
			
			reset ($objcollect);
			$htmlSuperArr=array();
			foreach( $objcollect as $childContact) {

				$sqlsel = "* from CONTACT where contact_id=".$childContact;
				$sqlo->Quesel($sqlsel);
				$sqlo->ReadArray();
				$sqlarr = $sqlo->RowData;
				
				
				$htmlSubArr = $contSubLib->sql2HtmlArr($sqlarr);
				$htmlSuperArr[$childContact] = $htmlSubArr;
			}
			
			$contSubLib->showHtmlTable($sqlo, $htmlSuperArr, 0);
			
			echo '</ul>'."\n";
		}
		
		
	}
}


/**
 * URl-parameters:
 * - $_REQUEST['tabArgu']['tree'][contactid]= 'expand', 'collapse'
 */
class obj_edit_ext extends gObj_edit_feat_ABS {
    
    function rightWindow( &$sqlo, &$sqlo2) {
        $id = $this->id;
        $o_rights = $this->o_rights;
        $viewmode = $this->viewmode;
	
    	//$notesPure 	   = $argu["NOTES"];
    	//$CCT_ACCESS_ID = $argu['CCT_ACCESS_ID'];
    	
    	$editAllow = $o_rights["write"];
    	if ($viewmode!= "edit" ) $editAllow = 0;
    	
    	?>
    	<script>
    	function showField(id)
    	{
    		if (document.getElementById(id).style.display == 'none')
    		{
    			document.getElementById(id).style.display = '';
    		}
    	}
    	</script>
    	<?php
    	
    	$mainLib = new oContactCommonGui($id, $editAllow);
    
    	//$imgtag = "<img src=\"images/greenball.gif\"> ";
    	echo "<ul>";
    	
    
    
    	$mainLib->showLogo();
    	$mainLib->showRelationEdit($sqlo);
       
        $contSubLib = new oContactSubOver($sqlo, $id  );
        $contSubLib->showUpPath($sqlo);
    	
    	$sqls = "select count(1) from CONTACT  where con_contact_id=".$id;
        $sqlo->query($sqls);
        $sqlo->ReadRow();
    	$contact_count = $sqlo->RowData[0];
    	
    	$child_is_started=0;
    	if ($editAllow) {
    	    $child_is_started=1;
    	    $contSubLib->startChildren($sqlo, $editAllow);
    	}
    	
    	if ($contact_count) {
    	    if (!$child_is_started) $contSubLib->startChildren($sqlo, $editAllow);
    		$contSubLib->show($sqlo);
    		$contSubLib->endTable();
        }
        
        $mainLib->showRelations($sqlo);
    	
    	echo "</ul>";
    	
    	return 0;
    }

}

