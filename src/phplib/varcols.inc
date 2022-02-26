<?php
/**
 * Class for accessing EXTRA_OBJ data
 *
 * @author Michael Howitz <mac@clondiag.com>
 * @version 1.2
 */

/**
 * database access
 */
require_once('db_access.inc');

/**
 * validation of data which is to be written to database
 */
require_once('validate.inc');

/**
 * all database operations on varcols (extra-objects)
 *
 * This class allowes you to comfortably access the varcols (extra-objects).
 * Varcols should only get accessed through this class. (necessary for easy change of datarepresentation)
 *
 * <b>This class expects the varcol-cache filled and up to date!</b>
 *
 * <b><i>I want to encourage you to use id's whereerver possible.</i></b>
 * (This class and also the cache is optimized for the usage of id's.)
 * 
 * @package Varcols
 * @author Michael Howitz <mac@clondiag.com>
 * @version 1.2
 * @since version 1.1: changes so that class conformes singleton design pattern
 * @version $Header: trunk/src/phplib/varcols.inc 59 2018-11-21 09:04:09Z $
 */
class Varcols
{
    /**
     * database handle
     * @access private
     * @var object instance of subclass of CDBAbstract (database handle)
     */
    var $_dbh;
    
    /**
     * handle to the varcol-cache-array
     * @access private
     * @var array
     */
    var $_vc;
    
    /**
     * handle to the error-handler
     * @access private
     * @var object instance of {@link ErrorHandler}
     */
    var $_error;

    /**
     * creates the instance 
     *
     * You should never instanciate this class by yourself, use {@link Varcols::get()} to get a handle of the instance
     *
     * Does not open the connection to the database, for this use {@link Varcols::open()}
     *
     * @access private
     */
    function __construct() {
        global $_s_i_varcol;

        $this->_dbh   = false;
        $this->_vc    = &$_s_i_varcol;
        $this->_error = & ErrorHandler::get();
    }

    /**
     * open connection to database
     *
     * opens the connection to the database. this method is called before using the class (done using {@link Varcols::get()})
     *
     * @access private
     * @param string user name on database
     * @param string password for user on database
     * @param string name of the database
     * @param type of the database (i.e. name of the class to handle database access)
     * @return void
     * @throws varcols 1 can not open DB-connection
     */
    function open($user, $passwd, $db, $_dbtype)
    {
        $this->_dbh = logon_to_db($user, $passwd, $db, $_dbtype);
        if ($this->_error->got())
            $this->_error->set('varcols', 1, 'Can\'t open a connection to the Database. Try to relogin.');
    }

    /**
     * get handle to only instance of this class
     *
     * static method!
     * call using <code>$varcol = & Varcols::get();</code>
     *
     * @return object handle to instance of this class
     * @access public
     * @since 1.1
     */
    public static function &get()
    {
        static $theVarcols = NULL; // only set first time called

        if (is_null($theVarcols)) {
            $error = & ErrorHandler::get();
            $theVarcols = new Varcols();
            $theVarcols->open($_SESSION['sec']['dbuser'], $_SESSION['sec']['passwd'], $_SESSION['sec']['db'], $_SESSION['sec']['_dbtype']);
            if ($error->got()) { // this should never happen
                $error->printLast();
                die();
            }
        }
        return $theVarcols;
    }

    /**
     * convert name of class to extra_class_id
     *
     * Convert the name of a class to the extra_class_id.
     *
     * @param string name of the table the class belonges to
     * @param string name of the class
     * @return int extra_class_id of the class
     * @access public
     * @throws varcols 11 class_name does not exist for specified table
     * @throws varcols  3 programming error: wrong param type
     */
    function class_name_to_id($table_name, $class_name)
    {
        if (!$this->_test_param('class_name_to_id', 'table_name', $table_name, 'scalar0')) return;
        if (!$this->_test_param('class_name_to_id', 'class_name', $class_name, 'scalar0')) return;

        foreach ($this->_vc as $e_class_id => $e_class_data) {
            if (($e_class_data['name'] == $class_name) and ($e_class_data['table_name'] == $table_name))
                return (int)$e_class_id; // extra_class_id
        }
        $this->_error->set('varcols', 11, 'Class &quot;'.$class_name.'&quot; does not exist for table &quot;'.$table_name.'&quot;.');
    }

    
    /**
     * convert name of attribute to extra_attrib_id
     *
     * converts the name of an attribute of a class to its database identifier
     *
     * @param string   name of the attribute which is to be converted
     * @param int  identifier of the class the attribute belonges to (Eventually get it from {@link Varcols::class_name_to_id()})
     * @return int extra_attrib_id of the attribute
     * @access public
     * @throws varcols 10 not existing extra_class_id
     * @throws varcols 12 not existing attribute
     * @throws varcols  3 programming error: wrong parameter datatype
     */
    function attrib_name_to_id($attrib_name, $extra_class_id)
    {
        if (!$this->_test_param('attrib_name_to_id', 'attrib_name', $attrib_name, 'scalar0')) return;
        if (!$this->_test_param('attrib_name_to_id', 'extra_class_id', $extra_class_id, 'scalar')) return;
        if (!$this->class_id_exists($extra_class_id)) return;

        foreach ($this->_vc[$extra_class_id] as $e_attrib_id => $e_attrib_data) {
            if (is_array($e_attrib_data) && ($e_attrib_data['name'] == $attrib_name))
                return (int)$e_attrib_id;
        }
        $this->_error->set('varcols', 12, 'Attribute &quot;'.$attrib_name.'&quot; does not exist in class with id='.$extra_class_id.'.');
    }
	
	/**
     * convert name of attribute to name_col (like N01 or S07)
     * @created 20050121 qbi
     * @param string   name of the attribute which is to be converted
     * @param int      identifier of the class the attribute belonges to (Eventually get it from {@link Varcols::class_name_to_id()})
     * @return int     map_col of the attribute
     * @access public
     * @throws varcols 10 not existing extra_class_id
     * @throws varcols 12 not existing attribute
     * @throws varcols  3 programming error: wrong parameter datatype
     */
    function attrib_name_to_mapcol($attrib_name, $extra_class_id)
    {
        if (!$this->_test_param('attrib_name_to_id', 'attrib_name', $attrib_name, 'scalar0')) return;
        if (!$this->_test_param('attrib_name_to_id', 'extra_class_id', $extra_class_id, 'scalar')) return;
        if (!$this->class_id_exists($extra_class_id)) return;

        foreach ($this->_vc[$extra_class_id] as $e_attrib_id => $e_attrib_data) {
            if (is_array($e_attrib_data) && ($e_attrib_data['name'] == $attrib_name))
                return $e_attrib_data['map_col'];
        }
        $this->_error->set('varcols', 12, 'Attribute &quot;'.$attrib_name.'&quot; does not exist in class with id='.$extra_class_id.'.');
    }

    /**
     * convert name of attribute to its nice_name
     *
     * converts the name of an attribute of a class to its nice_name
     * @param string  name of the attribute which is to be converted
     * @param int identifier of the class the attribute belonges to (Eventually get it from {@link Varcols::class_name_to_id()})
     * @return string nice_name of the attribute
     * @access public
     * @throws varcols 10 not existing extra_class_id
     * @throws varcols 12 not existing attribute
     * @throws varcols  3 programming error: wrong parameter datatype
     */
    function attrib_name_to_nice_name($attrib_name, $extra_class_id)
    {
        if (!$this->_test_param('attrib_name_to_nice_name', 'attrib_name', $attrib_name, 'string')) return;
        if (!$this->_test_param('attrib_name_to_nice_name', 'extra_class_id', $extra_class_id, 'int')) return;
        if (!$this->class_id_exists($extra_class_id)) return;

        foreach ($this->_vc[$extra_class_id] as $e_attrib_id => $e_attrib_data) {
            if (is_array($e_attrib_data) && ($e_attrib_data['name'] == $attrib_name))
                return $e_attrib_data["nice_name"];
        }
        $this->_error->set('varcols', 12, 'Attribute &quot;'.$attrib_name.'&quot; does not exist in class with id='.$extra_class_id.'.');
    }

    /**
     * test if an extra_class_id exists
     *
     * @param int id of an extra_class
     * @return bool true: if id exists; false: not existing id or not integer-id
     * @access public
     * @throws varcols 10 not existing extra_class_id (thrown on return == false)
     * @since 1.1
     */
    function class_id_exists($extra_class_id)
    {
        $ret = (is_scalar($extra_class_id) and isset($this->_vc[$extra_class_id]) and is_array($this->_vc[$extra_class_id]));
        if (!$ret)
            $this->_error->set('varcols', 10, 'Class with id='.$extra_class_id.' does not exist.');
        return $ret;
    }

    /**
     * test if an extra_attrib_id exists
     *
     * If false is returned also one of the exceptions is thrown
     *
     * @param int id of an extra_attrib
     * @return bool true: if id exists; false: not existing id or not integer-id
     * @access public
     * @throws varcols 10 not existing extra_class_id
     * @throws varcols 12 attrib_id not existing for specified extra_class_id 
     * @since 1.1
     */
    function attrib_id_exists($extra_class_id, $extra_attrib_id)
    {
        if (!$this->class_id_exists($extra_class_id)) return false;

        $ret = (is_scalar($extra_attrib_id) and isset($this->_vc[$extra_class_id][$extra_attrib_id]) and is_array($this->_vc[$extra_class_id][$extra_attrib_id]));
        if (!$ret)
            $this->_error->set('varcols', 12, 'Attribute with id='.$extra_attrib_id.' does not exist for class with id ='.$extra_class_id.'.');
        return $ret;
    }

    /**
     * create empty extra_object
     *
     * Instanciates extra_class (defined by id), set all values set to NULL.
     *
     * @param int  id of the extra_class to be used for the extra_object
     * @return int id of created extra_object
     * @access public
     * @throws varcols 105 error in creating empty extra-object
     * @throws varcols  10 extra_class_id does not exist
     * @throws varcols   3 programming error: wrong parameter datatype
     * @see Varcols::create_empty_extra_obj_by_name()
     */
    function create_empty_extra_obj_by_id($extra_class_id)
    {
        if (!$this->_test_param('create_empty_extra_obj_by_id', 'extra_class_id', $extra_class_id, 'scalar')) return;
        if (!$this->class_id_exists($extra_class_id)) return;

        $eobj_id  = $this->_dbh->QueryInsert("EXTRA_OBJ", '(extra_class_id) VALUES ('.$extra_class_id.')', 'extra_obj_id');
        
        if ($this->_error->got(CCT_ERROR_READONLY, 'db_access')) {
            $this->rollback();
            return $this->_error->set('varcols', 105, 'Error in creating empty extra-object (Varcols::create_empty_extra_obj_by_id()) 1');
        }

        return (int)$eobj_id;
    }

    /**
     * create empty extra_object
     *
     * Instanciates extra_class (defined by name), set all values set to NULL.
     *
     * @param string name of the table where the extra_class belonges to
     * @param string name of the extra_class from wich the extra_object is to be instanciated
     * @return int id of created extra_object
     * @access public
     * @throws varcols 105 error in creating empty extra-object
     * @throws varcols  11 class_name does not exist for specified table
     * @throws (varcols  10 extra_class_id does not exist) <i>should never get thrown</i>
     * @throws varcols   3 programming error: wrong parameter datatype
     * @see Varcols::create_empty_extra_obj_by_id()
     */
    function create_empty_extra_obj_by_name($table_name, $extra_class_name)
    {
        $e_class_id = $this->class_name_to_id ($table_name, $extra_class_name);
        if ($this->_error->got(CCT_ERROR_READONLY, 'varcols'))
            return;

        return $this->create_empty_extra_obj_by_id ($e_class_id);
    }

    /**
     * delete extra_object
     *
     * This method has to be called after setting extra_obj_id in its mother-object to NULL.
     * If mother-object gets deleted extra-object is automatically deleted by trigger.
     *
     * @param int id of extra_obj for delete
     * @access public
     * @return void
     * @throws varcols 13 can not delete extra_object
     * @throws varcols  3 programming error: wrong parameter datatype
     */
    function delete($extra_obj_id)
    {
        if (!$this->_test_param('delete', 'extra_obj_id', $extra_obj_id, 'scalar')) return;
        
        $this->_dbh->query('DELETE FROM extra_obj WHERE extra_obj_id = '.$this->_dbh->addQuotes($extra_obj_id));
        if ($this->_error->got(CCT_ERROR, 'db_access'))
            $this->_error->set('varcols', 13, 'Can\'t delete extra_object with id='.$extra_obj_id.' (Maybe still referenced by a mother-table).');
    }

    /**
     * convert extra_class_id to nice_name
     *
     * converts the extra_class_id to the nice_name of the extra_class
     * @param int id of class
     * @return string nice_name of class
     * @access public
     * @throws varcols  10 extra_class_id does not exist
     * @throws varcols   3 programming error: wrong parameter datatype
     */
    function class_id_to_nice_name($extra_class_id)
    {
        if (!$this->_test_param('class_id_to_nice_name', 'extra_class_id', $extra_class_id, 'scalar')) return;
        if (!$this->class_id_exists($extra_class_id)) return;

        return $this->_vc[$extra_class_id]['nice_name'];
    }

    /**
     * convert extra_class_id to class_name
     *
     * converts the extra_class_id to the class_name of the extra_class
     * @param int id of class
     * @return string name of class
     * @access public
     * @throws varcols  10 extra_class_id does not exist
     * @throws varcols   3 programming error: wrong parameter datatype
     */
    function class_id_to_name($extra_class_id)
    {
        if (!$this->_test_param('class_id_to_name', 'extra_class_id', $extra_class_id, 'scalar')) return;
        if (!$this->class_id_exists($extra_class_id)) return;

        return $this->_vc[$extra_class_id]['name'];
    }


    /**
     * convert extra_obj_id to extra_class_id
     *
     * converts an extra_object_id to database identfier of the extra_class
     * @param int id of the extra_obj
     * @return int id of the class to which the object belonges
     * @access public
     * @throws varcols 106 error in converting object_id to class_id
     * @throws varcols  14 extra_object does not exist
     * @throws varcols   3 programming error: wrong parameter datatype
     */
    function obj_id_to_class_id($extra_obj_id)
    {
        if (!$this->_test_param('obj_id_to_class_id', 'extra_obj_id', $extra_obj_id, 'scalar')) return;
    
        $this->_dbh->query('SELECT extra_class_id FROM extra_obj WHERE extra_obj_id = '.$this->_dbh->addQuotes($extra_obj_id));
        if ($this->_error->got(CCT_ERROR, 'db_access')) {
            $this->_error->set('varcols', 106, 'Error in converting object_id to class_id; function: obj_id_to_class_id');
        } elseif ($this->_dbh->ReadRow()) {
            return (int)$this->_dbh->RowData[0];
        } else {
            $this->_error->set('varcols', 14, 'An extra_object with the id='.$extra_obj_id.' does not exist.');
        }
    }

    /**
     * convert extra_obj_id to nice_name of its extra_class
     *
     * converts an extra_object_id to the nice_name of its extra_class
     * @param int id of the extra_obj
     * @return string nice_name of extra_class of given extra_obj
     * @access public
     * @throws varcols 106 error in converting object_id to class_id
     * @throws varcols  14 extra_object does not exist
     * @throws (varcols  10 extra_class_id does not exist) <i>should never get thrown</i>
     * @throws varcols   3 programming error: wrong parameter datatype
     */
    function obj_id_to_class_nice_name($extra_obj_id)
    {
        if (!$this->_test_param('obj_id_to_class_nice_name', 'extra_obj_id', $extra_obj_id, 'scalar')) return;

        $e_class_id = $this->obj_id_to_class_id($extra_obj_id);
        if (!$this->_error->got(CCT_ERROR_READONLY, 'varcols'))
            return $this->class_id_to_nice_name($e_class_id);
    }

    /**
     * convert extra_obj_id to nice_name of its extra_class
     *
     * converts an extra_object_id to the nice_name of its extra_class
     * @param int id of the extra_obj
     * @return string nice_name of extra_class of given extra_obj
     * @access public
     * @throws varcols 106 error in converting object_id to class_id
     * @throws varcols  14 extra_object does not exist
     * @throws (varcols  10 extra_class_id does not exist) <i>should never get thrown</i>
     * @throws varcols   3 programming error: wrong parameter datatype
     */
    function obj_id_to_class_name($extra_obj_id)
    {
        if (!$this->_test_param('obj_id_to_class_name', 'extra_obj_id', $extra_obj_id, 'scalar')) return;
        
        $e_class_id = $this->obj_id_to_class_id($extra_obj_id);
        if (!$this->_error->got(READONLY))
            return $this->class_id_to_name ($e_class_id);
    }

    /**
     * updates attributes extra_object
     *
     * Updates the attributes (specified by their id's) of the extra_object.
     * To set an attribute to NULL use '' or null as value in array of second parameter
     * Please note: this method uses <tt>attrib_id</tt>. For using names look at <tt>update_by_name</tt>.
     *
     * @access public
     * @param int   id of the extra_obj
     * @param array array(attrib_id1 => value1, ...), empty array for no changes
     * @return void
     * @throws varcols 107 update at extra_val failed, doing rollback
     * @throws varcols 106 error in converting object_id to class_id
     * @throws varcols  14 extra_object does not exist
     * @throws varcols  12 attrib does not exist for specified class_id
     * @throws varcols   3 programming error: wrong parameter datatype
     * @throws validate  1 unknown native datatype
     * @throws validate 10 data in column not of needed type
     * @throws validate 11 data in column not matching regular expression
     * @throws validate 12 data in column longer than allowed
     * @throws validate 13 data in column out of range
     * @throws (validate 31 class with extra_class_id does not exist) <i>should never get thrown</i>
     * @throws (validate 32 attribute does not belong to class) <i>should never get thrown</i>
     * @throws (validate 33 app data type of extra attribute not defined) <i>should never get thrown</i>
     * @see Varcols::update_by_name()
     */
    function update($extra_obj_id, $new_values)
    {
        if (!$this->_test_param('update', 'extra_obj_id', $extra_obj_id, 'scalar')) return;
        if (!$this->_test_param('update', 'new_values', $new_values, '!scalar')) return;

        $extra_class_id = $this->obj_id_to_class_id($extra_obj_id);
        if ($this->_error->got(CCT_ERROR_READONLY, 'varcols')) return;

        foreach($new_values as $attrib_id => $dummy) {
            if (!$this->_test_param('update', 'new_values['.$attrib_id.']', $attrib_id, 'scalar')) return;
            if (!$this->attrib_id_exists($extra_class_id, $attrib_id)) return;
        }
        
        $validLib = new fValidate();
        $validLib->check_extra_values ($extra_class_id, $new_values);
        if ($this->_error->got(CCT_ERROR_READONLY, 'validate')) return;

        $set_query = '';

        foreach($new_values as $attrib_id => $value) {
            $e_val_col = $this->_vc[$extra_class_id][$attrib_id]['map_col'];
            if (strlen($value)) {
                /* Piet, 14 Jan 02:  
                   Here, five cases can occur:
                   1)  $value = ''     ## a empty string
                   2)  $value = 'blub' ## a string
                   3)  $value = 0      ## a 'empty' number
                   4)  $value != 0     ## a 'none-zero' number
                   5)  $value undefined
                   Only case 1) and case 5) should be set to NULL.
                   strlen($value) does the trick and distinguishes these cases correctly.
                */
                $set_query .= $e_val_col.' = '.$this->_dbh->addQuotes($value).', ';
            } else {
                $set_query .= $e_val_col.' = NULL, ';
            }
        }
        $set_query = substr($set_query, 0, -2); // remove last ', ' (substr returnes bool(false) when $set_query is empty)

        if ($set_query) { // is something to change
            $this->_dbh->query('UPDATE extra_obj SET '.$set_query.' WHERE extra_obj_id = '.$extra_obj_id);
            if ($this->_error->got(CCT_ERROR, 'db_access')) {
                return $this->_error->set('varcol', 107, 'Update of class-data failed.');
            }
        }
    }


    /**
     * updates attributes extra_object
     *
     * Updates the attributes (specified by their names) of the extra_object.
     * To set an attribute to NULL use '' or null as value in array of second parameter.
     * Please note: this method uses <tt>attrib_name</tt>. For using names look at <tt>update</tt>.
     *
     * @access public
     * @param int   id of the extra_obj
     * @param array array('attrib_name1' => value1, ...), empty array for no changes
     * @return void
     * @throws varcols 107 update at extra_val failed, doing rollback
     * @throws varcols 106 error in converting object_id to class_id
     * @throws varcols  14 extra_object does not exist
     * @throws varcols  12 attrib does not exist for specified class_id
     * @throws (varcols  10 extra_class_id does not exist)  <i>should never get thrown</i>
     * @throws varcols   3 programming error: wrong parameter datatype
     * @throws validate  1 unknown native datatype
     * @throws validate 10 data in column not of needed type
     * @throws validate 11 data in column not matching regular expression
     * @throws validate 12 data in column longer than allowed
     * @throws validate 13 data in column out of range
     * @throws (validate 31 class with extra_class_id does not exist) <i>should never get thrown</i>
     * @throws (validate 32 attribute does not belong to class) <i>should never get thrown</i>
     * @throws (validate 33 app data type of extra attribute not defined) <i>should never get thrown</i>
     * @see Varcols::update()
     */
    function update_by_name($extra_obj_id, $new_values)
    {
        if (!$this->_test_param('update_by_name', 'extra_obj_id', $extra_obj_id, 'scalar')) return;
        if (!$this->_test_param('update_by_name', 'new_values', $new_values, '!scalar')) return;

        $extra_class_id = $this->obj_id_to_class_id($extra_obj_id);
        if ($this->_error->got(CCT_ERROR_READONLY, 'varcols')) return;

        $id_values = array();

        foreach($new_values as $attrib_name => $val) {
            $attrib_id = $this->attrib_name_to_id ($attrib_name, $extra_class_id);
            if ($this->_error->got(CCT_ERROR_READONLY, 'varcols')) return;
            $id_values[$attrib_id] = $val;
        }
        $this->update($extra_obj_id, $id_values);
    }

    /**
     * create extra_object filled with values (using attrib_id)
     *
     * Creates an extra_object and inserts the attribute values.
     * To set an attribute to NULL use '' or null as value in array of second parameter or leave it out completely.
     * Please note: this method uses <tt>attrib_id</tt>. For using names look at <tt>insert_by_name</tt>.
     * On error nothing gets inserted & transaction gets rolled back.
     *
     * @access public
     * @param int   id of the extra_class
     * @param array array(attrib_id1 => value1, ...), empty array or NULL for class with no attributes or for all attributes set to NULL
     * @return int id of created extra_obj
     * @throws varcols 107 update at extra_val failed, doing rollback
     * @throws varcols 106 error in converting object_id to class_id
     * @throws varcols 105 error in creating empty extra-object; function: create_empty_extra_obj_by_id
     * @throws (varcols  14 extra_object does not exist) <i>should never get thrown</i>
     * @throws varcols  12 attrib does not exist for specified class_id
     * @throws varcols  10 extra_class_id does not exist
     * @throws varcols   3 programming error: wrong parameter datatype
     * @throws validate  1 unknown native datatype
     * @throws validate 10 data in column not of needed type
     * @throws validate 11 data in column not matching regular expression
     * @throws validate 12 data in column longer than allowed
     * @throws validate 13 data in column out of range
     * @throws (validate 31 class with extra_class_id does not exist) <i>should never get thrown</i>
     * @throws (validate 32 attribute does not belong to class) <i>should never get thrown</i>
     * @throws (validate 33 app data type of extra attribute not defined) <i>should never get thrown</i>
     * @see Varcols::insert_by_name()
     */
    function insert($extra_class_id, $values)
    {
        // input parameters are checked in called methods

        $old_autocommit = $this->SetAutoCommit(false);
        $eobj_id        = $this->create_empty_extra_obj_by_id($extra_class_id);

        if ($this->_error->got(CCT_ERROR_READONLY, 'varcols')) {
            
            $this->rollback();
            $eobj_id = null;
            
        } elseif (!empty($values)) {
           
            $this->update($eobj_id, $values);
            if ($this->_error->got(CCT_ERROR_READONLY)) { // there may be 'varcols' and 'validate' errors
                $this->rollback();
                $eobj_id = null;
            }
        }
                
        if ($old_autocommit && $eobj_id) // commit only when autocommit was on & no error
            $this->commit();
        $this->SetAutoCommit($old_autocommit);

        
        return $eobj_id;
    }

    /**
     * create extra_object filled with values (using attrib_name)
     *
     * Creates an extra_object and inserts the attribute values.
     * To set an attribute to NULL use '' or null as value in array of second parameter or leave it out completely.
     * Please note: this method uses <tt>attrib_name</tt>. For using names look at <tt>insert</tt>.
     * On error nothing gets inserted & transaction gets rolled back.
     *
     * @access public
     * @param int   id of the extra_obj
     * @param array array('attrib_name1' => value1, ...), empty array or NULL for class with no attributes or for all attributes set to NULL
     * @return int id of created extra_obj
     * @throws varcols 107 update at extra_val failed, doing rollback
     * @throws varcols 106 error in converting object_id to class_id
     * @throws varcols 105 error in creating empty extra-object
     * @throws (varcols 14 extra_object does not exist) <i>should never get thrown</i>
     * @throws varcols  12 attrib does not exist for specified class_id
     * @throws varcols  11 extra_class_id does not exist
     * @throws (varcols 10 extra_class_id does not exist)  <i>should never get thrown</i>
     * @throws varcols   3 programming error: wrong parameter datatype
     * @throws validate  1 unknown native datatype
     * @throws validate 10 data in column not of needed type
     * @throws validate 11 data in column not matching regular expression
     * @throws validate 12 data in column longer than allowed
     * @throws validate 13 data in column out of range
     * @throws (validate 31 class with extra_class_id does not exist) <i>should never get thrown</i>
     * @throws (validate 32 attribute does not belong to class) <i>should never get thrown</i>
     * @throws (validate 33 app data type of extra attribute not defined) <i>should never get thrown</i>
     * @see Varcols::insert()
     */
    function insert_by_name($table_name, $class_name, $values)
    {
        // input parameters are checked in called methods
        $old_autocommit = $this->SetAutoCommit(false);
        $eobj_id        = $this->create_empty_extra_obj_by_name($table_name, $class_name);

        if ($this->_error->got(CCT_ERROR_READONLY, 'varcols')) {
            $this->rollback();
            $eobj_id = null;
        } elseif (!empty($values)) {
            $this->update_by_name($eobj_id, $values);
            if ($this->_error->got(CCT_ERROR_READONLY)) { // there may be 'varcols' and 'validate' errors
                $this->rollback();
                $eobj_id = null;
            }
        }
        if ($old_autocommit && $eobj_id) // commit only when autocommit was on & no error
            $this->commit();
        $this->SetAutoCommit($old_autocommit);
        return $eobj_id;
    }

    /**
     * get data of extra_object (with attrib_id's)
     *
     * Selects the attributes of the given extra_object.
     * In the returned array you find attrib_id's as indexes.
     *
     * @access public
     * @see Varcols::select_by_name()
     * @param int id of extra_obj for getting data
     * @return array array('extra_class_id' => $class_id, 'values' => array(extra_attrib.id => attribute_value, ...)); values are ordered by extra_attrib.pos
     * @throws varcols 108 select failed in Varcols::select_by_id()
     * @throws varcols 106 error in converting object_id to class_id; function: obj_id_to_class_id
     * @throws varcols  14 extra_object does not exist
     * @throws varcols   3 programming error: wrong parameter datatype
     */
    function select_by_id($extra_obj_id)
    {
        if (!$this->_test_param('select_by_id', 'extra_obj_id', $extra_obj_id, 'scalar')) return;

        $vals     = array();
        $cols     = array();
        $class_id = $this->obj_id_to_class_id($extra_obj_id);
        if ($this->_error->got(CCT_ERROR_READONLY, 'varcols')) return;
        
        foreach($this->_vc[$class_id] as $attrib_id => $attrib_data) {
            if (!is_array($attrib_data)) continue;
            $cols[$attrib_id] = $attrib_data['map_col'];
        }
        
        if (!empty($cols)) {
            $this->_dbh->query('SELECT '.implode(', ', $cols).' FROM extra_obj WHERE extra_obj_id = '.$extra_obj_id);
            if ($this->_error->got(CCT_ERROR, 'db_access')) {
                return $this->_error->set('varcols', 108, 'Select failed in Varcols::select_by_id()');
            }
            $this->_dbh->ReadArray();
            foreach($cols as $extra_attrib_id => $extra_obj_col) {
                $vals[$extra_attrib_id] = $this->_dbh->RowData[$extra_obj_col];
            }
        }
        return array('extra_class_id' => $class_id, 'values' => &$vals);
    }

    /**
     * get data of extra_object (with attrib_names)
     *
     * Selects the attributes of the given extra_object.
     * In the returned array you find attrib_names as indexes.
     *
     * @access public
     * @see Varcols::select_by_id()
     * @param int id of extra_obj for getting data
     * @return array array('extra_class_id' => $class_id, 'values' => array(extra_attrib.name => attribute_value, ...)); values are ordered by extra_attrib.pos
     * @throws varcols 108 select failed in Varcols::select_by_name()
     * @throws varcols 106 error in converting object_id to class_id; function: obj_id_to_class_id
     * @throws varcols  14 extra_object does not exist
     * @throws varcols   3 programming error: wrong parameter datatype
     */
    function select_by_name($extra_obj_id)
    {

        if (!$this->_test_param('select_by_name', 'extra_obj_id', $extra_obj_id, 'scalar')) return;

        $vals     = array();
        $cols     = array();
        $class_id = $this->obj_id_to_class_id($extra_obj_id);
        if ($this->_error->got(CCT_ERROR_READONLY, 'varcols')) return;
        
        foreach($this->_vc[$class_id] as $attrib_id => $attrib_data) {
            if (!is_array($attrib_data)) continue;
            $cols[$attrib_data['name']] = $attrib_data['map_col'];
        }

        if (!empty($cols)) {
            $this->_dbh->query('SELECT '.implode(', ', $cols).' FROM extra_obj WHERE extra_obj_id = '.$extra_obj_id);
            if ($this->_error->got(CCT_ERROR, 'db_access')) {
                return $this->_error->set('varcols', 108, 'Select failed in Varcols::select_by_name()');
            }
            $this->_dbh->ReadArray();
            foreach($cols as $extra_attrib_name => $extra_obj_col) {
                $vals[$extra_attrib_name] = $this->_dbh->RowData[$extra_obj_col];
            }
        }
        return array('extra_class_id' => $class_id, 'values' => &$vals);
    }

    /**
     * get data of extra_object (with attrib_nice_names)
     *
     * Selects the attributes of the given extra_object.
     * In the returned array you find attrib_nice_names as indexes.
     *
     * @access public
     * @see Varcols::select_by_id()
     * @param int id of extra_obj for getting data
     * @return array array('extra_class_id' => $class_id, 'values' => array(extra_attrib.nice_name => attribute_value, ...)); values are ordered by extra_attrib.pos
     * @throws varcols 108 select failed in Varcols::select_by_nice_name()
     * @throws varcols 106 error in converting object_id to class_id; function: obj_id_to_class_id
     * @throws varcols  14 extra_object does not exist
     * @throws varcols   3 programming error: wrong parameter datatype
     * @since version 1.2
     */
    function select_by_nice_name($extra_obj_id)
    {

        if (!$this->_test_param('select_by_name', 'extra_obj_id', $extra_obj_id, 'scalar')) return;

        $vals     = array();
        $cols     = array();
        $class_id = $this->obj_id_to_class_id($extra_obj_id);
        if ($this->_error->got(CCT_ERROR_READONLY, 'varcols')) return;
        
        foreach($this->_vc[$class_id] as $attrib_id => $attrib_data) {
            if (!is_array($attrib_data)) continue;
            $cols[$attrib_data['nice_name']] = $attrib_data['map_col'];
        }

        if (!empty($cols)) {
            $this->_dbh->query('SELECT '.implode(', ', $cols).' FROM extra_obj WHERE extra_obj_id = '.$extra_obj_id);
            if ($this->_error->got(CCT_ERROR, 'db_access')) {
                return $this->_error->set('varcols', 108, 'Select failed in Varcols::select_by_nice_name()');
            }
            $this->_dbh->ReadArray();
            foreach($cols as $extra_attrib_name => $extra_obj_col) {
                $vals[$extra_attrib_name] = $this->_dbh->RowData[$extra_obj_col];
            }
        }
        return array('extra_class_id' => $class_id, 'values' => &$vals);
    }


    /**
     * get nice_names of classes for table
     *
     * Selects all nice_names of classes which are defined for a table.
     * Order in returned array: random.
     *
     * @access public
     * @param  string name of the table
     * @return array empty array if no classes defined for table or table not existing; else: array(extra_class_id => nice_name)
     * @throws varcols   3 programming error: wrong parameter datatype
     */
    function get_class_nice_names($table_name)
    {
        if (!$this->_test_param('get_class_nice_names', 'table_name', $table_name, 'scalar0')) return;

        $ret = array();

        foreach ($this->_vc as $class_id => $class_data) {
            if ($class_data['table_name'] === $table_name)
                $ret[$class_id] = $class_data['nice_name'];
        }  
        asort($ret);  // sort by name
        return $ret;
    }

    /**
     * get names of classes for table
     *
     * Selects all names of classes which are defined for a table.
     * Order in returned array: random.
     *
     * @access public
     * @param  string name of the table
     * @return array empty array if no classes defined for table or table not existing; else: array(extra_class_id => name)
     * @throws varcols   3 programming error: wrong parameter datatype
     */
    function get_class_names($table_name)
    {
        if (!$this->_test_param('get_class_names', 'table_name', $table_name, 'scalar0')) return;
        
        $ret = array();

        foreach ($this->_vc as $class_id => $class_data) {
            if ($class_data['table_name'] === $table_name)
                $ret[$class_id] = $class_data['name'];
        }
        return $ret;
    }

    /**
     * get names of attributes of a class
     *
     * get a list of all names of the attributes of a class
     *
     * @return array array assigning attrib_id to attrib_name (ordered by extra_attrib.pos) or empty array if class has no attributes
     * @param int id of class those attributes are needed
     * @access public
     * @throws varcols 10 extra_class_id does not exist
     * @throws varcols  3 programming error: wrong parameter datatype
     */
    function get_attrib_names($extra_class_id)
    {
        if (!$this->_test_param('get_attrib_names', 'extra_class_id', $extra_class_id, 'scalar')) return;
        if (!$this->class_id_exists($extra_class_id)) return;
        
        $ret = array();
        
        foreach($this->_vc[$extra_class_id] as $attrib_id => $attrib_data) {
            if (!is_array($attrib_data))
                continue;
            $ret[$attrib_id] = $attrib_data['name'];
        }
        return $ret;
    }


    /**
     * get nice_names of attributes of a class
     *
     * get a list of all nice_names of the attributes of a class
     *
     * @return array array assigning attrib_id to attrib_nice_name (if nice_name is NULL, name is used) (ordered by extra_attrib.pos) or empty array if class has no attributes
     * @param int id of class those attributes are needed
     * @access public
     * @throws varcols 10 extra_class_id does not exist
     * @throws varcols  3 programming error: wrong parameter datatype
     */
    function get_attrib_nice_names($extra_class_id)
    {
        if (!$this->_test_param('get_attrib_nice_names', 'extra_class_id', $extra_class_id, 'scalar')) return;
        if (!$this->class_id_exists($extra_class_id)) return;

        $ret = array();
        
        foreach($this->_vc[$extra_class_id] as $attrib_id => $attrib_data) {
            if (!is_array($attrib_data))
                continue;
            $ret[$attrib_id] = $attrib_data['nice_name'];
        }
        return $ret;
    }

    /**
     * convert name of class to nice_name
     *
     * Converts the name of a class to its nice_name.
     *
     * @param string name of the table the class belonges to
     * @param string name of the class
     * @return string nice_name of the class
     * @access public
     * @throws varcols 11 class_name does not exist for specified table
     * @throws varcols  3 programming error: wrong param type
     */
    function class_name_to_nice_name($table_name, $class_name)
    {
        if (!$this->_test_param('class_name_to_nice_name', 'table_name', $table_name, 'scalar0')) return;
        if (!$this->_test_param('class_name_to_nice_name', 'class_name', $class_name, 'scalar0')) return;

        foreach ($this->_vc as $e_class_id => $e_class_data) {
            if (($e_class_data['name'] === $class_name) && ($e_class_data['table_name'] === $table_name))
                return $e_class_data['nice_name'];
        }
        $this->_error->set('varcols', 11, 'Class \'$class_name\' does not exist for table \'$table_name\'.');
    }

    /**
     * get sql-string to select all objects of a class
     *
     * returnes sql-string for selecting the identifiers (extra_obj_id) of all extra_objects of an extra_class
     * 
     * @access public
     * @param int id of the class
     * @return string ansi-sql-string to select all objects which are instances of the class
     * @throws varcols 10 extra_class_id does not exist
     * @throws varcols  3 programming error: wrong param type
     */
    function sql_get_objs_of_class($extra_class_id)
    {
        if (!$this->_test_param('sql_get_objs_of_class', 'extra_class_id', $extra_class_id, 'scalar')) return;
        if (!$this->class_id_exists($extra_class_id)) return;
        
        return 'SELECT extra_obj_id FROM extra_obj WHERE extra_class_id = '.$extra_class_id;
    }

    /**
     * sql-string for selecting the identifiers of the classes of extra_objects which are in a sub-select-list
     * @access public
     * @return string
     */
    function sql_get_classes_of_objs()
    { // not checked through automatical tests!
        return 'SELECT extra_class_id FROM extra_obj WHERE extra_obj_id IN ';
    }
	
	function sql_get_fromWhereAdd( $classid ) {
		$sqlJoin = ' join EXTRA_OBJ o ON x.EXTRA_OBJ_ID = o.EXTRA_OBJ_ID';
		$addWhere= 'o.EXTRA_CLASS_ID='.$classid;
		return array('j'=>$sqlJoin, 'w'=>$addWhere);
	}
	
    /**
     * generate parts of sql-string to select list of extra_objects by values
     *
     * generates parts of an sql-string to select a list of extra_objects by their values
     *
     * @access public
     * @param array array(extra_attrib_id => value ...), use '' as value to check for 'IS NULL'
     * @param strinf name of the master-table inside the sql-query (alias possible!) where the extra-object belonges to
     * @return array array('from' => add_to_from-clause, 'where' => add_to_where-clause) (hopefull ANSI-SQL)
     * @throws varcols 15 extra_attrib_id does not exist
     */
    function sql_search_object_by_values($id_val_pairs, $bo_tab_name)
    { // not really checked through automatical tests! (only error cases, not good result)
        $ret      = array();
        $e        = &$this->_error;
        $where1   = '';
        $from     = 'extra_obj eo';
        $class_id = null;
        $where2   = $bo_tab_name ? $bo_tab_name.".extra_obj_id = eo.extra_obj_id\n AND " : '';

        if (!$this->_test_param('sql_search_object_by_values', 'bo_tab_name', $bo_tab_name, 'scalar0')) return;
	
        if (empty($id_val_pairs) or !is_array($id_val_pairs))
            return array('from' => '', 'where' => '');

        foreach ($id_val_pairs as $attrib_id => $attrib_val) {
            $class_id  = $this->attrib_id_to_class_id($attrib_id);
            if ($this->_error->got(CCT_ERROR_READONLY, 'varcols'))
                return;
            $map_col = $this->_vc[$class_id][$attrib_id]['map_col'];
            $where1  .= 'eo.'.$map_col;

            if ($attrib_val == '')
                $where1 .= ' IS NULL';
            else 
                $where1 .= ' = '.$this->_dbh->addQuotes($attrib_val);
            
            $where1 .= "\n AND ";
        }
        if ($class_id)
            $where1 = 'eo.extra_class_id = '.$class_id. "\n AND ".$where1;
        $where1 = substr($where1, 0, strlen($where1)-6); // remove last 'and '
        $ret['from'] = $from;
        $ret['where'] = $where2 . $where1;
        return $ret;
    }
    
    /**
     * generate parts of sql-string to select list of extra_objects by (attrib-name => values)
     * - convert attrib names to IDs
     * @param array $val_pairs
     * @param string $bo_tab_name
     */
    function sql_search_object_by_an_vals($key_vals, $bo_tab_name, $extra_class_id) {
        $id_val_pairs=array();
        foreach($key_vals as $attr_name => $value) {
            $attr_id = $this->attrib_name_to_id($attr_name, $extra_class_id);
            $id_val_pairs[$attr_id] = $value;
        } 
        return $this->sql_search_object_by_values($id_val_pairs, $bo_tab_name);
    }

    /**
     * convert id of attribute to class_id
     *
     * converts the id of an attribute to its class identifier
     *
     * @access public
     * @param int id of attribute
     * @return int id of the class where attrib belonges to
     * @throws varcols 15 extra_attrib_id does not exist
     * @throws varcols  3 programming error: wrong param type
     */
    function attrib_id_to_class_id($attrib_id)
    {
        if (!$this->_test_param('attrib_id_to_class_id', 'attrib_id', $attrib_id, 'scalar')) return;
        
        foreach($this->_vc as $class_id => $class_data) {
            if (array_key_exists($attrib_id, $class_data))
                return $class_id;
        }
        $this->_error->set('varcols', 15, 'Extra_attrib with id='.$attrib_id.' does not exist.');
    }

    /**
     * Create empty extra_value if they do NOT exist.
     *
     * @access public
     * @param mixed
     * @return int 0
     * @deprecated since version 1.2, because it is not needed any more
     */
    function update_extra_val_after_adding_extra_attrib($extra_attrib_id)
    {
        return 0;
    }


    /**
     * Commit transaction
     *
     * only necessary with auto-commit == false
     * @access public
     * @return bool telling of success of attempt
     */
    function commit()
    {
        return (bool)$this->_dbh->commit();
    }

    /**
     * Rollback of transaction
     *
     * only possible with auto-commit == false
     * @access public
     * @return bool telling of success of attempt
     */
    function rollback()
    {
        return (bool)$this->_dbh->rollback();
    }

    /**
     * set autocommit-state
     *
     * @param bool new autocommit-state
     * @return bool autocommit-state wich was set before
     * @access public
     * @throws varcols   3 programming error: wrong parameter datatype
     */
    function SetAutoCommit($state)
    {
        if (!$this->_test_param('SetAutoCommit', 'state', $state, 'scalar0')) return;

        return (bool)$this->_dbh->SetAutoCommit((bool)$state);
    }

    /**
     * replace db handle
     *
     * replaces current db handle (instance of a subclass of {@link CDBAbstract})
     *
     *   This function helps to handle a higher transaction level. If you have two
     *   db handles in a transaction and it is not possible that transaction data is
     *   visible you will get a problem to create a simple object.
     *
     *   this will happen normally at this situation:
     *       open transaction for db handle
     *       open transaction for varcol
     *       create var col data (varcol handle)
     *       get new extra object id
     *       create object (db handle)
     *       ==> error because extra obj id constraint violated
     *           the extra object id doesn't exist because varcol handle is not
     *           commited, but a commit has to be done later, to have it all in one
     *           transaction.
     *
     * @access public
     * @param object instance of a subclass of {@link CDBAbstract}
     * @return object current db handle
     * @throws varcols   3 programming error: wrong parameter datatype
     */
    function &replace_db_handle(&$dbh)
    {
        if (!$this->_test_param('replace_db_handle', 'dbh', $dbh, '!scalar')) return;
        if (!(is_object($dbh) and is_a($dbh, 'CDBAbstract'))) {
            return $this->_error->set('varcols', 3, 'ProgrammingError in calling Varcols::replace_db_handle() $dbh is not an instance of CDBAbstract or a subclass thereof.');
        }
        $old_dbh    = &$this->_dbh;
        $this->_dbh = &$dbh;

        return $old_dbh;
    }

    /**
     * test if a variable is of the specified type
     *
     * warning: this method does an echo when $type is out of range and then returns false
     *
     * @param string name of the method which called this method (needed for error-creation)
     * @param string name of the tested variable (needed for error-creation)
     * @param mixed  value of $$variable
     * @param string needed type of $value (allowed: scalar, scalar0, !scalar) (scalar is int, float, bool, string without 0; scalar0 is like scalar but with 0; all other are !scalar)
     * @return bool true if $value is of $type, false otherwise (+ throws error)
     * @throws varcols  3 programming error: wrong param type
     * @access private
     * @since 1.1
     */
    function _test_param($method, $variable, $value, $type)
    {
        $txt = 'ProgrammingError in calling Varcols::'.$method.'() var: '.$variable.' type:'.gettype($value);

        switch ($type) {
        case 'scalar':
            if (is_scalar($value)) {
                if ($value == 0) {
                    $this->_error->set('varcols', 1, $txt.' is 0 (zero).');
                    return false;
                } else { // scalar & not zero
                    return true; // param okay
                }
            } else { // not scalar or NULL
                $this->_error->set('varcols', 2, $txt.'(type ='.gettype($value).') is not scalar.');
                return false;
            }
        case 'scalar0':
            if (is_scalar($value)) {
                return true;
            } else { // not scalar or NULL
                $this->_error->set('varcols', 3, $txt.'(type ='.gettype($value).') is not scalar.');
                return false;
            }
        case '!scalar':
            if ($ret = is_scalar($value)) {
                $this->_error->set('varcols', 4, $txt.'(type ='.gettype($value).') is scalar, non-scalar type expected.');
            }
            return !$ret;
        default:
            echo '_test_param called from ',$method,' for ',$variable,' with wrong type (',$type,')';
            $this->_error->set('varcols', 5, $txt.'(type ='.gettype($value).') has wrong parameter $type in calling Varcols::_test_param().');
            return false;
        }
        return false; // never reached
    }

    /**
     * get the comment of a class
     *
     * @access public
     * @param int id of class
     * @return string comment of class
     * @throws varcols 10 not existing extra_class_id
     * @throws varcols  3 programming error: wrong param type
     * @since 1.1
     */
    function get_class_comment($class_id)
    {
        if (!$this->_test_param('get_class_comment', 'class_id', $class_id, 'scalar')) return;
        if (!$this->class_id_exists($class_id)) return;

        return $this->_vc[$class_id]['comment'];
        
    }

    /**
     * get the comment of an attribute
     *
     * @access public
     * @param int id of class
     * @param int id of attribute
     * @return string comment of attrib
     * @throws varcols 12 attrib_id not existing for specified extra_class_id 
     * @throws varcols 10 not existing extra_class_id
     * @throws varcols  3 programming error: wrong param type
     * @since 1.1
     */
    function get_attrib_comment($class_id, $attrib_id)
    {
        if (!$this->_test_param('get_class_comment', 'class_id', $class_id, 'scalar')) return;
        if (!$this->_test_param('get_class_comment', 'attrib_id', $attrib_id, 'scalar')) return;
        if (!$this->attrib_id_exists($class_id, $attrib_id)) return;

        return $this->_vc[$class_id][$attrib_id]['comment'];
    }

    /**
     * get nice_name of an attribute
     *
     * If extra_attrib.nice_name is NULL, extra_attrib.name is returned. 
     *
     * @access public
     * @param int id of class
     * @param int id of attribute
     * @return string nice_name of attrib
     * @throws varcols 12 attrib_id not existing for specified extra_class_id 
     * @throws varcols 10 not existing extra_class_id
     * @throws varcols  3 programming error: wrong param type
     * @since 1.1
     */
    function attrib_id_to_nice_name($class_id, $attrib_id)
    {
        if (!$this->_test_param('attrib_id_to_nice_name', 'class_id', $class_id, 'scalar')) return;
        if (!$this->_test_param('attrib_id_to_nice_name', 'attrib_id', $attrib_id, 'scalar')) return;
        if (!$this->attrib_id_exists($class_id, $attrib_id)) return;

        return $this->_vc[$class_id][$attrib_id]['nice_name'];
    }

    /**
     * get name of an attribute
     *
     * @access public
     * @param int id of class
     * @param int id of attribute
     * @return string name of attrib
     * @throws varcols 12 attrib_id not existing for specified extra_class_id 
     * @throws varcols 10 not existing extra_class_id
     * @throws varcols  3 programming error: wrong param type
     * @since 1.1
     */
    function attrib_id_to_name($class_id, $attrib_id)
    {
        if (!$this->_test_param('attrib_id_to_name', 'class_id', $class_id, 'scalar')) return;
        if (!$this->_test_param('attrib_id_to_name', 'attrib_id', $attrib_id, 'scalar')) return;
        if (!$this->attrib_id_exists($class_id, $attrib_id)) return;

        return $this->_vc[$class_id][$attrib_id]['name'];
    }
    
    /**
     * get all CLASS-IDs of TABLE
     * @param string $table_name
     * @return array of IDs
     */
    function all_classes_of_t(string $table_name) {
        $classes=array();
        $sqlsel= "EXTRA_CLASS_ID from EXTRA_CLASS where TABLE_NAME='".$table_name."' order by EXTRA_CLASS_ID";
        $this->_dbh->Quesel($sqlsel);
        while ($this->_dbh->ReadRow() ) {
            $classes[] =  $this->_dbh->RowData[0];
        }
        return $classes;
    }

} // end of class

