<?php
require_once 'gui/f.html.diagram.inc';

/**
 * various tests
 * @package chart_test.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   int $go
 * @param string $action
 */
class chart_test_XPL extends gPlugin {
	
	
	function register() {
		
		$title		= 'GUI tests: CHART';
		
		$this->infoarr			 = NULL;
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'tool'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['locrow']   = array( array('rootsubs/test/index.php', 'Test home') );
		
		$this->infoarr['jsFile']   = f_html_diagram::get_js_script();
		
		
		$this->infoarr2 = array( 	
			);
	}
	
	function show_img_1() {
	    
	    echo "LINE-Graph<br>";
	    
	    $xaxis=array('t1', 't2', 't3');
	    $feature_data_vals = array(2.2, 5.5, 7.7);

	    $graph_lib = new f_html_diagram();
	    $grOptions = array('divid'=>'img1', 'type'=>'line');
	    $graph_lib->showStart($xaxis, $grOptions);

        $feature_name = 'example 1';
        $dataset = array(
            'label'=>$feature_name,
            'yaxis-id' => 1,
            'data' =>$feature_data_vals, 
        );
        
        $graph_lib->addDataSet($dataset);
	    $graph_lib->close();
	}
	
	function show_img_2() {
	    
	    echo "LINE-Graph with Error-Bar<br>";
	    
	    $xaxis=array('t1', 't2', 't3');
	    $feature_data_vals = array(2.2, 5.5, 7.7);
	    
	    $graph_lib = new f_html_diagram();
	    $grOptions = array('divid'=>'img2', 'type'=>'line');
	    $graph_lib->showStart($xaxis, $grOptions);
	    
	    $feature_name = 'example 1';
	    $dataset = array(
	        'label'=>$feature_name,
	        'yaxis-id' => 1,
	        'data' =>$feature_data_vals,
 
	        'data_band'=>array(
	            array(1.5,2.5),
	            array(2.5,6.2),
	            array(4.2,7.9),
	        ),
	    );
	    
	    $graph_lib->addDataSet($dataset);
	    $graph_lib->close();
	}
	
	function show_img_3_bar() {
	    
	    echo "BAR-Graph with Error-Bar<br>";
	    
	    $xaxis=array('t1', 't2', 't3');
	    $feature_data_vals = array(2.2, 5.5, 7.7);
	    
	    $graph_lib = new f_html_diagram();
	    $grOptions = array(
	        'divid'=>'img3', 
	        'data_band_box'=>1, 
	        'data_band_color'=>'gray',
	        'type'=>'bar'
	        
	    );
	    $graph_lib->showStart($xaxis, $grOptions);
	    
	    $feature_name = 'example 1';
	    $dataset = array(
	        'label'=>$feature_name,
	        'yaxis-id' => 1,
	        'data' =>$feature_data_vals,
	        'type' =>'bar',
	        // 'color'=>'orange',
	        'data_band'=>array(
	            array(1.5,2.5),
	            array(2.5,6.2),
	            array(4.2,7.9),
	        ),
	    );
	    
	    $graph_lib->addDataSet($dataset);
	    $graph_lib->close();
	}
	
	
	function startMain() {

		
		//$sqlo = &$this->_sqlo;
		//$go   = $_REQUEST['go'];
		//$parx = $_REQUEST['parx'];
		$action = $_REQUEST['action'];
		
		echo "action: ".$action."<br><br>\n";
		
		$this->show_img_1();
		$this->show_img_2();
		$this->show_img_3_bar();
		
	}
}
