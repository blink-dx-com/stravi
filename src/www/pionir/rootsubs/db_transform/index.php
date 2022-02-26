<?
/**
 * DB transform link index
 * $Header: trunk/src/www/pionir/rootsubs/db_transform/index.php 59 2018-11-21 09:04:09Z $
 * @package dbTransformIndex
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
extract($_REQUEST); 
session_start(); 


require_once ("reqnormal.inc");
require_once ('convertData.inc');

$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] );


$title  = "Transform DB versions";
$infoarr["title"] = $title;
$infoarr["form_type"]= "tool";
$infoarr["locrow"]   = array( array("../rootFuncs.php", "Administration") );
$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);


echo "<ul>";

if (  !glob_isAdmin()  ) {
  echo "Sorry, you must be root.";
  return 0;
}

$dbConvertObj = new dbConvertC();

$db_version     = $dbConvertObj->getDBVersion($sql);
$intern_version = $dbConvertObj->getInternVersion($sql);

echo "<table cellpadding=1 cellspacing=1 border=0>";
echo "<tr><td>Main DB-Version: &nbsp;</td><td><B>".$db_version."</B></td></tr>\n";
echo "<tr><td>Intern DB-Version: &nbsp;</td><td><B>".$intern_version."</B></td></tr>\n";
echo "</table><br>\n";

?>

<B>Main</B>
<UL><br>
<font size=+1><a href="convert.php"><B>[DB_Haeckelianer] </B>Upgrade DB-version</a></font>
</ul>                                                                   

<br>
<B>Misc</B>
<UL><br>
<li><a href="f.dbStructCrea.php">Frankenstein</a> - Database structure creator</li>
<li><a href="../../p.php?mod=DEF/root/g.table.drop">Drop tables</a></li>
<LI><a href="../o.user_group.secwri.php">DB-mods for 'security_write' mode</A></li>
<LI><a href="../access_all.php">Read-access modifications</A> (for HIGH security)</li>
<LI><a href="v1050.changeDateType.php">DateTypeChanger</a> - Change Application Data Type [date3] &gt; [date] (2010)</LI>
</UL>

<br>
<B>META table tools</B><ul>
<li> <a href="o.CCT_TABLE.subs.php">Generate entries in CCT_TABLE, CCT_COLUMN</a> (automatic)</li>
<li>CCT_TABLE &gt; <a href="o.CCT_TABLE.inttab.php">repair INTERNAL-flag</a></li>
</ul>

<br>
<B>Blink Specials</B><ul>
<li> <a href="../../p.php?mod=LAB/root/g.db_tmp_init">Create TEMP database for Experiment data</a></li>
<li> <a href="../../p.php?mod=DEF/root/install/g.postgres">Postgres-Init</a></li>

</ul>
		

		
</ul>

</body>
</html>

