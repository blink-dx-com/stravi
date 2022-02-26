<?php


/**
 * produce a relational graph, function start activates the drawing
 * @package gGraphRelWork.inc
 * @author marco_a
 * @param $filename: name of the graph
 * @param $datax: string of relations (node to node), seperated by \n, nodes are seperated by \t
 * @param $parx: graphType (default g), relationType (default obj2obj)
 * 
 */
class gGraphRelWork {
	
	var $data; // array[] = array(mother, child)
	var $relationType; /* 
		'group'  => mother and child have different name spaces
		'obj2obj'=> mother and child are in same name space
		*/

	/**
	 * 
	 * @param string $filename
	 * @param string $datax
	 * A \t B -- can contain \n newlines
	 * B \t Z
	 * B \t B
	 *
	 * @param array $parx
	 *   'relation' : relation type: group, obj2obj
	 *   'graphType' : 
	 *       [obj2obj]
	 *       g
	 *       neato
	 *   'obj_features' array of [NAME]=array(features like href)
	 *   'out_type' : ['png'], 'svg'
	 */
function __construct($filename, &$datax, $parx) {
	
	if ($parx['graphType']==NULL) $parx['graphType']='obj2obj';
	
	$this->datax=$datax;
	$this->parx = $parx;
	$this->relationType = $this->parx['relation'];
	$this->filename = $filename;
	$this->objectArr=NULL;
	$this->childArr =NULL;
	$this->recText=NULL;
	$this->graphText=NULL;
	$this->graphid = intval(0);
	$this->obj_features=array();
	
	if (!empty($parx['obj_features'])) {
	    $this->obj_features = &$parx['obj_features'];
	}
	$this->startSeq();
}

function startSeq() {
	// rankdir = graph ["LR" ]
	$graphType= $this->parx['graphType'];
	
	$this->chars=array();
	
	if ($graphType=='neato') {
	    $test_str='
            graph G  {
                 layout=neato
                 overlap=false
                 size="30,30"
            node [
                fontsize = "10"
                shape = "record"
                color=orange
                ];
            edge [
                ];
    	'."\n"; 
	    
	    $this->chars['relation.char']='--';
	    
	} else {
	    
	    $this->chars['relation.char']='->';
    	// rankdir ="LR",
    	$test_str='
    	digraph '.$graphType.' {
    	graph [	
    		
    		];
    	node [
    		fontsize = "10"
    		shape = "ellipse"
    		color=blue
    		];
    	edge [
    	];'."\n"; 
	}
	$this->outstr= $test_str;
	$this->endStr='}';
}

private function obj_full_name2intern($objectName) {
    $tmp=explode("\n",$objectName);
    if(sizeof($tmp)>1) {
        $obj_id   = $tmp[0];
        $nicename = $tmp[0]."\\n".$tmp[1]; // insert graphviz LINEFEED
    } else {
        $obj_id   = $objectName;
        $nicename = $objectName;
    }
    
    return array('id'=>$obj_id, 'nice'=>$nicename);
}

/**
 * set a new object
 * @return -
 * @param string $objectName : can contain real LIENFEEDs
 *    e.g. "h1 \n  comment"
 * @param $shapeOpt default: record, else shape of object
 */
function setObject($objectName, $ShapeOpt = NULL) {
	
    $tmp = $this->obj_full_name2intern($objectName);
    $obj_id   = $tmp['id'];
    $nicename = $tmp['nice'];
    
		
	//if ($oldInfo!=NULL) return; // already set
	$this->objectArr[$obj_id] = array( 'ok'=>1 );
	
	$fillIt='style=filled fillcolor="#D0D0FF" ';
	if ($ShapeOpt == 'circle')$fillIt='style=filled fillcolor="#FF0000" ';
	if ($ShapeOpt == 'doublecircle')$fillIt='style=filled fillcolor="#FFFF00" ';
	
	$newNode_start = '"no_'.$obj_id.'" [ '."\n".
	   'label = "<f0>'.$nicename.' "'."\n";
	$node_opt = ''; 
	$shape_f  = 'shape = "record" '.$fillIt;
	
	if ($ShapeOpt!= NULL){
	    $shape_f = 'shape = '.$ShapeOpt . " " . $fillIt."\n".
	   	    'fixedsize = 1 width = 0.7'."\n";	
	}
	if (!empty($this->obj_features[$objectName])) {
	    if ($this->obj_features[$objectName]['href']) {
	        $href=str_replace('&','&amp;',$this->obj_features[$objectName]['href']);
	        $node_opt = 'href="'.$href.'"'."\n";
	    }
	}

	$newNode = $newNode_start .$shape_f . $node_opt . '];'."\n";
	
	$this->recText .= $newNode;
}

// set node to a child
function setGraph($objectName, $child, $type=NULL) {
    
    $tmp = $this->obj_full_name2intern($objectName);
    $mo_obj_id   = $tmp['id'];
    
    $tmp = $this->obj_full_name2intern($child);
    $ch_obj_id   = $tmp['id'];
    $ch_nice     = $tmp['nice'];
    
    
	
	if ($this->relationType=='group') {
		// set extra child object
		if ($this->childArr[$ch_obj_id]==NULL) {
		    $nicename = $ch_nice;
			$this->childArr[$ch_obj_id] = array( 'ok'=>1 );
			$newNode = 
				'"no_'.$ch_obj_id.'" [ '."\n".
				'    label = "'.$nicename.'"'."\n".
				'    ];'."\n";
			$this->recText .= $newNode;
		}
	} else {
		// same name space
		if ($ch_obj_id == 'flow end')$this->setObject($child, 'circle');
		else $this->setObject($child);
	}
	
	$start='f0';
	$oneArrow = '"no_'.$mo_obj_id.'":'.$start.' '.$this->chars['relation.char'].' "no_'. $ch_obj_id.'" [id = '.$this->graphid.'];'."\n";
	$this->graphText .= $oneArrow;
	$this->graphid++;
	
	
}

function graph_create($sqlo) {
    $filename=$this->filename;
    $allout = $this->outstr ."\n". $this->recText . "\n". $this->graphText."\n".$this->endStr."\n";
    
    if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
        echo "<B>DEBUG:</B> <pre> ".htmlSpecialchars($allout)."</pre><br>\n";
    }
    // echo $allout."<br>\n";
    
    $sqls   = "select VALUE from GLOBALS where NAME='exe.graphviz'";
    $sqlo->query($sqls);
    $sqlo->ReadRow();
    $GRAPHVIZ_EXE = $sqlo->RowData[0];
    
    if ( ! is_executable($GRAPHVIZ_EXE) ) {
        echo 'ERROR:  executable "graph" not found.';
        return -1;
    }
    
    $out_type='png';
    if ($this->parx['out_type']!=NULL)  $out_type=$this->parx['out_type'];
    $img_file_short = "graph.".$filename. ".".$out_type;
    $this->tmp_graph_img = $_SESSION['globals']['http_cache_path'] ."/".$img_file_short;
    $this->tmp_graph_img_full = $_SESSION['s_sessVars']['httpCacheDir']."/".$img_file_short;
    
    $cmd = $GRAPHVIZ_EXE . " -T".$out_type." -o" .$this->tmp_graph_img. " ";
    // $cmd2 = $GRAPHVIZ_EXE . " -Tismap -o" .$tmp_graph_map. " ";
    // echo $cmd."<br>\n";
    
    $fp = popen ($cmd, "w");
    $retval = fputs ( $fp, $allout );
    $retval = pclose ($fp); 
}


function _outGraph(&$sqlo) {
    $this->graph_create($sqlo);
    $num=rand();
    echo "<img src=\"".$this->tmp_graph_img."?rando=".$num."\">";
}

function get_graph_filename_full() {
    return $this->tmp_graph_img_full;
}

/**
 * transform string to data array
 * @param object $sqlo
 * @param string $datax ... can also be an array of arrays ....
 *    STRING: "A\tB \n A1\tB1"
 *    ARRAY: array of 
 *      array("A", "B") 
 * @return array of array
 */
private function analyse_inp_data(&$datax) {
	$startpos=0;
	$dataarr=NULL;
	
	if (is_array($datax)) {
	    
	    $dataarr=$datax;
	    
	} else {
	
    	while ($newpos = strpos($datax,"\n",$startpos)) {
    		$oneArr=$this->_getOneLine($datax, $startpos, $newpos-$startpos);
    		$dataarr[]= $oneArr;
    		$startpos = $newpos+1;
    	}
    	$oneArr=$this->_getOneLine($datax, $startpos, strlen($datax)-$startpos);
    	$dataarr[]= $oneArr;
	}
	return $dataarr;
}

function _getOneLine(&$datax, $startpos, $length) {
	$oneLine=substr($datax,$startpos,$length);
	
	$oneLineArr=explode("\t",$oneLine);
	$result=array(trim($oneLineArr[0]),trim($oneLineArr[1]) );
	return $result;
}

function start_calc() {
    $datax=&$this->datax;
    $data = $this->analyse_inp_data($datax);
    
    $maxloop = sizeof($data);
    
    
    $cnt=0;
    while ($cnt<$maxloop) {
        
        $oneAssoc = $data[$cnt];
        $objectName=$oneAssoc[0];
        $child = $oneAssoc[1];
        if ($objectName == 'flow start') $this->setObject($objectName, 'circle');
        else $this->setObject($objectName);
        $this->setGraph( $objectName, $child );
        $cnt++;
    }
}

// run all 
function start(&$sqlo) {
    $this->start_calc();
    $this->_outGraph($sqlo);
}
	

}