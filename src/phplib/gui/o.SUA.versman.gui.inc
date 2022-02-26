<?php 
require_once ("visufuncs.inc");
require_once ("f.objview.inc");
require_once 'lev1/o.SUA.versman.sub.inc';
if ( file_exists($_SESSION['s_sessVars']['AppLabLibDir'].'/lablib/g.ERP.info.inc') ) 
	require_once ( $_SESSION['s_sessVars']['AppLabLibDir'].'/lablib/g.ERP.info.inc');
	
/**
 * o.ABSTRACT_SUBST > [VersMan] > gui
 *    
 * @package o.SUA.versman.gui.inc
 * @swreq   SREQ:0003324: o.ABSTRACT_SUBST > [VersMan] > Home einer Substanz 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/phplib/gui/o.SUA.versman.gui.inc 59 2018-11-21 09:04:09Z $
 */
class oSUA_gui {
	
	/**
	 * show version table
	 * @param  $sqlo
	 * @param  $sqlo2
	 * @param  $master_id
	 * @param  $suaid
	 */
	public function showVersTable(&$sqlo, &$sqlo2, $master_id, $suaid=0) {
		
		if (!$master_id) return;
		
		$sua_short = globTabGetSHORT('ABSTRACT_SUBST');
		$editallow=0;
		$o_rights = access_check($sqlo, 'ABSTRACT_SUBST', $master_id);
		if ( $o_rights["insert"]>0 ) {
			$editallow=1;
		}

		
		// get edit-link of BO + NAME + icon (object)
		$objLinkLib   = new fObjViewC();
		$versChildLib = new oSUA_versman_child();
		
		$ERP_lib_exists = 0;
		if ( class_exists('gERP_info') ) {
			$ERP_lib_exists = 1;
			$erpLib= new gERP_info();
		}
		
		
		$condurl = urlencode('x.ABSTRACT_SUBST_ID in (select CHILD_ID from SUA_VERS where ABSTRACT_SUBST_ID='.$master_id.')');
		echo ' [<a href="view.tmpl.php?t=ABSTRACT_SUBST&condclean=1&tableSCond='.$condurl.'">Select all version MAAs</a>]';
		echo "<br />\n";
		
		echo '<style type="text/css">'."\n";
		echo 'table.xVersions td:nth-of-type(2) { text-align:center }'."\n";
	    echo '</style>'."\n";
		
		
		$tabobj = new visufuncs();
		$headOpt = array( 
		   "title" => "Version overview",
		   "cssclass"=>"xVersions"
		  );
		$headx  = array ("", "Version", "".$sua_short."", $sua_short."-ID", "Vstat", $sua_short." status", 
			"created", "Version Start", "Version End", "Version-Notes", "ERP-MNR");
		$tabobj->table_head($headx,   $headOpt);
		
		$sqlsel = "* from SUA_VERS where ABSTRACT_SUBST_ID=".$master_id.' order by XVERS';
		$sqlo2->Quesel($sqlsel);
		while ( $sqlo2->ReadArray() ) {
			
			$verfeat = $sqlo2->RowData;
		    $childid = $verfeat['CHILD_ID'];
		    $vers    = $verfeat['XVERS'];
		    $MNR     = NULL;
		    
		    $sua_feats = glob_elemDataGet3( $sqlo, 'ABSTRACT_SUBST', array( 'ABSTRACT_SUBST_ID'=>$childid), 
		    	array('CCT_ACCESS_ID') );
		    $accinfo  = access_data_getai( $sqlo, $sua_feats['CCT_ACCESS_ID'] );
		    $tmphtml  = $objLinkLib->bo_display( $sqlo, 'ABSTRACT_SUBST', $childid );
		    $editHtml = NULL;
		    if ($editallow) $editHtml = '<a href="p.php?mod=DEF/o.SUA.versman.home&id='.$childid.
		    	'&act=editrow&"><img src="images/but.edit.gif" title="edit"></a>';
		    if ($ERP_lib_exists)
		    	$MNR   = $erpLib->getMatnrByPartisan($sqlo, 'ABSTRACT_SUBST', $childid );
		    
		    
			$vers_fea  		= $versChildLib->setChild($sqlo, $childid); 
			$validFlag 		= $versChildLib->getValidFlag();
		    $validFlagHtml  = $this->getVersFlagIcons($validFlag);
		    
		    $notesTmp = $verfeat['NOTES'];
		    if (strlen($verfeat['NOTES'])>30) {
		    	$notesTmp = substr($verfeat['NOTES'],0,30).' ...';
		    }
		    $notesTmp = htmlentities($notesTmp, 0, 'UTF-8');
		    
		    $dataArr = array($editHtml, $vers, $tmphtml, 
		    	$childid, 
		    	$validFlagHtml,
		    	$accinfo['h_log_name'], 
		    	'<span class="yGgray">'.substr($accinfo['crea_date'],0,10).'</span>', 
		    	substr($verfeat['DATE_START'],0,10), 
		    	substr($verfeat['DATE_END'],0,10), 
		    	$notesTmp, $MNR
		    	);
		    
		    $shopt=NULL;
		    if ($suaid==$childid) {
		    	$shopt=array('bgcolor'=>'#D0D0FF');
		    }
		    $tabobj->table_row ($dataArr, $shopt);
		}
	
		
	
		$tabobj->table_close();
	}
	
	/**
	 * get icon
	 * @param  $validFlag
	 * @param  int $outtype 0,1 with full text?
	 */
	public function getVersFlagIcons($validFlag, $outtype=0) {
		
		$outarr = NULL;
		$info=NULL;
		switch ($validFlag) {
			
			case 1:
				$outarr = 'i13_inwork.gif';
				$info='not yet active';
				break;
			case 2:
				$outarr ='i13_green.gif';
				$info='active';
				break;
			case 3:
				$outarr ='i13_gray.gif';
				$info='out of date range';
				break;
			default:
				$outarr = 'i13_warning.gif';
				$info='no date set';
				break;
		}
		
		if (!$outtype) 
			$outstr = '<img src="images/'.$outarr.'" title="'.$info.'">';
		else 
			$outstr = '<img src="images/'.$outarr.'"> '.$info;
		
		return $outstr;
		
	}
}
