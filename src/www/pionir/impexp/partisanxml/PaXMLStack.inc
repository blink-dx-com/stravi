<?

//
//  class:      PaXMLStack
//  descr:      class for remebering, which object has been touched already
//  version:    class version 0.2
//
//  2002-09-09 rogo start
//


class PaXMLStack
{
    var $db;
    var $html;

    function __construct(&$html, $user, $passwd, $db, $dbtype)
    {
        $this->db = logon_to_db($user, $passwd, $db, $dbtype);
        $this->html = $html;
    }

    function reset()
    {
        

		$stmt = "DELETE FROM t_export WHERE db_user_id = " .$_SESSION['sec']['db_user_id'];

        $this->html->statement($stmt);
		$this->db->Query($stmt);

    }
    
    
    
    function put($table, $filter)
    {
        
        
		// parameter check
		if ($table == null and !is_string($table))
			return null;

		if ($filter != null && is_array($filter))
		{
			// do it
			$keys  = array_keys($filter);
			$colstr="table_name, column_name, value, db_user_id";
			$valstr= "'" . $table . "', '" . $keys[0] . "', '" . $filter[$keys[0]] . "', '" . $_SESSION['sec']['db_user_id'] . "'";
            $stmt = "INSERTX: T_EXPORT: $valstr";
			$this->html->statement($stmt);
			$this->db->Insertx ( "T_EXPORT", $colstr, $valstr, "put" );
		}
    }
    
    
    
    
    function find($table, $filter)
    {
        

		// parameter check
		if ($table == null and !is_string($table))
			return false;

		if ($filter != null && is_array($filter))
		{
			// do it. query.
			$keys  = array_keys($filter);
			$stmt  = "SELECT table_name, column_name, value, db_user_id FROM t_export ";
			$stmt .= "WHERE db_user_id = '"  .$_SESSION['sec']['db_user_id'] . "'";
			$stmt .= " AND table_name = '" . $table . "'";
			$stmt .= " AND column_name = '" . $keys[0] . "'";
			$stmt .= " AND value = '" . $filter[$keys[0]] . "'";
            $this->html->statement($stmt);
			$this->db->Query($stmt);

            $result = $this->db->ReadRow();

			return $result;
		}
	    return false;


    }
}

?>
