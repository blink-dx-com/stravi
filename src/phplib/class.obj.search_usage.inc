<?php
/**
 * object tracking for normal object relations
 * @package class.obj.search_usage.inc
 * @swreq   UREQ:0001045: g > object tracking > HOME
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */


/**
 * class object tracking for S_OBJLINK
 * @swreq   UREQ:0001045: g > object tracking > HOME
 */
class oS_OBJLINK_track {
	function __construct($option) {
		$this->option =$option;
	}
	
	function setObj( $tablename, $prim_id, $showLevel ) {
		$this->tablename   = $tablename;
		$this->prim_id     = $prim_id;
		$this->moTableArr  = NULL;
		$this->showLevel   = $showLevel;
		
		$this->usedtabs  =NULL;
		$this->usedtabs_i=NULL;
		$this->obj_num_all = 0;
		
		$this->header_text = "<br />\n<font color=\"#999999\">Linked as child (meta) by objects:</font>\n<br><br>\n";
		
	}
	
	/**
	 * start object link analysis
	 * @param $sqlo
	 * @return number of tables
	 */
	function start(&$sqlo) {
		$tablename = $this->tablename;
		$prim_id   = $this->prim_id;
		
		$moTableArr=NULL;
		$sqlsel = "distinct(MO_TABLE) from S_OBJLINK where CH_TABLE=".$sqlo->addquotes($tablename).
			' and CH_ID='.$prim_id. ' order by MO_TABLE';
		$sqlo->Quesel($sqlsel);
		
		while ($sqlo->ReadRow() ) {
			$moTable = $sqlo->RowData[0];
			$moTableArr[]=$moTable;
		}
		$this->moTableArr=$moTableArr;
		
		return $this->moTableArr;
	}
	
	// @return 0,1
	function getTables() {
		return $this->moTableArr;
	}
	
	/**
	 * count/show usage of ONE parent table ( from PARAMS )
	   RETURN: 0,1 entry exists ?
				
	 * @param $sql
	 * @param $parent
	 * @param $parPkName
	 * @return int
	 */
	function oneTable( &$sqlo, $parent ) {
		$prim_id = $this->prim_id;
		$tablename = $this->tablename;
		
		$retval=1; // entry exists ?	
		$parPkName = PrimNameGet2($parent);

		$sqlAfter = "S_OBJLINK where CH_TABLE=" .$sqlo->addquotes($tablename). ' and CH_ID='.$prim_id.
			" and MO_TABLE=".$sqlo->addquotes($parent);
		$sqlCondEncoded = urlencode($parPkName. " in (select MO_ID from ".$sqlAfter.")");
		
		$sqlsel   = "count(1) from ".$sqlAfter;
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$found_obj_num  = $sqlo->RowData[0];
		
		if ($found_obj_num>0) {
			$this->usedtabs  [$parent]=$found_obj_num;
			$this->usedtabs_i[$parent]=$parPkName;
		}
		if ($this->showLevel > 0) {
			
			$parentNice = tablename_nice2($parent);
			
			if ($found_obj_num > 0) {  
				if ( !$this->obj_num_all ) {
					echo $this->header_text;
				}
				
				echo " <a href=\"view.tmpl.php?condclean=1&t=".$parent."&tableSCond=". $sqlCondEncoded ."\">";
				if ( $this->option["showicon"] ) {
					$icon = htmlObjIcon($parent, 1);
					$this->iconCache[$parent] = $icon;
					echo "<img src=\"$icon\" border=0> ";
				}	
				
				echo $parentNice."</a>: ";
				echo "<B>$found_obj_num</B>";
				if ( $this->option["showMotherObj"]>0 AND $found_obj_num==1 ) {
					//$showMother = 1;
				} else {
					//$showMother = 0;
				}
				
				
				echo "<br>\n";
								
				
			} else {
				if ($this->showLevel > 1) echo $parentNice.": none<br>\n";
			}
			
		}
		$this->obj_num_all = $this->obj_num_all + $found_obj_num;
		
		return ( $retval );
	}
}


/**
 * class object tracking for normal object relations
 * search for objects which use object, but NO assotiative tables (x.PRIMARY_KEY!=1) 
 * 
 * @swreq   UREQ:0001045: g > object tracking > HOME
 */
class object_usage {	

    var $tablename;
    var $prim_id;
    var $BO_is=0;
    var $usedtabs; /* name of tabs and number of usage */
    var $usedtabs_i; // contains name of tabs and Search_column
    var $obj_num_all;
    var $proj_num_use;
    var $showLevel;  /* level of information:
    					0 : show nothing
    					1 : show only linked objects 
    					2 : show also unlinked objects
    				 */
    var $header_text;
    var $current_table;
    var $option;	
    var $iconCache;
    
    /**
     * 
     * @param object $sql
     * @param string $tablename
     * @param string $prim_id
     * @param int $showLevel
     * @param string $header_text
     * @param array $opt
     *   "showicon" 0|1
    	 "hideHeader" : 0|1
    	"showMotherObj"  1: if ONE mother-object found: show direct link to it
     */
    function __construct( 
    	&$sql, 
    	$tablename, 
    	$prim_id, 
    	$showLevel, 
    	$header_text = NULL, 
        $opt=NULL 
         ) {
        
    
    	$this->option	   = $opt;
    	$this->tablename   = $tablename;
    	$this->prim_id     = $prim_id;
    	$this->BO_is       = cct_access_has2($tablename);
        $this->obj_num_all = 0;
    	$this->showLevel   = $showLevel;
        $this->header_text = $header_text;
    	
    	if ( $this->header_text=="" ) $this->header_text = "<font color=\"#999999\">Used by objects:</font>\n<br><br>\n";
    	if ($opt["hideHeader"]) $this->header_text = "";
    	
    	$this->usedtabs=array();
    	
    }    
    
    /**
     * build $this->_tab_track_info 
     * do NOT select VIEWS !
     * @param object $sqlo
     * @return array tab_track_info_STRUCT
     *    array('pa_t'=>parent table, 'pa_pk'=>paretnt table PK);
     */
    function start(&$sqlo) {
    	
    	$sqls = "select x.table_name, y.nice_name, x.column_name from cct_column x, cct_table y ".
    			" WHERE x.cct_table_name='". $this->tablename ."'".
    			" AND x.table_name=y.table_name AND x.PRIMARY_KEY!=1 AND IS_VIEW=0"  ;
    	$sqlo->query($sqls);
    	
    	$this->_tab_track_info = array();
    	while ( $sqlo->ReadRow() ) {
    	    $parent 	= $sqlo->RowData[0];
    	    $parPkName  = $sqlo->RowData[2];
    	    $this->_tab_track_info[]  = array('pa_t'=>$parent, 'pa_pk'=>$parPkName);
    	}
    	
    	return $this->_tab_track_info;
    }
    
    private function _next_level(&$sql2, $table_src, $cond_ori) {
        
        $tables_def=array(
            'EXP'           =>array('via'=>'EXP_HAS_PROTO', 'via_col'=>'CONCRETE_PROTO_ID'),
            'CONCRETE_SUBST'=>array('via'=>'CS_HAS_PR'    , 'via_col'=>'CONCRETE_PROTO_ID'),
        );
        
        if ($table_src=='CONCRETE_PROTO') {
            $use_tables=array('EXP', 'CONCRETE_SUBST');
        }
        
        echo " <img src=\"images/ic.arro2.gif\" hspace=10> ";
        
        if ($table_src=='SOCKET') {
            
            $mother='EXP';
            $motherNice = tablename_nice2($mother);
            $mother_pk  = PrimNameGet2($mother);
            $icon 	    = htmlObjIcon($mother, 1);
            $this->iconCache[$mother] = $icon;
            $cond_MotherObj  = "x.SOCKET_ID in (select SOCKET_ID from ".$table_src." where ".$cond_ori.")";
            $cond_html = urlencode($cond_MotherObj);
            echo "<a href=\"view.tmpl.php?condclean=1&t=".$mother."&tableSCond=".$cond_html."\">";
            if ( $this->option["showicon"] ) {
                echo "<img src=\"$icon\" border=0> ";
            }
            echo $motherNice."</a> \n";
            
            return;
        }

        $komma= ' ';
        foreach($use_tables as $mother) {
            
            $loop_defs = $tables_def[$mother];
            $via_table = $loop_defs['via'];
    
            $motherNice = tablename_nice2($mother);
            $mother_pk  = PrimNameGet2($mother);
            $icon 	    = htmlObjIcon($mother, 1);
            $this->iconCache[$mother] = $icon;
            
            $cond_MotherObj  = $mother_pk." in (select ".$mother_pk." from ".$via_table." where ". $loop_defs['via_col'] .
              " in (select ".$loop_defs['via_col']." from ".$table_src." where ".$cond_ori.")".
               ")" ;
            
            
            $cond_html = urlencode($cond_MotherObj);
            echo $komma."<a href=\"view.tmpl.php?condclean=1&t=".$mother."&tableSCond=".$cond_html."\">";
            if ( $this->option["showicon"] ) {
                echo "<img src=\"$icon\" border=0> ";
            }
            echo $motherNice."</a> \n";
            
           
        }
    }
    
    /**
     * show parent object
     * @param object $sql2
     * @param string $tabName
     * @param string $oriColName
     * @param int $showParent 0,1 show one found parent
     */
    function parent_objects_get( &$sql2, $tabName, $oriColName, $showParent	) {  
    	
        $sqls = "select column_name, cct_table_name from cct_column". 
            	" WHERE table_name='". $tabName . "' AND ".
            	" cct_table_name is not NULL AND PRIMARY_KEY=1";
        $sql2->query($sqls);
        if ( !$sql2->ReadRow() ) {
    		return;
    	}
    	
    	//$pk1    = $sql2->RowData[0];
    	$mother = $sql2->RowData[1];
    	
    	$motherNice = tablename_nice2($mother);
    	$mother_pk  = PrimNameGet2($mother);
    	$icon 	    = htmlObjIcon($mother, 1);
    	$this->iconCache[$mother] = $icon;
    	
    	$cond_MotherObj  = $mother_pk." in (select ".$mother_pk." from ".$tabName." where ". $oriColName ."='".$this->prim_id ."')" ;
    	
    	
    	if ($showParent == 1) {
    	    
    	    $motherImpCol = importantNameGet2($mother);
    	    $sqls = "select ".$mother_pk.", ".$motherImpCol." from ".$mother.
    	    " WHERE ". $cond_MotherObj;
    	    $sql2->query($sqls, "parent_objects_get:2");
    	    
    	    if ( !$sql2->ReadRow() ) {
    	        echo "<img src=\"images/ic.arro2.gif\" hspace=10> ";
    	        if ( $this->option["showicon"] ) {
    	            echo "<img src=\"$icon\" border=0> ";
    	        }
    	        echo $motherNice." ??? \n";
    	    } else {
    	        $motherObjID    = $sql2->RowData[0];
    	        $motherObjName  = $sql2->RowData[1];
    	        echo "<img src=\"images/ic.arro2.gif\" hspace=10> <a href=\"edit.tmpl.php?t=".$mother."&id=".$motherObjID."\">";
    	        if ( $this->option["showicon"] ) {
    	            echo "<img src=\"$icon\" border=0> ";
    	        }
    	        echo $motherNice.": ".$motherObjName."</a> \n";
    	    }
    	    
    	} else {
    	    
    	    $cond_html = urlencode($cond_MotherObj);
    	    echo "<img src=\"images/ic.arro2.gif\" hspace=10> <a href=\"view.tmpl.php?condclean=1&t=".$mother."&tableSCond=".$cond_html."\">";
    	    if ( $this->option["showicon"] ) {
    	        echo "<img src=\"$icon\" border=0> ";
    	    }
    	    echo $motherNice."</a> \n";
    	    
    	    if ($mother=='CONCRETE_PROTO' or $mother=='SOCKET') {
    	        $this->_next_level($sql2, $mother, $cond_MotherObj);
    	    }
    	   
    	}
         
    }
    
    /**
     * count/show usage of ONE parent table ( from PARAMS )
     * @param object $sql
     * @param string $parent    TABLE_NAME
     * @param string $parPkName PK column name of table
     * @return int 0,1 entry exists ?
     */
    function getNumOneParent( &$sql, $parent, $parPkName ) {
    	
    			
    	$retval=1; // entry exists ?	
    			
    	$sqls = "select count(1) from ". $parent ." WHERE ".$parPkName. "='".$this->prim_id."'";
    	// echo "DEB:SQL: $sqls <br>";
    	$sql->query($sqls);
    	$sql->ReadRow();
    	$found_obj_num     = $sql->RowData[0];
    	if ($found_obj_num>0) {
    		$this->usedtabs  [$parent]=$found_obj_num;
    		$this->usedtabs_i[$parent]=$parPkName;
    	}
    	if ($this->showLevel > 0) {
    		
    		$parentNice = tablename_nice2($parent);
    		
    		if ($found_obj_num > 0) {  
    			if ( !$this->obj_num_all ) {
    				echo $this->header_text;
    			}
    			
    			echo " <a href=\"view.tmpl.php?condclean=1&t=".$parent."&searchCol=x.". $parPkName."&searchtxt=". 
    				$this->prim_id."\">";
    			if ( $this->option["showicon"] ) {
    				$icon = htmlObjIcon($parent, 1);
    				$this->iconCache[$parent] = $icon;
    				echo "<img src=\"$icon\" border=0> ";
    			}	
    			
    			echo $parentNice."</a>: ";
    			echo "<B>$found_obj_num</B>";
    			if ( $this->option["showMotherObj"]>0 AND $found_obj_num==1 ) {
    				$showMother = 1;
    			} else {
    				$showMother = 0;
    			}
    			
    			if ($found_obj_num >0) $this->parent_objects_get( $sql, $parent, $parPkName, $showMother );
    			
    			echo "<br>\n";
    							
    			
    		} else {
    			if ($this->showLevel > 1) echo $parentNice.": none<br>\n";
    		}
    		
    	}
    	$this->obj_num_all = $this->obj_num_all + $found_obj_num;
    	
    	return ( $retval );
    }
    
      	
    /**
     * - show usage of ONE parent table from $sql->ReadRow()
       - VIEWS are NOT allowed

     * @param object $sql
     * @param string $parent table
     * @param string $parent $parPkName
     * @return number 0,1 entry exists ?
     */
    function getNumOneTab(&$sql, $parent, $parPkName) {
    	$this->current_table = $parent;
    	return ( $this->getNumOneParent( $sql, $parent, $parPkName ) );
    }
    
    /**
     * do a full scan without showing the info
     * @return 
     * @param object $sqlo
     */
    function analyseAll( &$sqlo ) {
        
        $parents = $this->start ($sqlo);
    	if (empty($parents)) return;
    	
    	foreach( $parents as $valarr) {
    		$parent    = $valarr['pa_t'];
    		$parPkName = $valarr['pa_pk'];
    		$this->current_table = $parent;
    		$this->getNumOneParent( $sqlo, $parent, $parPkName );
    	}
    	 
    	
    }
    
    function getProjUsage(&$sql ) {
      if ($this->BO_is != '') {
    	$sql->query('SELECT COUNT(*) FROM proj_has_elem WHERE'.
    				' table_name = '.$sql->addQuotes($this->tablename).' AND prim_key = '.
    			$sql->addQuotes($this->prim_id), "getProjUsage");
    	$sql->ReadRow();
    	$this->proj_num_use = $sql->RowData[0];
    	if ( $this->proj_num_use>0 ) {
    	  $this->usedtabs['PROJ'] = $this->proj_num_use;
    	}
    	if ($this->proj_num_use>0 && $this->showLevel > 0) {
    	  $tableSCond_url= rawurlencode('PROJ_ID in (select PROJ_ID from PROJ_HAS_ELEM where TABLE_NAME = '.
    									$sql->addQuotes($this->tablename).' AND PRIM_KEY = '.$sql->addQuotes($this->prim_id).')');
    	  echo '<a href="view.tmpl.php?t=PROJ&condclean=1&tableSCond='.$tableSCond_url.'">';
    	  if ( $this->option["showicon"] ) {
    			$icon = htmlObjIcon("PROJ", 1);
    			echo "<img src=\"$icon\" border=0> ";
    	  }
    	  echo "Project</a>: ";
    	  echo '<B>',$this->proj_num_use,'</B>';
    	}
      }
    }
    
    function getWorklistUsage(&$sql ) {
      if ($this->BO_is != '') {
    	$sql->query('select count(distinct WORKLIST_ID) FROM worklist_entry WHERE'.
    				' table_name = '.$sql->addQuotes($this->tablename).' AND objid = '.
    			$sql->addQuotes($this->prim_id), "getWorklistUsage");
    	$sql->ReadRow();
    	$this->wklist_num_use = $sql->RowData[0];
    	if ( $this->wklist_num_use>0 ) {
    	  $this->usedtabs['WORKLIST'] = $this->wklist_num_use;
    	}
    	if ($this->wklist_num_use>0 && $this->showLevel > 0) {
    	  $tableSCond_url= rawurlencode('WORKLIST_ID in (select WORKLIST_ID from WORKLIST_ENTRY where TABLE_NAME = '.
    									$sql->addQuotes($this->tablename).' AND OBJID = '.$sql->addQuotes($this->prim_id).')');
    	  echo '<a href="view.tmpl.php?t=WORKLIST&condclean=1&tableSCond='.$tableSCond_url.'">';
    	  if ( $this->option["showicon"] ) {
    			$icon = htmlObjIcon("WORKLIST", 1);
    			echo "<img src=\"$icon\" border=0> ";
    	  }
    	  echo "Worklist</a>: ";
    	  echo '<B>',$this->wklist_num_use,'</B>';
    	}
      }
    }
    
    function get_parent_obj( &$sql, $searchtable, $childtable, $oriColName_tmp ) {
        $ptable	= "";
        $ptable_nice= "";
        $pid	= 0;
        $retval = 0;
        $iconImg = "";
        $tabNameNice_tmp = tablename_nice2($searchtable);
        
        $id = $this->prim_id;
        // get name of primary KEY
        $sqls = "select column_name from cct_column  
                WHERE   table_name='". $searchtable . "' AND
    	                PRIMARY_KEY=1"  ;
    	$sql->query("$sqls");
        $sql->ReadRow(); 
        $stable_pk_name = $sql->RowData[0];
        //echo "DEBUG: $searchtable,$childtable, fk_column:$oriColName_tmp TAB_PK:$stable_pk_name<br>
        //               sqls: $sqls3";
           
        if ($oriColName_tmp=="" || $stable_pk_name=="") return (-1); 
        
        $sqls = "select ".$stable_pk_name." from ". $searchtable ." WHERE ".$oriColName_tmp. "='".$id."'";
    	// echo "DEB:SQL: $sqls <br>";
    	$sql->query("$sqls");
    	$sql->ReadRow();
    	$pid  = $sql->RowData[0];
         // now get the parent object info
        
        if (countPrimaryKeys($searchtable) <=1 ) { 
            // is the table of the parent object
            $ptable      = $searchtable;
            $ptable_nice = $tabNameNice_tmp;
            $ptable_pk   = $stable_pk_name;
        } else { 
            // is an assoc table
           $ptable = mothertable_get2($searchtable);
           if ($ptable=="") return (-1);
           $ptable_nice = tablename_nice2($ptable);
           $ptable_pk   = PrimNameGet2($ptable); 
        }
        $table_impcol = importantNameGet2($ptable);
        
        $sqls = "select ".$table_impcol." from ". $ptable ." WHERE " .$ptable_pk. "='".$pid."'";
    			// echo "DEB:SQL: $sqls <br>";
    	$sql->query("$sqls");
    	$sql->ReadRow();
    	$pobj_name  = $sql->RowData[0];
        $pobj_name_out=$pobj_name;
        if ($pobj_name=="") $pobj_name_out="ID:$pid";
    	$iconFile = $this->iconCache[$ptable];
        if ($iconFile!="") $iconImg =  "<img src=\"$iconFile\" border=0 title=\"".$ptable_nice."\"> ";
        echo "&nbsp;&nbsp;&nbsp;  <a href=\"edit.tmpl.php?t=$ptable&id=$pid\">".$iconImg." <B>$pobj_name_out</B></a><br>\n";
        
        return ( array( $retval, $ptable, $ptable_nice, $pid ) );
    }
    
    /**
     * start object link analysis
     * @param $sqlo
     */
    function objlink_start(&$sqlo) {
    	$this->linkLib = new oS_OBJLINK_track($this->option);
    	$this->linkLib->setObj($this->tablename,$this->prim_id, $this->showLevel);
    	$this->linkLib->start($sqlo);
    	return ($this->linkLib->getTables());
    }
    
    function objlink_oneTab(&$sqlo, $moTable) {
    	$this->linkLib->oneTable($sqlo,$moTable);
    }
    
    /**
     * 
     */
    function objlink_oneTable(&$sqlo) {
    	$this->linkLib->oneTable;
    }
    
    function getUsedTabs() {
    	return ($this->usedtabs);
    }
    
    function set_showLevel($showLevel) {
        $this->showLevel = $showLevel;
    }
	
}

