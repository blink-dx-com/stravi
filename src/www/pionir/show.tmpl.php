<?php
/**
 * - perform pure SQL-command
 * - this tool should also work:
 * 	 - if the application is NOT fully initialized
 *   - a least the user must be logged in ( logon_to_db() must work )
 * - export result as:
 * 		- html : html-table
 * 		- csv  : CSV format as out stream
 * 		- csv  : CSV format as download
 * - for user!="root": only SELECT allowed
 * $Header: trunk/src/www/pionir/show.tmpl.php 59 2018-11-21 09:04:09Z $
 * @package show.tmpl.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param string $sqls the SQL command
 * 		  string $outmode = "html", "csv", "csvTmp"
 *		  array $parx  ['count'] = 0,1 - count elements before showing ?
 */

// extract($_REQUEST); 
session_start(); 


require_once ("reqnormal.inc"); 
require_once ("down_up_load.inc");

require_once ('f.workdir.inc');
require_once ('func_form.inc');
require_once ('f.progressBar.inc');
require_once ('f.debug.inc');

class gSqlClient {

var $format; // 'html' or 'csv'
	
function __construct($sqls, $outmode, $parx) {
	
	if (!isset($sqls)) $sqls = '';
	$sqls = trim ($sqls);
	if ( $outmode=="" ) $outmode="html";
	
	$formatDef = array('html'=>'html', 'csv'=>'csv', 'csvTmp'=>'csv', 'hex'=>'hex');
	$this->format = $formatDef[$outmode];
	
	$this->sqls = $sqls;
	$this->outmode = $outmode;
	$this->parx = $parx;
	
	$this->LF_pattern = $this->parx['LF_pattern'];
	if ($this->sqls==NULL) $this->LF_pattern = '[LF]';
	 
}

public function line_out($text) {
    if ($this->outmode=='csv') return;
    echo $text."\n";
}

function startPage($sqlo) {
	
	$outmode = $this->outmode;
	
	$tmp_filename   = "SqlClient.txt";
	$this->flushLib = NULL;

	if ($outmode=="csv") {
		set_mime_type("application/octet-stream", $tmp_filename);
	} else {
 
		$this->flushLib = new fProgressBar( );
		$infoarr			   = array();
		$infoarr['title']      = 'SQL Hardcore';
		$infoarr['form_type']  = 'tool'; // 'tool', 'list'
		$infoarr['design']     = 'norm';
		$infoarr['css']        = $this->flushLib->getCss();
		$infoarr['javascript'] = $this->flushLib->getJS(); 
		$infoarr['locrow']   = array(  // defines the tool location-path
		    array('rootsubs/rootFuncs.php', 'Administration' )
		); 
		$pagelib = new gHtmlHead();
		$pagelib->startPage($sqlo, $infoarr);

	}
}
	
function form() {
	?>
    &nbsp;
	[<a href="<?php echo $_SERVER['PHP_SELF']?>">Reset form and show help</a>]
    <br /><br />
    <form action="show.tmpl.php" method="post"> 
     
      SQL&gt;<br>
      <textarea name="sqls" rows="10" cols="100"><?php echo $this->sqls ?></textarea>
      <br>
      <input type="submit" value="submit" class="yButton"> &nbsp;&nbsp;&nbsp; 
	<?
	$tmpsel = array("html"=>"html", "csv"=>"save as csv", "csvTmp"=>"save as CSV on Server (fast!)", 'hex'=>'Debug: Str2Hex (hexview of data');
	$seltext = formc::selectFget( 
		"outmode", 	  // FORM-variable-name
		$tmpsel, 		  // array ( ID => "nice name")
		"" // can be a singel STRING or when $option["multiple"] => array ("keyword1" => 1, "keyword2" => 1, ...) 
		); 
	echo 'format: '.$seltext;

	echo '&nbsp;&nbsp;<input type=checkbox name="parx[count]" value="1" checked> precount';
	
	
	echo '&nbsp;&nbsp;LF_pattern: <input type=text name="parx[LF_pattern]" '.
		 ' value="'.$this->LF_pattern .'" size=5 >';
	
    echo "</form>\n<br>\n";
	
	if ($this->sqls==NULL ) {
		htmlInfoBox( "Short help", "", "open", "HELP" );
		?>
		<ul>
			<li>Tipp: Statements for Oracle must <em>not</em> end with a semicolon (;) !!! </li>
	        <li>Restriction: if the user is NOT "root": only the command 'SELECT' is allowed!</li>
			<li>CSV-format: LINEFEED '\n' will be replaced by pattern <b>LF_pattern</b></li>
			<li>SQL-Example: select * from exp where exp_id < 1000</li>
	      
		</ul>
		<?
		htmlInfoBox( "", "", "close" );
	}
	echo "<br>";
	
 
	
}
 
function _errorout($message) {
      echo "<br><br>\n";
      htmlErrorBox("Error", $message );
      htmlFoot();
}

function print_row($arr, $isHeader=NULL ) {
  $format = $this->format;
  
  if ( $format=="html" ) {    
    if ( $isHeader ) $bgcolor="bgcolor=#D0D0D0";
    echo "  <tr $bgcolor>\n";
    foreach( $arr as $th0=>$th1) { 
        echo "    <td><pre>" . htmlentities( $th1 ) . "</pre></td>\n";
    }
    echo "  </tr>\n";
    return;
  } 
  
  if ( $format=="hex" ) {
  	$this->dRow = new fDebugC();
    if ( $isHeader ) $bgcolor="bgcolor=#D0D0D0";
    echo "  <tr $bgcolor>\n";
    foreach( $arr as $th0=>$th1) { 
        echo "    <td>".htmlentities($th1)."::<pre>" . $this->dRow->str2Hexinfo($th1) . "</pre></td>\n";
    }
    echo "  </tr>\n";
    return;
  } 
  
  if ( $format=="csv" ) {  
	 $retstr = NULL;
     foreach( $arr as $valRaw) {
     	if ( strstr($valRaw, "\n") != NULL and $this->LF_pattern!=NULL ) {
     		// replace Linefeeds, to support CSV format
     		$valOut = str_replace("\n", $this->LF_pattern, $valRaw); 
			$this->lineFeedsRep=1;
     	} else $valOut = $valRaw;
        $retstr .= $valOut."\t";
     }
     $retstr .= "\n";
	 
	 if ( $this->outmode=="csvTmp" ) {
		 $retVal = fputs( $this->fpout, $retstr ); 
	 } else echo $retstr;
	 return;
  }
}      

function _checkSelect( $sqls ) {
	$answer = 0;
	if ( ($tmppos = strpos($sqls, " ")) !== FALSE ) {   
		$tmp_str = substr($sqls,0,$tmppos);  // $sqls was TRIMMED, now should contain SELECT
		if ( strtoupper($tmp_str) != "SELECT" ) {
			$answer = -2;
		} else $answer = 1;
     } else {
         $answer = -1;
     }
	return ($answer);
}

function _getCount( &$sqlo, $sqls ) {
	global $error;
	$FUNCNAME= '_getCount';
	
	$fromAfter = stristr($sqls, 'from'); 
	if ($fromAfter=='') return;
	 
	$sqlsel = 'count (1) '.$fromAfter;
	$answer = $sqlo->Quesel($sqlsel);
	if ($error->Got(READONLY))  {
		$error->set( $FUNCNAME, 1, 'Error during ROW-count' );
		return;
	}
	$sqlo->ReadRow();
	$cnt = $sqlo->RowData[0];
	return ($cnt);
}

/**
 * the main export loop
 * @return 
 * @param object $sql
 * @global string $outmode
 * @global array $parx
 */
function doit( &$sql ) {
	global  $error;
	
	$parx    = $this->parx;
	$outmode = $this->outmode;
	$format  = $this->format;
	$sqls    = $this->sqls;
	
	$this->lineFeedsRep = 0;
	
	if ( $_SESSION['sec']['appuser'] != "root") {     
        // other user tests
        // only "select is allowed !!! 
        $answer = $this->_checkSelect( $sqls );
        if ($answer==-2) {
			 $this->_errorout("only 'select' command allowed for NON-Admins.");
		}
		if ($answer==-1) {
			 $this->_errorout("expecting at least one WHITE_SPACE in the SQL-command");
		} 
    }

 	// WHY did this? : $sqls = str_replace ("\\", "", $sqls );
	$cnt = 0;
    //     echo "SQL-statement: ". htmlentities($sqls) ."<P>";
	ob_end_flush ();
	
	$objcnt = 0; // default
	// get number of results ...
	$docnt  = 0;
	$isSelect = $this->_checkSelect( $sqls );		
	if ($isSelect==1 and $this->parx['count']>0 ) {
		$docnt  = 1;
		$objcnt = $this->_getCount($sql, $sqls);
		if (  $error->printAll() ) {
			$error->reset();
		}
	} 
	
	if ($docnt>0) {
	    $this->line_out("<font color=gray>Expected number of rows:</font> <b>".$objcnt."</b><br>");
	} 
	
	if ( $outmode=="csvTmp" ) {
		$workdirObj = new workDir();
		$subDir = "SqlClient";
		$tmpdir = $workdirObj->getWorkDir ( $subDir );
		if ($error->Got(READONLY))  {
			$error->printAll();
		}
		
		$fileName= 'SQL_result.dat';
		$fileout = $tmpdir . '/'.$fileName;
		$fileDown= $subDir . '/'.$fileName;
		echo "Tmp-file: $fileout<br>\n"; 
		echo 'Download: <b><a href="f_workfile_down.php?file='.$fileDown.'">'.$fileName.'</a></b><br>'."\n"; 
		
		$this->fpout = fopen($fileout, 'w');
		if (!$this->fpout) {
			htmlFoot("Error", "Could not open temp-file '".$fileout."' for write.");
		}
		$prgopt=NULL;
		$prgopt['objname']='rows';
		if ( $objcnt>0 ) $prgopt['maxnum'] = $objcnt;
		$this->flushLib->shoPgroBar($prgopt);
	}
	
	while (@ob_end_flush()); 
	
    if ($sql->query($sqls)) {
        if (!@$sql->ReadRow()) 
            echo "No result or insert/update successful.";
        else {
            
            if ( $format=="html" or $format=="hex") echo "<table border=1>\n";
           
            $colName_list = $sql->ColumnNames();
            
            $this->print_row( $colName_list, 1);
	
            
            if ( $format=="html" or $format=="hex") {
                echo "  <tr>\n";
                foreach( $sql->RowData as $th0=>$th1) { 
                    echo "    <th>" . $th0 . "</th>\n";
                }
                echo "  </tr>\n";
                
            }
            
            $this->print_row($sql->RowData);
            $cnt++;
			
            while($sql->ReadRow()) {
                $this->print_row( $sql->RowData );
				if ( $outmode=="csvTmp" ) {
					$this->flushLib->alivePoint($cnt);
				} else {
					if ( ($cnt/2000.0) == intval($cnt/2000.0) ) {	
						while (@ob_end_flush()); // send all buffered output
					}
				}
				$cnt++;
            }
			
            if ( $format=="html"  ) {
                echo "</table>\n";
                echo "<br><br>" . $sql->NextRowNumber . " lines selected.<br>\n";
            }
        }
    }
	
	if ( $outmode=="csvTmp" ) {
		fclose( $this->fpout );
		$this->flushLib->alivePoint($cnt, 1);
		
		if ( file_exists($fileout) ) {
			$size = filesize($fileout);
			echo 'Size of file: <b>'.$size.'</b> bytes <br>'."\n";
			if ( $this->lineFeedsRep>0) echo 'Info: Linefeeds in data were replaced by "'.$this->LF_pattern.'"<br>'."\n";
		}
	}
}

function closePage() {
	$this->line_out( "<br><br><hr>\n</body></html>");
}

}

// ------------------------
global $error;
$sql   = logon2( $_SERVER['PHP_SELF'] );
$error = & ErrorHandler::get();

$sqls    = $_REQUEST["sqls"];
$parx    = $_REQUEST["parx"];
$outmode = $_REQUEST["outmode"];

$mainlib = new gSqlClient($sqls, $outmode, $parx);
$mainlib->startPage($sql);

if ( !$_SESSION['s_suflag'] ) {
	echo "No SU_FLAG permission!<P>";
	return -1;
}
$mainlib->line_out( "<ul>" );

if ( $outmode!="csv" ) {
	$mainlib->form();
}

if ($mainlib->sqls != NULL ) {    
	$mainlib->doit( $sql );
}

$mainlib->line_out("</ul>");

$mainlib->closePage();


    

