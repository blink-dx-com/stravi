<?php
require_once 'f.text.inc';

/**
 * TEXT work for HTML output
 * @package f.text_html.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class f_text_html {

    /**
     * - parse NOTES-fields of objects
     * - direct OUTPUT to console, if $output_var=0
     * - recognize KEY-TEXT and interprete it
     *
     * <pre>
     KEY-TEXT:
         db:...  link to DB-object, example db:EXP:9394
         http://... link to a website
         https://... link to a website
     
     DEPRECATED: [[file://...]] link to a file server
     DEPRECATED: HTML-tags: [[B]] [[/B]] [[UL]] [[/UL]] [[U]]
     Code remark: should not call CLASS-sub-functions !!!
     </pre>
     @param string $html_txt -- do NOT change!
     @param $showPreFormatted : 0,1 : if 1: add HTML-PRE-tags
     @param int $output_var 0: to console, 1: as return string
     
     @todo FUTURE: RECODING-request: use code of ptxt_markdown !!!

     */
    static function notes_out( &$sql, &$html_txt, $showPreFormatted=0, $output_var=0 ) {
 
        $out='';
        $pos_end=0;
        $len_html_text   = strlen ($html_txt);

        if (!$len_html_text) {
            return; // no data
        }
        
        if ($showPreFormatted) {
            $tmp = '<pre style="white-space: pre-wrap;">'; // wrap, if line is too long for display
            if ($output_var) $out.=$tmp;
            else echo $tmp;
        }
        
        $found=1;  // initial found val
        $pos_end_fix = strlen($html_txt)-1;
        
        while ( $found ) {
            
            $found      = 0;
            $pos        = $pos_end;
            $pattern_ok = 0;
            $pos_end    = $pos_end_fix; // default
            
            //echo "DDD1: pos:$pos<br>";
            
            do {
                // pos_end, str_url (mit http, db)
                // search for the NEXT pattern ...
                
                if ($pos>=$pos_end_fix) break; // end reached
                
                $posarr=array();
                $posarr[0] = strpos($html_txt, "http://",  $pos);
                $posarr[1] = strpos($html_txt, "https://", $pos);
                $posarr[2] = strpos($html_txt, "db:",    $pos);
                // find next ...
                $minind = -1;
                $minpos = $len_html_text-1;
                foreach($posarr as $ind=>$end_pos) {
                    if ($end_pos===FALSE) {
                        unset($posarr[$ind]);
                    } else {
                        if ($end_pos<$minpos) {
                            $minind = $ind;
                            $minpos = $end_pos;
                        }
                    }
                }
                
                if ($minind<0) {
                    // nothing found ...
                    break;
                }
                
                $tmp = substr($html_txt, $pos, $minpos-$pos); // print text before pattern ...
                if ($output_var) $out.=$tmp;
                else echo $tmp;
                
                //echo "DDD word:|$full_word| pos_end:$pos_end<br>";
                //echo "DDD_START minpos:$minpos;pos_end:$pos_end full_word: ".$full_word."|<br>";
                
                if ( $minind==2 ) { // db:EXP:456
                    
                    $found = 1;
                    
                    $full_word   = f_text::find_db_obj($html_txt, $minpos);
                    if ($full_word==NULL) {
                        break;
                    }
                    $word_len = strlen($full_word);
                    $pos_end  = $minpos + $word_len;

                    $tab_code = "";
                    $tmp_id   = "";
                    
                    $tab_pos =strpos($full_word, ":");  // start search
                    $tab_pos2=strpos($full_word, ":" , $tab_pos+1 );
                    if ($tab_pos>0 && $tab_pos2>$tab_pos) {
                        $tab_code = substr($full_word, $tab_pos+1, $tab_pos2-$tab_pos-1);
                    }
                    
                    $id_pos = $tab_pos2;
                    $id_separator_len=1;
                    if ($id_pos>0) {
                        $tmp_id = substr($full_word, $id_pos+$id_separator_len, $word_len-$id_pos-$id_separator_len);
                    }
                    
                    if ($tab_code=="" || !is_numeric($tmp_id) ) {
                        $pattern_ok = 0;
                        break;
                    }

                    $nice_name='';
                    if ($tab_code) $tmp_tab   = globTablebyShort($tab_code) ;
                    if ($tmp_tab)  $nice_name = tablename_nice2($tmp_tab);
                    if ($nice_name!="") {
                        $icon     = file_exists("images/icon.".$tmp_tab.".gif") ?
                        "images/icon.".$tmp_tab.".gif":
                        "images/icon.UNKNOWN.gif";
                        $on_opt=array();
                        if ($tmp_tab=='CONCRETE_PROTO') $on_opt=array("absName"=>1);
                        $tmp_name = obj_nice_name($sql, $tmp_tab, $tmp_id, $on_opt);
                        $tmp = "<a href=\"edit.tmpl.php?t=".$tmp_tab."&id=".$tmp_id."\" ><img src=\"$icon\" border=0> $tmp_name</a>";
                        if ($output_var) $out.=$tmp;
                        else echo $tmp;
                        
                        $pattern_ok = 1;
                    }
                    break;
                }
                
                
                if ($minind==0 or $minind==1) {
                    
                    $pos_end   = f_text::find_wordend($html_txt, $minpos);
                    $full_word = substr($html_txt, $minpos, $pos_end-$minpos+1);
                    
                    $tmp = "<a href=\"".$full_word. "\" target=\"help\">".$full_word."</a>";
                    if ($output_var) $out.=$tmp;
                    else echo $tmp;
                    
                    $found      = 1;
                    $pattern_ok = 1;
                    break;
                }
                
                
            } while (0);
            
            if ($found) {
                // echo "<br> ST: table:$tmp_tab, id:|$tmp_id| pos:$pos, END: pos_end:$pos_end str_url:|$str_url|<br>";
                $pos_end=$pos_end; 
            } else {
                $pos_end=$pos; // back to start pos ...
            }

            if ($found and !$pattern_ok) { // print raw text
                $tmp = $full_word;
                if ($output_var) $out.=$tmp;
                else echo $tmp;
            }
            
        }
        
        //echo  '<b>DDD_END: pos_end:$pos_end REST: '.substr($html_txt, $pos_end)."</b>\n";
        
        $tmp = substr( $html_txt, $pos_end );
        if ($output_var) $out.=$tmp;
        else echo $tmp;
        
        if ($showPreFormatted) {
            $tmp = "</pre>"; // close PRE tag
            if ($output_var) $out.=$tmp;
            else echo $tmp;
        }
        
        if ($output_var) {
            return $out;
        }
    }
    
    
 
    /**
     * transform ptxt => markdown
     * @param object $sqlo
     * @param string $txt
     * @return string markdown text
     */
    static function ptxt_markdown( &$sqlo, &$txt ) {
        
        $out = '';
        $pos_end =0;
        $len_html_text = strlen ($txt);
        
        if (!$len_html_text) {
            return; // no data
        }
        
        $icon_cache=array();

        $found=1;  // initial found val
        
        // echo  "DDX0: ALL(100):".substr($txt, 0, 100)."<br>"; 
        
        while ( $found ) {
            
            $found      = 0;
            $pos        = $pos_end;
            $pattern_ok = 0;
            
            //NEW: $pos_end    = strlen($txt)-1; // default
            
            //echo  "-DDX1: CHAR(START):$pos |".substr($txt, $pos, 0)."<br>"; // print text before ...
            
            do {
                // pos_end, str_url (mit http, db)
                // search for the NEXT pattern ...
                $posarr=array();
                $posarr[0] = strpos($txt, "file://",  $pos); // future
                $posarr[1] = strpos($txt, "db:",    $pos);
                
                // find next ...
                $minind = -1;
                $minpos = $len_html_text-1;   // start of match 
                foreach($posarr as $ind=>$end_pos) {
                    if ($end_pos===FALSE) {
                        unset($posarr[$ind]);
                    } else {
                        if ($end_pos<$minpos) {
                            $minind = $ind;
                            $minpos = $end_pos;
                        }
                    }
                }
                
                if ($minind<0) {
                    // nothing found ...
                    break;
                }
                
                $out .= substr($txt, $pos, $minpos-$pos); // print text before pattern ...
                
 
                //echo "DDD_START minpos:$minpos;pos_end:$pos_end full_word: ".$full_word."|<br>";
                
                if ( $minind==1 ) { // db:EXP:4567
                    
                    
                    $found = 1;
                    
                    $full_word   = f_text::find_db_obj($txt, $minpos);
                    if ($full_word==NULL) {
                        break;
                    }
                    $word_len  = strlen($full_word);
                    $pos_end  = $minpos + $word_len;
                    
                   
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
                    
                    if ($tab_code=="" || !is_numeric($tmp_id) ) {
                        $pattern_ok = 0;
                        break;
                    }

                    if ($tab_code) $tmp_tab   = globTablebyShort($tab_code) ;
                    
                    if ($tmp_tab!="") {
                        // build markdown text for link ...
                        if ($icon_cache[$tmp_tab]) {
                            $icon = $icon_cache[$tmp_tab];
                        } else {
                            $icon     = file_exists("images/icon.".$tmp_tab.".gif") ?
                                "images/icon.".$tmp_tab.".gif":
                                "images/icon.UNKNOWN.gif";
                            $icon_cache[$tmp_tab] = $icon;
                        }   
                        $tmp_name = obj_nice_name($sqlo, $tmp_tab, $tmp_id);
                        $out .=  '![i]('.$icon.') '.$tab_code.':'.$tmp_id.':['.$tmp_name.'](edit.tmpl.php?t='.$tmp_tab.'&id='.$tmp_id.')';
                        $pattern_ok = 1;
                    }
                    break;
                }
                
                if ($minind==0) { // file://
                    
                    $pos_end   = f_text::find_wordend($txt, $minpos);
                    $full_word = substr($txt, $minpos, $pos_end-$minpos+1);
                    
                    $out .=  "[link](".$full_word.")";
                    $found      = 1;
                    $pattern_ok = 1;
                    $pos_end = $pos_end +1;
                    break;
                }

            } while (0);
            
            if ($found) {
                
                if (!$pattern_ok) { // print raw text
                    $out .= substr($txt, $minpos, 1);
                    $pos_end = $minpos + 1;
                } else {
                    $pos_end = $pos_end;
                }
                // echo "<br> ST: table:$tmp_tab, id:|$tmp_id| pos:$pos, END: pos_end:$pos_end str_url:|$str_url|<br>";
                
            } else {
                $pos_end=$pos; // back to start pos ...
            }

            //echo  '<b>DDD_LOOP: pos_end:$pos_end REST: '.substr($txt, $pos_end)."</b>\n";
            
        }
        
        $out .= substr( $txt, $pos_end );
        
        return $out;
    }
    
    /**
     * convert ptxt (Partisan-Text) => Markdown => html
     * @param object $sqlo
     * @param string $ptxt
     
     */
    static function ptxt_markdown_html( &$sqlo, &$ptxt, $show_raw=0 ) {
        require_once 'ots/Parsedown/Parsedown.php';
        
        $len_html_text   = strlen($ptxt);
        if (!$len_html_text) {
            return; // no data
        }
        
        $mtxt = self::ptxt_markdown($sqlo, $ptxt);
        
        $Parsedown = new Parsedown();
        $Parsedown->setSafeMode(true);
        echo $Parsedown->text($mtxt);
    }
    
    static function markdown_out( &$sql, &$html_txt, $show_raw=0 ) {
        require_once 'ots/Parsedown/Parsedown.php';
        
        $len_html_text   = strlen($html_txt);
        if (!$len_html_text) {
            return; // no data
        }
        
        $Parsedown = new Parsedown();
        $Parsedown->setSafeMode(true);
        echo $Parsedown->text($html_txt);
    }
    
}