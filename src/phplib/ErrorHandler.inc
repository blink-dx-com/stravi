<?php
/**
 * - global error handling support and application log
 * LOGs:
 *   - can save info in logfile $globals["app.logfile"]
 *   - if  $globals["app.logkey"] == "error.print" : save all errors in LOG-file
 *   - format example of info-string:
       <head func="printAll" phpfile="www/pionir/view.tmpl.php" >
			<info>hard error detected in sql</info>
	   </head>
 * TEMP-INFO-Logs: use $this->_infolog_stack
 * @author Michael Howitz <mac@clondiag.com>
 * @package ErrorHandling
 * @swreq SREQ:0000984: g > ErrorHandling : concept for error handling  
 * @swreq UREQ:0000935 g.app.logfile : manage application log
 * @version $Header: trunk/src/phplib/ErrorHandler.inc 59 2018-11-21 09:04:09Z $
 */

/**
 * class for storing error as data
 */
require_once('SingleError.inc');


/**
 * all Errors
 */
define("CCT_ALL", 10401);
/**
 * like CCT_ALL, but does not reset error-flag
 */
define("CCT_ALL_READONLY", 241177);

/**
 * synonym to CCT_ERROR_READONLY (since 2020-09-03)
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
/**
 * like CCT_WARNING, but does not reset error-flag
 */
define("CCT_WARNING_READONLY", 151176);



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
 * @author Michael Howitz <mac@clondiag.com>
 * @version 1.1
 * @package ErrorHandling
 * @since version 1.1 real singleton-class
 * @since version 1.1 real saving of current state
 * @see also session-variable $savedErrors in www/pionir/main.php for error-persistence
 */
class ErrorHandler 
{

    const IS_WARNING_1 = 1;
    const IS_WARNING_5 = 5;
    
    /**
     * array of the last errors
     *
     * oldest with lowest index
     *
     * @var array
     * @access private
     */
    var $_error_stack;
    /**
     * did a real error occur? - not affected by warnings ...
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
	 * array of info texts
	 * @var array - array('func'=>, 'text'=>)
	 * @access private
	 */
	var $_infolog_stack;
	
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
        $this->_infolog_stack  = array();
        $this->_is_error       = false;
        $this->_restored_state = NULL;
		$this->_logxFlag	   = 1;
		date_default_timezone_set('Europe/Berlin'); // import, if we use gettimeofday() anywhere in PARTISAN
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
    public static function &get()
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
     * tell error handler that an error/warning has occurred
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
 
        array_push($this->_error_stack, new SingleError($caller, $id, $text, $warning));
        if (!$warning) $this->_is_error = true;
        
    }
    
    /**
     * set the error-has-occured flag
     *
     * only necessary if you cleared the error-flag using {@link ErrorHandler::got()} without handling the error using a get- or print-method
     *
     * This is a special method! normally you should use {@link ErrorHandler::got(READONLY)} instead of got() followed by set_error_flag()
     * but there may be some special cases where you may want to restore the error_flag
     * @access public
     * @return void
     */
    function set_error_flag () {
        $this->_is_error = true;
    }
    
    /**
     * did an error (of specified conditions) occur in the last time?
     *
     * side effect: if called with parameter *READONLY: no side effects
     *              else resets the error-flag to false
     * WARNING: CCT_ERROR*, CCT_WARNING* and when called with parameters (origin and id) or origin do not rely on error flag!
     *          then the whole error stack is searched for specified errors
     *
     * @param int    use one of the handling constants defined in this file to tell what which kind of errors you want to see and how to handle them
     * @param string check only for errors of this specified origin (error-type)
     * @param int    check only for errors of this specified error-id (works only in combination with origin)
     * @return boolean did an error of the specified conditions occur?
     * @see int CCT_ALL
     * @see int CCT_ALL_READONLY
     * @see int READONLY
     * @see int CCT_ERROR
     * @see int CCT_ERROR_READONLY
     * @see int CCT_WARNING
     * @see int CCT_WARNING_READONLY
     * @access public
     */
    function got($handling = 0, $origin = '', $error_id = 0)
    {
        $use_sub = ($origin != '');
        if ($use_sub)
            $error_stack = & $this->_get_sub_array_($origin, $error_id);
        else
            $error_stack = & $this->_error_stack;
        $ret = $this->_is_error;
        
        switch ($handling) {
        case 0: // no parameters
            $this->_is_error = false;
            break;
        case CCT_ALL:
            $this->_is_error = false;
        case CCT_ALL_READONLY:
        
            $ret = $use_sub ? (count($error_stack) > 0) : $ret;
            break;
        case CCT_WARNING:
            $this->_is_error = false;
        case CCT_WARNING_READONLY:
            $ret = false;
            foreach($error_stack as $s_err)
                if ($s_err->warning > 0) {
                    $ret = true;
                    break; // out of foreach
                }
            break; // out of switch
        case CCT_ERROR:
            $this->_is_error = false;
        case READONLY:  // @todo maybe it is faster to have an extra stack for WARNINGS ..... 
        case CCT_ERROR_READONLY:
            $ret = false;
            foreach($error_stack as $s_err)
                if ($s_err->warning == 0) {
                    $ret = true;
                    break; // for foreach
                }
            break; // for switch
        default:
            echo '<br>WARNING: programming error in function calling errorhandler\'s got-method: undefined value for first parameter of got-method<br>';
            $ret = true;
        }
        return $ret;
    }
    
    /**
     * empty the error-stack and set error-flag to false
     *
     * when only a part of the stack is removed the error-flag stayes untouched
     *
     * when you use parameters: you always have to use both or nothing will happen!
     * when you use parameters: the positions of the errors in the stack are not reordered (there may be gaps in the index list then)
     * when you use parameters: if errors are twice on stack, the newer one (the one with the higher index) is removed
     *
     * @param boolean remove only a part of the stack? (if true: remove {@link SingleError} objects; if false: clean whole stack)
     * @param array   the {@link SingleError} objects which should be removed from error-stack
     * @access public
     * @return void
     */
    function reset($remove_elements = false, $to_be_removed = array())
    {
        if ($remove_elements) {
            $error_stack = array_reverse($this->_error_stack, true); // remove newest errors first
            foreach($to_be_removed as $s_err){ // remove each error in array
                foreach ($error_stack as $pos_no => $s_err_all) {
                    if ($s_err_all->equal($s_err)) {
                        unset($this->_error_stack[$pos_no]);
                        unset($error_stack[$pos_no]);
                        break;
                    }
                }
            }
            $this->_is_error = (count($this->_error_stack) > 0);
        } else {
            $this->_error_stack = array();
            $this->_is_error    = false;
        }
    }
    

    /**
     * remove warnings from error-stack
     *
     * removes all warnings with warning_level > max_warn_level
     * default: remove all warnings
     * warning: the positions of the errors in the stack are not reordered (there may be gaps in the index list!)
     *
     * @param int warn-level over which (warnings with given warn-level are not removed!) all warnings should be removed (higher warn-level means less important warning), so calling with value >= 5 makes no sense
     * @return void
     * @access public
     */
    function resetWarnings($max_warn_level = 0)
    {
        foreach ($this->_error_stack as $pos_no => $s_err_all) {
            if ($s_err_all->warning > $max_warn_level)
                unset($this->_error_stack[$pos_no]);
        }
    }


    /**
     * get the last occured error
     *
     * - if $handling = 0 : removes the returned error from the stack. (But stack does not get reordered!)
     * - You may specify origin or (origin and id) for the error you want to get.
     * - Method sets error-flag only to false when error-stack gets empty.
     *
     * Automatically stores current state (i.e. the state before removing returned error).
     *
     * @param string get last error of this origin (error-type) (leave out for all origins)
     * @param int    get last error of this error-id (works only in combination with origin) (leave out for all id's)
	 * @param int    $handling = READONLY or CCT_WARNING_READONLY: does not remove the error
     * @return object instance of {@link SingleError} or NULL if no error or no error of specified conditions
     * @access public
     */
    function getLast($origin = '', $error_id = 0, $handling = 0)
    {
        $use_sub = ($origin != '');
        if ($use_sub)
            $error_stack = & $this->_get_sub_array_($origin, $error_id);
        else
            $error_stack = & $this->_error_stack;

        //  $this->store(); // store current state
		
		if ($handling==READONLY) {
        	$ret = end($error_stack);
			reset ($error_stack);
		} else {
			$ret = array_pop($error_stack);
        	$this->reset(true, array($ret));
		}
        return $ret;
    }


    /**
     * get all occured errors
     *
     * Removes the returned errors from the stack. (But stack does not get reordered!)
     * You may specify origin or (origin and id) for the errors you want to get.
     * Method sets error-flag only to false when error-stack gets empty.
     *
     * @param string get all errors of this origin (error-type) (leave out for all origins)
     * @param int    get all errors of this error-id (works only in combination with origin) (leave out for all id's)
     * @return array array (lowest index: oldest error, indexes in ascending order but with gaps) of objects of {@link SingleError} or empty array if no error or no error of specified conditions
     * @access public
     */
    function getAll($origin = '', $error_id = 0)
    {
        $use_sub = ($origin != '');
        if ($use_sub)
            $error_stack = & $this->_get_sub_array_($origin, $error_id);
        else
            $error_stack = & $this->_error_stack;
        
        $ret = $error_stack;
        $this->reset($use_sub, $error_stack);
        return $ret;
    }
    
    function &getAllNoReset()    {
        return $this->_error_stack;
    }
    // save error in LOG automatically by app-flag
    
    function save_ErrLogAuto() {
        $FUNCNAME=__FUNCTION__;
        if ( strstr($_SESSION['globals']["app.logkey"], "error.print") !=NULL ) {
            $this->logxMeta( "ERROR", 5, $FUNCNAME, $_SERVER['PHP_SELF'], 
                $this->getAllAsText($this->_error_stack), "[/ERR]" );
        }
    }
    
    /**
     * set error stack
     * @param array $error_stack
     */
    function setAll(array &$error_stack) {
        $this->_error_stack = &$error_stack;
    }
    
    /**
     * get all WARNINGS
     * @return array of SingleError
     */
    function getAllWarnings()  {
       
        $error_stack = & $this->_error_stack;
        reset($error_stack);
        $warn_stack=array();
        
        foreach($error_stack as $s_err) {
            if ($s_err->warning > 0) {
                $warn_stack[]=$s_err;
            }
        }

        return $warn_stack;
    }
    
    /**
     * store current error handler state
     *
     * Normally the error-stack is empty when you open an new php-page.
     * If you want to access the errors from the previous page you have to store them using this method.
     * You can only store one state. Each new call of this method overwrites the saved state.
     *
     * Uses global session variable $savedErrors.
     *
     * @return void
     * @access public
     * @see ErrorHandler::restore()
     * @since 1.1
     */
    function store()
    {
        $_SESSION['savedErrors'] = serialize($this);
    }

    /**
     * restore error handler state from saving
     *
     * This method is <u>not</u> static, you need the ErrorHandler instance to call it.
     * You have to call {@link ErrorHandler::get()} to make the restored error handler state the current one.
     * You can only restore a state once, because it gets deleted ater restoring.
     *
     * Uses global session variable $savedErrors.
     *
     * @return void
     * @access public
     * @see ErrorHandler::store()
     * @since 1.1
     */
    function restore()
    {
        $savedErrors = empty($_SESSION['savedErrors']) ? NULL : $_SESSION['savedErrors'];
        
        if (!$savedErrors) return;
        
        $this->_restored_state  = unserialize($savedErrors);
        $_SESSION['savedErrors'] = NULL;
    }

    
    /**
     * count number of occured errors
     *
     * Counts the numer of occurred errors according to specified conditions.
     * Does not change neither error-stack nor error-flag. 
     *
     * With wrong handling type always 0 is returned.
     *
     * @param int one of the following handling constants CCT_ALL, CCT_ERROR, CCT_WARNING
     * @param string check only for errors of this specified origin (error-type)
     * @param int    check only for errors of this specified error-id (works only in combination with origin)
     * @return int number of errors of the specified conditions
     * @see int CCT_ALL
     * @see int CCT_ERROR
     * @see int CCT_WARNING
     */
    function count($handling = CCT_ALL, $origin = '', $error_id = 0)
    {
        $count   = 0;
        $use_sub = ($origin != '');
        
        if ($use_sub)
            $error_stack = & $this->_get_sub_array_($origin, $error_id);
        else 
            $error_stack = & $this->_error_stack;
        
        switch ($handling) {
        case CCT_ALL:
            $count = count($error_stack);
            break;
        case CCT_ERROR:
            foreach($error_stack as $err)
                if (!$err->warning) // warning = 0
                    $count++;
            break;
        case CCT_WARNING:
            foreach($error_stack as $err)
                if ($err->warning)
                    $count++;
            break;
        default: 
            echo '<br>WARNING: programming error in function calling errorhandler\'s count-method: undefined value for first parameter of count-method<br>';
        }
        return $count;
    }
    

    /**
     * print last error
     * @deprecated use ErrorDisplay::print (which does not yet exist)
     * @since 1.1
     */
  function printLast (      $origin = "",    $error_id = 0) {
##    bool printLast (string origin = "", int error_id = 0)
# prints the last error to the screen or the last error which matches the specified criteria
# removes the printed error from the stack.
# returnes boolean telling if an error was printed to the screen.
# side effects: cleares error-flag & error-stack (a copy is saved an a link is print to screen which points to this copy)

    

	$ret = false;
    $r   = $this->getLast($origin, $error_id);

    if (is_object($r)) {
      $r->printMsg();
	  echo '<small><a href="'.$_SESSION['s_sessVars']['loginURL'].'/errors.all.php">show whole error stack</a></small><br>';
      $ret = true;
    }
    return $ret;
  }
  
  /**
   * get all errors as array
   * @param  array OPTIONAL alternative $error_stack
     @return array $ErrorArray
   */
  function getAllArray( $error_stack=NULL, $origin='', $error_id=0) {
  
  	$retarr = NULL;
	
	if ($error_stack==NULL) $error_stack = $this->getAll($origin, $error_id);
	
	if (count($error_stack) == 0)
		return; // don't print anything if no error in stack.
	
	foreach($error_stack as $single_error) {
		$retarr[] = $single_error->getAsArray();
	}
	
	return ($retarr);
  }
  
  /**
   * - append an $ErrorArray (from an other error-object) to the error-stack
   * - usage: get $ErrorArray from getAllArray()
   * - used for: saving error-stack for later actions
   */
  function appendErrArr( $errArr ) {
  
	if (!sizeof($errArr)) return; 
	
	foreach( $errArr as $valarr) {
		$this->set( $valarr["ori"], $valarr["id"], $valarr["text"] );
	}
	reset ($errArr); 
  }
  
  
  function getTextAsArray() {
  
  	$allarr = $this->getAllArray();
  	$retarr=array();
	foreach( $allarr as $oneerror) {
		$retarr[] = $oneerror["text"];
	}

	return $retarr;
  }
  
  /**
   * get all errors as one TEXT-string
   * @param  array OPTIONAL alternative $error_stack
   * @param  string OPTIONAL alternative lineend-char
     @return array $ErrorArray
   */
  function getAllAsText($error_stack=NULL, $lineend="\n" ) {
	
	$allarr = $this->getAllArray($error_stack);
	$tmpSep = "";
	$retstr = "";
    if (!is_array($allarr))  return NULL;
	foreach( $allarr as $oneerror) {
		$retstr .= $tmpSep . $oneerror["ori"].":".$oneerror["id"].":". $oneerror["text"];
		$tmpSep  = $lineend;
	}
	reset ($allarr);
	
	return ($retstr);
  }
  
  /**
   * get all errors as one TEXT-string
   * @param  string $MODULE module name, e.g. from XML-rpc
   * @param  int $prio
     @return array $ErrorArray
   */
  function getAsTextAndLog( $MODULE, $prio, $act=READONLY ) {
	// FUNCTION: get all details as text
	$FUNCNAME='getAsTextAndLog';
	
	$error_stack=NULL;
	$logerrTxt = $this->getAllAsText($error_stack, "[/ERR]");
	$this->logxMeta( "ERROR", $prio, $FUNCNAME, $MODULE,  $logerrTxt);
	
	$retstr = $logerrTxt;
	
	if ($act!=READONLY) $this->reset();
	
	return ($retstr);
  }
  
  /**
   * - meta call for writing a log-message
   * - if PHP-sacript is "p.php" : log also the input-parameter "mod" : the module-name
   * @param $key: 'ERROR', 'WARN',
   * @param prio 1 (high) ... 5 (low)
   * @param funcname  function name
   * @param phpfile   name of PHP-file
   * @param info info text, can contain HTML-chars; will be encoded in this function
   */
  function logxMeta( $key, $prio, $funcname, $phpfile, $info ) {
  	if (substr($phpfile, -5)=='p.php') {
  		// get module-name
  		$phpfile .= '?mod='.$_REQUEST['mod']; // append module-name
  	}
  	$message = '<head func="'.$funcname.'" phpfile="'.$phpfile.'" >'.
			'<info>'.htmlentities($info).'</info></head>';
  	$this->logx( $key, $prio, $message );
  }
  
  /**
   * - log all errors on LOG-file
   * - reset error
   */
  function logError() {
	$FUNCNAME= "logError";
	$error_stack = NULL;
	$prio = 5;
	$this->logxMeta( "ERROR", $prio, $FUNCNAME, $_SERVER['PHP_SELF'], $this->getAllAsText($error_stack, "[/ERR]") );
	$this->reset();
  }

  /**
   * see also htmlErrorClassBox()
   * - print all errors to the screen.
   * - removes the shown errors from the error-stack.
   * - returns wether an error was printed to the screen or not.
   * - save error-stack in log-file
   * - see also: printAllPrio(), printAllEasy()
   * @todo ? use ErrorDisplay::print (which does not yet exist)
   * return boolean error got
   */
  function printAll (      $origin = "",    $error_id = 0) {
	global $globals;
	
	$FUNCNAME= "printAll";
    $error_stack = $this->getAll($origin, $error_id);

    if (count($error_stack) == 0)
      return false; // don't print anything if no error in stack.
    
    echo '
    <div class="y-msg-error">
      <div class="y-msg-error-header"><img src="res/img/alert-triangle.svg" style="margin-right:20px;"> Errors and Warnings</div>
      <div class="y-msg-error-container">'."\n";
    
    $single_error = current($error_stack);
    echo '<p>'.$single_error->text.'</p>'."\n";
    echo "<p>\n";
    echo '<table style="color: #808080; font-size:0.8em; padding-right:3px;">'."\n";
    foreach($error_stack as $single_error) {
        $tmp = $single_error->getAsArray();
        $tmp_id = $tmp['id'];
        if ($tmp['warn']) $tmp_id = 'WARN:'.$tmp_id;
        echo '<tr valign=top>';
        echo '<td>',$tmp['ori'],'</td>';
        echo '<td>',$tmp_id,'</td>';
        echo '<td>',$tmp['text'],'</td>';
        echo '</tr>'."\n";
    }
    
    echo '</table></p>'."\n";
    echo '</div>
    </div>'."\n";
	
//     echo '<table border="0" cellspacing="1" cellpading=0 bgcolor=#808080>';
//     echo '<tr bgcolor="red"><td colspan="4" align=center><font color="#FFFFBB"><b>errors and warnings</b></td></tr>';
//     // '<td nowrap><font size=-1><b>warning level</b></font></td>'.
// 	echo '<tr bgcolor="#BBBBBB" align="center">'.
// 		'<td><font color=#EFEFEF size=-1><b>component</b></font></td>'.
// 		'<td nowrap><font color=#EFEFEF size=-1><b>id</b></font></td>'.
// 		'<td><font color=#EFEFEF size=-1><b>message</b></font></td></tr>';

// 	foreach($error_stack as $single_error) {
// 	    $tmp = $single_error->getAsArray();
// 	    $tmp_id = $tmp['id'];
// 	    if ($tmp['warn']) $tmp_id = 'WARN:'.$tmp_id;
// 	    echo '<tr bgcolor="#FFFFBB" align="center">';
// 	    echo '<td><font size="-1">',$tmp['ori'],'</font></td>';
// 	    echo '<td><font size="-1">',$tmp_id,'</font></td>';
// 	    echo '<td align="left">',$tmp['text'],'</td>';
// 	    echo '</tr>',"\n";
// 	}
        
//     echo "</table>";
	
	if ( strstr($globals["app.logkey"], "error.print") !=NULL ) {
		$this->logxMeta( "ERROR", 5, $FUNCNAME, $_SERVER['PHP_SELF'], $this->getAllAsText($error_stack, "[/ERR]") );
	}

    return true;
  }  

  /**
   * print all errors, but no logging of error in log file
   * @param $prio: priority: 1- high, 10- low, 0- nothing
   * @return boolean error got ?
   */
  function printAllPrio ($prio=0) {
  	$this->_logxFlag = 0;
  	$retval = $this->printAll();
  	$this->_logxFlag = 1;
  	return $retval;
  }
  
   /**
    * - prints all or the specified errors to the screen (only nice description)
    * - removes the shown errors from the error-stack.
    * - returnes wether an error was printed to the screen or not.
    * @param int $prio priority: 1- high, 10- low
    */
   function printAllEasy ( $prio=0 ) {

	global $globals;
	$FUNCNAME= "printAllEasy";

	
    $error_stack = $this->getAll();

    if (count($error_stack) == 0)
      return false; // don't print anything if no error in stack.
    
    echo '
    <div class="y-msg-error">
      <div class="y-msg-error-header"><img src="res/img/alert-triangle.svg" style="margin-right:20px;"> Errors and Warnings</div>
      <div class="y-msg-error-container">
        ';
    foreach($error_stack as $single_error)
        echo '<p>'.$single_error->text.'</p>'."\n";
    
    echo '</div>
    </div>
    ';
	
//     echo '<table border="0" cellspacing="0" cellpadding=1 bgcolor=red>';
//     echo '<tr bgcolor="red"><td align=center><font color="white"><b>&nbsp;errors and warnings&nbsp;</b></td></tr><tr><td>';
// 	echo '<table border="0" cellspacing="3" cellpadding=0 width=100% bgcolor=#FFFFBB>'."\n";
//     foreach($error_stack as $single_error)
//         $single_error->printTRs();
//     echo "</table>";
// 	echo "</td></tr></table>\n";
	
	if ( strstr($globals["app.logkey"], "error.print") !=NULL ) {
		$usePrio=5;
		if ($prio>0) $usePrio=$prio;
		$this->logxMeta( "ERROR", $usePrio, $FUNCNAME, $_SERVER['PHP_SELF'], $this->getAllAsText($error_stack, "[/ERR]") );
	}

    return true;
  }

  /**
   * filters the errors out which match certain conditions
   *
   * filters the errors out which match given conditions
   * and returnes a new array which contains references to the {@link SingleError} objects in the error-stack
   *
   * @param string origin of error
   * @param int    if of errror
   * @access private
   * @return array reference to containing references to the {@link SingleError} objects in the error-stack
   */
  function &_get_sub_array_($origin, $error_id = 0)
  {
      $checking_method = ($error_id != 0) ? 'error_is_of_origin_and_id' : 'error_is_of_origin';
      $ret             = array();
	  

      foreach ($this->_error_stack as $error_pos => $s_error) {
          if ($s_error->$checking_method($origin, $error_id))
              $ret[] = &$this->_error_stack[$error_pos]; // make sure we get the original object, not a copy
      }
      return $ret;
  }
  
  /**
   * switch logx-action on/off
   *
   * @param boolean   0,1
   */
  function doLog( $flag ) {
	  $this->_logxFlag = $flag;
  }
  
  /**
   * log is active ???
   * @return bool 0,1
   */
  function isLog() {
	  return ( $this->_logxFlag );
  }
  
   /**
   * - write message to the GLOBAL error-file
   * - replace NEWLINE by [LF] to keep the message in one line
   * @param string $messType  type of message
   * 			"INFO", 	just info
   * 			"WARN", 	warning
   * 			"INTERROR", internal error !!!
   * 			"ERROR", 	normal error
   * 			"USERERROR" user error
   * @param int $messPrio   priority of message: 1 - high, 10 - low
   * @param string $message the free message
   * @global string $globals["app.logfile"]
   * @format DATE, IP, DB_serial, USER, $messType, MessID, $message
   */
  function logx( $messType,  $messPrio,  $message ) {
  	global $globals; // need to be global here, because on initial Login-errors ...
  	
  	if (is_array($globals)) $tmp_globals = &$globals;
  	else {
  		$tmp_globals = &$_SESSION['globals'];
  	}
  	
  	$MAX_LENGTH = 2000;
	
	if ( $tmp_globals["app.logfile"]=="" ) return;
	if ( !$this->_logxFlag ) return;
	
	$filename = $tmp_globals["app.logfile"];
	
	if (!($_filePtr = @fopen($filename, "a"))) {
        return -1;
    } 
	
	if ( $message !=NULL ) {
		// remove linefeeds !
		$message = str_replace( array("\r\n", "\n","\r"), "[LF]", $message);
	}
	if ( strlen($message)>$MAX_LENGTH ) {
		$oldlen = strlen($message);
		$message = substr($message,0,$MAX_LENGTH).'... !!!MESSAGE_CUT: original-length:'.$oldlen.' chars';
	}
	
	$info = date('Y-m-d H:i:s')."\t".$_SERVER['REMOTE_ADDR'].
			"\t".$_SESSION["globals"]["magasin_serial"].
			"\t".$_SESSION['sec']['appuser']."\t".$messType.
			"\t".$messPrio."\t".$message."\n";
	
	fwrite($_filePtr, $info);
	
	fclose($_filePtr);
	
	return 0;
	
  }
  
  /**
   * save an infolog event
   * @param string $func
   * @param string $text
   */
  function infolog_set($func,$text) {
  	$this->_infolog_stack[] = array('func'=>$func, 'text'=>$text);
  }
  
  /**
   * get all infolog events
   */
  function infolog_getall() {
  	return $this->_infolog_stack;
  }
} 

