<?php
require_once('gui/glob.xfunc_abs.inc');

/**
 * 
 * @author skube
 * @package obj.cct_table.xfunc.inc
 */
class cCCT_TABLE extends xfunc_ABS {
  var $ident = 'CCT_TABLE';
  var $xcolarr = array( array('def_cols' => 'defined columns'),
                        array('notes'    => 'notes')
                        ); // xtended column array 
  

  
  function xColDataGet (&$sql, $colname, $idarr ) {  // get extra information
	
    $output = "";

	if ( $colname == "def_cols" ) {
    
        $sql->query('SELECT COUNT(1) FROM cct_col_view WHERE table_name = '.$sql->addQuotes($idarr[0]));
        $existing_cols = $sql->ReadRow() ? $sql->RowData[0] : 0;

        $sql->query('SELECT COUNT(1) FROM cct_column WHERE table_name = '.$sql->addQuotes($idarr[0]));
        $defined_cols = $sql->ReadRow() ? $sql->RowData[0] : 0;

        if (($existing_cols != $defined_cols) or ($defined_cols == 0))
            $output =  '<font color="#ff0000">'.$defined_cols.'/'.$existing_cols.'</font>';
        else
            $output =  $defined_cols.'/'.$existing_cols;
    } 
    if ( $colname == "notes" ) {
        $sql->query("SELECT COMMENTS FROM cct_tab_view WHERE NAME = '".$idarr[0]."'");
        $sql->ReadRow();
        $output =  $sql->RowData[0];
    }
    return ($output);
  }
}
