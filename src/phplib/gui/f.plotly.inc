<?php
/**
 *  plotly wrapper
 *  see docu: https://plot.ly/javascript/line-and-scatter/
 *  @package f.plotly.inc
 */
class f_plotly {
    
    private $graphid;
    private $dataarr;
    
    static function get_js_link() {
        return 'res/js/plotly-latest.min.js';
    }
    
    function set_graph_id($graphid) {
        $this->graphid = $graphid;
    }
    
    /**
     * 
     * @param array $one_trace
     *   'x'=> array( numbers)
     *   'y'=> array( numbers)
     *   
     *   OPTIONAL:
     *    'type': 'scatter', 'line', ...
     */
    function add_trace( &$one_trace ) {
        $this->dataarr[] = $one_trace;
    }
    
    /**
     * show the graph
     * @param array $options
     *   'type' : [scatter]
     *   'mode' : 'lines'
     *   'RAW'  : RAW plotly options
     *   'textbox': text of extra text box
     *   'title': title
     *   'xaxis.title'
     *   'yaxis.title'
     */
    function show_graph( $options=array() ) {
        
        if ($this->graphid==NULL) {
            throw new Exception('graphid not set.');
        }
        
        ?>
        <script>
        GRAPH_OBJ = document.getElementById('<?php echo $this->graphid;?>');

        var data = [
        <?php 
        // build the data cube 
        
        foreach($this->dataarr as $one_trace) {
            
            $mode_loop = $options['mode'];
            if ($one_trace['mode']!=NULL) {
                $mode_loop = $one_trace['mode'];
            }
            $type_loop = $options['type'];
            if ($one_trace['type']!=NULL) {
                $type_loop = $one_trace['type'];
            }
            
            echo '  {'."\n";
            echo '   x: [';
            foreach($one_trace['x'] as $x) {
                echo $x.', ';
            }
            
            echo     '  ],'."\n";
            echo '   y: [';
            foreach($one_trace['y'] as $y) {
                echo $y.', ';
            }
            echo     '  ],'."\n";
            
            
            echo "  mode: '".$mode_loop."',"."\n";
            echo "  type: '".$type_loop."',"."\n";
            
            if ($one_trace['RAW']!=NULL) {
                echo $one_trace['RAW']."\n";
            }
             
            echo '  }, '."\n\n";
        }
        
        ?>
		];
        
        /* layout */
        /* margin: { t: 0 }, */
        var layout = {
        	title: {
        		text:'<?php echo $options['title'];?>',
        		xref: 'paper',
        		x: 0.05,
        	},
            
        <?php 
        if ($options['textbox']) {
            echo "
            annotations: [
            {
              x: 0.05,
              y: 1.1,
              xref: 'paper',
              yref: 'paper',
              text: '".$options['textbox']."',
              showarrow: false
            }
          ],
            ";
        }
        ?>
            xaxis: {
                title: '<?php echo $options['xaxis.title'];?>',
                },
            yaxis: {
                title: '<?php echo $options['yaxis.title'];?>',
                },
        
        };
        
        /* {showSendToCloud:true}*/
        
        Plotly.plot( GRAPH_OBJ, data, layout  );
        </script>
        <?php 
    }
}