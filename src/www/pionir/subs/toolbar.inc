<?php
require_once ('o.PROJ.paths.inc');
require_once ('f.objview.inc');

/**
 * show toolbar for edit.tmpl.php, view.tmpl.php
 * @package toolbar.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class fObjToolbar {
	
	var $mode; // mode of toolbar: "single", "list", "info", "home"
	
	/**
	 * options
	 * @var array $opt
	 *  "mother" : e.g. EXP
		"isAssoc": 0,1
		"pk1", "pk2"
	 */
	private $opt;
  
	/**
	 * 
	 * @param string $tablename
	 * @param int $id is "isAssoc": ID of mother
	 * @param string $mode
	 * @param array $opt : see definiton 
	 */
    function __construct($tablename, $id, $mode,  $opt = NULL ) {
    	$this->tablename = $tablename;
    	$this->id = $id;
    	
    	if ($mode=="")  $mode="list";
    	$this->mode = $mode;
    	$this->opt = $opt;
    }
    
    function _getShortKey($tablename, $obj_id) {
    	$shortx = globTabMetaByKey( $tablename, 'SHORT' );
    	if ($shortx!=NULL) $namex = $shortx.':'.$obj_id;
    	else $namex = 'ID:'.$obj_id;
    	return '<span style="color:gray;">'.$namex.'</span>';
    }
    
    /**
     * SLIM object head
     * @param object $name_obj
     * @param int $id
     * @param string $headtxt optional extra text
     * @param array $xmodes
     */
    function FSlim($name_obj, $id, $headtxt, &$xmodes ) {
    
    	$tablename = $this->tablename;
    	$objcolor  = globTabMetaByKey( $tablename, 'COLOR');
    	if ( $objcolor=="" ) $objcolor="#E0E0E0";
        
    	//echo "<table cellpadding=1 cellspacing=0 border=0 width=100% ><tr><td >\n"; // bgcolor=#3366cc
        echo "<table cellpadding=0 cellspacing=0 border=0 width=100% bgcolor=#efefef><tr>\n";
        
        echo '<td width=25 style="background-color:'.$objcolor.'"><img src="images/0.gif" height=30 width=25></td>';
        echo '<td width=100%>';
        
        $nicename= tablename_nice2($tablename);
        $icon="images/icon.".$tablename.".gif";
        if ( !file_exists($icon) ) $icon="images/icon.UNKNOWN.gif";
    	  
        echo "<img src=\"" .$icon. "\" border=0 hspace=7> ";
        echo '<B>'.$name_obj."</B>";
    		
    	if (is_numeric($id)) {
    	    echo "&nbsp;&nbsp;&nbsp;";
    		echo $this->_getShortKey($tablename, $id);
    	}
    	echo '&nbsp;&nbsp;&nbsp;&nbsp; <span style="color:gray; font-size:1.0em;">type: '.$nicename. '</span>&nbsp;';
        echo "</td>\n";
               
       
        
        //echo "</td>\n</tr>\n";
        echo "</tr></table>\n"; 
    	//echo "</td></tr></table>\n";    
    }
    
    /**
     * single form
     * @param $icon
     * @param array $opt
     *   "name_obj" name of object
     *   "obj_id"  ID of object
     *   "mother" table name of mother - for ASSOC elements
     *   "mo_obj" => array(t, id, nice: name of object, 'found'=>int)
     * @param string $info - notes string
     */
    private function _single( $icon, &$opt, $info ) {
    	//$objLinkLib = new fObjViewC();
    	//$objLinkLib->
    	
    	$tablename = $this->tablename;
    	$nicename  = tablename_nice2($this->tablename); 
    	echo '<td valign=top width=100%>'."\n"; // FULL column
    	echo "<table cellpadding=0 cellspacing=0 width=100% style=\"padding-left:5px;\">\n";
    	echo "<tr><td NOWRAP width=100%>";
    	
    	$motab=NULL;
    	$objspace=NULL;
    	$extraline=NULL;
    	$obj_name_font_size=1.2;
    	$obj_icon='<img src="' .$icon. '" border=0 hspace=0>';
    	
    	if ( $this->opt["mother"] != NULL ) {
    		$motab  = $this->opt["mother"];
    		$moicon	= "images/icon.".$motab.".gif";
    		if ( !file_exists($moicon) ) $moicon="images/icon.UNKNOWN.gif";
    		echo "<img src=\"" .$moicon. "\" border=0> ";
    		$objspace = "<img src=\"images/ic.tree3_13.gif\" hspace=1>";
    		$extraline= "<br>\n<img src=\"0.gif\" width=16 height=1><img src=\"images/ic.tree3_13.gif\" hspace=1>".
    		  '&nbsp;<span style="color:gray;">one element</span>';
    	}
    	if (  is_array($opt["mo_obj"]) ) {
    	    // show mother object ...
    	    
    	    $mo_obj = $opt["mo_obj"];
    	    $motab  = $mo_obj['t'];
    	    $obj_name_font_size = 1.4; // make object name larger ...
    	    $moicon			= "images/icon.".$motab.".gif";
    	    if ( !file_exists($moicon) ) $moicon="images/icon.UNKNOWN.gif";
    	    echo '<div style="padding-top:4px;"></div><a class="yGgray" href="edit.tmpl.php?t='.$motab.'&id='.$mo_obj['id'].'">'.
    	       '<img src="' .$moicon. '" border=0> '. $mo_obj['nice'].' [ID:'.$mo_obj['id'].']</a>';
    	    if ($mo_obj['found']>1) {
    	        echo ' ... and more ...';
    	    }
    	    $objspace='<img src="0.gif" width=35 height=1>';
    	    echo "<br>\n<img src=\"0.gif\" width=16 height=1><img src=\"images/ic.tree3_13.gif\" hspace=1> ".$obj_icon.' ';
    	    $obj_icon=NULL; // reset it ..
    	}
    	
    	
    	if ( $opt["name_obj"]!="" ) echo '<span style="font-weight:bold; font-size:'.$obj_name_font_size.'em;">'.$opt["name_obj"].'</span>&nbsp;&nbsp;&nbsp;';
    	if (is_numeric($opt["obj_id"])) {
    		echo $this->_getShortKey($tablename, $opt["obj_id"]);
    	}
    	if ($opt['NamePost']!=NULL) {
    		echo ' '.$opt['NamePost'];
    	}
    	echo "<img src=0.gif width=1 height=22>"; //  minimum height of row
    	echo "</td>\n</tr>\n";
    	// if ( $info!="" ) {	NOWRAP
    	echo '<tr><td style="white-space: nowrap; padding-top:5px;">'.$objspace. $obj_icon. '&nbsp;<span style="color:gray; font-weight:bold; font-size:1.2em;">'.
    			$nicename. "</span>&nbsp;&nbsp;  ".$info;
    	echo $extraline;		
    	echo "</td></tr>\n";
    	//}
    }
    
    private function _home(&$info) {
        echo '<td valign=top width=100%  style="padding-left:10px;">'."\n"; 
    	echo $info;
    }
    
    /**
     * get right menu texts
     * @param $opt
     *    "menushow" 0|1
     */
    function getMenuRight($opt) {
    	
    	$defs=array(
    		'single'=>array('bgimage'=>"ich.single.gif"),
    		'list'  =>array('bgimage'=>"ich.list.gif"),
    		'home'  =>array('bgimage'=>"ich.home.gif")
    		);
    	$mode      = $this->mode;
        $tablename = $this->tablename;
        $defObj    = $defs[$mode];
    
        $outstr = NULL;
       
    	if ($opt["menushow"]) {
    		
    		//KEY:TABLE_MENU
    		if ($defObj['bgimage']!=NULL ) $bgimage=$defObj['bgimage'];
    		
    		if ($mode=='list' or $mode=='single') $outstr .= '<a href="ohome.php?t='.$tablename.'">'.
    			'<img src="images/ich.home_sh.gif" border=0 title="object home"></a>';
    		if ($mode=='single') {
    			$outstr .= '<a href="view.tmpl.php?t='.$tablename.'"><img src="images/ich.list_sh.gif" border=0 title="list view"></a>';
    		}
    		/*
    		if ($mode=='list') {
    			echo '<a href="edit.tmpl.php?t='.$tablename.'&getIdHist=1"><img src="images/ich.single_sh.gif" title="single object" border=0></a>';
    		}
    		*/
    		$outstr .= "<img src=\"images/".$bgimage."\">"; // space for the MENU
    	}
    	return $outstr;
    }
    
    /**
     * show main toolbar
     * - the height of the bar can be controlled by param $info ...
     * - KEY:TABLE_1    : outer table
     * - KEY:TABLE_MENU : the menu table
     * - KEY:TABLE_2    : inner table
     * @param $sql
     * @param string $info
     *   with EMPTY string: the height of the header is SLIM!
     * @param $opt  
     *    "formslim" = 1 
     *    "headtxt"   header text
     *    "menushow" 0|1
     *    "name_obj", 
     *    "obj_id", 
     *    "NamePost" : for _single() text after object-name (e.g. SUA-Version)
     * @return -
     */
    function toolbar_show( &$sql, $info, $opt=NULL ) {
    
    
       $mode      = $this->mode;
       $tablename = $this->tablename;
       $this->menushow =  $opt["menushow"];
       
    	
       $tablename_l = strtolower($tablename);
       $bgimg		= "images/obj.".$tablename_l.".jpg";
       $icon		= "images/icon.".$tablename.".gif";
       if ( !file_exists($icon) ) $icon="images/icon.UNKNOWN.gif";
       
       $objcolor = globTabMetaByKey( $tablename, 'COLOR');
       if ( $objcolor=="" ) $objcolor="#E0E0E0";
       
       $modedefs=array();
       $modedefs['TAB_MENU_bg']='#dfdfef';
       switch ( $mode ) {
    		case "home":  
    			//$modedefs['TAB_MENU_bg']='#efefef';
    			$modedefs['Colspan']=' colspan=2';
    			break;
       }
    	
        //KEY:TABLE_1
       echo "<table cellpadding=0 cellspacing=0 border=0 width=100%><tr><td>"; // old: blue frame: bgcolor=#3366cc cellpadding=1
        
    	
        
        echo '<!-- KEY:TABLE_2 //-->'."\n";
    	echo '<table cellpadding=1 cellspacing=0 border=0 width=100%  bgcolor="#efefef"><tr>';
        
    	echo '<td style="';
    	if ( file_exists($bgimg) )
    		echo 'background-image:url('.$bgimg.'); background-repeat:no-repeat;';
    	echo ' white-space:nowrap;  background-color:'.$objcolor.'; width:75px; ">';
        echo '<img src="0.gif" height=1 width=75></td>';
    	echo '<td width=7 bgcolor='.$objcolor.'><img src=0.gif width=7 height=1></td>'."\n";
    	
        
    	// TABLE_LEFT:START
        switch ( $mode ) {
    		case "single":  
    			$this->_single($icon, $opt, $info);
    			echo "</table>\n"; // TABLE_LEFT:END
    			break;
    			
    		case "home":  
    			$this->_home($info);
    			echo "</tr></td></table>\n"; // TABLE_LEFT:END
    			break;
    		
         	case "list":
         		echo '<td>';
    			$nicename	= tablename_nice2($tablename); 
    			echo '<!-- KEY:TABLE_3 //-->'."\n";
    			echo '<table style="margin-left:5px;" cellpadding=0 cellspacing=0 border=0><tr>';
    			echo '<td valign=top width=10%>'."\n";
    			
    			echo '<!-- KEY:TABLE_LEFT //-->'."\n";
    			echo "<table cellpadding=0 cellspacing=0>\n";
    			echo '<tr><td style=" white-space: nowrap; padding-top:6px; padding-bottom:8px;">';
    			//echo "<img src=\"0.gif\" height=3 width=1><br>";
    			echo "<img src=\"" .$icon. "\" border=0 hspace=3> ";
    			$shortx = globTabMetaByKey( $tablename, 'SHORT' );
    			if ($shortx!=NULL) {
    				$shortout = '&nbsp;&nbsp;&nbsp;<span style="color:gray;">('.$shortx.')</span>';
    			} else $shortout = NULL;
    			echo '<span style="color:#0049BB; font-size:1.2em; font-weight:bold; ">'.$nicename. '</span>'.$shortout.' &nbsp;';
    			echo "</td>\n</tr>\n";
    			if ( $info!="" ) {	
    				echo "<tr><td NOWRAP>".$info."</td></tr>\n";
    			}
    			echo '<!-- KEY:TABLE_LEFT //-->'."\n";
    			echo "</table>\n";
    			break;
    				
        }
        
        echo "</td>\n";
         
    }
    
    
    function toolbar_end( $lastLine=NULL ) {
    	
    	
    	if ( $this->mode=='list') {
    		echo '<!-- KEY:TABLE_3 //-->'."\n";
    		echo "</tr></table></td>\n"; 	//KEY:TABLE_3
    	}
    	
    	echo '<!-- KEY:TABLE_2 //-->'."\n";
    	echo "</tr></table>\n"; 	//KEY:TABLE_2
    	
    	echo "</td></tr></table>\n"; //KEY:TABLE_1
    }

}
