<?php
require_once 'o.JOUR_ENTRY.subs2.inc';
/**
 * javascript-source: https://idojo.co/how-to-create-a-tag-cloud-with-javascript-and-css/
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @module o.JOUR_ENTRY.histogram.inc
 */
class o_JOUR_ENTRY_histogram_XPL extends gPlugin {
	
	
	function register() {
	    $tablename='JOUR_ENTRY';
		$title		=tablename_nice2($tablename).' - keyword tag cloud';
		
		$this->infoarr			 = NULL;
		
		$this->infoarr['title']    = $title;	
		$this->infoarr['form_type']= 'list';
		$this->infoarr['obj_name'] = $tablename;

		$this->infoarr['css'] = '
		#tag-cloud {
		display: block;
    	}
    	
    	#tag-cloud li {
    	display: inline-block;
    	padding-right: 20px;
        }
        
        #tag-cloud li:nth-last-child(1) {
        padding-right: 0px;
        }
        
        #tag-cloud li a {
        text-decoration: none;
        color: grey;
        }
        
        #tag-cloud li a:hover {
        color: black;
        }
        ';
		$this->infoarr2 = array(
		    'accCheck' => array('tab'=>array('read', 'write'), 'obj'=>array() ),
		   
		);
	}
	
	private function x_js_code() {
	    ?>
	var ulTag = document.getElementById("tag-cloud")
    var minFontSize = 1
    var maxFontSize = 2
    var liTags = ""
    var weights = []

    // populating weights
    jsonData.forEach((result) => {
        numberOfArticles = result.numx
        if (!weights.includes(numberOfArticles)) {
            weights.push(numberOfArticles)
        }
    })

    // sorting contents of weights in assending order
    weights.sort((a, b) => a - b)

    // generating <li> tags and putting them in 'liTags'
    jsonData.forEach((result) => {
        title = result.title
        url = result.url
        numberOfArticles = result.numx
        m = weights.indexOf(numberOfArticles) + 1
        a = ((m - weights[0]) / (weights.length - weights[0]))
        b = (maxFontSize - minFontSize)
        c = minFontSize
        font_size = a * b + c
        liTags = liTags.concat(
            `<li>
                <a href="${url}" style="font-size: ${font_size}em;">
                    ${title} [${numberOfArticles}]
                </a>
            </li>`
        )
    })

    /* inserting generated li tags into ul */
    ulTag.innerHTML = liTags
	
	<?php
	
	}
	

	function startMain() {
		global $error;
		global $s_formState;
		$FUNCNAME='startMain';
		
		$sqlo  = &$this->_sqlo;

		
		$histo_arr = oJOUR_ENTRY_subs::keys_histogram($sqlo);
		$tag_data=array();
		foreach($histo_arr as $key=>$xnum) {
		    $tag_data[] = array(
		        "title"=> $key,
		        "url"=> "obj.jour_entry.list.php?parx[action]=search&parx[search_keys]=".urlencode($key),
		        "numx"=> $xnum
		    );
		}
		
		?>
		
	<ul id="tag-cloud">
	</ul>
	<script>
    var jsonData = <?php echo json_encode($tag_data);?>; 
    <?php
    $this->x_js_code();
    echo '</script>'."\n";
    
	}
    
	
}
