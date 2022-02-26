<?php
require_once('gui/glob.xfunc_abs.inc');
require_once("o.EXP.subs.inc");

/**
 * show optional attributes/features of experiment in project-navigator + list view
 * @package obj.exp.xfunc.inc
 * @swreq   UREQ:0000949
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com
 * @version $Header: trunk/src/www/pionir/obj.exp.xfunc.inc 59 2018-11-21 09:04:09Z $
 */
class cEXP extends xfunc_ABS {
var $ident="EXP";

function __construct( ) {
    parent::__construct();
	$this->xcolarr[]=array ("SAMPLES"  => "samples", "rem" => "sample names");
	$this->xcolarr[]=array ("SAMPLE-ID"  => "sample-ID", "rem" => "IDs of samples");

	return 1;
}

// show following attributes in project
function inProjShowAtt(&$sql, $prim_id, $options=NULL ) {
	$tmp_optinfo = "";
	
    if ( $options["sample"] ) {  
        
        $sampleIDs = oEXP_subs::getSamples($sql, $prim_id);
		$samples   = oEXP_subs::getSampleNamesOfArr($sql, $sampleIDs);
		
		if (!is_array($samples)) return;
		$len_tmp = count( $samples);
		if ( $len_tmp ) {
			$tmp_name = current($samples);
			$output   = $tmp_name;
			if ($tmp_name=="" ) $output = "NO NAME";
			if ($len_tmp>1) $output = $output . ", ... ". ($len_tmp-1). " more" ; 
		    $tmp_optinfo .= "&nbsp; <font color=gray>S:</font>".$output;
        }
    }
    
   
	
    if ( $options["SHORTRES"]>0 ) { 
        $sql->query('SELECT SHORTRES FROM EXP WHERE exp_id = '.$prim_id);
        if ($sql->ReadRow()) {
            $SHORTRES = $sql->RowData[0];
            if ($SHORTRES!=NULL) 
                $tmp_optinfo .= "&nbsp; shortres:<B>".$SHORTRES."</B>";
        }
    } 
	 
	
	
    return $tmp_optinfo;
}

//  get extra information for list view
function xColDataGet (&$sql, $colname, $idarr )
{
	$output = NULL;

	switch ($colname) {
	    
		
		
		case "SAMPLES":
			$sampleIDs = oEXP_subs::getSamples($sql, $idarr[0]);
			$samples   = oEXP_subs::getSampleNamesOfArr($sql, $sampleIDs);
			if (!is_array($samples)) break;
			$len_tmp   = count( $samples );
			
			if ( $len_tmp > 10 ) { // max show direct
				$tmp_name = current($samples);
				$output   = $tmp_name;
				if ($tmp_name=="" ) $output = "NO NAME";
				if ($len_tmp>1) $output = $output . ", ... ". ($len_tmp-1). " more" ; 
			} else {
			    $output = implode(", ", $samples);
			}
		    break;
		    
		case "SAMPLE-ID":
		    	$sampleIDs = oEXP_subs::getSamples($sql, $idarr[0]);
	
		    	if ( count($sampleIDs) ) {
		    		$output = implode($sampleIDs,', ');
		    	}
		    	break;
		
		
	}
	
	return ( $output );
}


}

