<? 
/*MODULE: obj.link.c_query_sub2.inc
  DESCR:  contains sub functions for executing a query
  AUTHOR: qbi
  RETURN: 
  VERSION: 0.1 - 20040217 	
*/   

require_once("o.LINK.c_query_subs.inc");

class oLinkQueryGui {

function __construct() {
	$this->queryObj = new myquery();
}

/**
 * manage the query form
 * 
 * @param object $sql
 * @param array $values  class values
 * @param array $myquvarval input variables
 * @param string $formNextUrl
 * @param int $extra_obj_id
 * @param int $myqu_go go flag, if no variables are set
 * @param array $foption   e.g. 'formVarBase' 
 * @return int error_value
 */
function form_manage( 
    &$sql, 
    &$values,      
    &$myquvarval,  
    $formNextUrl, 
    $extra_obj_id,  
    $myqu_go,       
    $foption = NULL 
    ) { 
    
    global $error, $varcol;
    
    if ( $values["sqltext"]=="" || $values["table"]==""  ) {
       echo "<font color=gray>Query-document contains no SQL-parameters to perform a query!</font><br>\n";   
       return;
    }
    
    $options = array(); 
    if ( $foption['formVarBase']!=NULL ) $options['formVarBase'] = $foption['formVarBase'];
    
    $uservars = $this->queryObj->get_uservars( $values["sqltext"] );
    // echo "DEB: myqu_go: $myqu_go";
    if ( !$myqu_go OR (($myqu_go==1) && sizeof($uservars) ) ) {   // prepare OR (try to execute, but have parameters)
         $options["action"] = "prepare";  
          
         $this->queryObj->ShowFrom( $sql, $extra_obj_id, NULL, $formNextUrl, $options ); 
         if ($error->got(READONLY)) {
            $error->set('show query form', 1, 'showing of query form failed');
            $error->printLast();
            return -1;
         }
    } else {  // $myqu_go == 2 OR (==1 and no parameters)
        // get_uservars( $sqltext );
        echo "<font color=gray><B>Execute query now!. Please wait....</B></font><br>";
        $options["action"] = "exec";  
        $this->queryObj->ShowFrom( $sql, $extra_obj_id, $myquvarval, $formNextUrl, $options ); 
        if ($error->got(READONLY)) {
            $error->set('show query form', 1, 'execution of query failed');
            $error->printLast();
            return -1;
        }
    }
    return (0);
}
 
}
