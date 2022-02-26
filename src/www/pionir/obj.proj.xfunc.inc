<?
require_once("o.PROJ.subs.inc");
require_once('gui/glob.xfunc_abs.inc');

/**
 * class extendsions for PROJECT
 * @package obj.proj.xfunc.inc
 * @author steffen
 *
 */
class cPROJ  extends xfunc_ABS  {
var $ident="PROJ";
var $projLabView;   // save object

function __construct( ) {
    parent::__construct();
	$this->xcolarr[]=array ("PATH" => "full path", "rem"=>"show full path");
	$this->xcolarr[]=array ("numObj" => "Num of Objects", "rem"=>"number of objects in project");
	$this->projLabViewO = NULL;
	return 1;
}

function inProjShowAtt( &$sql, $prim_id, $options=NULL ) {
/* FUNCTION: show following attributes in project*/
    $tmp_optinfo ="";
    
    if ($options["owner"]) {
        $sqls="select u.nick from PROJ x, CCT_ACCESS a, DB_USER u where x.PROJ_ID=".$prim_id. " AND x.CCT_ACCESS_ID=a.CCT_ACCESS_ID".
			  " AND a.DB_USER_ID=u.DB_USER_ID";
	    $sql->query("$sqls");
	    $sql->ReadRow();
	    $name=$sql->RowData[0]; 
        if ($name!="") $tmp_optinfo .= "&nbsp;($name)";      
    }
	return ($tmp_optinfo);
}

function xColDataGet (&$sql, $colname, $idarr ) /* get extra information */
{
	global $varcol;
	$output = "";

	if ( $colname === "PATH" ) {
		$proSubLib = new cProjSubs();
		$pather    = $proSubLib->getPathArr( $sql, $idarr[0] );
		$cnt = sizeof($pather)-1;
		while ( $cnt >= 0) {
			$master_name    = $pather[$cnt][1];
			$output .= "/".$master_name;
			$cnt--;
		}
	}
	
	if ( $colname === "numObj" ) {
		$sqlsel = "count(1) from PROJ_HAS_ELEM where PROJ_ID=".$idarr[0];
		$sql->Quesel($sqlsel);
		$sql->ReadRow();
		$output = 'Obj: '.$sql->RowData[0];
		
		$sqlsel="count(1) FROM proj WHERE PRO_PROJ_ID=".$idarr[0];
		$sql->Quesel($sqlsel);
		$sql->ReadRow();
		$output .= '; Projects: '. $sql->RowData[0];
		
	}
	
	
	return ( $output );
}


}

