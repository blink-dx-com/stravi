<?php
/**
 * - component protocol editor
 * - it contains code, which will be included in edit.tmpl.php, when the rider is set to "MODE"
 * $Header: trunk/src/www/pionir/obj.chip_reader.xmode.prot.inc 59 2018-11-21 09:04:09Z $
 * @swreq UREQ:FS-LIM08 Support device/instrument development
 * @package    edit.tmpl.php
 * @subpackage obj.chip_reader.xmode.prot.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */

class oCHIP_READER_xmode_prot {
	
function __construct( &$editLib ) {
}		

function xmode_start( &$sqlo, &$sqlo2, $id, &$arguByKey, &$x_rights, &$extraobj_o, &$tabArgu, $editAllow) {
	
	
	$tablename  = 'CHIP_READER';
	$c_proto_id  = $arguByKey['CONCRETE_PROTO_ID'];
	$a_reader_id = $arguByKey['A_CHIP_READER_ID'];
	
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
			$optRows[] = array('Delete', "<a href=\"obj.chip_reader.proto_act.php?t=CHIP_READER&obj_id=".$id.
				"&xact=delete\">[x] Delete protocol</a>");
		
			$archTxtOut .= '&nbsp;&nbsp;&nbsp;<font color=gray>Actions:</font> ';
			if ($archInfo['isOnArch']>0) {
				$archTxtOut .=  
				' is committed '.
				' | <a href="p.php?mod=DEF/o.CHIP_READER.protArch&id='.$id.'&action=new">Create new</a>';
			} else {
				$archTxtOut .=  
				'<a href="p.php?mod=DEF/o.CHIP_READER.protArch&id='.$id.'&action=commit">Commit</a>'.
				' | Create new';
			
			}
		}
		$optRows[] = array('Archive', $archTxtOut);
		
		$edFormLib = new oConcProtoEdform($sqlo, $c_proto_id);
		
		$editProtoAllow = $edFormLib->showIntro( $sqlo, $editAllow, 'Component protocol', $optRows );
		$devopt = array("backurl"=>"edit.tmpl.php?t=".$tablename."&id=".$id.'&xmode=prot');
		$edFormLib->showform( $sqlo, $editProtoAllow, $devopt );
	
	} else {
		echo '<ul>No protocol. ';
	
		if ( $editAllow ) {
			if ( $a_reader_id ) {
				$sqlsel ="ABSTRACT_PROTO_ID from a_chip_reader where a_chip_reader_id=".$a_reader_id;
				$sqlo->Quesel($sqlsel);
				$sqlo->ReadRow();
				if ( $sqlo->RowData[0] ) {
					echo "[<a href=\"obj.chip_reader.proto_act.php?t=CHIP_READER&obj_id=".
						$id."&xact=new_abstract\">New from device (abstract)</a>] \n";
				}  else {
					echo "[<font color=gray>New from device (abstract)</font>] \n";
				}
			}
				
			echo "[<a href=\"glob.obj.crea_wiz.php?tablename=CONCRETE_PROTO&table_name_conn=".
				"CHIP_READER&id_conn=$id\">New from wizard</a>] ";
		
		}
	}
	
}

}