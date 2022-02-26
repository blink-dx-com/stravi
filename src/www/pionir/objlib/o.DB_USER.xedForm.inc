<?php
require_once 'f.password.inc';

class oDB_USER_xedForm  extends gui_xedForm_ABS {

    
    /**
     * check input
     * @throws Exception
     * @return -
     */
    function paramCheck(&$sqlo, &$argu, &$extraobj_o, $action, $showCols ) {
            
      if ($action=="insert") {
    
    	## Disable SQL logging before sending the insert statement to the SQL query object:
    	## (SQL logging will be automatically resumed if a new SQL query object is
    	##  requested by calling 'logon_to_db()'. This typically happens within the 
    	##  next PHP script executed.)
    
    	
            $log = & SQL_log::getHandle();
            $log->close();
            
            if ( $argu["PASS_WORD"] == "" ) {
                $pw = fPasswordC::createPasswd();
                $argu["PASS_WORD"]= fPasswordC::hash_pw($pw);
            }
            echo "INFO: Random password generated. Do not forget to change the password.<br>";
      }
      
    
    }

}
