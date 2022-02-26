<?php

/**
 * - SI unit things
 * - allow translation between SI-groups
 * @package o.H_UNIT.siSubs.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class oH_UNIT_siSubs {
    
    static $_id_cache=array();
    var $sifact;
    private $PHYS_QUANT_concentration;
	
	function __construct() {
		
		// as UTF8 string !
	    $micro = chr(194).chr(181);
	    
	    $this->PHYS_QUANT_concentration='Concentration';
	    
	    // SUA QUANTITY MEAS UNITS: Volume, mass, ...
		$this->sifact = array(

		    // mass
		    $micro.'g'=>0.000001,
			'milligram'=>0.001,
			'gram'=>1,
			'kg'	=>1000,
		    
		    // concentration
		    $micro.'g/'.$micro.'l' => 0.000001,
				
		    // volume
		    $micro.'l'=>0.000001,
			'ml'=>0.001,
			'litre'=>1,
		    
		    'mol/l'=>1.0,
		    'mmol/l'=>0.001,
		    $micro.'mol/l' =>0.000001,
		    
		    'mmol'=>0.000001,
		    'nmol'=>0.000000001,
			
			'units'=>1,
		    'rxns' =>1
		 );
	}
		 
	function getFactor($unitName) {
		if ($unitName==NULL) return;
		// $unitName = str_replace( chr(194).chr(181), chr(181), $unitName); // repair UTF-8 problem!
		$siFactor = $this->sifact[$unitName];
		return ( $siFactor );
	}
	
	function getFactorByID(&$sqlo, $id) {
		if ( !$id ) return;
		
		$hunitNa  = glob_elementDataGet( $sqlo, 'H_UNIT', 'H_UNIT_ID', $id, 'NAME');
		$hunitFac = $this->getFactor($hunitNa);
		return ($hunitFac);
	}
	
	// get SQL where condition for SI factor H_UNITs
	function get_si_sql_cond($sqlo) {
	    
	    $goodKeys = array_keys($this->sifact);
	    $good_arr=array();
	    foreach( $goodKeys as $key) {
	        $good_arr[] = $sqlo->addQuotes($key);
	    }
	    
	    $where_condition = 'NAME in ('.implode(',',$good_arr).')';
	    return $where_condition;
	}
	
	// get SQL where condition for CONCENTRATION H_UNITs
	function get_concentration_sql_cond($sqlo) {
	    $where_condition = 'NOTES like '.$sqlo->addQuotes($this->PHYS_QUANT_concentration);
	    return $where_condition;
	}
	
	/**
	 * get NAME of H_UNIT by ABSTRACT_SUBST_ID
	 * @param object $sqlo
	 * @param int $sua_id
	 * @return string $hunit_name - can be NULL
	 */
	function getUnitNameFromSUA(&$sqlo, $sua_id) {
		if (!$sua_id) {
			 sys_error_my( 'input param "sua_id" missing.' );
		}
		$sqlsel = "h.NAME from ABSTRACT_SUBST s join H_UNIT h on s.H_UNIT_ID=h.H_UNIT_ID".
				" where s.ABSTRACT_SUBST_ID=".$sua_id;
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$hunit_name = $sqlo->RowData[0];
		return $hunit_name;
	}
	
	/**
	 * get ID of H_UNIT
	 * @param object $sqlo
     * @param string $hunit_name -
	 * @return string $hunit_id
	 */
	static function name2id(&$sqlo, $name) {
	    
	    $sqlsel = "H_UNIT_ID from H_UNIT where NAME=".$sqlo->addQuotes($name);
	    $sqlo->Quesel($sqlsel);
	    $sqlo->ReadRow();
	    $hunit_id = $sqlo->RowData[0];
	    return $hunit_id;
	}
	
	/**
	 * get NAME by ID, with CACHE
	 * @param object $sqlo
	 * @param int $id - can be NULL
	 * @return string $name
	 */
	static function id2name(&$sqlo, $id) {
	    
	    if (!$id) return '';
	    
	    if (self::$_id_cache[$id]) {
	        return self::$_id_cache[$id];
	    }
	    
	    $sqlsel = "NAME from H_UNIT where H_UNIT_ID =".$id;
	    $sqlo->Quesel($sqlsel);
	    $sqlo->ReadRow();
	    $name = $sqlo->RowData[0];
	    self::$_id_cache[$id] = $name;
	    
	    return $name;
	}
}
