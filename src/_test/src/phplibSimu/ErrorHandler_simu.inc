<?php
/**
 * - SIMULATION for global error handling support and application log
 * - can be used for exported code to other systems like: 129.PIMA_Kprod/111.CellController_Frontend
 * @package ErrorHandler_simu.inc
 * @swreq   SREQ:0001806:003 need errorHandler simulation
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
// $Header: trunk/src/www/_tests/phplibSimu/ErrorHandler_simu.inc 59 2018-11-21 09:04:09Z $

/**
 * all Errors
 */
define("CCT_ALL", 10401);
/**
 * like CCT_ALL, but does not reset error-flag
 */
define("CCT_ALL_READONLY", 241177);
/**
 * synonym to CCT_ALL_READONLY
 */
define("READONLY", 241177);
/**
 * errors but no warnings
 */
define("CCT_ERROR", 81201);
/**
 * like CCT_ERROR, but does not reset error-flag
 */
define("CCT_ERROR_READONLY", 101201);
/**
 * only warnings, no errors
 */
define("CCT_WARNING", 80302);

class SingleError {
	/**
     * instanciates the class and sets the variables
     * @access public
     * @param string function-name or module in which the error occured
     * @param int    identifier of the error (must be bigger than 0) (otherwise id gets set to 24111977)
     * @param string textual description of the error
     * @param int    real error? (error: 0; warning: 1-5 (5..least important warning))
     * @return void
     */
    function __construct($origin, $id, $text, $warning)
    {
        $this->origin  = (string)$origin;
        $this->id      = (int)$id;
        $this->text    = (string)$text;
        $this->warning = (int)$warning;
   
    }
    
}

/**
 * class for handling a stack of errors
 *
 * This class can distinguish between errors and warnings.
 * If you do not explicitly say that you want to use this distinction, all warnings are threaded like errors.
 *
 * Designed acconding to 'singleton' design-pattern. To get a handle of the class' instance call
 * $error = & ErrorHandler::get();
 *
 * The contents of the object of this class are not persistant, they last only till the end ot the page. (but look under <b>see</b>)
 *
 * @author Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version 1.1
 * @package ErrorHandling
 */
class ErrorHandler 
{

    /**
     * array of the last errors
     *
     * oldest with lowest index
     *
     * @var array ( array($caller, $id, $text, $warning)  )
     * @access private
     */
    var $_error_stack;
    
    /**
     * did an error occur?
     *
     * @var boolean 
     * @access private
     */
    var $_is_error;

    /**
     * state restored by {@link ErrorHandler::restore()}
     *
     * When you have stored the current state using {@link ErrorHandler::store()} and later called {@link ErrorHandler::restore()}, then the stored state is saved in this variable.
     * You have to call {@link ErrorHandler::get()} to make this stored state the current state.
     *
     * @var object
     * @access private
     */
    var $_restored_state;

	 /**
     * state for logx()
     *
     *
     * @var boolean
     * @access private
     */
	var $_logxFlag;
	
    /**
     * creates the instance of this class
     *
     * you should NEVER instancicate this class by hand.
     * use <code>$error = &ErrorHandler::get();</code> therefore.
     *
     * @access private
     * @since version 1.1 private constructor
     * @see ErrorHandler::get()
     */
    function __construct() {
        $this->_error_stack    = array();
        $this->_is_error       = false;
        $this->_restored_state = NULL;
		$this->_logxFlag	   = 1;
    }

    /**
     * static method: get a handle to the only instance of the class
     *
     * static method!
     * call using <code>$error = & ErrorHandler::get();</code>
     *
     * @since version 1.1
     * @access public
     * @return object handle to the only instance of the class ErrorHandler
     * @staticvar object the only instance of the class ErrorHandler
     */
    function &get()
    {
        static $theErrorHandler = NULL; // is only set when called for first time
        
        if (is_null($theErrorHandler)) {
            $theErrorHandler = new ErrorHandler(); // warning: leave this way, because references are not stored statically!
        } elseif (!is_null($theErrorHandler->_restored_state)) { // restored state is only possible after calling first time
            $theErrorHandler = $theErrorHandler->_restored_state;
            $theErrorHandler->_restored_state = NULL;
        }
        return $theErrorHandler;
    }
    
 	/**
     * tell error handler that an error has occurred
     *
     * error-object is pushed to the stack so maybe existing gaps in the index list are not filled (so do not rely on indices!)
     *
     * @return void
     * @param string function-name or module in which the error occured
     * @param int    identifier of the error (depending on $caller, no global error_id)
     * @param string textual description of the error
     * @param int    tells, if this error is a real error (warning = 0) or only a warning (1-5, 5..least important warning)
     * @access public
     */
    function set($caller, $id, $text, $warning=0) {
        array_push($this->_error_stack, array($caller, $id, $text, $warning) );
        $this->_is_error = true;
    }
    
    /**
     * got error ?
     * @return $result
     * 	0: no Error
     *  1: error got
     */
    function got() {
    	if (!sizeof($this->_error_stack)) return 0;
    	else return 1;
    }
    
    function reset() {
    	$this->_error_stack = array();
    }
    
    /**
     * get last error
     * @return object SingleError
     */
	function getLast() {
        if (!sizeof($this->_error_stack)) return NULL;
        
        $ret = end($this->_error_stack);
		reset ($this->_error_stack);
		
		$answerObj = new SingleError($ret[0], $ret[1], $ret[2], $ret[3]);
		
        return $answerObj;
    }
    
	/**
     * get all occured errors
     * - do not change the error-stack
     * @return array ( array('caller'=>, 'id'=>, 'text'=>) )
     */
    function getAll() {
    	if (!sizeof($this->_error_stack)) return NULL;
        reset ($this->_error_stack);
        
        foreach( $this->_error_stack as $key=>$ret) {
        	$retout[] = array('caller'=>$ret[0], 'id'=>$ret[1], 'text'=>$ret[2]);
		}
		reset ($this->_error_stack); 
        
        return $retout;
    }
}
