<?
/**
 * @version $Header: trunk/src/www/pionir/obj.concrete_proto.xview.inc 59 2018-11-21 09:04:09Z $
 */
function obj_selection() {

    $argu=array();	
	$argu[] = array("url"=>"obj.concrete_proto.m_comgui.php", "txt"=>"SimpleCompare protocols");
	$argu[] = array("url"=>"p.php?mod=DEF/o.CONCRETE_PROTO.pq",   "txt"=>"SuperCompare protocols (ProtoQuant)");
	//$argu[] = array("url"=>"obj.concrete_proto.quant.php", "txt"=> "[ProtoQuant]");
	//$argu[] = array("url"=>"obj.concrete_proto.quant.php?parx[format]=csv", "txt"=> "export protocol steps");
	$argu[] = array("url"=>"obj.exp.imp_sample.php", "txt"=> "import and update protocol steps");
	$argu[] = array("url"=>"p.php?mod=DEF/o.CONCRETE_PROTO.impcrea", "txt"=> "import and create full protocols");
	
	
    $menu = array('func'=>$argu );
    
	return ($menu);
}

?>

