<?php
/**
 * - manage QC protocol steps
 * - it contains code, which will be included in edit.tmpl.php, when the rider is set to "MODE"
 * @todo new module: obj.chip_reader.qcProt.php: new module based on p.php !!!
 * @swreq UREQ:FS-LIM08 Support device/instrument development
 * @package    edit.tmpl.php
 * @subpackage obj.chip_reader.xmode.qcprot.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
require_once ("visufuncs.inc");
require_once ("f.objview.inc");
require_once ('f.help.inc');

class ocCHIP_READER_qcProtoGui {
	function __construct(&$sqlo, $id, &$arguByKey) {
		$tablename='CONCRETE_SUBST';
		
		$abs_device_id 		  = $arguByKey['A_CHIP_READER_ID'];
		$this->con_qcproto_id = $arguByKey['QC_CPROTO_ID'];
		$this->device_id      = $id;
		$this->arguByKey      = &$arguByKey;
		$this->abs_device_id  = $abs_device_id;
		if ($abs_device_id){
			$sqlsel = 'QC_ABSPROTO_ID from A_CHIP_READER where A_CHIP_READER_ID='.$abs_device_id;
			$sqlo->Quesel($sqlsel);
			$sqlo->ReadRow();
			$this->abs_qcproto_id = $sqlo->RowData[0];
		}
	}
	
	function _infoLine($key, $val) {
		echo '<font color=gray>'.$key.':</font> '.$val.'<br />'."\n";
			
	}
	
	function getHelpText() {
		$helpLib = new fHelpC();
		$helpText = '['.fHelpC::link_show("o.CONCRETE_SUBST.qcprot.html", "help").']';
		return $helpText;
	}
	/**
	 * get planned and concrete QC protocols
	 */
	function showProtos(&$sqlo, $editAllow) {
		global $error;
		$FUNCNAME= 'showProtos';
		
		if (!$this->con_qcproto_id) {
			$helpText = $this->getHelpText();
			echo $helpText."&nbsp;&nbsp;";
		}
		
		$aProtoID = $this->abs_qcproto_id;
		if (!$aProtoID) {
			$this->_infoLine(  'QC protocol', 'No planned QC protocols found.');
			return;
		} 

		$infoHTML = NULL;
		
		if (!$this->con_qcproto_id) {
			
			if ($editAllow) {
				$infoHTML .= ' [<a href="p.php?mod=DEF/o.CHIP_READER.qcProtCrea&id='.
					$this->device_id.'"><b>create new</b></a>]&nbsp;&nbsp;';
			} else {
				$infoHTML = '<font color=gray>[create new]</font> &nbsp;&nbsp;';
			}
			$infoHTML .= '<br />planned QC protocol: '.fObjViewC::bo_display( $sqlo, "ABSTRACT_PROTO", $aProtoID ) ;
		}
		
		echo $infoHTML;

	}
}


class oCHIP_READER_xmode_qcprot {
	
function __construct( &$editLib ) {
}		

function xmode_start( &$sqlo, &$sqlo2, $id, &$arguByKey, &$x_rights, &$extraobj_o, &$tabArgu, $editAllow) {
	
	
	$tablename = 'CHIP_READER';
	$c_proto_id = $arguByKey['QC_CPROTO_ID'];
	$abs_device = $arguByKey['A_CHIP_READER_ID'];
	
	echo '<ul>'."\n";
	$QcProtlib= new ocCHIP_READER_qcProtoGui($sqlo, $id, $arguByKey);
	$QcProtlib->showProtos($sqlo, $editAllow);
	echo '</ul>'."\n";
	
	if ( $c_proto_id ) {
		
		require_once ("gui/o.CONCRETE_PROTO.edit.inc");
		require_once ("o.REA_PRO_ARCH.subs.inc");
		
		$protArchLib = new oREA_PRO_ARCH_subs();
		if ( $protArchLib->isActive() ) {
			$archInfo = $protArchLib->getProtInfo($sqlo, $id, $c_proto_id);
		} else $archInfo = array();
		
		$cpvers_out =$archInfo['cpvers'];
		if (!$archInfo['cpvers']) $cpvers_out = 'not commited';
		$archTxtOut = '<font color=gray>Version:</font> <b>'.$cpvers_out.'</b> (of this protocol)';
		$optRows = NULL;
		if ($editAllow>0) {
			
		
			$archTxtOut .= '&nbsp;&nbsp;&nbsp;<font color=gray>Actions:</font> ';
			if ($archInfo['isOnArch']>0) {
				$archTxtOut .=
				' is committed '.
				' | <a href="p.php?mod=DEF/o.CHIP_READER.protArch&id='.$id.'&action=newqc">Create new</a>';
			} else {
				$archTxtOut .=
				'<a href="p.php?mod=DEF/o.CHIP_READER.protArch&id='.$id.'&action=commitqc">Commit</a>'.
				' | Create new';
					
			}
		}
		$optRows[] = array('Archive', $archTxtOut);
		
		// -----------------------------
		
		require_once ("gui/o.CONCRETE_PROTO.edit.inc");
		
		$edFormLib = new oConcProtoEdform($sqlo, $c_proto_id);

		$helpText = $QcProtlib->getHelpText();
		

		$optarr  = array('headAdd'=>'&nbsp;'.$helpText);
		$editProtoAllow = $edFormLib->showIntro( $sqlo, $editAllow, 'QC protocol', $optRows, $optarr );
		$devopt = array("backurl"=>"edit.tmpl.php?t=".$tablename."&id=".$id.'&xmode=qcprot');
		$edFormLib->showform( $sqlo, $editProtoAllow, $devopt );
		
	} else {
		// 
	}
	
}

}