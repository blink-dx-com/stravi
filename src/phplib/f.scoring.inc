<?php

/**
 * scoring lib
 * @package f.scoring.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class fScoring {

    function __construct($MAX_SAVE_SCORES) {
        $this->score_vals=array();
        $this->score_index=array();
        $this->MAX_SAVE_SCORES=$MAX_SAVE_SCORES;
    }
    
    /**
     * save score in arrays
     * @param int $score
     * @param int $pra_id
     */
    function save_score($score, $pra_id) {
        
        //$min_old = min($this->score_vals);
        if (!empty($this->score_vals)) $max_old = max($this->score_vals);
        else $max_old = 10000;
        
        $do_save=0;
        $old_len=0;
        
        if (sizeof($this->score_vals) < $this->MAX_SAVE_SCORES) {
            $do_save=1;
            $old_len=sizeof($this->score_vals);
        }
        
        if ($score<$max_old or $do_save) {
            
            $cache_vals =$this->score_vals;
            $cache_index=$this->score_index;
            $arr_len= sizeof($cache_vals);
            
            foreach($this->score_vals as $index => $oldval) {
                
                
                if ($score<$oldval) {
                    // insert ...
                    $this->score_vals[$index]=$score;
                    $this->score_index[$index]=$pra_id;
                    
                    //echo "- DDDX: $score : $oldval ; index:$index<br>";
                    
                    
                    // put ONE element in the middle
                    // pop last element
                    //    5
                    // 4,    8,   12
                    // 4, 5, 8,   12
                    $this->score_vals  = array_merge( array_slice($this->score_vals, 0, $index+1), array_slice($cache_vals , $index, $arr_len-$index) );
                    $this->score_index = array_merge( array_slice($this->score_index,0, $index+1), array_slice($cache_index, $index, $arr_len-$index) );
                    break;
                }
            }
            
            if ($do_save) {
                if($old_len==sizeof($this->score_vals)) {
                    // values are not yet saved ...
                    $this->score_vals[] =$score;
                    $this->score_index[]=$pra_id;
                }
            } else {
                array_pop($this->score_vals);
                array_pop($this->score_index);
            }
            
            
        }
        
        //echo "DDD:_save_score: score:$score pra:$pra_id<br>";
        //echo "DDD: VALS:".print_r($this->score_vals,1)."<br>";
        //echo "DDD: INDX:".print_r($this->score_index,1)."<br>";
    }
    
    function get_scores() {
        return $this->score_vals;
    }
    
    function get_objects() {
        return $this->score_index;
        
    }
}