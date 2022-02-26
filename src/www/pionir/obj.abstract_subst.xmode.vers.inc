<?php
require_once 'lev1/o.SUA.versman.sub.inc';
require_once 'gui/o.SUA.versman.gui.inc';

require_once ("visufuncs.inc");
require_once ("func_form.inc");
require_once ("f.objview.inc");	
require_once ('f.help.inc');

/**
 * it contains code, which will be included in edit.tmpl.php, when the rider is set to "MODE"
 * $Header: trunk/src/www/pionir/obj.abstract_subst.xmode.vers.inc 59 2018-11-21 09:04:09Z $
 * @package    edit.tmpl.php
 * @subpackage obj._default.xmode.MODE.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class oABSTRACT_SUBST_xmode_vers {
	
	

	private function _line_out($text) {
		echo $text . "<br>\n";
	}
	
	function initx(&$sqlo) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$this->versSubLib = new oSUA_versman_sub();
		$this->versType = $this->versSubLib->getSUA_type($sqlo, $this->objid);
		
		$this->master_id = NULL;
		if ($this->versType==1) $this->master_id = $this->objid;
		if ($this->versType==2) {
			$this->versParams  = $this->versSubLib->getMainParams($sqlo, $this->objid);
			$this->master_id = $this->versParams['ABSTRACT_SUBST_ID'];	
		}
		
		$this->sua_short = globTabGetSHORT('ABSTRACT_SUBST');
		$this->versGuiLib = new oSUA_gui();
	}
	
	
	
	public function showSuaDetail(&$sqlo) {
		$master = $this->master_id;
		$suaid  =  $this->objid;
		
		$colarr = array('XVERS', 'DATE_START', 'DATE_END', 'NOTES');
		
		$sua_feats = glob_elemDataGet3( $sqlo, 'ABSTRACT_SUBST', array( 'ABSTRACT_SUBST_ID'=>$suaid), 
		    	array('CCT_ACCESS_ID') );

		$versChildLib 	= new oSUA_versman_child();
		$vers_fea  		= $versChildLib->setChild($sqlo, $suaid); 
		$validFlag 		= $versChildLib->getValidFlag();
		$validFlagHtml 	= $this->versGuiLib->getVersFlagIcons($validFlag, 1);

		$objLinkLib = new fObjViewC();
		$tabobj 	= new visufuncs();
		
		$headOpt 	= array( 
		   "title" => "Version Details"
		   );
		$headx  = array ("","");
		echo '[<a href="p.php?mod=DEF/o.SUA.versman.home&id='.$suaid.'&act=editrow">Edit Parameter</a>]';
		$tabobj->table_head($headx,   $headOpt);
		
		$viewopt=array("showID"=>1);
		
		$masterHtml = '<img src="images/ic.cube.png"> '.$objLinkLib->bo_IDLink('ABSTRACT_SUBST', $master, $master);
		$tabobj->table_row ( array('VersionMaster', $masterHtml) );
		
		reset ($colarr);
		foreach( $colarr as $dummy=>$colname) {
			$colnice = columnname_nice2('SUA_VERS', $colname);
			$val = $vers_fea[$colname];
			switch ($colname) {
				case 'DATE_START':
					$val = substr($val,0,10);
					break;
				case 'DATE_END':
					$val = substr($val,0,10);
					break;
			}
			$dataArr = array( $colnice, $val );
			$tabobj->table_row ($dataArr);
			
		}
		
		$accinfo  = access_data_getai( $sqlo, $sua_feats['CCT_ACCESS_ID'] );
		$suaStatus= $accinfo['h_log_name'];
		
		if ($suaStatus!='released' and $suaStatus!='D_released' and $suaStatus!='R_released') {
			$suaStatus = '<img src="images/i13_warning.gif" title="not released"> '.$suaStatus;
		}
		$tabobj->table_row ( array($this->sua_short.' status', $suaStatus ) );
		$tabobj->table_row ( array('Version status',   $validFlagHtml ) );
	
		$tabobj->table_close();
	}

	public function showActions(&$sqlo, &$sqlo2) {
		$suaid = $this->objid;
		$actionHtml==NULL;
		
		echo '<span class="yGgrayBold">VersionManagement:</span> &nbsp;'."\n";
		
		switch ($this->versType) {
			case 0:
				// is not a version
				$intro='<span class="yGgray">[no Version]</span>';
				$actionHtml='[<a href="p.php?mod=DEF/o.SUA.versman.home&id='.$suaid.'&act=activate">Activate VersionManagement</a>]';
				break;
			case 1:
				$intro='<span class="yGgray">[<img src="images/ic.cube.png"> VersionMaster]</span>';
				$actionHtml='---';
				break;
			case 2:
				$intro='<span class="yGgray">[Version]</span>';
				$actionHtml='[<a href="p.php?mod=DEF/o.SUA.versman.new&id='.$suaid.'">New version</a>]';
				break;
		}
		
		$helpLib = new fHelpC();
		$hlpopt = array ( "object"=>"icon" );
		$helpText = $helpLib->link_show("o.ABSTRACT_SUBST.x_vers.html", "help for VersManagement", $hlpopt );
		
		$this->_line_out($intro.' &nbsp;&nbsp;&nbsp;<span style="color:gray">Actions:</span> '.$actionHtml.
			' &nbsp;&nbsp;&nbsp;'.$helpText);
		
		$this->_line_out('');
		
		if ( !$this->versType ) {
			echo "<br>\n";
			htmlInfoBox( "Info for VersionManagement", "", "open", "INFO" );
			$sua_short = globTabGetSHORT('ABSTRACT_SUBST');
			$suc_short = globTabGetSHORT('CONCRETE_SUBST');
			
			?>
			Keep in mind: after activation of this <?php echo $sua_short?>, it must be:
			<ul>
			<li>in a "released" status</li> 
			<li>has a validity period (G&uuml;ltigkeitszeitraum) </li>
			</ul>
			... to use it as template for the <?php echo $suc_short?>!
			<?php
			htmlInfoBox( "", "", "close" );
			return;
		}
		
		if ( $this->versType==1 ) {
			
			$this->versGuiLib->showVersTable($sqlo, $sqlo2, $suaid, 0);
			return;
		}
		
		if ( $this->versType==2 ) {
			$this->showSuaDetail($sqlo);
		}
	}

// PLEASE use this function; still optional 
function xmode_start( &$sqlo, &$sqlo2, $id, &$arguByKey, &$x_rights, &$extraobj_o, &$tabArgu, $editAllow) {
	global $error, $varcol;
	
	$this->objid   = $id;
	echo '<ul>'."\n";
	
	$this->initx($sqlo);
	$this->showActions($sqlo, $sqlo2);
	echo "\n".'</ul>';
}

}

