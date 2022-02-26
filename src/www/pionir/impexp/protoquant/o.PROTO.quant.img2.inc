<?php
require_once ("gui/f.html.diagram.inc");

/**
 * build a proto quant image
 * @author Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @package o.PROTO.quant.img2.inc
 * @swreq UREQ:0001578: o.PROTO > ProtoQuant : Analyse protocol step
 */
class oProtoQuant_img2C {
	
	private $imgErr;
	/**
	 * @var array of 
	 *   index => data, where index ~ SUC_ID
	 */
	private $imgQuant;
	private $imgBand; // band array for box plot, needs set_option('data_band_box'=>1);
	private $imgObj;
	
	/**
	 * 
	 * @var array
	 *   'data_band_box' => 0,1
	 */
	private $options;
	
	function __construct() {
	    $this->options=array();
		$this->imgErr = NULL;
		$this->imgQuant=array();
		$this->imgObj  =array();
		$this->imgBand =array();
		
		$this->imgx = array();
		$this->imgx["quantDetMax"] = 15;  // max number of step details
		$this->imgx["objMaxNum"]   = 100; // max number of objects
	}
	
	/**
	 * 
	 * @param string $key
	 * @param variant $val
	 */
	function set_option($key, $val) {
	    $this->options[$key] = $val;
	}
	
	/**
	 * add a data row to diagram
	 * @param array $dataarr
	 *    first column contains object ID
	 * @param int $pos_datasep
	 * @param string $nicename - name of data row
	 * @param array $databand
	 *   index=>array('min'=>, 'max'=> )
	 */
	function add_datarow($dataarr, $nicename, $data_band=NULL) {
		
		if ( sizeof($this->imgObj) > $this->imgx["objMaxNum"] ) {
			$this->imgErr = "Too many objects (max ".$this->imgx["objMaxNum"].")";
			return;
		}
		
		$pos_datasep=1;
		$this->imgQuant[] = array_slice($dataarr, $pos_datasep);  // get the quantities
		$this->imgObj[]   = array('name'=>$nicename ); // get the name
		$this->imgBand[]  = $data_band;
	}
	
	/**
	 * init image
	 * @param array $axis_info
	 *  POS => array(
	 *     "NAME"=>
	 *     "TYPE"=>
	 
	 * @param string $title
	 */
	function img_init( $axis_info, $title) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$this->title = $title;
	
		
		// get quantitiy-infos
		
		$this->quant_inf = array();
		
		do {
			if ( $this->imgErr!="" ) {
				$error->set("_imgShow", 1, $this->imgErr );
				break;
			}
			$this->quant_inf=array();
			foreach( $axis_info as $valarr) {
			    $this->quant_inf[] = array("name"=> $valarr["NAME"], "ty"=>$valarr["TYPE"] );
			}	
			$this->subtitle = "Selected protocol: ";
			
		
		} while (0);
		
// 		if ($error->Got(READONLY))  {
// 			$error->printAll();
// 			$error->reset();
// 			return;
// 		}
		
	}
	function show_img() {
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;

		$xaxis=array();
		foreach($this->imgObj as $obj_info) {
			$xaxis[]=$obj_info['name'];
		}
		
		$graph_lib = new f_html_diagram();
		$grOptions = array('type'=>'bar');
		if ($this->options['data_band_box']>0) {
		    $grOptions['data_band_box'] = 1;
		    $grOptions['data_band_color'] = 'gray';
		}
		
		$graph_lib->showStart($xaxis, $grOptions);

		
		foreach($this->quant_inf as $data_col_id=> $feature_info) {
			
			$feature_data_vals=array();
			foreach($this->imgObj as $index => $obj_info ) {
				$feature_data_vals[] = $this->imgQuant[$index][$data_col_id];
			}	
			
			$feature_name = $feature_info['name'];
			$dataset = array(
					'label'=>$feature_name,
					'yaxis-id' => 1,
					'data' =>$feature_data_vals,
					'type' =>'bar'
			);
			if ($this->options['data_band_box']>0) {
			    $tmp_arr=array();
			    
			    $i=0;
			    foreach($this->imgObj as $index => $obj_info ) {
			        $row = &$this->imgBand[$index][$data_col_id];
			        if (empty($row)) {
			            $tmp_arr[$i]=NULL;
			        } else {
    			        if (array_key_exists('min', $row)) {
    			            $tmp_arr[$i]=array($row['min'], $row['max']);
    			        } else {
    			            $tmp_arr[$i]=NULL;
    			        }
			        }
			        $i++;
			    }
			    $dataset['data_band']=$tmp_arr;

			    if ( $_SESSION["userGlob"]["g.debugLevel"]>=3 ) {
			        debugOut('(157) dataset:'.print_r($dataset,1), $FUNCNAME, 2);
			    }
			}
			
			
			$graph_lib->addDataSet($dataset);
		}

		$graph_lib->close();
	}
}
