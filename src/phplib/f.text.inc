<?php
/**
 * BASIC test work: static methods
 * @package f.text.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class f_text {
    
    /**
     * find the end of a WORD : SPACE. NEWLINE : suport for URLs, FILE-PATHS
     * TBD: must be recoded: test for complex URLs ...
     * allow LETTER, NUM-DIGITS, /, \, :, ?. #, %, 
     * point to the LAST Character
     *      x
     *  amsnd nextword
     * @param string $text
     * @param int $startpos
     * @return number (pos of LAST character)
     */
    static function find_wordend(&$text, $startpos) {
        
        $p_min=strlen($text);
        $sea_keys=array(' ', "\r", ",");
       
        foreach($sea_keys as $k) {
            $pos = strpos($text, $k , $startpos);
            if ($pos===FALSE) continue;
            $p_min = min($p_min, $pos);
            
        }
        $res_pos = $p_min - 1;
        // echo " DDX3:|CHAR:".substr($text,$res_pos,1)."  STR:".substr($text,$startpos, $res_pos-$startpos+1)."|<br>";

        return $res_pos;
    }
    
    // find database object CODE
    // @return string
    static function find_db_obj(&$text, $startpos) {
        $result='';
        $matches=NULL;
        $answer = preg_match ("/^db:[A-Z]+:[0-9]+/", substr($text,$startpos), $matches);
        //echo "XXX:INPUT:$text| START:$startpos|  CHAR:".substr($text,$startpos,1 )."<br>";
        if ($answer) {
            $result = $matches[0];
            //echo " -Result:$result|<pre>".print_r($matches,1)."</pre><br>";
        }

        return $result;
    }

}