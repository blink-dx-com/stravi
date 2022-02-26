<?php
require_once 'o.CHIP_READER.subs.inc';

/**
 * initial update related CONTAINER.LOCATION
 * @package o.CHIP_READER.upd_loc.inc
 * @author  Steffen Kube (steffen.kube@gmx.de)
 * @param int $go
 */
class  o_CHIP_READER_upd_loc_XPL extends gPlugin {
    
    

	function register() {
		
		$title		  = 'Update ALL related CONTAINER.LOCATION';

		$this->infoarr			 = NULL;
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'list';
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = 'CHIP_READER';
		$this->infoarr['obj_cnt']  = 1;
		$this->infoarr['locrow']   = array( array('rootsubs/init/index.php', 'INIT - home') );

		$this->infoarr2 = array(
				'onlyAdmin'=>1
		);

	}
	
	function form1() {
	    
	    $initarr   = NULL;
	    $initarr["title"]       = "Update!"; // title of form
	    $initarr["submittitle"] = "Submit"; // title of submit button
	    $initarr["tabwidth"]    = "AUTO";   // table-width: AUTO
	    
	    $hiddenarr = array();
	    $formobj = $this->_html->formInit($initarr, $hiddenarr, 0);
	    $formobj->close( TRUE ); // close form, sshow submit-button
	}
	
	function startMain() {
	    global $error;
	    // $FUNCNAME='startMain';

	    
	    $go   = $_REQUEST['go'];
	    $sqlo = &$this->_sqlo;
	    $sqlo2= &$this->_plug_regSql2();
	    
	    $this->go = $go;
	    
        
        if (!$go) {
            $this->form1();
            return;
        }
        
        $dev_lib = new oCHIP_READER_subs();
        
        $sqlsel = "CHIP_READER_ID, LOCATION from CHIP_READER order by CHIP_READER_ID";
        $sqlo2->Quesel($sqlsel);
        $i=0;
        while ( $sqlo2->ReadRow() ) {

            $dev_id  = $sqlo2->RowData[0];
            $dev_loc = $sqlo2->RowData[1];
            if($dev_loc==NULL) $dev_loc=''; // force to string
            
            $dev_lib->set_dev($sqlo, $dev_id);
            $upd_cnt = $dev_lib->update_container_loc($sqlo, $dev_loc);
            
            if ($upd_cnt) echo "- STO:".$dev_id.': Updated continers: '.$upd_cnt."<br>\n";
            $i++;
        }
        echo "<br>";
        echo "Analyzed devices: ".($i+1)."<br>";
	    
	}
}