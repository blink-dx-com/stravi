<?
require_once('gui/glob.xfunc_abs.inc');

/**
 * 
 * extension for container
 * @namespace core::
 * @author  Steffen Kube 
 * @package obj.db_user.xfunc.inc
 */
class cDB_USER extends xfunc_ABS {
var $ident="DB_USER";

function __construct( ) {
    parent::__construct();
	$this->xcolarr[]=array ("show_fotos" => "show fotos");
	$this->xcolarr[]=array ("groups"     => "member in groups");
	$this->xcolarr[]=array ("roles"      => "has roles");
	
	return 1;
}

function xColDataGet ( &$sqlo, $colname, $idarr ) /* get extra information */
{
	require_once("f.data_dir.inc");
	$output="";
	
	if ( $colname == "show_fotos" ) {
		$id = $idarr[0];
		$pfilename = datadirC::datadir_filename( "DB_USER", $id ,"jpg" );
		if ( file_exists($pfilename) ) {
			$output="<img src=\"glob.obj.img_show.php?tablename=DB_USER&primid=".$id."&extension=jpg\" height=50>";
		}
	}
	
	if ( $colname == "groups" ) {
		$id = $idarr[0];
		$groups = array();
		$sqlsel = "g.NAME from DB_USER_IN_GROUP ug JOIN USER_GROUP g on ug.USER_GROUP_ID=g.USER_GROUP_ID ".
			      " where ug.DB_USER_ID=".$id. ' order by g.NAME';
		$sqlo->Quesel($sqlsel);
		while ( $sqlo->ReadRow() ) {
		    $groups[] = $sqlo->RowData[0];
		} 
		if (sizeof($groups)) {
			$output = implode(', ',$groups);
		}
	}
	
	if ( $colname == "roles" ) {
		$roles = array();
		$id = $idarr[0];
		$sqlsel = "g.NAME from USER_ROLES ug JOIN ROLE g on ug.ROLE_ID=g.ROLE_ID ".
			      " where ug.DB_USER_ID=".$id. ' order by g.NAME';
		$sqlo->Quesel($sqlsel);
		while ( $sqlo->ReadRow() ) {
		    $roles[] = $sqlo->RowData[0];
		} 
		if (sizeof($roles)) {
			$output = implode(', ',$roles);
		}
	}
	
	return ( $output );
}


}

