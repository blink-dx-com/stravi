<?
require_once('gui/glob.xfunc_abs.inc');

/**
 * extension for container
 * @namespace core::
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @package obj.container.xfunc.inc
 */

class cCONTAINER extends xfunc_ABS {
  var $ident="CONTAINER";
  var $xcolarr;
	
  function __construct( ) {
      parent::__construct();
  	  $this->xcolarr[]=array ("CNT(subst)" => "number of MACs", "rem" => "count the number MACs");
  	  $this->xcolarr[]=array ("Location" => "Location", "rem" => "Location of storage");
  }
  
  function inProjShowAtt( &$sqlo, $prim_id, $options=NULL ) {

      
      $tmp_optinfo = $this->get_abs_object_name($sqlo, $prim_id);
      
      return ($tmp_optinfo);
  }
  
  /**
   * - get extra information for an artificial column
   * - this will be shown e.g. in the list view
   * @param  object $sql
   * @param  string $colname artificial column name like "CNT(exp_raw_result)"
   * @param  array $idarr IDs of primary keys 
		[0] : first PK, [1] second PK, [2] third PK
   * @return -
   */
  function xColDataGet (&$sqlo, $colname, $idarr ) {
    //   intern Code-EXAMPLE:
    if ($colname=="CNT(subst)") {
		$sqlsel = "count(1) from CONT_HAS_CSUBST where CONTAINER_ID=". $idarr[0].' and CONCRETE_SUBST_ID>0';
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$output = $sqlo->RowData[0];
    }
    if ($colname=="Location") {
        $sqlsel = "c.LOCATION from CONTAINER x join CHIP_READER c on x.CHIP_READER_ID=c.CHIP_READER_ID where CONTAINER_ID=". $idarr[0];
        $sqlo->Quesel($sqlsel);
        $sqlo->ReadRow();
        $output = $sqlo->RowData[0];
    }
	return ( $output );
  
  }
}
