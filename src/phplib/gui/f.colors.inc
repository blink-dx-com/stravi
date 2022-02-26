<?php
/**
 * 
 * @author skube
 * @package gui/f.colors.inc
 */
class fColors {
    
    const DIFF_COLORS=array(
        
        '#808080',
        '#ffa500',
        '#ffff00',
        '#008000',
        '#0000ff',
        '#4b0082',
        '#ee82ee',
        '#ff0000',
        '#323436',
        '#77a8d0',
        '#4d80aa',
        '#6c9007',
        '#B200FF',
        '#00BE0E',
        '#495b51',
        '#b4f6cb',
        '#33f9ba',
        '#135857',
        '#69384a',
        '#303030',
        '#404040',
        '#505050',
        '#606060',
        '#707070',
        '#808080',
        '#909090',
        '#A0A0A0',
    );
    
    
    /**
     * calc forground color of background
     * @param string $bg_r '#a0f4D1'
     * @return string
     */
    static function get_fg_of_bg(string $bg_r) {
        
        $bg = substr($bg_r,1); // ignore '#'
        $r = hexdec(substr($bg,0,2));
        $g = hexdec(substr($bg,2,2));
        $b = hexdec(substr($bg,4,2));
        
        $squared_contrast = (
            $r * $r * .299 +
            $g * $g * .587 +
            $b * $b * .114
            );
        
        if($squared_contrast > pow(130, 2)){
            return '#000000';
        } else {
            return '#FFFFFF';
        }
    }
    
    /**
     *
     * @param object $sqlo
     * @param array $id_arr
     *    array of object-IDs
     * @return array difference index array
     *     obj-id => INDEX
     */
    static function calc_diff_index($id_arr) {
        $diff_array_x = array_unique($id_arr);
        $diff_array = array_flip($diff_array_x);
        return $diff_array;
    }
}