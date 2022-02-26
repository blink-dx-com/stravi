<?php
/**
 * class to manage logging of API data
 * GLOBAL vars:
 *  - $globals["xml_rpc.debug_dir"]
 *  - $globals["xml_rpc.debug_keys"]
 * @package api/lib/api_debug.inc
 * @author skube
 *
 */
class api_debug {
    
    private $output_debug;
    
    /**
     * 
     * @param int $output_debug : do debug ?
     * @return number
     */
    function __construct($output_debug) {
        global $globals;
        
        $this->output_debug = $output_debug;
        $this->deb = array();
        $this->deb["hostip"]= $_SERVER['REMOTE_ADDR'];
        
       
        $filename  = 'xml_rpc_log.log';
        $dirname   = $globals["xml_rpc.debug_dir"];
        if ( !is_dir($dirname) )  {
            $this->output_debug = 0;
            return -1;
        }
        $this->deb["full_name"] = $dirname ."/". $filename;
        
    }
    
    
    /**
     * manage debugging of XMLRPC request: input parameters
     * - global INPUT:
     *   - $this->deb["full_name"] - logfile name
     *   - $this->output_debug : see $globals["xml_rpc.debug_level"]
     *   - $globals["xml_rpc.debug_keys"] : if this is set, only special ymlrpc-methods are logged ...
     *        array('whitefuncs'=>array( array of methodNames  ) log only these methods
     * - global OUTPUT: $this->methodName
     * @param  string $request_xml
     * @return int $loginfo
     * 		 1 : o.k. logged
     * 		10 : can not write to log-file
     * 		11 : method not in white list; no log
     */
    function log_input( $method, &$in_dict ) {
        
        global $globals;
        
        if (!$this->output_debug) return 5;
        
        $this->methodName = $method;
        $my_file = fopen( $this->deb["full_name"], 'a' );
        if (!$my_file) return 10;
        

        if (isset($globals['xml_rpc.debug_keys']['whitefuncs']) ) {
            // only log listed methods
            $whitelist_meths = &$globals['xml_rpc.debug_keys']['whitefuncs'];
            if (!in_array($this->methodName, $whitelist_meths)) {
                return 11;
            }
        }
        
        fwrite($my_file,"<OPENCOMM TYPE=\"REQ\" API=\"JSONRPC\" HOST=\"".$this->deb["hostip"]."\" date=\"".date("Y-m-d H:i:s")."\"");
        fwrite($my_file," method=\"".$this->methodName."\"");
        fwrite($my_file,"/>\n");
        
        if ( $this->output_debug>1 ) {
            fwrite($my_file, '<PARAMS_IN>'."\n". json_encode($in_dict)."\n".'</PARAMS_IN>'."\n");
        }
        
        fclose($my_file);
        
        return 1;
    }
    
    /**
     * manage debug to logfile for XMLRPC-output
     * @param array $out
     */
    function log_output( &$out_dict ) {
        
        if (!$this->output_debug) return 5;
        
        // $my_file is closed ...
        $my_file = fopen( $this->deb["full_name"], 'a' );
        if (!$my_file) return 10;
        
        fwrite($my_file,"<OPENCOMM TYPE=\"OUT\" API=\"JSONRPC\" HOST=\"".$this->deb["hostip"]."\" method=\"".$this->methodName."\" date=\"".date("Y-m-d H:i:s")."\"/>\n");
        if ( $this->output_debug>2 ) {
            fwrite($my_file, '<PARAMS_OUT>'."\n". json_encode($out_dict)."\n".'</PARAMS_OUT>'."\n");
        }
        
        fclose($my_file);
    }
    
}
