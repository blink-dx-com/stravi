<?php
/**
 * @namespace core::xmlrpc
 */

/**
 * close session
 *
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param    array $f_param <pre>
 * 	 [0] sess_id
 * 	</pre>
 * @see     "usedby:external:ICONOCLUST"
 * 
 */
 
function close_xml($f_name,$f_param){
     switch(sizeof($f_param)){
         case 1:
            $my_s_id=$f_param[0];
            break;

         default:
            return array(1,"Incorrect number of parameters !");
            //Incorrect number of parameters
       };

    $xmlHelpLib = new xmlrpcHelper(); 
    $answer = $xmlHelpLib->startSession($my_s_id, 'close'); 
    if ($answer[0]>0) return $answer;    




     session_destroy();

     return array(0,"Everything OK !");; //everything is ok

};