<?php
/**
 * exception handling
 * @package exception.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class MyExcept {
    static function stack_extract($e) {
        
        $stack_new=array();
        $err_stack = $e->getTrace();
        foreach ($err_stack as $row) {
            $stack_new[]=array('class'=>$row['class'], 'function'=>$row['function'],  'line'=>$row['line']);
        }
        return $stack_new;
    }
    
    static function stack_nice($e) {

        $err_stack = $e->getTrace();
        $stack_str = '';
        $newline   = "";
        foreach ($err_stack as $row) {
            $stack_str .= $newline . $row['class'].':'.$row['function'].':['. $row['line'].']';
            $newline = "\n";
        }
        return $stack_str;
    }
}