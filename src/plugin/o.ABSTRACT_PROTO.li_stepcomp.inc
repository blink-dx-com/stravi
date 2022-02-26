<?php
/**
 * 
 *    
 * @package o.ABSTRACT_PROTO.li_stepcomp.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param $_REQUEST['go']
 * @param $_REQUEST['parx'] 
 *   
 */

class oAbsProtoOneStepCmp {
    
    
    function setProtoID($protoid) {
        $this->protoid=$protoid;
    }
    
    function getStepNrs( &$sqlo, &$stepNrArr) {
        $sqlsLoop = "* from ABSTRACT_PROTO_STEP ".
            " where  ABSTRACT_PROTO_ID=".$this->protoid. " order by STEP_NR";
        $sqlo->Quesel($sqlsLoop);
        while ( $sqlo->ReadArray() ) {
            $stepnr = $sqlo->RowData['STEP_NR'];
            $stepNrArr[$stepnr] = $sqlo->RowData;
        }
        
        foreach($stepNrArr as $stepnr => $valarr) {
            if ($valarr['H_UNIT_ID']) {
                $nice = obj_nice_name ( $sqlo, 'H_UNIT', $valarr['H_UNIT_ID'] ); 
                $stepNrArr[$stepnr]['H_UNIT_ID.nice'] = $nice;
            }
        }
    }
    
}

class oAbsProtoLiStepComp {
    
    private $stepNrArr;
    private $stepNrExtArr;
    
    function __construct($sqlAfter, $parx) {
        $this->sqlAfter = $sqlAfter;
        $this->parx=$parx;
        $this->singProtoLib = new oAbsProtoOneStepCmp();
        
        $this->show_arr=array(
            'NAME'=>'Name',
            'ABSTRACT_SUBST_ID'=>'material',
            'ABS_DEV_ID' =>'eqipment template',
            'QUANTITY'   =>'quantity',
            'H_UNIT_ID'  => 'meas unit',
            'NOTES'      => 'notes'
        );
        
    }
    
    function get_key_arr() {
        return $this->show_arr;
    }
    
    private function _getAllSteps( &$sqlo, &$sqlo2) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        // get all Steps in a MIX
        
        $this->stepNrArr = array(); // array[STEPNR] = 1;
        $this->stepNrExtArr=array();
        
        $sqlsLoop = "SELECT x.ABSTRACT_PROTO_ID, x.NAME FROM ".$this->sqlAfter;
        $sqlo2->query($sqlsLoop);
        while ( $sqlo2->ReadRow() ) {
            $protoid = $sqlo2->RowData[0];
            $this->singProtoLib->setProtoID($protoid);
            $this->singProtoLib->getStepNrs($sqlo, $this->stepNrArr);
            
        }
        
        ksort($this->stepNrArr);
        
        foreach( $this->stepNrArr as $step=>$val) {
            
            $this->stepNrExtArr[$step]=array();
            foreach($this->show_arr as $key=>$nice) {
                $has_vals = $this->_has_value_of_ST_KEY( $sqlo, $key, $step);
                if ($has_vals) {
                    $this->stepNrExtArr[$step][]=$key;
                }
            }
        }
        
        
        debugOut('stepNrArr:'.print_r($this->stepNrArr,1), $FUNCNAME, 1);
        debugOut('stepNrExtArr:'.print_r($this->stepNrExtArr,1), $FUNCNAME, 1);
        
    }
    
    
    private function _has_value_of_ST_KEY( &$sqlo, $key, $step_nr) {
        // get all Steps in a MIX
        
        $has_values = 0;
        
        
        $sqlsLoop = "1 FROM ABSTRACT_PROTO_STEP where ABSTRACT_PROTO_ID in (select x.ABSTRACT_PROTO_ID from ".$this->sqlAfter .")".
            " and STEP_NR=".$step_nr." and ".$key.' is not NULL';
        $sqlo->Quesel($sqlsLoop);
        if ($sqlo->ReadRow() ) {
            $has_values = 1;
        }
        
        return $has_values;
    }
    
    
    function _tabHeader() {
        // INPUT: $this->stepNrArr
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $keys_selected = $this->parx;
        
        $this->step_details_out = array();
        foreach( $this->stepNrArr as $step=>$val) {
            
            $step_vals_has = $this->stepNrExtArr[$step];
            $this->step_details_out[$step]=array();
            foreach($keys_selected as $key=>$val) {
                if(!$val) continue;
                if (in_array($key,$step_vals_has)) {
                    $this->step_details_out[$step][] = $key;
                }
            }
        }
        debugOut('step_details_out:'.print_r($this->step_details_out,1), $FUNCNAME, 1);
        
        
        echo "<tr bgcolor=#D0D0FF><td>Protocol / STEP_NR</td>";
        foreach( $this->stepNrArr as $step=>$val) { 
            
            $step_vals_show = $this->step_details_out[$step];
            
            $i=0;
            if (sizeof($step_vals_show)) {
                echo "<td>".$step."</td>";
                $lenx=sizeof($step_vals_show)-1;
                while ($i<$lenx) {
                    echo "<td></td>";
                    $i++;
                }
            }
        } 
        echo "</tr>\n";
        
        
        echo "<tr bgcolor=#D0D0FF><td>Protocol / Features</td>";
        foreach( $this->stepNrArr as $step=>$val) {
            
            $step_vals_show = $this->step_details_out[$step];
            
            foreach($step_vals_show as $key) {
                $nice = $this->show_arr[$key];
                echo "<td>".$nice."</td>";
            }
        } 
        echo "</tr>\n";
    }
    
    function _lineOut( $protoid, $name, $stepsOut ) {
        
        echo "<tr><td><b>$name</b> [$protoid]</td>";
        
        foreach( $this->stepNrArr as $step=>$val) {
            
            $step_vals_show = $this->step_details_out[$step];

            if (sizeof($step_vals_show)) {
                foreach($step_vals_show as $key) {
                    $nice = $stepsOut[$step][$key];
                    if ($key=='H_UNIT_ID') {
                        $nice = $stepsOut[$step]['H_UNIT_ID.nice'];
                    }
                    echo "<td>".$nice."</td>";
                }
            }
        } 
        
        
        echo "</tr>\n";
        
    }
    
    function anaOne( &$sqlo, $protoid, $name) {
        $this->singProtoLib->setProtoID($protoid);
        $steparr = array();
        $this->singProtoLib->getStepNrs( $sqlo, $steparr );
        $this->_lineOut( $protoid, $name, $steparr );
    }
    
    function doLoop(&$sqlo, &$sqlo2) {
        global $error;
        $FUNCNAME= "doLoop";

        $this->_getAllSteps($sqlo, $sqlo2);
        
        if ( !sizeof($this->stepNrArr) ) {
            $error->set( $FUNCNAME, 1, "No steps found in protocols." );
            return;
        }
        
        echo "<table width=100%>\n";
        
        $this->_tabHeader();
        
        $sqlsLoop = "SELECT x.ABSTRACT_PROTO_ID, x.NAME FROM ".$this->sqlAfter;
        $sqlo2->query($sqlsLoop);
        while ( $sqlo2->ReadRow() ) {
            $protoid = $sqlo2->RowData[0];
            $name = $sqlo2->RowData[1];
            $this->anaOne($sqlo, $protoid, $name);
        }
        
        echo "</table>";
    }
    
}


/**
 * @author steffen
 *
 */
class o_ABSTRACT_PROTO_li_stepcomp_XPL extends gPlugin {

	
	function register() {
		$tablename	= 'ABSTRACT_PROTO';
		$title		= 'Step compare';
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';		
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'list'; 
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_cnt']  = 1;

		$this->infoarr2 = array( 
			'accCheck' => array('tab'=>array('read'), 'obj'=>array() ),
			'listChkSelect'   => 1,
		    'listCheckNumMax' => 100
			);
	}
	
	function form1($parx, $show_arr) {
	    
	   
	    $initarr   = NULL;
	    $initarr["title"]       = "Parameters";
	    $initarr["submittitle"] = "Show";
	    $initarr["tabwidth"]    = "AUTO";
	    
	    $hiddenarr = NULL;
	    
	    $formobj = $this->_html->formInit($initarr, $hiddenarr);
	    
	    foreach($show_arr as $key=>$title) {
    	    $fieldx = array (
    	        "title" => $title,
    	        "name"  => $key,
    	        "object"=> "checkbox",
    	        "val"   => $parx[$key],
    	    );
    	    $formobj->fieldOut( $fieldx );
	    }
	    $formobj->close( TRUE );
	}

	function startMain() {
		global $error;
		$FUNCNAME='startMain';
		
		
		$sqlo = &$this->_sqlo;
		$sqlo2= &$this->_plug_regSql2();
		$go   = $_REQUEST['go'];
		$parx = $_REQUEST['parx'];
		
		debugOut('parx:'.print_r($parx,1), $FUNCNAME, 1);
		
		$sqlAfter = $this->listLib->getSqlAfter();
		
		$mainLib = new oAbsProtoLiStepComp($sqlAfter, $parx);
		
		$key_arr = $mainLib->get_key_arr();
		$this->form1($parx, $key_arr);
		if (!$go) {
		    return;
		}
		echo "<br>";
		
		
		
		
		$mainLib->doLoop( $sqlo, $sqlo2 );
		
	}
	
}
