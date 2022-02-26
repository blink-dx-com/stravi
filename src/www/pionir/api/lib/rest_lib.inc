<?php
require_once 'reqnormal.inc';

/**
 * REST output : JSON dict
 * @author Steffen
 */
class rest_lib {
    
    
    private static function get_protocol() {
        $protocol = (isset($_SERVER['SERVER_PROTOCOL']) ? $_SERVER['SERVER_PROTOCOL'] : 'HTTP/1.0');
        return $protocol;
    }
    
    /**
     * OUTPUT API-output-structure (see doc)
     * @param int $code
     * @param string $text
     */
    static function error_out($code, $text) {
        global $error;
        $FUNCNAME= __FUNCTION__;
        $error = & ErrorHandler::get();
        
        $json_str = json_encode(  array('num'=>$code, 'text'=>$text)  );
        
        header(self::get_protocol()." 500 Internal Server Error");
        header('error-description: '.$json_str);
        header("Content-type: text/html");
        
        echo "ERROR: ".$json_str;
        
        if ( is_object($error) )  {
            $error->set($FUNCNAME,$code, $text);
            $error->getAsTextAndLog( basename(__FILE__), 1);
        }
        
        exit(4);
    }
    
    /**
     * OUTPUT API-output-structure (see doc)
     * @param array $data
     */
    static function output_start( $sess_id, $header_arr=array() ) {
        
        header(self::get_protocol()." 200 OK");
        header("session_id:".$sess_id );
        
        
        if (sizeof($header_arr)) {
            foreach($header_arr as $row) {
                header($row);
            }
        }
        
        
    }
}