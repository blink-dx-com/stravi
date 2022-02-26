<?php
/**
 * Partisan-Frankenstein
 * - sub functions for SQL-genearation
 * $Header: trunk/src/www/pionir/rootsubs/db_transform/f.dbStructCrea.inc 59 2018-11-21 09:04:09Z $
 * @package f.dbStructCrea.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */


class fDbStructCrea {
	
var $insarr;  // SQL-commands
var $metaarr; // META-commands
var $infox;   /*
	"PK_max" : max number of primary keys
	"pkname" : PK-name
	"pks" => array(PKNUM)=colname : all PKs
	"EXTRA_OBJ_ID" : 0,1
	*/
var $noMeta; // if 1 : do produce all in SQL-code, NO META-commands

/**
 * @var array $tabStruc array( 
 * 		"cct_column"=>array(COLNAMES),
 * 		"cct_table" =>array(FEATUERS)
 * )
 */
var $tabStruc; 

function __construct() {
	
	$this->cct_cols=NULL;
	$this->cct_cols["names"] = array(
			"COLUMN_NAME", 
	 		"NICE_NAME", 
			"CCT_TABLE_NAME", 
			"APP_DATA_TYPE_ID", 
			"PRIMARY_KEY", 
			"MOST_IMP_COL", 
			"NOT_NULL",
   			"POS", 
			"comment" 
			);
	
	
	$this->noMeta = 0;
	$this->_creaActive=0;
}	

function _set_coltypes() {
    
    $this->_config=array();
    $this->_config['SEQUENCES_need'] = 1;
    
    // ORACLE
    $this->COMMENT_PREFIX='';
    $this->colTypes=array(
        "INT"   => "NUMBER",
        "STRING"=> "VARCHAR",
        "DATE"  => "DATE",
        "FLOAT" => "NUMBER",
        "DATE2" => "VARCHAR(255)",
    );
    
    if ($this->settings['db_type']=='POSTGRES') {
        
        $this->colTypes=array(
            "INT"   => "BIGINT",
            "STRING"=> "VARCHAR",
            "DATE"  => "TIMESTAMP",
            "FLOAT" => "DOUBLE PRECISION",
        );
        
        $this->COMMENT_PREFIX='E';
        $this->_config['SEQUENCES_need'] = 0;
    }
    
    
}
	
function init( $tabStruc, $settings   ) {
	$this->tabStruc = $tabStruc;
	$this->settings  = $settings; 
	
	$this->_set_coltypes();
}

function _log($text, $prio) {
	echo "<font color=gray><b>INFO:</b></font> ".$text."<br>\n";
}

function _oneColAnalyse(&$sql, $db_col, $valarr) {
	// analyse one column
	if ($db_col=="EXTRA_OBJ_ID") {
		$this->infox["EXTRA_OBJ_ID"] = 1;
	}

	if ($valarr["PRIMARY_KEY"]==1) {
		$this->infox["pkname"] = $db_col;
	}
	if ( $valarr["PRIMARY_KEY"] > $this->infox["PK_max"] ) {
		$this->infox["PK_max"] = $valarr["PRIMARY_KEY"];
	}
	if ($valarr["PRIMARY_KEY"]>0) {
		$this->infox["pks"][$valarr["PRIMARY_KEY"]] = $db_col;
	}
	
	if ($valarr["MOST_IMP_COL"]==1) {
		$this->infox["MOST_IMP_COL"] = $db_col;
	}
}

function _anaColumns(&$sql) {

	foreach( $this->tabStruc["cct_column"] as $db_col=>$valarr) {
		$this->_oneColAnalyse($sql, $db_col, $valarr);
	}
	reset ($this->tabStruc["cct_column"]); 

}

function getStruct() {
	return ($this->tabStruc);
}

/**
 * generate the first columns
 */
function _generateFirstCols() {
	/*"cct_table" => array (),
				"cct_column" => array()*/
				
	$cct_table = &$this->tabStruc["cct_table"];
	$isBo = 0;
	$isAssoc= 0;
	if ($cct_table["TABLE_TYPE"]=="BO")  $isBo = 1;
	if ($cct_table["TABLE_TYPE"]=="BO_ASSOC")  $isAssoc= 1;

	
	// the FIRST Primary KEY !!!
	$CCT_TABLE_NAME = NULL;
	$colname  = $cct_table["TABLE_NAME"]."_ID";
	if ($isAssoc) {
		$CCT_TABLE_NAME = $cct_table["CCT_TABLE_NAME"]; // reference to mother
		$colname  = $CCT_TABLE_NAME.'_ID';
	}
	
	$colAcc   = array(
			"NICE_NAME"       => NULL, // NOT needed! $this->tabStruc["cct_table"]["NICE_NAME"]. " id", 
			"CCT_TABLE_NAME"  => $CCT_TABLE_NAME, 
			"APP_DATA_TYPE_ID"=> 1, 
			"PRIMARY_KEY"	  => 1, 
			"MOST_IMP_COL"	  => 0, 
			"NOT_NULL"		  => 1, 
   			"POS"			  => 1,
			"comment"  	      => "id of ".$cct_table["NICE_NAME"]
			);
	$this->tabStruc["cct_column"][$colname] = $colAcc;
	
	if ( !$isAssoc ) {
	    
		$colname = "NAME";
		$colAcc  = array(
				"NICE_NAME"       => "name", 
				"CCT_TABLE_NAME"  => NULL, 
				"APP_DATA_TYPE_ID"=> 2, 
				"PRIMARY_KEY"	  => 0, 
				"MOST_IMP_COL"	  => 1, 
				"NOT_NULL"		  => 0, 
	   			"POS"			  => 2,
				"comment"  	      => "name of the ". $cct_table["NICE_NAME"]
				);
		$this->tabStruc["cct_column"][$colname] = $colAcc;
	}
	
	if ( $isBo ) {
		$colname = "CCT_ACCESS_ID";
		$colAcc  = array(
				"NICE_NAME"       => "cct access id", 
				"CCT_TABLE_NAME"  => "CCT_ACCESS", 
				"APP_DATA_TYPE_ID"=> 1, 
				"PRIMARY_KEY"	  => 0, 
				"MOST_IMP_COL"	  => 0, 
				"NOT_NULL"		  => 1, 
				"POS"			  => 3,
				"comment"  	      => "id of the access rights"
				);
		$this->tabStruc["cct_column"][$colname] = $colAcc;
		
		$colname = "EXTRA_OBJ_ID";
		$colAcc  = array(
				"NICE_NAME"       => "extra obj id", 
				"CCT_TABLE_NAME"  => "EXTRA_OBJ", 
				"APP_DATA_TYPE_ID"=> 1, 
				"PRIMARY_KEY"	  => 0, 
				"MOST_IMP_COL"	  => 0, 
				"NOT_NULL"		  => 0, 
				"POS"			  => 4,
				"comment"  	      => "id of the extra obj"
				);
		$this->tabStruc["cct_column"][$colname] = $colAcc;
	}
	
	if ( !$isAssoc ) {
		$colname = "NOTES";
		$colAcc  = array(
				"NICE_NAME"       => "notes", 
				"CCT_TABLE_NAME"  => NULL, 
				"APP_DATA_TYPE_ID"=> 3, 
				"PRIMARY_KEY"	  => 0, 
				"MOST_IMP_COL"	  => 0, 
				"NOT_NULL"		  => 0, 
	   			"POS"			  => 15,
				"comment"  	      => "notes"
				);
		
		$this->tabStruc["cct_column"][$colname] = $colAcc;
	}
}

function _getColType(&$sql, $id ) { 
	
	$sqls = "select * from APP_DATA_TYPE where  APP_DATA_TYPE_ID=".$id;
	$sql->query($sqls);
	$sql->ReadArray();
	$feat = $sql->RowData;
	
	$typeout="NUMBER"; 
			
	$typeout =	$this->colTypes[$feat["DATA_TYPE"]];
	if ($typeout=="VARCHAR") {
		$typeout .= "(".$feat["MAX_LENGTH"].")";
	}
	return ($typeout);
}

function _oneColumn(&$sql, $db_col, $valarr) {
	
	$coltype = $this->_getColType($sql, $valarr["APP_DATA_TYPE_ID"] );
	$notnull = $valarr["NOT_NULL"] ? " not null" : NULL;
	
	if ( !$this->_config['SEQUENCES_need'] and $valarr['PRIMARY_KEY']==1 and ($this->infox["PK_max"]==1) ) {
	    // primary key
	    $notnull = '';
	    $coltype = 'SERIAL'; 
	}
	
	$sqlcom = "  ".$db_col."   ".$coltype . $notnull.",\n";
	return ($sqlcom);
}

function _getInsertString( $colNames,$valarr, $cct_table ) {

	foreach( $colNames as $dummy=>$colname) {

		if ($colname=="comment") continue;
		if ($colname=='notes') continue; // non-used column from general form !

		$valx = $valarr[$colname];
		do {
			if ( $valx!="" and !is_numeric($valx) )  {
				$valx="'".$valx."'";
				break;
			}
			if ( $valx===NULL ) {
				$valx="NULL";
				break;
			}
			if ( $valx==="" ) {
				$valx="NULL";
				break;
			}
			if ( $valx==0 ) $valx="0";
		} while (0);
		$sqlpre .= $tmpkomma . $colname;
		$sqlval .= $tmpkomma . $valx;
		$tmpkomma=", ";
	} 
	reset ($colNames );

	$newstr = "INSERT INTO ".$cct_table." (".$sqlpre.") values (".$sqlval .")"; // CODE_INFO: insert allowed here
	return ($newstr);
}

/**
 * create $meta[] array for DB_Haeckelianer data-files
 * @param $colNames
 * @exampleOutput:
 *  array( "act"=>"insupd", "tab"=>"CCT_COLUMN", "vals"=>array( 
 *	'TABLE_NAME'=>'ABSTRACT_PROTO_STEP', 'COLUMN_NAME'=>'ABS_DEV_ID', 'NICE_NAME'=>'device (abstract)', 'CCT_TABLE_NAME'=>'A_CHIP_READER', 
 *  'APP_DATA_TYPE_ID'=>1, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>8.5, 'VISIBLE'=>1, 'EDITABLE'=>1) );
 */
function _getInsertMeta( $colNames, $valarr, $cct_table ) {

	$keyvals = NULL;
	$tmpkomma= NULL;
	foreach( $colNames as $dummy=>$colname) {

		if ($colname=="comment") continue;
		if ($colname=='notes') continue; // non-used column from general form !

		$valx = $valarr[$colname];
		do {
			if ( $valx!="" and !is_numeric($valx) )  {
				$valx="'".$valx."'";
				break;
			}
			if ( $valx===NULL ) {
				$valx="NULL";
				break;
			}
			if ( $valx==="" ) {
				$valx="NULL";
				break;
			}
			if ( $valx==0 ) $valx="0";
		} while (0);
		$keyvals .=  $tmpkomma. "'". $colname. "'=>".$valx;
		$tmpkomma =", ";
	} 
	reset ($colNames );

	$newstr = "array( 'act'=>'insupd', 'tab'=>'".$cct_table."', 'vals'=>array(".$keyvals.") )";
	return ($newstr);
}

/**
 * check len of ORACLE identifier
 * - if too long, make first key shorter and add a number (e.g. 1) ...
 */
function _checkIdentLen($key1, $key2=NULL) {
	$MAXLEN=30;
	$keyOld = $key1.$key2;
	$newkey = $keyOld;
	$lendiff = strlen($keyOld)- $MAXLEN;
	if ($lendiff>0) {
		// make first key shorter
		$key1Len = strlen($key1);
		$key1n   = substr($key1,0,$key1Len-$lendiff);
		$newkey  = $key1n.$key2;
		$this->_log('_checkIdentLen: key ('.$keyOld.') was too long; made shorter ('.$newkey.')',2);
	}
	return $newkey;
}

/**
 * one column
 */
function _oneCol_CCT_COL( &$sql, $db_col, $valarr, $noMeta=0 ) {
	
	$cct_table = &$this->tabStruc["cct_table"];
	$tablename = $cct_table["TABLE_NAME"];
	
	$colNames=NULL;
	foreach( $this->cct_cols["names"] as $dummy=>$colname) {
		$colNames[]=$colname;
	}
	reset ( $this->cct_cols["names"]  );
	
	$tmpkomma="";
	
	$addarr = array(
		"TABLE_NAME"=>$tablename,
		"VISIBLE"  => 1,
		"EDITABLE"=> 1
	);
	
	foreach( $addarr as $col=>$val) {
		$colNames[]=$col;
		$valarr[$col]=$val;
	}
	reset($addarr);
	
	$valarr["COLUMN_NAME"] = $db_col;
	unset( $colNames["comment"] ) ;
	
	if ( $noMeta>0 ) $sqlstr = $this->_getInsertString( $colNames, $valarr, 'CCT_COLUMN' );
	else {
		$sqlstr = $this->_getInsertMeta( $colNames, $valarr, 'CCT_COLUMN' );
	}
	return ($sqlstr);
}

function _newTable( &$sql ) {
	// create table
	$cct_table = &$this->tabStruc["cct_table"];
	$tablename = $cct_table["TABLE_NAME"];
	$pkname    = "";
	
	if ( $this->infox["PK_max"]>1 ) {
		$is_assoc = 1;
	} else $is_assoc = 0;
			
	$sqlcom = "create table ".$tablename."  ( \n";
	
	foreach( $this->tabStruc["cct_column"] as $db_col=>$valarr) {
		$sqlcom .= $this->_oneColumn($sql, $db_col, $valarr);
	}
	
	
	$primaryKeys = $this->infox["pkname"];
	if ( $is_assoc ) {
		$pkarr = $this->infox["pks"];
		$primaryKeys .= ', '.$pkarr[2];
		if ($pkarr[3]!=NULL) $primaryKeys .= ', '.$pkarr[3];
	}
	$sqlcom .= "  constraint PK_".$tablename." primary key (".$primaryKeys.") \n";
	$sqlcom .= ")";
	
	$this->insarr[] = $sqlcom;
}

function _newComments($sql) {
	$cct_table = &$this->tabStruc["cct_table"];
	$tablename = $cct_table["TABLE_NAME"];
	
	$this->insarr[] = "comment on table ".$tablename." is ".$this->COMMENT_PREFIX. "'".$cct_table["notes"]."'";
	
	foreach( $this->tabStruc["cct_column"] as $db_col=>$valarr) {
		$this->insarr[] = "comment on column ".$tablename.".".$db_col." is ".$this->COMMENT_PREFIX. "'".$valarr["comment"]."'";
	}
	reset ($this->tabStruc["cct_column"]); 
}

/**
 * init createSQL
 */
function _newInit(&$sql) {

	$this->_creaActive=1;
	$this->insarr=NULL;
	$this->infox=NULL;
	
	$this->_anaColumns($sql);
	
}

function _newIndices($sql) {
	// sequence
	$cct_table = &$this->tabStruc["cct_table"];
	$tablename = $cct_table["TABLE_NAME"];
	
	$this->insarr[] = "/* indices */";
	
	if ($this->infox["MOST_IMP_COL"]!="") $this->insarr[] = "create index ".$tablename."_AK2 on ".
			$tablename." (".$this->infox["MOST_IMP_COL"]." ASC)";
		
	if ($cct_table["TABLE_TYPE"]=="BO") {
		
		$insarr[] = "create index ".$tablename."_FK2 on ".$tablename." (extra_obj_id ASC)";
		$insarr[] = "create index ".$tablename."_FK3 on ".$tablename." (cct_access_id ASC)";
	}
}

function _newConstraint(&$sql) {
	$cct_table = &$this->tabStruc["cct_table"];
	$tablename   = $cct_table["TABLE_NAME"];
	$mothertable = $cct_table["CCT_TABLE_NAME"];
	$isUsed= 0;
	$fkcnt = 1;
	foreach( $this->tabStruc["cct_column"] as $db_col=>$valarr) {
		
		if ( $valarr["CCT_TABLE_NAME"]!="" ) {
			
			if ( !$isUsed) {
				$this->insarr[] = "/* constraints */";
				$isUsed=1;
			}
			$frTab = $valarr["CCT_TABLE_NAME"];
	
			$newConstrName = $this->_checkIdentLen( "FK_".$frTab, "2".$tablename );
			
			$oneConst = "alter table ".$tablename." \n".
   			" add constraint ".$newConstrName." foreign key (".$db_col.") \n".
      		" references ".$frTab." (".$db_col.")";
			
			
			$doOnDelete=0;
			if ( $cct_table['TABLE_TYPE']=='BO' and $frTab=='CCT_ACCESS' ) {
				$doOnDelete = 1; // add OnDelete for CCT_ACCESS_ID; special for PARTISAN
			}
			if ( $this->infox["PK_max"]>1 and $frTab==$mothertable) {
				$doOnDelete = 1; // constraint to mother table for ASSOC-table
			}
			
			if ($doOnDelete) {
				$oneConst .= "\n" . ' on delete cascade';
			}
			
			$this->insarr[] = $oneConst;
			
			$fkcnt++;
		}
	}
	reset ($this->tabStruc["cct_column"]); 
	
}

/**
 * create triggers
 * - no trigger if number of PKs > 1
 */
function _newTriggers(&$sql) {

	$cct_table = &$this->tabStruc["cct_table"];
	$tablename = $cct_table["TABLE_NAME"];
	$pkname    = $this->infox["pkname"];

	if ( $this->infox["PK_max"]>1 ) {
		return; // no trigger
	}
	
	if ( $this->_config['SEQUENCES_need'] ) {
    	$this->insarr[] = "/* trigger: primary-key */";
    	
    	$seqname = $tablename."_SEQ";
    	$this->insarr[] = "create sequence ".$seqname." start with 1 nocache";
    
    	$this->insarr[] = 
    		"create or replace trigger ".$tablename."_AIC \n".
    		" before insert\n".
    		" on ".$tablename."\n".
    		" for each row\n".
    		" \n".
    		" begin\n".
    		" 	IF :new.".$pkname." is null or :new.".$pkname." > 0  THEN\n".
    		" 		select ".$seqname.".nextval into :new.".$pkname." from dual;\n".
    		"   END IF;\n".
    		" end;";
    		
    	$this->insarr[] = "alter trigger ".$tablename."_AIC compile";
	}
	
	if ($this->infox["EXTRA_OBJ_ID"] ) {
	    
	    $this->insarr[] = "/* trigger: delete extra_obj */";
	    
	    if ($this->settings['db_type']=='POSTGRES') {
	        //TBD: warning $ DOLLAR !!!
	        $trigger_name=$tablename."_deo";
	        $this->insarr[] = 'CREATE FUNCTION trigger_'.$trigger_name.'() RETURNS trigger
	        LANGUAGE plpgsql
	        AS $$
	        BEGIN
	        delete from extra_obj where extra_obj_id = OLD.extra_obj_id;
	        RETURN OLD;
	        end
	        $$;
            ';
	        $this->insarr[] = 'CREATE TRIGGER '.$trigger_name.' AFTER '.
	   	        'DELETE ON '.$tablename.' FOR EACH ROW WHEN ((old.extra_obj_id > 0)) EXECUTE PROCEDURE trigger_'.$trigger_name.'();';
	        
	    } else {
	        
	        // ORACLE
	
    		
    		// extra_obj delete manage
    		$this->insarr[] = 
    			"create or replace trigger ".$tablename."_DEO \n".
    			" after delete  \n".
    			" on ".$tablename." \n".
    			" for each row \n".
    			" when (old.extra_obj_id > 0) \n".
    			" begin \n".
    			" 		delete from extra_obj where extra_obj_id = :old.extra_obj_id; \n".
    			" end;";
    		$this->insarr[] = "alter trigger  ".$tablename."_DEO compile";
	    }
	}
}

/**
 * - new CCT_TABLE and CCT_COLUMN
 * - create SQL-commands for this SYS-tables
 * @param &$sql sql-handle
 */
function _newCCT_TABLE_sql( &$sql ) {

	$cct_table = &$this->tabStruc["cct_table"];
	$tablename = $cct_table["TABLE_NAME"];
	$cctcols   = array_keys($cct_table);
	
	$insOne = $this->_getInsertString( $cctcols, $cct_table, 'CCT_TABLE' );
	$this->insarr[] = $insOne; 

	foreach( $this->tabStruc["cct_column"] as $db_col=>$valarr) {
		
		$this->insarr[] = $this->_oneCol_CCT_COL($sql, $db_col, $valarr, 1);
	}
	reset ($this->tabStruc["cct_column"]); 
}

/**
 * - new CCT_TABLE and CCT_COLUMN as META-commands
 * - create table
 * @param  &$sql sql-handle
 * @global array $this->metaarr
 */
function _newCCT_TABLE_meta( &$sql ) {

	$cct_table_arr = &$this->tabStruc["cct_table"];
	$tablename = $cct_table_arr["TABLE_NAME"];
	$cctcols   = array_keys($cct_table_arr);
	
	$insOne = $this->_getInsertMeta( $cctcols, $cct_table_arr, 'CCT_TABLE' );
	$this->metaarr[] = $insOne; 

	foreach( $this->tabStruc["cct_column"] as $db_col=>$valarr) {
		$this->metaarr[] = $this->_oneCol_CCT_COL($sql, $db_col, $valarr, 0);
	}
	reset ($this->tabStruc["cct_column"]); 
}

function _newCCT_TABLE( &$sql) {
	if ( $this->noMeta>0 ) {
		$this->_newCCT_TABLE_sql( $sql );
	} else {
		$this->_newCCT_TABLE_meta( $sql );
	}
}

function createSQL( &$sql ) {
	/* FUNCTION: create SQL-structures
			- create table
			- comments
			- index
			- triggers
	   OUTPUT: $this->insarr // SQL-commands
			   $this->infox
	*/
	$this->_newInit($sql);
	$this->_newTable($sql);
	$this->_newComments($sql);
	$this->_newIndices($sql);
	$this->_newConstraint($sql);
	$this->_newTriggers($sql);
	
	$this->_newCCT_TABLE($sql);
	
	echo "<br>";
}

function dropSQL($sql) {
/* FUNCTION: drop table
	   OUTPUT: $this->insarr // SQL-commands
			   $this->infox
*/
	$this->_newInit($sql);
	// indexes
	$cct_table = &$this->tabStruc["cct_table"];
	$tablename = $cct_table["TABLE_NAME"];
	
	$this->insarr[] = "drop trigger ".$tablename."_AIC";
	$seqname = $tablename."_SEQ";
	$this->insarr[] = "drop sequence ".$seqname;
	
	if ( $this->infox["EXTRA_OBJ_ID"] )  $this->insarr[] = "drop trigger ".$tablename."_DEO";
	$this->insarr[] = "drop table ".$tablename;
}

function show_text($key) {
	if ($key=='newline') echo "<br>\n";
} 

function show_SQL() {
	// FUNCTION: show commands for insert with db_transform
	if (!sizeof($this->insarr)) {
		return;
	}
	foreach( $this->insarr as $sqlstr) {
		$newstr = htmlspecialchars($sqlstr);
		$newstr = str_replace("\n", "\\n\".<br>\n&nbsp;&nbsp;&nbsp;\"", $newstr);
		$newstr = str_replace('$', '\$', $newstr);
		echo "\$insarr[]=\"".$newstr."\";<br>\n";
	}
	
}

// FUNCTION: show pure SQL commands
function show_SQL_pure() {
    
    if (!sizeof($this->insarr)) {
        return;
    }
    echo '<pre>'."\n";
    foreach( $this->insarr as $sqlstr) {
        $is_comment=0;
        $newstr = htmlspecialchars($sqlstr);
        if (substr($sqlstr,0,2)=='/*') $is_comment=1;
        echo $newstr;
        if (!$is_comment) echo ';';
        echo "\n";
    }
    echo '</pre>'."\n";
}

function show_META() {
	// FUNCTION: show commands for META with db_transform
	if (!sizeof($this->metaarr)) {
		return;
	}
	foreach( $this->metaarr as $dummy=>$sqlstr) {
		$newstr = htmlspecialchars($sqlstr);
		$newstr = str_replace("\n", "\\n\".<br>\n&nbsp;&nbsp;&nbsp;\"", $newstr);
		echo '$meta[]='.$newstr.';<br>'. "\n";
	}
	reset($this->metaarr);
}


function showForInsert() {
	$this->show_SQL();
	$this->show_text('newline');
	$this->show_META();
}

	
}
