<?php
/**
 * extension for PUR
 * @package obj.pur.xedit.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
require_once ("visufuncs.inc");
require_once ("f.objview.inc");	
require_once 'o.CCT_ACCLOG.subs.inc';
require_once ('func_form.inc');


/**
 * show PUR
 * @package    edit.tmpl.php
 */
class oPUR_xedit_acts {
    
    const table='PUR';
    
    const goArray = array(
        1=>'started',
        2=>'sent',
        3=>'ordered',
        4=>'received',
        5=>'closed',
    );
    
    function __construct($sqlo, $id) {
        
        $this->id = $id;
        
        $features = glob_elemDataGet3( $sqlo, self::table, array('PUR_ID'=>$this->id), array('CCT_ACCESS_ID', 'ABSTRACT_SUBST_ID') );
        $CCT_ACCESS    = $features['CCT_ACCESS_ID'];
        $this->SUA_ID  = $features['ABSTRACT_SUBST_ID'];
        
        $accLogLib   = new oAccLogC();
        $accLogLib->setObject( $sqlo, self::table, $this->id, $CCT_ACCESS );
        
        $this->status_name = $accLogLib->getLastLogName($sqlo);
        $this->go_index    = array_search($this->status_name, self::goArray);
        
    }
    
    
    /**
     * show status progress
     * @param string $dochelp e.g. 'lab_blk/...'
     * @param string $list_link e.g. 'LAB/o.CYTORD.sover'
     */
    function progressShow($editAllow) {
        
        $tablename= self::table;
        $goArray  = self::goArray;
        $lastGo_ID= sizeof($goArray);
        
        $status_name = $this->status_name;
        
        $extratext=NULL;
        $link_array = array();
        
        $goID = array_search($status_name,$goArray);
        if ($goID<$lastGo_ID) {
            
            $next_go = $goID+1;
            $nextstatus=$goArray[$next_go];
            
            if ($editAllow) {
                
                $status_set_url='glob.obj.acclock.php?t='.$tablename.'&id='.$this->id.
                '&parx[statName]='.$nextstatus.'&parx[status.mod]=-1';
                
                $extratext .= '<a href="'.$status_set_url.'"><img src="images/arrow.but.gif"> Next Status</a>'.
                    ' &nbsp;&nbsp;<a href="glob.obj.acclock.php?t='.$tablename.'&id='.$this->id.
                    '&parx[statName]=rejected"><img src="images/but13.del.gif"> REJECT</a>';
                
                $link_array = array();
                $next_go_loop = $next_go;
                
                $nextstatus = $goArray[$next_go_loop];
                $status_set_url='glob.obj.acclock.php?t='.$tablename.'&id='.$this->id.
                '&parx[statName]='.$nextstatus.'&parx[status.mod]=-1';
                
                $link_array[$next_go_loop] = $status_set_url;
                
                $next_go_loop = $next_go_loop + 1;
                
            }
        }
        
        echo '<div style="display:flex;">'."\n";
        
        $formPageLib = new FormPageC();
        $formPageLib->init( $goArray);
        $formPageLib->goInfo2( $goID, $link_array );
        
        echo '</div>'."\n";
        
        if ($extratext) echo "</br>".$extratext."</br>\n";
    }
    
}


class obj_edit_ext extends gObj_edit_feat_ABS {
    
    function rightWindow( &$sqlo, &$sqlo2) {
        $id = $this->id;

        $o_rights = &$this->o_rights;
        $viewmode = $this->viewmode;
	
        $tablename='PUR';
        $editAllow = $o_rights["write"];
        if ($viewmode!= "edit" ) $editAllow = 0;
        
    	echo "<ul>\n";
    	
    	$help_lib = new oPUR_xedit_acts($sqlo, $id);
    	
    	if ($editAllow) {
    	    
    	    $mord_features = glob_elemDataGet3( $sqlo, $tablename, array('PUR_ID'=>$id), array('CCT_ACCESS_ID') );
    	    $CCT_ACCESS  = $mord_features['CCT_ACCESS_ID'];
    	    $accLogLib   = new oAccLogC();
    	    $accLogLib->setObject( $sqlo, $tablename, $id, $CCT_ACCESS );
    	    $status_name = $accLogLib->getLastLogName($sqlo);
    	    
    	    if ($status_name==NULL or $status_name=='started') {
    	    
        	    $nextstatus='sent';
        	    $extratext .= '<a href="glob.obj.acclock.php?t='.$tablename.'&id='.$id.
        	    '&parx[statName]='.$nextstatus.'"><img src="images/arrow.but.gif"> '.
        	    'Send '.tablename_nice2($tablename).'</a> (senden+beauftragen)';
        	    echo $extratext."<br><br>";
        	    
    	    } else {
    	        $help_lib->progressShow($editAllow);
    	    }
    	} else {
    	    $help_lib->progressShow(0);
    	}
 
    	echo "</ul>\n";
    }

}
