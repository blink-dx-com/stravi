<?php
/**
 * import columns 
 * @package obj.h_exp_raw_desc.import.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   $id
  	 $go
	 $datafile  - uploaded file
	 $line_txt
	 $line_val
	 $sel_cols[]
	 $startlineno [default=1]
 */
session_start(); 

		
require_once ('reqnormal.inc');
require_once ('func_form.inc');
require_once ("insertx.inc");
require_once ("o.EXP.RESULT.inc");

function this_formshow($id) {

    ?>
    <I>The file contains the column names and sample data (numbers or strings)</I>
    <I>Max file size: <B><?php echo ($_SESSION['globals']["F.ASCI_TABLE.IMPORT.UPLOAD_MAX_SIZE"]/1E6); ?></B>&nbsp;MBytes.</I><br>
    Format:
    <table border=1>
    <tr><td>Mean1</td><td>Mean2</td><td>Bg2</td></tr>
    <tr><td>5.4</td><td>2.4</td><td>7.8</td></tr>
    </table>
    <br>
    <?php
    
    $initarr   = NULL;
    $initarr["action"]      = $_SERVER['PHP_SELF'];
    $initarr["title"]       = "Upload file";
    $initarr["submittitle"] = "Upload";
    $initarr["tabwidth"]    = "AUTO";
    $initarr["ENCTYPE"] = "multipart/form-data" ;
    
    $hiddenarr = NULL;
    $hiddenarr["id"]     = $id;
    
    $formobj = new formc($initarr, $hiddenarr, 0);
    
    $fieldx = array (
        "title" => "Data file",
        "name"  => "datafile",
        "object"=> "file",
        "namex" => TRUE,
        "notes" => ""
    );
    $formobj->fieldOut( $fieldx );
    
    $fieldx = array (
        "title" => "Header line number",
        "name"  => "startlineno",
        "object"=> "text",
        "val"   => '1',
        "namex" => TRUE,
        "notes" => "give line number if header is not the first line"
    );
    $formobj->fieldOut( $fieldx );
    
    $formobj->close( TRUE );
}

global $error, $varcol;
$FUNCNAME= 'MAIN';
$error = & ErrorHandler::get();
$sql    = logon2( $_SERVER['PHP_SELF'] );
$title = "Import columns into H_EXP_RAW_DESC";

$id = $_REQUEST["id"];
$go = $_REQUEST["go"];
$datafile = $_REQUEST["datafile"];
$line_txt = $_REQUEST["line_txt"];
$line_val = $_REQUEST["line_val"];
$sel_cols = $_REQUEST["sel_cols"];
$startlineno = $_REQUEST["startlineno"];
 

$infoarr=array();
$infoarr["title"] = $title;
$infoarr["form_type"]= "obj";
$infoarr["obj_name"] = "H_EXP_RAW_DESC";
$infoarr["obj_id"] = "$id";
$infoarr["checkid"]  = 1;
$pagelib = new gHtmlHead();

$pagelib->startPage($sql, $infoarr);

$expResultLib = new gSpotResC();
$resultTable = 'EXP_RAW_RESULT';

$max_col_possible=20;
$mapped_label="";

echo "<blockquote>\n";

if ( $_SESSION['sec']['appuser']!="root" ) { // access_check()
	echo "INFO: only root can run this script!<br>";
	return;
}


$tmp_info = "<font color=red>no entry</font>";


$i=1;
$max_raw_col="";
echo "<i>This tool imports column names for type EXP_RAW_RESULT.</i><br>\n";
echo "<B>Already defined data columns:</B><br><br>\n";
echo "<table bgcolor=#EFEFEF border=0>";
echo "<tr bgcolor=#CFCFCF ><td><B>original</B></td><td><B>internal</B></td></tr>";

// echo "<tr><td>$tmp_info</td><td>SPOT_NAME</td></tr>";
			 	
while ($i<=20) {

	$col_name = "V". str_pad( $i, 2, "0", STR_PAD_LEFT );
 	$sqlsel = $expResultLib->sqlsel_map2nice($id, $resultTable, $col_name);
	$sql->Quesel($sqlsel);
 	if ( $sql->ReadRow() ) {
 		$mapped_name = $sql->RowData[0];
		 
		echo "<tr><td><font color=gray>&lt;</font>$mapped_name<font color=gray>&gt;</font></td><td>$col_name</td></tr>";
		$max_raw_col=$i;
	} 
	$i++;
}

echo "</table>\n<br>";

if ($max_raw_col) {
	
	echo "<font color=red>WARNING:</font> Some data columns are already defined.<br>\n";
	
	if ($max_raw_col>=$max_col_possible) {
		 echo "<B><font color=red>INFO:</font> Columns exceeded: you can not create more than $max_col_possible columns</B><br>\n";
		 echo "Possible solution: delete some columns or create a new entry in H_EXP_RAW_DESC.<br>";
		 return;
	} 
}


if ( !$go ) {
    
  this_formshow($id);
  return 0;
  
}

if ( $go==1 ) {
	
	$datafile_size = $_FILES['datafile']['size'];
	$datafile  = $_FILES['datafile']['tmp_name'];
	$datafile_name = $_FILES['datafile']['name'];
	$datafile_type = $_FILES['datafile']['type'];
	
	if ($datafile == "" || $datafile == "none") {
    		 echo "<br><br><br><center><font color=darkblue><b>ERROR!<b></font><br>\n";
    		 echo "You did not specify a file for upload or it is bigger than allowed. --&gt; Aborted.<br><br>";
    		 return false;
  	}
	$FH = fopen("$datafile", 'r');
	if ( !$FH ) {
	  echo "Upload failed!<P>";
	  echo " file_size:$datafile_size userfile:$datafile <br>";
	  echo " file_name:$datafile_name file_type:$datafile_type <br>";
	  return;
	}
	
	if ($startlineno>1) {
		$i=1;
		while( !feof ( $FH ) && ($i<$startlineno) ) { 
            $line = fgets($FH, 8000);
			$i++;
		}
	}
	 
    $line_txt = trim(fgets($FH, 8000));
    $line_val = trim(fgets($FH, 8000));
	
	fclose ( $FH);
	
} else {
	$line_txt = rawurldecode ( $line_txt );
	$line_val = rawurldecode ( $line_val );
}


if ( !strlen($line_txt) ) {
	echo "<B>ERROR:</B> column line is empty.<br>";
	return;
}

$datafields   = explode("\t", $line_txt);
$colFieldNum = sizeof ($datafields);

if ($colFieldNum<2) {
	echo "<B>ERROR:</B> at least two TAB-separated columns must be provided!<br>";
	echo "Content of header line:<font color=gray>$line_txt</font><br>";
	return;
}

$valfields   = explode("\t", $line_val);
$valfieldNum = sizeof ($valfields);

if ($valfieldNum != $colFieldNum) {
	echo "<B>ERROR:</B> Number of header columns  ($colFieldNum) <B>NOT EQUAL</B> to number of data columns ($valfieldNum) !<br>";
	echo "Cols:".$line_txt. " Values:".$line_val."<br>";
	return;
}


if ( $go==1 ) {
	echo "<form name='editform' method='post' action=\"".$_SERVER['PHP_SELF']."?id=$id&go=2\">";
	echo "<input type=hidden name=line_txt value=\"".rawurlencode ($line_txt)."\">";
	echo "<input type=hidden name=line_val value=\"".rawurlencode ($line_val)."\">";
	
}

echo "<B>Possible new mapping</B> (Number of header columns: $colFieldNum)<br><br>";
echo "<table bgcolor=#EFEFEF border=0>";
echo "<tr bgcolor=#CFCFCF >";
if ( $go==1) {
	echo "<td><B>map</B></td>";
}
echo "<td><B>original</B></td><td><B>internal</B></td><td><B>first data row</B></td></tr>";

$map_id= $max_raw_col + 1;
$i=1;

$insertLib = new insertC();

foreach($datafields as $th0=>$th1) {
    
	$import_possible=1;
	$ori_name=trim($th1);

	$err_txt = "";
	
	$col_name = "V". str_pad( $i, 2, "0", STR_PAD_LEFT );
	$err_map_name = $expResultLib->nice2map( $sql, $id, $resultTable, $col_name );
	
 	if ( $err_map_name!=NULL ) {
 		$import_possible=0;
		$err_txt = "<font color=red>DENIED:</font> Original column exists on <B>$err_map_name</B>";
	}
	
	
	if ( $sel_cols[$i] || ($go==1)) {
		
		 echo "<tr><td>";
		 if ( $go==1 ) {
		 	if ( $import_possible )  echo "<input type=checkbox name=sel_cols[$i] value=1 checked>";
			else echo "&nbsp;";
			echo "</td><td>";
		 }

		 echo $ori_name."</td><td>";

		if ( $import_possible && ($map_id>$max_col_possible) ) {
			$import_possible=0;
		 	$err_txt = "<font color=red>Columns exceeded</font>";
		}

		 if ( !$import_possible ) {
			 echo $err_txt;
			 $import_possible=0;
		 } else {
			 
			 $col_name = "V". str_pad( $map_id, 2, "0", STR_PAD_LEFT );
			 $map_id++;
			 echo $col_name;
		 }


		 if ( ($go == 2) && $import_possible ) {
			 

			 $valarr = array( 
			 	'H_EXP_RAW_DESC_ID'=>$id, 
			 	'MAP_COL'=>$col_name, 
				'TABLE_NAME'=>$resultTable, 
				'NAME'=>$ori_name 
				);	 
 			 $args = array('vals'=>$valarr);
			 $insertLib->new_meta($sql, "H_EXP_RAW_DESC_COL", $args);	
		
			 if ($error->Got(READONLY))  {
				$error->set( $FUNCNAME, 1, 'column '.$ori_name );
				$pagelib->chkErrStop();
			 }

		 }

		 echo "</td><td>";
		 $show_data= $valfields[$i-1];
		 echo $show_data . "&nbsp;</td>";
		 
		 echo "</tr>\n";
	}
	$i++;
}
echo "</table>\n";
echo "<br>";

if ( $go==1 ) {
	echo "<input type=submit class=\"yButton\" value=\"Create columns\">";
	echo "</form>";
}

$pagelib->htmlFoot();

