<?php
function obj_selection() {

    $argu=array();	
    $argu[] = array("url"=>"p.php?mod=DEF/o.MORDER.li_calendar",  "txt"=>"Calendar");
    $argu[] = array("url"=>"p.php?mod=DEF/o.MORDER.li_calendar2", "txt"=>"Gantt-Chart");
    $menu = array('func'=>$argu );
    
	return ($menu);
}