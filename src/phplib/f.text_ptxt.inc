<?php
require_once 'f.text.inc';
/**
 * PTXT analysis (Partisan-Text)
 * @package f.text_ptxt.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class f_text_ptxt {
    
    
    /**
     * analyse PTXT : get objects, if exist
     * - the result removes DOUBLETTES
     * @param object $sqlo
     * @param string $txt
     * @return array of array('t'=>$tmp_tab, 'id'=>$tmp_id);
     */
    static function get_objects( &$sqlo, &$txt ) {
        
        $obj_cache = array();
        
        $START_TAG_LEN = strlen('db:');
        $pos_end = 0;
        $len_html_text = strlen ($txt);
        
        if (!$len_html_text) {
            return array(); // no data
        }

        $found=1;  // initial found val
        //echo  'DDD_START: '.substr($txt, 0)."\n";
        
        while ( $found ) {
            
            $found      = 0;
            $pos        = $pos_end;
            $pos_end    = strlen($txt)-1; // default
            
            // echo  substr($txt, $pos_old, $pos-$pos_old); // print text before ...
            
            do {
                // pos_end, str_url (mit http, db)
                // search for the NEXT pattern ...
                
                $pos_start = strpos($txt, "db:",    $pos);
                if ($pos_start===FALSE) {
                    // nothing found ...
                    break;
                }
                $minpos = $pos_start;
                $found = 1;
 
                $full_word   = f_text::find_db_obj($txt, $minpos);
                if ( $_SESSION["userGlob"]["g.debugLevel"]>1 ) {
                    echo "DEBUG:get_objects() POS:$pos_start, OBJ:$full_word|  string_short:".substr($txt, $minpos, 12)."| <br>";
                }
                
                if ($full_word==NULL) {
                    $pos_end  = $minpos + $START_TAG_LEN;
                    break;
                }
                $word_len  = strlen($full_word);
                $pos_end  = $minpos + $word_len;
                
                //echo "DDD word:|$full_word| pos_end:$pos_end<br>";
                //echo "DDD_START minpos:$minpos;pos_end:$pos_end full_word: ".$full_word."|<br>";
                $word_len  = strlen($full_word);
                
                $tab_code = "";
                $tmp_id   = "";
                
                $tab_pos =strpos($full_word, ":");  // start search
                $tab_pos2=strpos($full_word, ":" , $tab_pos+1 );
                if ($tab_pos>0 && $tab_pos2>$tab_pos) {
                    $tab_code = substr($full_word, $tab_pos+1, $tab_pos2-$tab_pos-1);
                }
                // find the ID ...
                $id_pos = $tab_pos2;
                $id_separator_len=1;
                if ($id_pos>0) {
                    $tmp_id = substr($full_word, $id_pos+$id_separator_len, $word_len-$id_pos-$id_separator_len);
                }
                
                if ($tab_code==NULL || !is_numeric($tmp_id) ) {
                    break;
                }

                // exists ?
                $tmp_tab   = globTablebyShort($tab_code) ;
                if ($tmp_tab==NULL) break; 
                
                if(!gObject_exists ($sqlo, $tmp_tab, $tmp_id) ) {
                    break;
                }
                
                $tmp_obj = array('t'=>$tmp_tab, 'id'=>$tmp_id);
                if (!in_array($tmp_obj, $obj_cache)) {
                    $obj_cache[] = $tmp_obj;
                }
                
                
                

            } while (0);
            
            
            $pos_end = $pos_end+1;
            if( $pos_end>=$len_html_text) {
                break;
            }

        }

        return $obj_cache;
    }
    
  
}