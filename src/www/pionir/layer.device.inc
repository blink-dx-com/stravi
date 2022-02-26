<?php
/**
 * equipment theme park
 * GLOBAL: $_SESSION['userGlob']['g.theme.device']
 * @namespace core::gui::homeLayer
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq Task:0001458: g > Devices
 * @param $showopt=0,1 show options
 * @param $theme_go: 0,1
 * @param $themevar
 *   'query.select'
 *   'projid.imp'
 * @version $Header:  59 2018-11-21 09:04:09Z $
 */
require_once ("f.textOut.inc");
require_once ('f.wiid_roid.inc');
require_once ("func_form.inc");
require_once ("f.help.inc");
require_once ("gui/o.CHIP_READER.statusView.inc");
require_once ("lev1/o.CHIP_READER.serviceInfoRule.inc");
require_once ('f.sql_query.inc');
require_once ("f.objview.inc");	
require_once 'o.LINK.c_query_subs.inc';

class themeCls extends gThemeClsAbs {
	
	var $themevar; /** theme variables
		'query.select' : LINK_ID to query
	*/
	var $homeProjID;

	function layer_HtmlInit() {
	
		$cssstr = ''; //".xBodyNorm { background-image: url(images/themepark.device.jpg); ".
			          // " background-repeat: no-repeat; } \n";
	
		echo '<style type="text/css">'."\n";
		echo $cssstr;
		echo '</style>'."\n";
	}

    function layer_params() {
    		$lparams = array ( "bodycss" => "xBodyNorm");
    		return ($lparams);
    }
    
    /**
     * Init 
     * @param $sqlo
     */
    function initTheme( &$sqlo ) {
    	global $error;
    	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
    	
    	$projid_ori = 197; 

    	$this->homeProjID = $projid_ori;
    	
    	$this->themevar ==NULL;
    	if ( $_SESSION['userGlob']['g.theme.device']!=NULL ) {
    		$this->themevar = unserialize($_SESSION['userGlob']['g.theme.device']);
    	}
    	
    	if ( $_REQUEST['theme_go']>0) {
    		$this->themevar['query.select'] = $_REQUEST['themevar']['query.select'];
    		$this->themevar['projid.imp']   = $_REQUEST['themevar']['projid.imp'];
    		$_SESSION['userGlob']['g.theme.device'] = serialize($this->themevar);
    	}
    }
    
    /**
     * show selectable query-types
     */
    function _showQuerySelection(&$sqlo) {
    	global $error;
    	$tablename='Query';
    	
    	if (!$this->homeProjID) {
    		echo "Home-project fehlt.<br>";
    		return;
    	}
    
    	$wichtigeGeraete = array(); // TBD: LINK_IDs from $this->homeProjID
    	
    	$wiidLib    = new fWiidRoidC();
    	$wichtiGerateArr=array();
    	foreach( $wichtigeGeraete as $oriid=>$tmpname) {
    		
    		$projid = $wiidLib->getObjID ($sqlo, 'PROJ', 1, $oriid);
    		if (!$projid) {
    			$tmpname .= ' ???';
    			$error->reset();	
    		}
    		$wichtiGerateArr[$projid] = $tmpname;
    	}
    	reset ($wichtigeGeraete); 
    	
    	
    	
    	$useQuery  = $this->themevar['query.select'];
    	
    	$workSelArr=NULL;
    	$projsubLib = new cProjSubs();
    	$sqlafter = $projsubLib->getTableSQL ( $this->homeProjID, 'LINK' );
    	// get all device-queries
    	$sqlAfter2 = "LINK x , extra_obj o, extra_class c ".
    		" WHERE ( c.NAME='query') AND x.extra_obj_id = o.extra_obj_id AND c.extra_class_id = o.extra_class_id".
    		" and S01='CHIP_READER'".
    		" and x.LINK_ID IN (".$sqlafter.") order by x.NAME";
    	
    	$sqlsel = "x.LINK_ID, x.NAME from ".$sqlAfter2;
    	$sqlo->Quesel($sqlsel);
    	$showIds=array();
    	while ( $sqlo->ReadRow() ) {
    	    $tmpid = $sqlo->RowData[0];
    	    $docname = $sqlo->RowData[1];
    	    $showIds[$tmpid]=$docname;
    	}
    
    	require_once ('func_form.inc');
    	
    	$proj_Link = '<span style="font-weight:normal;">[<a href="edit.tmpl.php?t=PROJ&id='.$this->homeProjID.
    		'">Vorlagen-Project</a>]</span>';
    	
    	
    	echo "<br>\n";
    	
    	$initarr   = NULL;
    	$initarr["action"]      = $_SERVER['PHP_SELF'];
    	$initarr["title"]       = "Auswahl &nbsp;&nbsp;&nbsp;".$proj_Link;
    	$initarr["submittitle"] = "Speichern";
    	$initarr["tabwidth"]    = "AUTO";
    
    	$hiddenarr = NULL;
    	$hiddenarr["theme_go"]     = '1';
    
    	$formobj = new formc($initarr, $hiddenarr, 0);
    	
    
    	
    	$fieldx = array ( 
    		"title" => "Ger&auml;teauswahl", 
    		"name"  => "themevar[query.select]",
    		"object"=> "select",
    		"val"   => $useQuery, 
    		"inits" => $showIds,
    		"namex" => TRUE,
    		"notes" => " alle Geraete einer Gruppe"
    		 );
    	$formobj->fieldOut( $fieldx );
    	
    	$fieldx = array ( 
    		"title" => "Wichtige Ger&auml;te", 
    		"name"  => "themevar[projid.imp]",
    		"object"=> "select",
    		"optional"=>1,
    		"namex" => TRUE,
    		"val"   => $this->themevar['projid.imp'], 
    		"inits" => $wichtiGerateArr,
    		"notes" => '... werden in der Tabelle "Wichtige Ger&auml;te" gelistet'
    		 );
    	$formobj->fieldOut( $fieldx );
    
    	$formobj->close( TRUE );
    	
    
    	
    	
    }
    
    
    private function getDeviceSelection($sqlo) {
    	$buildSqlLib = new fSqlQueryC ('CHIP_READER'); // init class
    	
    	$sqlInfo = $buildSqlLib->get_sql_info();
    	if ($sqlInfo==NULL) {
    		return array('cnt'=>0, 'info'=>'no selection active!');
    	}
    	
    	$sqlAfter    = $buildSqlLib->get_sql_after (); // get ActiveTableCondition
    	if ($sqlAfter==NULL) {
    		return array('cnt'=>0, 'info'=>'no selection active!');
    	}
    	
    	$sqlsel = 'count(1) from '. $sqlAfter ; // build a selection
    	$sqlo->Quesel( $sqlsel );
    	$sqlo->ReadRow ();
    	$cnt = $sqlo->RowData [0];
    	
    	return array('cnt'=>$cnt, 'info'=>$sqlInfo);
    }
    
    /**
     * activate the current query
     * @param object $sqlo
     */
    private function _activate_query($sqlo) {
    	global $error, $varcol;
    	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
    	
    	$doc_id = $this->themevar['query.select'];
    	if (!$doc_id) {
    		$error->set( $FUNCNAME, 1, 'No selection activated.' );
    		return;
    	}
    	
    	$extra_obj_id =  glob_elementDataGet( $sqlo, 'LINK', 'LINK_ID', $doc_id, 'EXTRA_OBJ_ID');
    	$values_all = $varcol->select_by_name ($extra_obj_id);
    	$values = &$values_all["values"];
    	
    	$queryObj = new myquery();
    	$options=array();
    	$options["action"] = "exec";
    	$options['exec_after_url'] = 'home.php?layer=device';
    	$myqu_varval = array(); // user parameters
    	$formNextUrl = NULL;
    	$queryObj->ShowFrom( $sqlo, $extra_obj_id, $myqu_varval, $formNextUrl, $options );
    	if ($error->got(READONLY)) {
    		return;
    	}
    }
    
    /**
     * get all devices, where I am responsible 
     * @todo make new
     * 1. get my contact
     * 2. search for my contact in the devices
     * @param object $sqlo
     */
    private function getMyDevices(&$sqlo) {
    	
        $sqlsel_base = oCHIP_READER_servicInfoRule::getAllDevOneUser_SQL_B($sqlo, $_SESSION['sec']['db_user_id']);
    		 
    	$sqls_count = 'count(1) from '.	$sqlsel_base; 
    	$sqlo->Quesel( $sqls_count );
    	$sqlo->ReadRow ();
    	$cnt = $sqlo->RowData [0];
    	
    	$sqlWhere = oCHIP_READER_servicInfoRule::getAllDevOneUser_SQL_W($sqlo, $_SESSION['sec']['db_user_id']);
    	
    	return array('retval'=>1, 'cnt'=>$cnt, 'sqlWhere'=>$sqlWhere);
    }
    
    function layer_show( &$sqlo, &$sqlo2 ) {
    	global $error;
    	$FUNCNAME= 'layer_show';
    	
    	$this->initTheme($sqlo);
    
    	echo '<blockquote>'."\n";
    
    	$this->__homeFuncObj->home_showHead("device", NULL);
    	?>
    	<br><br>
    	
    	<table border=0 ><tr><td width=20></td>
    	<td align=left>
    	<br>
    	<? 
    	
    	if (!$this->homeProjID) {
    	    echo 'Admin: Please set Equipment-Home-Project!' . "<br>\n";
    	    
    	    //$error->printAll();
    	}
    
    	if ( $_REQUEST['theme_go']>0) {
    		echo '<br><br>... activate the equipment selection.<br><br>'."\n";
    		$this->_activate_query($sqlo);
    		return;
    	}
    	
    	// $showForm = $_REQUEST['showopt'];
    	$showForm = 1;
    	
    	if ( $showForm ) {
    		
    		$this->_showQuerySelection($sqlo);
    		echo "<br>";
    		echo "<br>\n";
    	}
    	
    	$selectInfo    = $this->getDeviceSelection($sqlo);
    	$myDevicesInfo = $this->getMyDevices($sqlo) ;
    	
    	echo '<table><tr valign=top><td>';
    	
    	$txtopt = array("imgFullPath" => 1);
    	$textoutObj = new textOutC($txtopt);
    	// "icon"=>"images/icon.CHIP_READER.gif", "li"=>"br" 
    	
    	/* -----------------------
    	$flist[] = array("ty"=>"head", "txt"=>"Allgemein" , "lay"=>"1" );
    	
    	$helpLib = new fHelpC();
    	$helpOpt = array('location'=>'wiki');
    	$helpHtml = $helpLib->link_show ('misc/theme.device', 'Hilfe', $helpOpt );
    	$flist[] = array("ty"=>"lnk",  "txt"=>$helpHtml, 'icon'=>'images/help.but.gif', "li"=>"br" );
    
    	$flist[] = array("ty"=>"headclose");
    	$textoutObj->linksOut($flist);
    	
    	*/
    	
    	// -----------------------
    	$flist=NULL;
    	$flist[] = array("ty"=>"head", "txt"=>"Ger&auml;teauswahl" , "lay"=>"1" );
    	
      
    	$listEntry = array('icon'=>'images/i13.system3.png', "ty"=>"lnk", "li"=>"br" );
    	if ($this->themevar['query.select']>0) {
    		$tmpname = obj_nice_name ( $sqlo, 'LINK', $this->themevar['query.select'] );
    		$listEntry["txt"]='Aktiviere Auswahl: <b><a href="' .
    			'obj.link.c_query_exec.php?myqu_go=1&id='.$this->themevar['query.select'] .'">' .
    			$tmpname.'</a></b>';
    			 
    	} else {
    		$listEntry["txt"]= "kein Query aktiv.";
    	}
    	$flist[] = $listEntry;
    	
    	
    	$urltmp = 'view.tmpl.php?t=CHIP_READER&condclean=1&tableSCond='.urlencode($myDevicesInfo['sqlWhere']);
    	$infoTmp = $myDevicesInfo['cnt'].' (verantwortlich)';
    	
    	$flist[] = array("ty"=>"lnk",  "txt"=>"Meine Ger&auml;te: ".$infoTmp, "href"=>$urltmp,
    		"icon"=>"images/but.my.gif", "li"=>"br"  );
    	
    	$flist[] = array("ty"=>"headclose");
    	$textoutObj->linksOut($flist);
    	
    	// -----------------------
    	$flist=NULL;
    	$flist[] = array("ty"=>"head", "txt"=>"F&uuml;r die aktuelle Ger&auml;te-Auswahl" , "lay"=>"1" );
    	
    	$txt='Selektierte Ger&auml;te: <b>'.$selectInfo['cnt']. '</b> &nbsp;&nbsp;&nbsp;&nbsp;'.
    		'<span style="color:gray;">Condition:</span> ';
    	if (strlen($selectInfo['info'])>20)  $txt .= substr($selectInfo['info'],0,20);
    	else $txt .= $selectInfo['info'];
    		
    	$flist[] = array("ty"=>"lnk",  "txt"=>$txt
    			);
    	
    	$flist[] = array("ty"=>"lnk",  "txt"=>"Suche im Service-Log" , 
    			"href"=>'p.php?mod=DEF/o.CHIP_READER.li_servAna1', 'notes'=>' (ServiceAnalyse1)',
    			 );
    	$flist[] = array("ty"=>"lnk",  "txt"=>"Service fehlt" , "href"=>'p.php?mod=LAB/o.CHIP_READER.li_servAna2',
    			 'notes'=>'(ServiceAnalyse2)' );
    
    	$flist[] = array("ty"=>"headclose");
    	$textoutObj->linksOut($flist);
    	
    	$flist=NULL;
    	$flist[] = array("ty"=>"head", "txt"=>"Admin-Tools " , "lay"=>"1" );
    	
    	$flist[] = array("ty"=>"lnk",  "txt"=>"Serviceplanlauf starten" , "href"=>'p.php?mod=LAB/o.CHIP_READER.sriPlanAll',
    			'notes'=>'(nur fuer Equipment-Admin; ALLE QC-Geraete)' );
    	$flist[] = array("ty"=>"lnk",  "txt"=>"Maintenance check + send emails" , "href"=>'p.php?mod=DEF/root/o.CHIP_READER.REG_analyse',
    		'notes'=>'(ALLE QC-Geraete)' );
    	
    
    	$flist[] = array("ty"=>"headclose");
    	$textoutObj->linksOut($flist);
    	
    	echo '</td><td>';
    	
    	// ----------------------------
    	// zeige wichtige Objekte
    	// ----------------------------
    	
    	$flist=NULL;
    	$flist[] = array("ty"=>"head", "txt"=>"Wichtige Objekttypen" , "lay"=>"1" );
    	
    	$objLinkLib = new fObjViewC();
    	
    	$tableList = array(
    		"A_CHIP_READER",
    		"CHIP_READER",
    	 	"DEVSIC",
    		);
    	foreach($tableList as $oneTable) {
    		$tmpLink = $objLinkLib->tableViewLink($oneTable);
    		$flist[] = array( "ty"=>"lnk",  "txt"=>$tmpLink, "li"=>"br" );
    	}
    	$flist[] = array("ty"=>"headclose");
    	$textoutObj->linksOut($flist);
    	
    	
    	echo "</td></tr></table>\n";
    	
    
    	if ($this->themevar['projid.imp']) {
    		echo "<b>Wichtige Geraete</b><br><br>\n";
    		$deviceLogLib = new oCHIP_READER_statusView();
    		$deviceLogLib->showLogList($sqlo, $this->themevar['projid.imp']);
    	}
    	$error->printAll();
    	
    	echo "</td></tr></table>\n";
    	echo '</blockquote>'."\n";
    }

}