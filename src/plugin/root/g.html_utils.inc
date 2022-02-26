<?php
require_once ("visufuncs.inc");

/**
 * HTML utils for root/admin
 *    
 * @package g.html_utils.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq   UREQ:0001140: g > HTML utils for root/admin 
 * @param   $_REQUEST['method']: 'urlencode' : needs $value
 * @param   $_REQUEST['value'] : needed for various methods
 *   
 */
class g_html_utils_XPL extends gPlugin {
	
	function __construct(&$sqlo) {
		parent::__construct($sqlo);
	}
	
	function register() {
		$title		= 'Admin HTML utils';
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'tool'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['locrow']= array (
			array ('home.php', 'home'),
			array ('rootsubs/rootFuncs.php', 'Administration' )
			);
		
		$this->infoarr2 = array( 
			'progressBar' =>1,
			'onlyAdmin'=>1
		);
				
		
	}
	
	function _urlencode() {

		$parx = $_REQUEST['parx'];
		$initarr=array();
		$initarr["title"]       = "Give a string to urlencode";
		$initarr["submittitle"] = "Submit";
		$initarr["tabwidth"]    = "AUTO";
		
		$hiddenarr = NULL;
		$hiddenarr['method'] = $this->method;
		$formobj = $this->_html->formInit($initarr, $hiddenarr);
		$fieldx = array ( 
			"title" => "String", 
			"name"  => "value",
			"object"=> "text",
			"val"   => $parx["value"], 
			"notes" => "the string to urlencode"
			 );
		$formobj->fieldOut( $fieldx );
		$formobj->close( TRUE );
		
		if ($this->go ) {
			
			$value= $parx['value'];
			
			$valueUrl = urlencode($value);
			$this->_html->line_out( 'original: ' .htmlspecialchars($value)    );
			$this->_html->line_out( 'urlencode: '.htmlspecialchars($valueUrl) );
		}
	}
	
	private function getKeyNice($text) {
		return '<span style="color:gray;">'.$text.'</span>: ';
	}
	
	function _str_getInfo($val) {
		$nameFLen= strlen($val);
		$info=NULL;
		
		$info .= $this->getKeyNice('VALUE:')."\n".htmlentities( htmlentities($val,NULL, 'UTF-8') )."<hr>\n";
		
		$pos=0;
		while ($pos<$nameFLen) {
		
			$charFile = substr($val,$pos,1);
			$ordnum   = ord($charFile);
			$xinf = $ordnum >127 ? ' <span style="color:red;">>&gt;x80</span>' : NULL;
			$charcodeStr = sprintf("%x", ord($charFile) );
			$info .='pos:'.str_pad( ($pos+1), 2, " ").' char:'.$charFile.' ord:'. 
				str_pad($ordnum,3," ") .' hex:'.str_pad($charcodeStr,2," ").$xinf."\n";
			
			$pos++;
			
		}
		return $info;
	}
	
	function _stringShowORD(&$sqlo) {
	
		$parx = $_REQUEST['parx'];
		$initarr=array();
		$initarr["title"]       = "Give table, ID, Attribute";
		$initarr["submittitle"] = "Submit";
		$initarr["tabwidth"]    = "AUTO";
		
		$hiddenarr = NULL;
		$hiddenarr['method'] = $this->method;
		$formobj = $this->_html->formInit($initarr, $hiddenarr);
		$fieldx = array ( 
			"title" => "Table", 
			"name"  => "table",
			"object"=> "text",
			"val"   => $parx["table"], 
			"notes" => "DB table"
			 );
		$formobj->fieldOut( $fieldx );
		$fieldx = array ( 
			"title" => "Object-ID", 
			"name"  => "id",
			"object"=> "text",
			"val"   => $parx["id"], 
			"notes" => "Object-ID"
			 );
		$formobj->fieldOut( $fieldx );
		$fieldx = array ( 
			"title" => "Column-Name", 
			"name"  => "col",
			"object"=> "text",
			"val"   => $parx["col"], 
			"notes" => "Column-name (e.g. NAME)"
			 );
		$formobj->fieldOut( $fieldx );
		
		
		$formobj->close( TRUE );
		
		if ($this->go ) {
			$parx = $_REQUEST['parx'];
			
			$table= $parx['table'];
			$id   = $parx['id'];
			$col= $parx['col'];
			if ($table==NULL or $id==NULL or $col==NULL) {
				$this->_html->line_out( 'One parameter missing.');
				return;
			}
			
			$pkname = PrimNameGet2($table);
			if ($pkname==NULL) {
				$this->_html->line_out( 'Table unknown');
				return;
			}
			$sqlsel = $col. ' from '.$table . ' where '.$pkname.'='.$id;
			$sqlo->Quesel($sqlsel);
			$sqlo->ReadRow();
			$val = $sqlo->RowData[0];
			
			$this->_html->line_out( 'table: '.$table  );
			$this->_html->line_out( 'col: '.$col  );
			$this->_html->line_out( 'id: '.$id  );
			$this->_html->line_out( 'len text: '.strlen($val)  );
			$this->_html->line_out( '<pre>'  );
			$info = $this->_str_getInfo($val);
			$this->_html->line_out(  $info );
			
		}
	}
	
	
	
	function _form0() {
	    $initarr=array();
		$initarr["title"]       = "Method selection";
		$initarr["submittitle"] = "Submit";
		$initarr["tabwidth"]    = "AUTO";
		
		$hiddenarr = NULL;
		$formobj = $this->_html->formInit($initarr, $hiddenarr);
		$fieldx = array ( 
			"namex" => TRUE, 
			"title" => "Method", 
			"name"  => "method",
			"object"=> "select",
			"val"   => $parx["value"], 
			'inits'=> $this->methodDef,
			"notes" => ""
			 );
		$formobj->fieldOut( $fieldx );
		$formobj->close( TRUE );
	}
	
	function images_show($parx) {
	    
	    $initarr=array();
	    $initarr["title"]       = "Image list";
	    $initarr["submittitle"] = "Submit";
	    $initarr["tabwidth"]    = "AUTO";
	    
	    $hiddenarr = array();
	    $hiddenarr['method']='images_show';
	    
	    $formobj = $this->_html->formInit($initarr, $hiddenarr);
	    $fieldx = array (
	        "title" => "Image list",
	        "name"  => "images",
	        "object"=> "textarea",
	        "val"   => $parx["images"],
	        "notes" => ""
	    );
	    $formobj->fieldOut( $fieldx );
	    $formobj->close( TRUE );
	    
	    
	   
	    $tabobj = new visufuncs();
	    $headOpt = array( "title" => "Images" );
	    $headx  = array ("#", "Value");
	    $tabobj->table_head($headx,   $headOpt);

	    
	    if ($parx["images"]) {
	        
	        $image_arr = explode("\n", $parx["images"]);
	        
	        foreach($image_arr as $i => $img_file) {
	            $img_file=trim($img_file);
	            if (!$img_file) continue;
	            
	            $fullpath='images/'.$img_file;
	            $icon='<img src="'.$fullpath.'"  style="max-height: 40px;" >';
	            
	            $dataArr=array($i+1, $img_file, $icon);
	            $tabobj->table_row ($dataArr);
	            
	        }
	        $tabobj->table_close();
	        
	    }
	}
	
	function svg_mtp() {
	    
	    echo "Show MTP<br>";
	    
	    $svg = '<svg width="500" height="400">'."\n";
	    
	    $MAX_X_NUM = 4;
	    $MAX_Y_NUM = 3;
	   
	    $xd=25;
	    $yd=25;
	    
	    $border_c = '#AD00C6';
	    $well_c   = '#FF6318';
	    
	    $x_start=20;
	    $y_start=20;

	    for ($xi=0;$xi<$MAX_X_NUM;$xi++) {
	        for ($yi=0;$yi<$MAX_Y_NUM;$yi++) {
	            $x=$x_start+$xi*$xd;
	            $y=$y_start+$yi*$yd;
	            
	            $svg .= '<circle cx="'.$x.'" cy="'.$y.'" r="10" stroke="'.$well_c.'" stroke-width="1" fill="'.$well_c.'" />'."\n";
	        }
	    }
	    
	    
	    $x_len = ($MAX_X_NUM)*$xd + 10;
	    $y_len = ($MAX_Y_NUM)*$yd + 10;
	    
	    $svg .= '<rect x="3" y="3" width="'.$x_len.'" height="'.$y_len.'" rx="10" ry="10"
            style="stroke:'.$border_c.';stroke-width:5;fill-opacity:0.1;stroke-opacity:0.9" />'."\n";
	    
	    $svg .= '</svg>'."\n";
	    
	    echo $svg."<br>\n";
	}
	
	function svg_other() {
	    
	    echo "Show SVG<br>";
	    
	    $show_test_frame = 0;
	    
	    $x_len = 100;
	    $y_len = 62;
	    $border_c = '#E8E355';
	    $fill_c='#A57B42';
	    $font_size='80';
	    $stroke=5;
	    
	    $svg_x=$x_len+6;
	    $svg_y=$y_len+6;
	    
	    $svg_x_center = $svg_x*0.5;
	    $svg_y_center = $svg_y*0.5;
	    $radius_max   = $svg_y*0.5-5;
	    
// 	    $svg = '<svg width="'.$svg_x.'" height="'.$svg_y.'">'."\n";
// 	    if ($show_test_frame) $svg .= '<rect x="0" y="0" width="'.$svg_x.'" height="'.$svg_y.'" style="stroke:#808080;stroke-width:1;" />'."\n";
// 	    $svg .= '<path d="M5 '.$svg_y_center.' L30 5 H'.($svg_x-$stroke).' V'.($svg_y-$stroke).' H30 Z" rx="10" ry="10" 
//            style="stroke:'.$border_c.';stroke-width:4;fill:'.$fill_c.';stroke-opacity:0.9"  />';
// 	    $svg .= '<text x="'.$svg_x*0.30.'" y="'.($svg_y*0.80).'" font-size="'.$font_size.'" font-family="Arial, Helvetica, sans-serif" style="fill:'.$border_c.';">e</text>';
// 	    $svg .= '</svg>'."\n";
// 	    echo 'EXP1: ';
// 	    echo $svg."<br>\n";
	    
	    $rx=$radius_max;
	    $svg = '<svg width="'.$svg_x.'" height="'.$svg_y.'">'."\n";
	    if ($show_test_frame) $svg .= '<rect x="0" y="0" width="'.$svg_x.'" height="'.$svg_y.'" style="stroke:#808080;stroke-width:1;" />'."\n";
	    
	    $svg .= '<path d="M'.($rx+$stroke).' ' . $y_len . ' a'.$rx.','.$rx.' 0 0,1 0,'.(-2*$rx).
	       ' H'.($svg_x-$stroke).' V'.($svg_y-$stroke).' Z" '.
           ' style="stroke:'.$border_c.';stroke-width:4;fill:'.$fill_c.';stroke-opacity:0.9"  />';   
	    $svg .= '<text x="'.$svg_x*0.30.'" y="'.($svg_y*0.80).'" font-size="'.$font_size.'" font-family="Arial, Helvetica, sans-serif" style="fill:'.$border_c.';">e</text>';
	    $svg .= '</svg>'."\n";
	    echo 'EXP: ';
	    echo $svg."<br>\n";
	    
	    // SUC
	    $radius   = $y_len*0.45;
	    $border_c = '#E8E355';
	    $fill_c   = '#FF6318';
	    $object_border_c = '#808080';
	    
	    $svg = '<svg width="'.$svg_x.'" height="'.$svg_y.'">'."\n";
	    if ($show_test_frame) $svg .= '<rect x="0" y="0" width="'.$svg_x.'" height="'.$svg_y.'" style="stroke:#808080;stroke-width:1;" />'."\n";
	    $svg .= '<circle cx="'.$svg_x_center.'" cy="'.$svg_y_center.'" r="'.$radius.'" stroke="'.$object_border_c.'" stroke-width="3" fill="'.$fill_c.'" />'."\n";
	    $svg .= '</svg>'."\n";
	    echo 'SUC: ';
	    echo $svg."<br>\n";
	    
	    // #FF9A69
	    // SUA
	    $radius   = $y_len*0.45;
	    $border_c = '#FF9A69';
	    $fill_c   = '#FFFFFF';
	    $object_border_c = '#FF9A69';
	    
	    $svg = '<svg width="'.$svg_x.'" height="'.$svg_y.'">'."\n";
	    if ($show_test_frame) $svg .= '<rect x="0" y="0" width="'.$svg_x.'" height="'.$svg_y.'" style="stroke:#808080;stroke-width:1;" />'."\n";
	    $svg .= '<circle cx="'.$svg_x_center.'" cy="'.$svg_y_center.'" r="'.$radius.'" stroke="'.$object_border_c.'" stroke-width="5" fill="'.$fill_c.'" />'."\n";
	    $svg .= '</svg>'."\n";
	    
	    echo 'SUA: ';
	    echo $svg."<br>\n";
	    
	   
	    $fill_c   = '#313163';
	    $object_border_c = '#DDDDDD'; // DDDD00
	    $rx  = intval($y_len*0.25);
	    $rx2 = intval($y_len*0.50);
	    $svg = '<svg width="'.$svg_x.'" height="'.$svg_y.'">'."\n";
	    if ($show_test_frame) $svg .= '<rect x="0" y="0" width="'.$svg_x.'" height="'.$svg_y.'" style="stroke:#808080;stroke-width:1;" />'."\n";
	    $svg .= '<circle cx="'.$svg_x_center.'" cy="'.($rx+3).'" r="'.$rx.'" stroke="'.$object_border_c.'" stroke-width="3" fill="'.$fill_c.'" />'."\n";
	    $svg .= '<path d="M'.($svg_x_center-$rx2).' ' . ($y_len+2) . ' a'.$rx2.','.$rx2.' 0 0,1 '.(2*$rx2).',0 Z" '.
	        'stroke="'.$object_border_c.'" stroke-width="3" fill="'.$fill_c.'" />';
	    $svg .= '</svg>'."\n";
	    
	    echo 'USR: ';
	    echo $svg."<br>\n";
	    
	}
	

	function startMain() {
	
		
		$this->go = $_REQUEST['go'];
		$method   = $_REQUEST['method'];
		$sqlo = &$this->_sqlo;
		
		$this->methodDef = array(
			'urlencode' => 'urlencode',
			'stringShowORD'=>'show object-attribute: char encoding-analysis',
		    'svg_mtp'=>'MTP: SVG draw',
		    'svg_other'=>'svg_other',
		    'images_show' => 'show image-icons from list'
		);
		
		$this->method=$method;
		
		if ($method==NULL) {
			$this->_form0();
			return;
		}
		
		$this->_html->line_out( 'method: '.$method );
		
		switch ($method) {
			case 'urlencode':
				$this->_urlencode();
				break;
			case 'stringShowORD':
				$this->_stringShowORD($sqlo);
				break;
			case 'svg_mtp':
			    $this->svg_mtp();
			    break;
			case 'svg_other':
			    $this->svg_other();
			    break;
			    
			case 'images_show':
			    $this->images_show($_REQUEST['parx']);
			    break;
		}
		
		$this->_html->line_out( 'ready.');
		
	}
	
}
