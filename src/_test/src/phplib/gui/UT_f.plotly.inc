<?php
// f.plotly.inc

class UT_f_plotly_inc extends gUnitTestSub {
	
function __construct() {
	$this->module_noPreLoad =0;  // 0,1 - do NOT preload ? can be set in constructor of class
	$this->GUI_test_flag    =1;
}

// return: 0 : not passed, 1: passed
function dotest( &$sqlo, $options ) {

    
    $jsfile= '../../../pionir/'. f_plotly::get_js_link();
    echo '<script language="JavaScript" src="'.$jsfile.'"></script>';
    
    ?>
	<div id="plotgraph" style="width:900px;height:600px;"></div>
	<?php 
		
		
		$plot_lib = new f_plotly();
		$plot_lib->set_graph_id('plotgraph');
		$one_trace = array('x'=>array(1,2,3,4), 'y'=>array(1.4,5,3, 2.2) );
		$plot_lib->add_trace($one_trace);
		$one_trace = array('x'=>array(2,3,4,5), 'y'=>array(5,5,3, 8) );
		$plot_lib->add_trace($one_trace);
		
		$plotopt = array(
		    'mode'=> 'lines',
		    'type'=> 'scatter'
		    );
		$plot_lib->show_graph($plotopt);
		
	
	
	
	return (1);
}

}
