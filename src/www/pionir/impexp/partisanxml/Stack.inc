<?

//
//  class:      Stack
//  descr:      simple class representing general stack functionality
//  version:    class version 0.1
//

class Stack
{
    var $stackData;



    // constructor
    function __construct()
    {
        $this->stackData = array();
    }



    // push data to stack
    function push($data, $ddd=NULL)
    {
    	//echo "<br>DDD:Stack:'.$ddd.' PUSH>> ".print_r($data,1);
        array_push($this->stackData, $data);
    }



    // pop data from stack
    function pop($ddd=NULL)
    {
    	//$temp = end($this->stackData);
    	//echo "<br>DDD:Stack:'.$ddd.'  POP<< ".print_r($temp,1);
    	return array_pop($this->stackData);
    }
    
    
    
    // get stack size
    function getSize()
    {
        return count($this->stackData);
    }
}


// end.
?>
