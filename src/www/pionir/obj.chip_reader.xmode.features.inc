<?php
require_once ("f.objview.inc");
require_once 'o.CHIP_READER.subs.inc';
require_once 'o.CHIP_READER.prodline.inc';

class obj_edit_ext extends gObj_edit_feat_ABS {
    
    function rightWindow( &$sqlo, &$sqlo2) {
        $id = $this->id;
        $argu= &$this->argu;

        echo '<ul>'."\n";
	 
        $greenball = '<img src="images/greenball.gif">';
    	$tablename='CHIP_READER';
    	//$editAllow = $o_rights["write"]; 
    	//if ($viewmode!= "edit" ) $editAllow = 0; 
    	
    
    	$a_reader_id = $argu["A_CHIP_READER_ID"];
    	$c_proto_id  = $argu["CONCRETE_PROTO_ID"];
    	
    	// is prod line?
    	$prod_lib = new o_CHIP_READER_prodline($sqlo, $id);
    	if ($prod_lib->is_prod_line($sqlo)) {
    	    echo '<img src="images/ic.factory.svg" height=40 title="is a production line">';
    	    echo "<br>";
    	}
                                           
    	if ( $c_proto_id  ) {
    		$sqls="select a.NAME from CONCRETE_PROTO c, ABSTRACT_PROTO a where c.CONCRETE_PROTO_ID=".$c_proto_id." AND  a.ABSTRACT_PROTO_ID=c.ABSTRACT_PROTO_ID";
    		$sqlo->query($sqls);
    		$sqlo->ReadRow();
    		$a_proto_name = $sqlo->RowData[0];
    	}
    	
    	
    	$tmpHeadline = $greenball . ' <span style="color:gray"><B>Component protocol:</B></span> ';
    	if ($c_proto_id) {
    		echo $tmpHeadline."'".$a_proto_name."' ";
    		echo "<br>\n";
    	}
    	
    	
    	
    	if ( $c_proto_id ) {
    		require_once ("gui/o.CONCRETE_PROTO.edit.inc");
    		
    		$editProtoAllow = 0;
    		$edFormLib = new oConcProtoEdform($sqlo, $c_proto_id);
    		$devopt    = array("backurl"=>"edit.tmpl.php?t=CHIP_READER&id=".$id);
    		$edFormLib->showform( $sqlo, $editProtoAllow, $devopt );
    	
    	}
    	
    	
    	// show thumbnail
    	require_once('gui/o.SATTACH.imgshow.inc');
    	$attachLib = new oSATTACH_imgshow();
    	$relid = $attachLib->getThumbAttach( $sqlo, $tablename, $id);
    	if ($relid) {
    		$htmlTmp = $attachLib->getImgTag($relid);
    		echo '<br>'.$greenball. '<span style="color:gray;">Thumbnail:</span><ul>'.$htmlTmp.'</ul>'."\n";
    	}
    	if (!$relid and $a_reader_id) {
    		// try to find the thumbnail of the abstract device ...
    		$relid = $attachLib->getThumbAttach( $sqlo, 'A_CHIP_READER', $a_reader_id);
    		if ($relid) {
    			$htmlTmp = $attachLib->getImgTag($relid);
    			echo '<br>'.$greenball. '<span style="color:gray;">Thumbnail: (from '. tablename_nice2('A_CHIP_READER').')</span><ul>'.$htmlTmp.'</ul>'."\n";
    		} 
    	}
    	
    	$dev_sub_lib = new oCHIP_READER_subs();
    	$dev_sub_lib->set_dev($sqlo, $id);
    	$container_arr = $dev_sub_lib->get_containers_all($sqlo);
    	if (!empty($container_arr)) {
    	    
    	    // require_once('gui/o.CONTAINER.one_show.inc');
    	    $objLinkLib = new fObjViewC();
    	    echo $greenball . ' Connected with '.tablename_nice2("CONTAINER") .'s: <ul>';
    	    
    	    foreach($container_arr as $cont_id) {
    	        echo $objLinkLib->bo_display( $sqlo, "CONTAINER", $cont_id  ).'<br>'."\n";
        	    //$cont_lib = new oCONTAINER_one_show($container_id);
        	    //$cnt = $cont_lib->count_suas($sqlo);
        	    //$cont_lib->get_CHA_Summary($sqlo, $sqlo2);
    	    }
    	    echo "</ul>\n";
    	}
    	
    	echo "\n".'</ul>';
    }

}


