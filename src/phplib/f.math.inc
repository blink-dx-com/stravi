<?php

/**
 * math, statistics lib
 * @package f.math.inc
 * @swreq   UREQ:0000019: global need
 * @author  Stefan Wicklein, Steffen Kube
 */
class fMathC {

	static function standard_deviation ( &$a )
	{
	  //variable and initializations
	  if (count($a)<=1) return 0;
		
	  $the_standard_deviation = 0.0;
	  $the_variance = 0.0;
	  $the_mean = 0.0;
	  $the_array_sum = array_sum($a); //sum the elements
	  $number_elements = count($a); //count the number of elements
	
	  //calculate the mean
	  $the_mean = $the_array_sum / $number_elements;
	
	  //calculate the variance
	  for ($i = 0; $i < $number_elements; $i++)
	  {
	    //sum the array
	    $the_variance = $the_variance + ($a[$i] - $the_mean) * ($a[$i] - $the_mean);
	  }
	
	  $the_variance = $the_variance / ($number_elements-1);
	
	  //calculate the standard deviation
	  $the_standard_deviation = pow( $the_variance, 0.5);
	
	  //return $the_standard_deviation
	  return $the_standard_deviation;
	}
	
	/**
	 * Calculate 3 Sigma 
	 * @param array $a
	 * @return number
	 */
	static function sigma3( &$a )
	{
	    //variable and initializations
	    if (count($a)<=1) return 0;
	    
	    $stddev = self::standard_deviation ( $a );
	    $sigma3 = 3*$stddev;
	   
	    return $sigma3;
	}

	//berechnet den Mittelwert der Eintraege eines (eindimensionales) arrays
	static function mean($input){
		$summe=0.0;
		$n=0;
	 	      
	    foreach( $input as $val){
	    	$n++;
	    	$summe+=$val;
	    }
	    if ($n>0)
	    	return ($summe/$n);
	    else
	    	return NULL; 		
	}

	//berechnet die (korrigierte) Standardabweichung der Eintraege eines (eindimensionales) arrays
	function sd($input){
		$summe=0.0;
		$mw=$this->mean($input);
		$n=0;
	 	reset ($input);	        
	    foreach( $input as $key=>$val){
	    	$n++;
	    	$summe+=pow($val-$mw,2);
	    }
	    if ($n>1) 
	    	return sqrt($summe/($n-1));
	    else
	    	return NULL;  		
	}
	
	//berechnet den (korrigierten) CV der Eintraege eines (eindimensionales) arrays
	function cv($input){
		$s=$this->sd($input);
		$mw=$this->mean($input);
		if ($mw != 0) return $s/$mw;
	}
	
	//Anzahl der Elemente kleiner $limit
	function count_lower($input, $limit){
		$n=0;
	 	reset ($input);	        
	    foreach( $input as $key=>$val){
	    	if ($val<$limit) $n++;
	    }
	    return $n;
	}
	
	//Anzahl der Elemente kleinergleich $limit
	function count_lowerequal($input, $limit){
		$n=0;
	 	reset ($input);	        
	    foreach( $input as $key=>$val){
	    	if ($val<=$limit) $n++;
	    }
	    return $n;
	}

	//Anzahl der Elemente groesser $limit
	function count_greater($input, $limit){
		$n=0;
	 	reset ($input);	        
	    foreach( $input as $key=>$val){
	    	if ($val>$limit) $n++;
	    }
	    return $n;
	}
	
	//Anzahl der Elemente groessergleich $limit
	function count_greateequal($input, $limit){
		$n=0;
	 	reset ($input);	        
	    foreach( $input as $key=>$val){
	    	if ($val>=$limit) $n++;
	    }
	    return $n;
	}
	
	/**
	 * berechnet den Median eines Arrays mit numerischen values
	 * bei gerader Anzahl von Elementen wird der Mittelwert der "beiden mittleren Werte" genommen
	 * Achtung: Wird Input-Array als Referenz uebergeben, dann wird es sortiert  
	 * @param unknown_type $input
	 * @return unknown_type 
	 */
	function median($input){
		$n=count($input);
		if ($n==0) return NULL;
		sort($input); 
		//Achtung: Jetzt ist $input ein Array mit Schluesseln von 0 ... n-1, n=Anzahl der Eintraege		
		if ($n%2==0){
			//gerade Anzahl von Eintraegen Bsp: [0,1,2,3,4,5] n=6, obere=n/2=3, untere=n/2-1=2
			$index_oberemitte=(int)($n/2);
			$index_untermitte=(int)($n/2-1);
			return 0.5*($input[$index_oberemitte]+$input[$index_untermitte]);
		}
		else{
			//ungerade Anzahl von Eintragen, Bsp: [0,1,2,3,4] n=5,n-1=4,$index_mitte=(n-1)/2=2
			$index_mitte=(int)(($n-1)/2);
			return $input[$index_mitte];
		}			
	}
	

}