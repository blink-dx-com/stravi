<?php
/**
 * Class for one single error with all its data
 *
 * This class is a wrapper for a single error with some methods to display the error on user's screen.
 * Objects of this class are returned by some methods of the {@link ErrorHandler}.
 *
 * You should not instanciate this class by yourself.
 * For reporting an error use the {@link ErrorHandler}. 
 *
 * @author Michael Howitz <mac@clondiag.com>
 * @package ErrorHandling
 * @access public
 * @version 1.0
 */
class SingleError {
    /**
     * function-name or module in which the error occured
     * @var const string
     * @access public
     */
    var $origin;
    
    /**
     * identifier of the error (depending on {@link $origin}, no global error_id!)
     * @var const string
     * @access public
     */
    var $id;

    /**
     * textual description of the error
     * @var const string
     * @access public
     */
    var $text;

    /**
     * tells if this error is a real error (warning = 0) or only a warning (1-5, 5..least important warning)
     * @var const string
     * @access public
     */
    var $warning;


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
        if ($this->id <= 0)
            $this->id = 24111977;
        $this->text    = (string)$text;
        $this->warning = (int)$warning;
        if ($this->warning < 0) $this->warning = 0;
        if ($this->warning > 5) $this->warning = 5;
    }

    /**
     * print one row of an html-table of the error-list
     * @return void
     * @access public
     */
    function printTR()
    {
        echo '<tr bgcolor="#FFFFBB" align="center">';
        echo '<td><font size="-1">',$this->origin,'</font></td>';
        echo '<td><font size="-1">',$this->id,'</font></td>';
        echo '<td><font size="-1">',($this->warning ? $this->warning : 'error'),'</font></td>';
        echo '<td align="left"><font size="-1">',$this->text,'</font></td>';
        echo '</tr>',"\n";
    } 
	
	/**
     * print one EASY row of an html-table of the error-list
     * @access public
     * @return void
     */
	function printTR2()
    {
        echo '<tr bgcolor="#FFFFBB" align="center">';
        echo '<td><font size="-1">',$this->origin,'</font></td>';
        echo '<td><font size="-1">',$this->id,'</font></td>';
        echo '<td align="left">',$this->text,'</td>';
        echo '</tr>',"\n";
    } 
    
    function printTRs() { 
        echo '<tr>';
        echo '<td>',$this->text,'</td>';
        echo '</tr>',"\n";        
    }

    /**
     * print the error as text separated by tabs, followed by newline
     * @return void
     * @access public
     */
    function printText()
    {
        echo $this->origin."\t".$this->id."\t".$this->warning."\t".$this->text."\n";
    }

    /**
     * print a whole message of the error
     * @return void
     * @access public
     */
    function printMsg() 
    {
        echo '<br>';
        if ($this->warning == 0)
            echo '<font color="#ff0000">ERROR:</font> ';
        else
            echo '<font color="#808000">WARNING:</font> (warning level ',$this->warning,' of 5 (5..lowest importance))<br>';

        echo $this->text,'<br><small>Origin: '.$this->origin.'; id: '.$this->id.'</small><br>',"\n";
    }

    /**
     * tests, if this error is of a specified origin
     *
     * helper-function for selection of errors of a certain origin
     *
     * second parameter unsed only for compatibility with method {@link error_is_of_origin_and_id}
     * 
     * @access public
     * @return bool
     * @param string function-name or module in which the error occured
     * @param int    optional and unused (only for compatibility (see above))
     */
    function error_is_of_origin($origin, $error_id = 0)
    {
        return $origin == $this->origin;
    }

    /**
     * tests, if this error is of a specified origin and error_id
     *
     * helper-function for selection of errors of a certain origin & certain error_id
     *
     * @access public
     * @return bool
     * @param string function-name or module in which the error occured
     * @param int    identifier of the error
     */
    function error_is_of_origin_and_id($origin, $error_id)
    {
        return (($origin == $this->origin) and ($error_id == $this->id));
    }

    /**
     * check, if error fullfilles given criteria of origin, id and warnlevel
     *
     * @access public
     * @param string  function-name or module in which the error occured
     * @param int     identifier of the error
     * @param int     real error? (error: 0; warning: 1-5 (5..least important warning))
     * @return bool
     */
    function error_is_of_origin_id_and_warnlevel($origin, $error_id, $warn_level)
    {
        return (($origin == $this->origin) and ($error_id == $this->id) and ($warn_level == $this->warning));
    }

    /**
     * checks if this error is equal to given one
     *
     * @access public
     * @return bool
     * @param  object other SingleError-object for compare
     */
    function equal($otherErr)
    {
        if (!is_object($otherErr))
            return false;
        
        if (!is_a($otherErr, get_class($this)))
            return false;

        return (
                ($otherErr->origin == $this->origin) &&
                ($otherErr->id == $this->id) &&
                ($otherErr->text == $this->text) &&
                ($otherErr->warning == $this->warning)
                );
    }
	
	/**
     * get error-features as array
     *
    
     */
	function getAsArray() {
	    return array( "ori"=>$this->origin, "id"=>$this->id, "text"=>$this->text, 'warn'=>$this->warning );
	}
}
