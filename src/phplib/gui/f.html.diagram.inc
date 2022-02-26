<?php 
/**
 * support dynamic diagrams based on http://chartjs.org
 * @var ONE_KPI_DATASET_struct = array of one KPI dataset
 *   'type'  : [line], bar
 *   'label' : label
 *   'data'  : array of data (index=> value)
 *   'color' : OPTIONAL
 *   'yaxis-id' : NUMBER (OPTIONAL)
 *   'data_band' : array of min-max-databand array of DATEID => array(0=>min, 1=>max)
 *                 only works for 'type'=line
 *                 see option 'data_band_box'

 *   
	@example f.html.diagram.inc
	$xaxis=array('2017-01', '2017-02', '2017-03', '2017-04', );
	$temlib = new f_html_diagram();
	$temlib->showStart($xaxis);
	
	$dataset = array(
			'label'=>'test data',
			'yaxis-id' => 1,
			'data' =>array(2,3,4,3),
			'data_band'=>array(
					array(1.5,2.5),
					array(2.5,3.2),
					array(4.2,5),
					array(2.1,2.8),
			),
			'maxval'=>5,
			
	);
	
	$temlib->addDataSet($dataset);
	$temlib->close();
 */
class f_html_diagram {
	
	/**
	 * array of ONE_KPI_DATASET_struct
	 * @var array $all_y_data
	 */
	private $all_y_data;
	
	/**
	 * 
	 * @param array $xaxis
	 * @param array $options
	 *   'divid' => ID of DIV-element, needed, if more diagrams per page
	 *   'multi-y-axis' => 0,1
	 *   'dia-X-size'  : [auto] diagram X-len
	 *   'dia-Y-size'  : [auto] diagram Y-height
	 *   'lineSplines' :   -1, [1]
	 *   'data_band_box': [-1], 1 : show 'data_band' as box-plot
	 *   'data_band_color': COLOR-code for 'data_band'
	 *   'y-axis': -Y-Axis defs
	 *      'min': value
	 *      'max': value
	 *   'type' = [line], 'bar'
	 */
	public function showStart($xaxis, $options=NULL) {
		
		$this->color_def = array(
			array(0,0,255),
			array(0,255,0),
			array(255,0,0),
			array(255,255,0),
			array(0,100,100),
			array(100,100,0),
			array(75,0,130),
			array(255,140,0),
			array(100,0,0),
			array(0,100,0),
			array(0,255,255),
			array(0,255,255),
			array(173,255,47),
			
				);
		
		$this->options = $options;
		$this->type  = 'line';
		if ($this->options['type']!=NULL) $this->type  = $this->options['type'];
		$this->xAxis = $xaxis;
		$this->all_y_data = array();
		$this->y_axis_ids = array();
		$dia_width_height='width="700" height="350"';
		
		if ($options['dia-Y-size']>0) {
			$dia_width_height = 'width="'.$options['dia-X-size'].'" height="'.$options['dia-Y-size'].'"';
		}
		
		
		$this->divid='myChart';
		if ($this->options['divid']!=NULL) $this->divid = $this->options['divid'];
		//<div style="width: 640px; height:auto;">
		?>
		<div >
		<canvas id="<?php echo $this->divid;?>" <?php echo $dia_width_height;?>></canvas>
		</div>
		
		<script type="text/javascript" >
		<?php
	}
	
	/**
	 * get JS script
	 * relative path to www/pionir ...
	 */
	static function get_js_script() {
		return array(  
		    'res/js/Chart.bundle.js',
		    'res/js/Chart_js.Plugin.Errorbars.min.js' // https://github.com/datavisyn/chartjs-plugin-error-bars
		); 
	}
	
	/**
	 * add data set
	 * @param array ONE_KPI_DATASET_struct $dataset
	 * @param $options
	 *  
	 */
	public function addDataSet($dataset, $options=NULL) {
		
		$cnt = sizeof($this->all_y_data);
		
		if ($dataset['color']==NULL) {
			$colortuple = $this->color_def[$cnt];
			$dataset['color']='rgba('.$colortuple[0].', '.$colortuple[1].', '.$colortuple[2].', 0.75)';
		}
		
		
		
		$this->all_y_data[] = $dataset;
		
	}
	
	/**
	 * draw one line (for band)
	 * @param array $dataset
	 * @param int $minmax
	 *   0: is min
	 *   1: is max
	 */
	private function _one_line( $data, $minmax ) {
		
		$fill='false';
		if ($minmax==1) $fill="'-1'";
		
		echo ' {' ."\n";
		echo "   label: '', \n"; // no label
		if ($this->options['lineSplines']==-1) {
			echo '  lineTension: 0, '."\n";
		}
		echo "   spanGaps: true , \n";
		echo "   backgroundColor: 'rgba(200,200,200,  0.6)', \n";
		echo "   borderColor: 'rgba(200,200,200,  0.6)', \n";
		echo "   fill: ".$fill.", \n";
		echo "   data:  ".json_encode(array_values($data)) . "\n";
		echo " }, \n";
	}
	
	/**
	 * draw ONE band for one KPI
	 * @param array $data_band 
	 */
	private function draw_KPI_band($data_band) {
		
		$minmax_data=array();
		foreach($data_band as $minmax) {
			$minmax_data[0][]=$minmax[0];
			$minmax_data[1][]=$minmax[1];
		}
		
		$this->_one_line( $minmax_data[0], 0 );
		$this->_one_line( $minmax_data[1], 1 );
	}
	
	private function draw_error_bar(&$data) {
	    echo "errorBars: {\n";
	    $i=0;
	    $error_bars=array();
	    foreach($this->xAxis as $i=>$label ) {
	        $minmax = $data['data_band'][$i];
	        if (!empty($minmax)) {
	           $error_bars[]= "'".$label."'".': {plus: '.$minmax[1].', minus: '.$minmax[0].'}';
	        }
	        $i++;
	    }
	    echo implode(",\n",$error_bars) . "}\n";
	}
	
	
	

	
	public function close() {
		?>
		var chartdata = {
				labels: <?=json_encode($this->xAxis);?>,
				datasets : [
				<?php 
				
				foreach ($this->all_y_data as $dataset) {

				    if ( is_array($dataset['data_band'])  )  {
				        if ( $this->options['data_band_box']<=0 ) {
    						// draw band before the line
    						$this->draw_KPI_band($dataset['data_band']);
				        }
					}

					echo ' {' ."\n";
					if ($this->options['lineSplines']==-1) {
						echo '  lineTension: 0, '."\n";
					}
					if ($dataset['type']!=NULL) {
						echo '  type: ' ."'". $dataset['type'] ."'". ', '."\n";
					}
					echo "   label: '".$dataset['label']."', \n";
					echo "   borderColor: '".$dataset['color']."', \n";
					
					if ($dataset['type']=='line') {
						if ($dataset['pointRadius']>0) {
							echo "   pointRadius: ".$dataset['pointRadius'].", \n";
						}
						if ($dataset['pointStyle']) {
							echo "   pointStyle: '".$dataset['pointStyle']."', \n";
						}
						echo "   pointBackgroundColor: '".$dataset['color']."', \n";
						
					}
					
					if ($dataset['type']=='bar') {
						echo "   backgroundColor: '".$dataset['color']."', \n";
					}
					if ($dataset['type']!='bar') {
						echo "   fill: false, \n";
					}
					if ($dataset['spanGaps']>0) {
						echo "   spanGaps: true , \n";
					}
					
					if ($dataset['yaxis-id']) {
						echo "   yAxisID: 'y-axis-".$dataset['yaxis-id']."', \n";
						$this->y_axis_ids[$dataset['yaxis-id']] = 1;
					}
					
					echo "   data:  ".json_encode(array_values($dataset['data'])) . "\n";
					if ( is_array($dataset['data_band']) and $this->options['data_band_box']>0  )  {
					    echo ", \n";
					    $this->draw_error_bar($dataset);
					
					}
					
					echo " }, \n";
				}
					
			?>	
					]
			};

		var ctx = document.getElementById("<?php echo $this->divid;?>").getContext('2d');
		<?php 
		/*
		if ($this->options['dia-Y-size']>0) {
			echo 'ctx.height='.$this->options['dia-Y-size'].';'."\n";
		}
		*/
		?>
		var barGraph = new Chart(ctx, {
				type: '<?php  echo $this->type;?>',
				data: chartdata,
		<?php 
		echo "   options: { \n";
		echo "     maintainAspectRatio: false,\n";
		echo "     responsive: true,\n";
	   
		if ($this->options['y-axis']['max']!==NULL) {
			echo "   scales: { \n";
			echo "		yAxes: [{\n";
			echo "        ticks: {\n";
			echo "			suggestedMin: ".$this->options['y-axis']['min'].", \n";
			echo "			suggestedMax: ".$this->options['y-axis']['max']." \n";
			echo "		  }\n";
			echo "		}]\n";
			echo "	  },\n";
		}
		
		if ( sizeof($this->y_axis_ids)) {
			
			echo "   scales: { \n";
			echo "		yAxes: [\n";
			foreach($this->y_axis_ids as $oneid=>$tmpval) {
				echo '   { position: "left", '."\n";
				echo '	   "id": "y-axis-'.$oneid.'" }, '."\n";
			}
			echo "             ] \n";
			echo "           }, \n";
			
		}
		
		if ($this->options['data_band_box']>0) {
		    echo "     plugins: {
		         chartJsPluginErrorBars: {
		            width: '60%',
		            absoluteValues: true,\n";
		    if ($this->options['data_band_color']) {
		        echo "     color: '".$this->options['data_band_color']."',\n";
		    }
		    echo "  
		        }
		     },\n";
            
		}
		
		echo "          } \n";
		
		?>
			});
		</script>
		<?php 
	}
}
