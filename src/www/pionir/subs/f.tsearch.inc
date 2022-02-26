<?php

//LATER: require_once 'o.S_VARIO.subs.inc';
/**
 * produce a search-form for a table, used in view.tmpl.php
 *  $_SESSION['userGlob']['o.{TABLE}.condX'] : array ("col":..., "op":...)
 * $_SESSION['s_formState']['{TABLE}.view.search.f'] = array('gui': [slim], 'full' )
 * @namespace core::gui
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/www/pionir/subs/f.tsearch.inc 59 2018-11-21 09:04:09Z $
 */
class tSearch_form {
	
	private $vario_defKeys;
	private $is_bo;

	function __construct($tablename) {
		$this->tablename = $tablename;
		$this->is_bo = cct_access_has2($tablename);
		if ($tablename=='DB_USER') $this->is_bo = 1;
		
		$this->sess_settings = $_SESSION['s_formState'][$tablename.'.view.search.f'];
		if (!is_array($this->sess_settings)) $this->sess_settings=array();
		if ($this->sess_settings['gui']== NULL) {
		    $this->sess_settings['gui']='full'; // default for all otables (also Multi-PK
		    if ($this->is_bo) $this->sess_settings['gui']='slim'; // default for business objects
		}
		
	}
	/**
	 * show select box for CLASSES
	 * @param  $sql
	 * @param  $tablename
	 * @param  $classname
	 * @param  $class_tab_has
	 * @return -
	 */
	function tform_class( &$sql, $classname, &$class_tab_has ) {
	  $tablename = $this->tablename;
	  if ( $class_tab_has ) {
	
		echo "<select name=searchClass>";
		echo "<option value=\"\">--- select class ---\n";
	
		$sqls="SELECT NAME, NICE_NAME from extra_class where table_name='".$tablename."' order by UPPER(NICE_NAME)";
		$retVal = $sql->query($sqls);
	
		while ( $sql->ReadRow() ) {
		      $selflag="";
		      $class_name_tmp=$sql->RowData[0];
		      $class_nice   = $sql->RowData[1];

		      if ( $classname==$class_name_tmp ) $selflag="selected";
		      echo '<option value="'. $class_name_tmp . '" '.$selflag.'> '. $class_nice. "\n";
		} 
		echo "</select>";
	  }
	}
	
	/*
	private function tform_vario(&$sqlo) {
		$tablename = $this->tablename;
		
		if ( $this->vario_defKeys==NULL or !sizeof($this->vario_defKeys) ) return;
		
		if ($this->searchVario>0) $tmp_checked=' checked';
		else $tmp_checked='';
		echo "<input type=checkbox name=searchVario value=1 ".$tmp_checked."> Vario-columns?";
		
	}
	*/
	
	/*
	private function _showVarioCols(&$sqlo) {
		$tablename = $this->tablename;
		
		echo "<option > -- VARIO params --";
		$sqlo->Quesel("KEY, NICE FROM S_VARIO_DESC ".
			" WHERE TABLE_NAME='".$tablename."' order by NICE");
		
		$cnt=0;
		while ( $sqlo->ReadRow() ) {
			// construct "class." EXTRA_CLASS_ID "." MAP_COL for unique search-columns
			$nice = $sqlo->RowData[1];
			if ( $nice==NULL ) $nice=$sqlo->RowData[0];
			echo "<option value=\"vario." .$sql->RowData[0] . "\"> " .$nice. "\n";
		}	
	}
	*/
	
	/**
	 * show search form
	 * @param object $sql
	 * @param string $tablename
	 * @param array selectCols_STRUC $selectCols (defined in view.tmpl.inc)
	 * @param array $classInfo
	 * @param int $access_id_has
	 * @param array $option  
	 * 	"mode"     = ["list"], "left" - left frame
		"operShow" = [1] -- show anyway
					  -1 -- no
	 				  show operators "NEW, "AND", "OR" ???
	 	'searchVario' : 0,1
	 * @return -
	 */
	function tform_show( &$sql, &$selectCols, &$classInfo, $access_id_has, $option=NULL ) {
		// WARNING:  close form out after calling this function : echo "</form>\n";
		
		?>
		<script>
		var y_tform_flag = '<?php echo $this->sess_settings['gui'];?>';
		function tform_switch() {
			if (y_tform_flag=='slim') {
				full_d='block';
				slim_d='none';
				y_tform_flag='block';
				alias ='';
			} else {
				full_d='none';
				slim_d='block';
				y_tform_flag='slim';
				alias ='q_search';
			}
			document.getElementById("y_search_full").style.display=full_d;
			document.getElementById("y_search_slim").style.display=slim_d;
			document.edit_search.searchAlias.value=alias;
		}
		</script>
		<?php
		$opt=array();
		$f_full_css ='display:none';
		$f_slim_css ='display:block';
		$searchAlias='q_search';
		if ( $this->sess_settings['gui']=='full' ) {
		    $f_full_css='display:block';
		    $f_slim_css='display:none';
		    $searchAlias='';
		}
		
		$tablename = $this->tablename;
		$class_params_show  = $classInfo["parashow"]; // only aflag 0|1
		$classname		    = $classInfo["name"];
		$class_tab_has		= $classInfo["classhas"];
		
		$_condGlobKey = "o.".$tablename.".condX";
		$this->searchVario = $option['searchVario'];
		
		$_condX = null;
	 	if ( $_SESSION['userGlob'][$_condGlobKey]!=NULL ) $_condX = unserialize( $_SESSION['userGlob'][$_condGlobKey] );
		
		$tmp_col 	   = $_condX["col"];
		$tmp_cond_last = $_condX["op"];
		
		/* check VARIO definitions ...
		$varioLib = new oS_VARIO_sub($tablename);
		$this->vario_defKeys = $varioLib->getDefKeys($sql);
		*/
		
		if (empty($option["mode"])) $option["mode"] = "list";
		if ($option["mode"]=="left") {
			$opt["brchar"]="<br>"; // break-char for fields (e.g. "<br>")
			$opt["ftarget"]=" target=unten";
		}
		
		// add yinf=search to see the tag in apache-log-files
		
		echo '<form style="display:inline;" method="post" name="edit_search" action="view.tmpl.php?t='.$tablename.'&yinf=search" '.$opt["ftarget"].">\n";
		echo '<div id="y_search_full" style="'.$f_full_css.'">'."\n";
		
		$nextSpace = "";
		
		
		echo '<input type=submit value="Search" class="yButSmall" >&nbsp;';
		if ( $option["operShow"] != -1 ) {
			
			$boolarr = array("NEW", "AND", "OR");
			$boolAdd = $_condX["add"];
			if ( empty($boolAdd) ) $boolAdd="AND";
			echo '<select name=searchOp_i>';
			foreach( $boolarr as $val ) {
			
				$tmpcheck = "";
				if ($boolAdd==$val) $tmpcheck = "selected";
				echo "<option value=\"".$val."\" ".$tmpcheck.">".$val."</option>";
			}
			
			echo "</select>\n";
			
		} else {
			echo "<input type=hidden name=searchOp_i value=\"NEW\">\n";
		}
		if ($option["mode"]=="list") {
			echo "&nbsp;&nbsp;&nbsp;<a href=\"javascript:open_info('help/robo/Search_in_table.html')\">help</a></font>";
			echo "&nbsp;&nbsp;<a href=\"frame.left.nav.php?listtab=".$tablename."&amode=lsearch\" target=left>".
			 "<img src=\"images/b14.lfram.gif\" border=0 valign=middle TITLE=\"show form in left frame\"></a>";
			if ($this->is_bo ) {
			   echo '&nbsp;<a href="javascript:tform_switch()">'.
			     '<img src="images/but.40.config.png" height=20 title="switch to quick search"></a>';
			}
		}
		//echo $opt["brchar"];
		echo "<br>";
		echo '<span style="font-size:0.8em;">';
		echo '<select name=searchCol>';
	
		foreach( $selectCols as $valarr ) {
			
			$selected="";
		    $db_key  = key($valarr);
		    $db_nice = current($valarr);
		    reset($valarr);
		
	
			$db_show = $valarr["show"];
			$db_conc = $valarr["nice"];
			$db_fkf  = $valarr["key"];
			
			if ($tmp_col == $db_key) $selected=" selected";
			
			if ( $db_key=="x.EXTRA_OBJ_ID" )  $db_show=0;
			if ( $db_fkf ) $db_conc= "ID of ".$db_conc;
			if ( $db_key=="a.DB_USER_ID" ) {
				$db_conc= "user";
				$db_key = "a.user_name";
			}
		 	if ($db_show) echo '<option value="'. $db_key . '"'.$selected.'> '. $db_conc. "\n";
		}
		
		
		if ( !empty($class_params_show) ) {
	       	echo "<option > -- class params --";
	       	$sqltmp="select y.name, y.extra_class_id, y.map_col, y.NICE_NAME from extra_class x, extra_attrib y".
	       		" where x.name='".$classname."' AND x.extra_class_id=y.extra_class_id order by POS";
			$sql->query($sqltmp);
	 		//$cnt=0;
	  		while ( $sql->ReadRow() ) {
	            // construct "class." EXTRA_CLASS_ID "." MAP_COL for unique search-columns
	            $nice = $sql->RowData[3];
	            if ( $nice==NULL ) $nice=$sql->RowData[0];
	 			echo "<option value=\"class." .$sql->RowData[1]. "." .$sql->RowData[2]. "\"> " .$nice. "\n";
			}
		}
		
		
		echo "</select>\n "; 
		echo $opt["brchar"];
	
		$tmp_arr=array();
		$tmp_arr['=']           = '=';
		$tmp_arr['LIKE']        = 'LIKE';
		$tmp_arr['NOT LIKE']    = 'not LIKE';
		$tmp_arr['>']           = '&gt';
		$tmp_arr['<']           = '&lt;';
		$tmp_arr['>=']          = '&gt;=';
		$tmp_arr['<=']          = '&lt;=';
		$tmp_arr['!=']          = '!=';
		$tmp_arr['is NULL']     = 'NULL';
		$tmp_arr['is NOT NULL'] = 'not NULL';
		$tmp_arr['NOT LIKE strict'] = 'not LIKE strict';
	
	     
		echo "<select name=searchBool>\n";
		foreach( $tmp_arr as $cond_tmp=> $cont_txt) {
		 
       		$selected="";
			if ($cond_tmp == $tmp_cond_last) $selected=" selected";
       		echo "<option value=\"".$cond_tmp."\" ".$selected.">".$cont_txt."\n";
       	}
	       
		echo "</select>\n";
		echo $opt["brchar"];
		
		echo "<input name=searchtxt>";
		echo "<br>\n";
		
		if ( $option["mode"]=="list" ) {
			// echo "<img src=\"0.gif\" height=1 width=60>";
		} else {
			$this->tform_class( $sql, $classname, $class_tab_has );
			
		}
		
		if ($option["mode"]=="list") {
			echo $nextSpace;
			$this->tform_class( $sql, $classname, $class_tab_has );
			// $this->tform_vario( $sql );
			
			
			echo "\n<span id=yNavGray>&nbsp;[<a href=\"obj.link.c_query_mylist.php?t=$tablename\"".$opt["ftarget"].">my searches</a>]";
			echo "[<a href=\"obj.link.c_query_new1.php?t=$tablename\"".$opt["ftarget"].">save</a>]";
			if ($access_id_has) {
				echo "&nbsp;&nbsp;&nbsp;[<a href=\"view.tmpl.php?t=".$tablename."&searchAlias=my_data&searchOp_i=NEW\"".$opt["ftarget"].">myData</a>]\n";
				echo " [<a href=\"view.tmpl.php?t=".$tablename."&searchAlias=my_today&searchOp_i=NEW\"".$opt["ftarget"].">myToday</a>]\n";
			}
			echo "<span>\n";
		} else {
			echo " <a href=\"javascript:open_info('help/robo/Search_in_table.html')\"><img src=\"images/help.but.gif\" TITLE=\"help for search\" border=0></a>";
		}
		echo ' </span>'."\n";
		echo '</div>'."\n"; // id="y_search_full"
		echo '<div id="y_search_slim" style="'.$f_slim_css.'">'."\n";
		echo '<input type=submit value=" Search " class="yButSmall" style="font-size:1.4em;">&nbsp;';
		echo '<input type=text   name=searchidx style="font-size:1.4em;" placeholder=" Name or ID">';
		echo '<input type=hidden name=searchAlias value="'.$searchAlias.'">';
		echo '&nbsp;<a href="javascript:tform_switch()"><img src="images/but.40.config.png" height=30 title="switch to full search"></a>';
		echo '</div>'."\n"; // id="y_search_slim"
		echo "</form>\n";
	
	}

}