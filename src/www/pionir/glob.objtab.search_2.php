<?
/**
 * search for objects with equal EXTRA_OBJ attributes
 * @package glob.objtab.search_2.php
 * @swreq UREQ:
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param  
 *       $tablename
         $go = "", 1, 2
  	     $extra_class_id
         $attrib_id[ATTRIB_ID]  attributes to compare
         $search_order ="", EXTRA_OBJ_ID  ( sort search ???)
 * @version $Header: trunk/src/www/pionir/glob.objtab.search_2.php 59 2018-11-21 09:04:09Z $
 */
session_start(); 

require_once ('reqnormal.inc');
require_once("f.sql_query.inc");
require_once ("visufuncs.inc");

class glob_objtab_search2_help {
	
	function __construct(&$sqlo, $tablename, $extra_class_id) {
		
		$this->tablename=$tablename;
		$this->extra_class_id=$extra_class_id;
		
		$utilLib = new fSqlQueryC($tablename);
		$utilLib->addJoin('EXTRA_OBJ');
		$utilLib->_WhereAdd('o.extra_class_id='.$this->extra_class_id, 'AND'); // only this class
		$this->sqlAfter = $utilLib->get_sql_after( );
	}
	
	
	function go_basic($sql) {
		$sqls = "SELECT name FROM extra_class WHERE extra_class_id=".$this->extra_class_id;
		$sql->query($sqls);
		$sql->readRow();
		$niceclass=$sql->RowData[0];
		if ($niceclass=="") {
			htmlFoot("Error","Need a valid extra_class_id.");
		}
		echo "Class: <B>$niceclass</B><br>";
		$this->niceclass = $niceclass;
	}
	
	function go1(&$sql) {
		
		$extra_class_id= $this->extra_class_id;
		$tablename=$this->tablename;
		
		$errcode=0;
		do {
		
			
			$sqls = "SELECT extra_attrib_id, NAME, NICE_NAME FROM extra_attrib WHERE extra_class_id=".$extra_class_id. 
			" order  by POS";
			$sql->query($sqls);
			$attrib_arr = array();
			while ($sql->readRow()) {
				$tmp_name=strtolower($sql->RowData[1]);
				if ($sql->RowData[2]!="") $tmp_name=strtolower($sql->RowData[2]);
				$attrib_arr[$sql->RowData[0]]=$tmp_name;
			}
			reset ($attrib_arr);
			if ( !sizeof($attrib_arr) ) {
				$errcode=1;
				break;
			}
		
			echo '<form method="post" name=xform action="' .$_SERVER['PHP_SELF']. '?go=2&tablename='.$tablename.'">';
			echo '<input type=hidden name="extra_class_id" value="'.$extra_class_id.'">';
			echo "<B>Select attributes to be compared:</B> <br>";
			foreach( $attrib_arr as $attrib_id=>$attrib_name) {
				echo '&nbsp;&nbsp; <input type=checkbox name="attrib_id['.$attrib_id.']" value=1 checked> ' .$attrib_name. "<br>\n";
			}
			reset ($attrib_arr);
		
			echo "<br>";
			echo '<input type=checkbox name="search_order" value="EXTRA_OBJ_ID" > Order by creation<br>';
		
		
			echo '<input type=submit value="Search">';
			echo "</form>";
			return;
		
		
		} while (0);
		
		if ($errcode)
			switch ($errcode) {
				case 1:
					echo "INFO: class '".$this->niceclass ."' has no attributes.<br>";
					return;
					break;
				default:
					echo "INFO: unknown Error!<br>";
					return;
		}
	}
	
	/**
	 * build the Super-SQL-command
	 * @param object $sql
	 * @param int $attrib_id
	 * @return multitype:string multitype:
	 */
	private function go2_build_sql(&$sql, $attrib_id, $search_order) {
		
		$extra_class_id = $this->extra_class_id;
		$tablename      = $this->tablename;
		
		$att_col =array(); // attribute col_map
		$nice_col=array();
		$sql_columns="";
		$sql_max_columns="";
		$komma = "";
		$cnt=0;
		
		foreach( $attrib_id as $tmp_attrib_id=>$flag) {
			if ($flag) {
				$sqls = "SELECT NAME, NICE_NAME, MAP_COL FROM extra_attrib WHERE extra_attrib_id=".$tmp_attrib_id;
				$sql->query($sqls);
				$sql->ReadRow();
				$tmp_name=strtolower($sql->RowData[0]);
				if ($sql->RowData[1]!="") $tmp_name=strtolower($sql->RowData[1]);
				$att_col[$cnt][0]=$tmp_name;
				$att_col[$cnt][1]=$sql->RowData[2];
		
				$sql_columns     = $sql_columns. $komma . 'o.'.$sql->RowData[2];
				$sql_max_columns = $sql_max_columns. $komma . "max(o.".$sql->RowData[2].")";
				$komma=", ";
				$nice_col[] = $tmp_name;
		
				$cnt++;
			}
		}
		
		
		// build super query
		
		$sql_super = 
		    "select max(o.extra_obj_id), count(o.".$att_col[0][1]."), " .$sql_max_columns.
		    " from ".$this->sqlAfter.	 	
		 	" group by $sql_columns having count(o.".$att_col[0][1].") > 1";
		
		// 		$sql_super = "select max(extra_obj_id), count(".$att_col[0][1]."), " .$sql_max_columns. 
		// 			" from extra_obj eo
		// 					where eo.extra_class_id = ".$extra_class_id."
		// 							group by $sql_columns having count(".$att_col[0][1].") > 1";
		
		if ( $search_order == "EXTRA_OBJ_ID") {
			$sql_super = $sql_super . " ORDER BY count(o.extra_obj_id) DESC";
		}
		
		return array('sql_super'=>$sql_super, 'att_col'=>$att_col, 'nice_col'=>$nice_col);
	}
	
	function go2(&$sql, &$sql2, $attrib_id, $search_order) {
		
		$extra_class_id = $this->extra_class_id;
		$tablename      = $this->tablename;
		
		
		$attrib_num = sizeof($attrib_id);
		if ( !$attrib_num ) {
			echo "<B>INFO:</B> You must select at least one attribute!<br>";
			return;
		}
		
		$pk_arr = primary_keys_get2($tablename);
		$first_key = $pk_arr[0];
		
		
		
		if ( $search_order == "EXTRA_OBJ_ID") {
			echo "Order by: EXTRA_OBJ_ID<br>\n";
		}
		
		
		
		$tabobj = new visufuncs();
		$headOpt = array( "title" => "Object > Attribute histogram");
		
		$answer = $this->go2_build_sql($sql, $attrib_id, $search_order);
		$sql_super = $answer['sql_super'];
		$att_col   = $answer['att_col'];
		$nice_col  = $answer['nice_col'];
		
		$headx  = array ("#", "One of the objects", "Count");
		$headx  = array_merge($headx, $nice_col);
		
		$tabobj->table_head($headx, $headOpt);
		
		// echo "DEBUG-super:  $sql_super<br>";
		
    	if ( !$sql->query($sql_super) ) {
			echo "ERRROR in query:<br>$sql_super<br>\n";
			return;
		}
		$cct_access_sql = "";
		$cct_access_has = 0;
		if ( cct_access_has2($tablename) ) $cct_access_has = 1;
		if ($cct_access_has)  $cct_access_sql = ", cct_access_id";
		
		$objcnt=0;
		$row_id=0;
		
		while ($sql->ReadRow()) {
			
			$xobj=$sql->RowData[0];
			$ocnt=$sql->RowData[1];
			$acc_str = "";
	
			$sqls = "select $first_key, name $cct_access_sql from $tablename where extra_obj_id=".$xobj;
			$sql2->query($sqls);
			$obj_id=0;
			if ($sql2->ReadRow()) {
				$obj_id  =$sql2->RowData[0];
				$obj_name=$sql2->RowData[1];
				$access_id=$sql2->RowData[2];
		
				if ($cct_access_has && $access_id) {
					$sqls = "select u.nick, a.crea_date from cct_access a, db_user u where a.cct_access_id=".$access_id. " AND a.db_user_id=u.db_user_id";
					$sql2->query($sqls);
					if ( $sql2->ReadRow() ) {
						$nick  =$sql2->RowData[0];
						$date  =$sql2->RowData[1];
						$acc_str = " $nick::$date";
					}
				}
			}
		
	        $i=0;
	        $data_col_arr=array();
	        $tmpand="";
			$sql_query="";
		
	        while ( $i<$attrib_num ) {
				if ($sql->RowData[$i+2]=="") $sql_query .= $tmpand . "eo.".$att_col[$i][1]. " is NULL";
				else    $sql_query .= $tmpand . "eo.".$att_col[$i][1]. "='" .$sql->RowData[$i+2]."'";
					$data_col_arr[] = $sql->RowData[$i+2];
					$tmpand=" AND ";
					$i++;
			}
			$sql_query=str_replace ( "%", "#", rawurlencode($sql_query) ); // $sql_query
	
			$dataArr = array(
					($row_id+1),
					"<a href=\"javascript:linkto($obj_id)\">$obj_name</a>  $acc_str",
					"<a href=\"javascript:searchlink('". $sql_query ."')\">$ocnt</a>",
			);
			
			$dataArr = array_merge($dataArr, $data_col_arr);
			
			$tabobj->table_row ($dataArr);

			$objcnt = $objcnt + $ocnt;
			$row_id++;
			
    	}
    	
    	$dataArr=array('','<b>object-SUM:</b>','<b>'.$objcnt.'</b>');
    	$tabobj->table_row ($dataArr);
    	
		$tabobj->table_close();
	}
	
}

$tablename   = $_REQUEST['tablename'];
$go          = $_REQUEST['go'];
$extra_class_id= $_REQUEST['extra_class_id'];
$attrib_id   = $_REQUEST['attrib_id'];
$search_order= $_REQUEST['search_order'];

$varcol= & Varcols::get();
$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] );
$sql2  = logon2( );

$title= 'Search for objects with equal extra attributes';

// query first part for javascript call
$pre_query = $tablename ."_ID in (select ".$tablename ."_ID from ".$tablename ." x, extra_obj eo where
eo.extra_obj_id=x.extra_obj_id AND ";   
$bracket_close = rawurlencode(')');
$pre_query_raw = rawurlencode($pre_query); 

$nicetable = tablename_nice2($tablename);

$infoarr=array();
$infoarr["title"]    = $title;
$infoarr["form_type"]= "list";
$infoarr["obj_cnt"]  = 1;
$infoarr["locrow"]   = array( array("searchAdvance.php?tablename=".$tablename, "list view" ) );
$infoarr["obj_name"] = $tablename;
$infoarr['design']   = 'norm';
$infoarr["locrow"]   = array(
			array("searchAdvance.php?tablename=".$tablename, "Search advanced"),
			array("glob.objtab.coluniq.php?tablename=".$tablename, "Analyze unique columns")
			);
			

$pagelib = new gHtmlHead();
$pagelib->_PageHead ( $infoarr["title"],  $infoarr );



?>
<script>
<!--

function linkto( obj_id ) {
    url_name="edit.tmpl.php?tablename=<?php echo $tablename ?>&id=" + obj_id;
    InfoWin = window.open( url_name, "<?php echo $tablename?>", "scrollbars=yes,status=yes,width=650,height=500");
}

function searchlink( sql_where ) {
    <?
    // the following code looks starnge, but IE does not recognize the REG_EXP condition /#/g
    ?> 
    sql_where_neu = new String(sql_where);  
    while ( sql_where_neu.search ("#") != -1 ) {   
            sql_where_neu = sql_where_neu.replace( "#", "%" );
    }  
        
    js_query = '<?php echo $pre_query_raw?>' + sql_where_neu + '<?php echo $bracket_close ?>';
    
    url_name="view.tmpl.php?t=<?php echo $tablename ?>&condclean=1&tableSCond=" + js_query;
    InfoWin = window.open( url_name, "<?php echo $tablename?>", "scrollbars=yes,status=yes,width=650,height=500");
}

//-->
</script>
<?

$retarr = $pagelib->_startBody($sql, $infoarr);

if ($tablename=="") {
    echo "<br>ERROR: give tablename.<br>";
    return;
}
if (!$retarr['obj_cnt']) {
	htmlFoot('USERERROR', 'please select objects!');
}

$helplib = new glob_objtab_search2_help($sql, $tablename, $extra_class_id);

echo " [<A HREF=\"".$_SERVER['PHP_SELF']."?tablename=$tablename\">Start again</a>]<br><br>\n";


if ( !$go ) {
   htmlFoot("Warning", "Please select a class first");
}

if (!$extra_class_id) {
	htmlFoot("Warning", "Please select a class first");
}

// -------------------------------------------


if ( $go>=1) {
    $helplib->go_basic($sql);
}

if ($go==1) { // SELECT ATTRIBUTES

	$helplib->go1($sql);
    return;

} 

if ($go==2) { // do search
	$helplib->go2($sql, $sql2, $attrib_id, $search_order);
}



htmlFoot('<hr>');
