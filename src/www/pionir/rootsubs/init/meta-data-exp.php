<?php
/**
 * export META data
 * $Header: trunk/src/www/pionir/rootsubs/init/meta-data-exp.php 59 2018-11-21 09:04:09Z $
 * @package meta-data-exp.php
 * @author  qbi
 * @param string $out_type 
 * @param string $acct 
 * @param int $info_level : If ==0 script will produce only plain text
 * @misc use $_REQUEST
 */

extract($_REQUEST); 
session_start(); 


require_once ("reqnormal.inc");

// --------------------------------------------------- 
global $error, $varcol;

$error = & ErrorHandler::get();
$sqlo  = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();
$title = "export META data as SQL-insert";
$infoarr			 = NULL;
$infoarr["scriptID"] = "";
$infoarr["title"]    = $title;
$infoarr["form_type"]= "tool";
$infoarr["locrow"]   = array( array("i_meta_export.php", "export home") );


$info_level=1;

if (empty($out_type))   $out_type = "";
if (empty($acct))       $acct = "";
if($out_type=="btable") $info_level=2;
if($out_type=="btext")  $info_level=1;
if($out_type=="text")   $info_level=0;

if(!$info_level) header("Content-Type: text/ibz");
else {
    $pagelib = new gHtmlHead();
	$headarr = $pagelib->startPage($sqlo, $infoarr);    
	echo "<ul>";
}


function export_table($table_name, &$sql, $dummy1, $dummy2, $order)
{
    global $info_level;

    $error = & ErrorHandler::get();
    
    $sql->Query("select * from $table_name $order");
    if ($error->got()) {
        if ($info_level == 2)
            $error->printLast();
        else {
            $e = $error->getLast();
            echo "ERROR:";
            $e->printText();
        }
    }
    if ($info_level==2) echo("<table>\n");
    $i=0;
    while ($sql->ReadArray()){
        if ($info_level==2) echo("<tr>\n");
        if ($info_level==2)
            if($i++%2) echo ("<td bgcolor=\"#cccfff\">");
            else echo("<td bgcolor=\"white\">");
        
        $que=("insert into $table_name (");  // CODEINFO: insert-string allowed: used for PRINT
        $que_val="";
        while ($fields = each($sql->RowData))
            {
                $que=$que.$fields[0].", ";
                if($fields[1]!="") $que_val .= $sql->addQuotes($fields[1]).', ';
                else $que_val .= 'NULL, ';
            }
        $que=substr($que,0,strlen($que)-2);
        $que=$que.") VALUES (";
        $que_val=substr($que_val,0,strlen($que_val)-2);
        $que=$que.$que_val;
        $que=$que.");";
        if($info_level==0 || $info_level==2) $que=$que."\n";
        if($info_level==1) $que=$que."<p>\n";
        echo($que);
        if($info_level==2) echo("</td> </tr> \n");
    }
    if ($info_level==2) echo("</table>\n");
}


function export_update_table($table_name, &$sql, $pk, $pk1, $order){
    global $info_level;
    $sql->Query("select * from $table_name $order");
    if ($info_level==2) echo("<table>\n");
    $i=0;
    while ($sql->ReadArray()){
        if ($info_level==2) echo("<tr>\n");
        if ($info_level==2)
          if($i++%2) echo ("<td  bgcolor=\"#cccfff\">");
            else echo("<td  bgcolor=\"white\">");
        $que=("update $table_name set "); // CODEINFO: update-string allowed: used for PRINT
        $que_val="";
        while ($fields = each($sql->RowData))
          {
            if($fields[0]!=$pk && $fields[0]!=$pk1) {
              if($fields[1]!="") $que .= $fields[0].' = '.$sql->addQuotes($fields[1]).', ';
              }
              else {
                if($fields[0]==$pk) $pk_val=$fields[1];
                if($fields[0]==$pk1) $pk_val1=$fields[1];
              };
          }

        $que=substr($que,0,strlen($que)-2);
        $que=$que." where $pk='$pk_val'";
        if($pk1!="") $que=$que." and $pk1='$pk_val1';";
                  else $que=$que.";";

        if($info_level==0 || $info_level==2) $que=$que."\n";
        if($info_level==1) $que=$que."<p>\n";
        echo($que);
        if($info_level==2) echo("</td> </tr> \n");
    };
    if ($info_level==2) echo("</table>\n");
}

if ($acct == 1) {
  $sql = logon2( $_SERVER['PHP_SELF'] );
  
  $export_func = ($acc_type=="insert") ? 'export_table' : 'export_update_table';
  
  switch(true){
  case ($table_name === 'app_data_type' or $table_name === 'all'):
	$export_func('app_data_type', $sql, 'APP_DATA_TYPE_ID', '', '');
	if ($table_name !== 'all') break;
  case ($table_name === 'cct_table' or $table_name === 'all'):
	$export_func('cct_table', $sql,'TABLE_NAME', '', 'ORDER BY cct_table_name DESC, table_name DESC');
	if ($table_name !== 'all') break;
  case ($table_name === 'cct_column' or $table_name === 'all'):
	$export_func('cct_column', $sql, 'TABLE_NAME', 'COLUMN_NAME', 'ORDER BY table_name, column_name');
	if ($table_name !== 'all') break;
  case ($table_name === 'globals' or $table_name === 'all'):
	$export_func('globals',$sql, 'NAME','', '');
	if ($table_name !== 'all') break;
  }
} else { 
?>
       <form action="meta-data-exp.php?acct=1" method="POST">
         <table border=0>
          <tr>
           <td>
           Table name:
           </td>
           <td width="10">&nbsp;</td>
           <td align=right>
           <select name="table_name">
              <option value="all">all</option>
              <option value="app_data_type">app_data_type</option>
              <option value="cct_table">cct_table</option>
              <option value="cct_column">cct_column</option>
              <option value="globals">globals</option>
           </select>
           </td>
          </tr>
          <tr>
           <td>
           Action:
           </td>
           <td width="10">&nbsp;</td>
           <td align=right>
           <select name="acc_type">
               <option value="insert">insert</option>
               <option value="update">update</option>
           </select>
           </td>
          </tr>
          <tr>
           <td>
           Output type:
           </td>
           <td width="10">&nbsp;</td>
           <td align=right>
           <select name="out_type">
               <option value="btext">Browser - Text</option>
               <option value="btable">Browser - Table</option>
               <option value="text">File - Text</option>
           </select>
           </td>
          </tr>
          <tr>
           <td> &nbsp; </td>
           <td width="10">&nbsp;</td>
           <td align=right>
           <input type="submit" name="submit" value="Export">
           </td>
          </tr>
         </table>
       </form>
<?
}

if($info_level) {
	htmlFoot();
}

