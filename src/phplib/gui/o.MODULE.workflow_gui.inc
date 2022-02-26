<?php
/**
 * 
 * @package o.MODULE.workflow_gui.inc
 * @swreq 
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @created 2012-01-01 
 */
/**
 * @package o.MODULE.workflow_gui.inc
 * show form for state connection (Workflow)
 */
class gui_MOD_Workflow {
    
    function __construct() {
    }
    
    function _getValByKey($sqlo, $mod_id, $key) {
        $sqlo->Quesel( "VALUE FROM MOD_META where MXID = " . $mod_id.' and KEY='.$sqlo->addQuotes($key) );
        $sqlo->ReadRow();
        $val =  $sqlo->RowData[0];
        return $val;
    }
    
    function init_by_module($sqlo, $mod_id) {
        
        
        $this->statesConnArr=NULL;
        $statesArrUnserial = $this->_getValByKey($sqlo, $mod_id, 'flow.def');
        if ($statesArrUnserial!=NULL) $this->statesConnArr=unserialize($statesArrUnserial);
        

    }
    
    /**
     * show form for state connection
     * - form-parameters: $garr['.$state_id.']['.$state_id2.']
     * @param string $destUrl
     * @param array $hiddenarr
     * @param array $statesConnArr $garr['.$state_id.']['.$state_id2.']
     * @return -
     */
    function showConnForm(&$sqlo, $title, $destUrl, $hiddenarr) {
        
        $statesConnArr = $this->statesConnArr;
        $module_id = $hiddenarr['id'];
        
        $object_table_sql = "VALUE from MOD_META where KEY = 'TABLE_NAME' and MXID = " . $module_id;
        $sqlo->Quesel($object_table_sql);
        $sqlo->ReadRow();
        $object_tablename = $sqlo->RowData[0];
        
        echo '<table border=0 cellspacing=1 cellpadding=0 bgcolor=#A0A0A0 ><tr><td>'.
            '<table border=0 cellspacing=0 cellpadding=2 bgcolor=#A0A0A0 width=100%><tr><td>';
        echo     '<font color=#FFFFFF><B>'.$title.'</b></font></td></tr></table>';
        echo '</td></tr><tr><td>';
        
        echo '<table cellspacing=0 cellpadding=0 bgcolor=#EFEFEF><tr><td>'."\n";
        echo '<form name="groups" style="display:inline;" method="post" action="'.$destUrl.'">'."\n";
        
        
        $id2names   = oH_ALOG_ACT_subs::getActionEntries($sqlo, $object_tablename);
        $stateNames = array_flip($id2names);
        
        $reopen_ACT_ID = $stateNames['reopen'];
        unset($stateNames['reopen']);
        
        $stateNames = array_merge( array('flow start'=>0), $stateNames);
        $stateNames['flow end'] = -2;
        $stateNames['reopen']   = $reopen_ACT_ID; // put to the end 
        
        $colspan  = sizeof($stateNames);
        
        oMODULE_helpx::showHiddenParams($hiddenarr);
        echo '<table cellspacing="1" border="0" frame="void">'."\n";
        echo '<tr valign=top bgcolor=#D0D0FF>'."\n";
        echo '<th>initial state</th>'."\n";
        echo '<th colspan="'.$colspan.'">destination state</th>'."\n";
        echo '</tr>'."\n";
        
        echo '<tr align="center" bgcolor=#D0D0FF>'."\n";
        echo '<td>&nbsp;</td>';
        foreach( $stateNames as $statename=>$stateid) {
            echo '<td>&nbsp;'.$statename.'&nbsp;</td>';
        }
        
        
        echo '</tr>'."\n";
        
        echo "<tr align=center bgcolor=#D0D0D0>";
        
        $statesFlowArr=array();
        reset ($stateNames);
        
        echo "</tr>\n";
        
        if ($statesConnArr != NULL){
            foreach ($statesConnArr as $state_id => $stateArr){
                foreach ($stateArr as $state_conn_id => $value){
                    if ($value == 1){
                        $statesFlowArr[$state_conn_id] = 2;
                    }
                }
                $statesFlowArr[$state_id] = 1;
            }
            
        } else {
            $statesFlowArr[0] = 2;
        }
        
        foreach ($statesFlowArr as $state => $value){
            if ($statesConnArr[$state])$statesFlowArr[$state] = 1;
        }
        
        
        $stateNames2 = $stateNames;
        if ( sizeof($stateNames) ) {
            foreach($stateNames as $state_name => $state_id) {
                $sqlo->query("SELECT name FROM H_ALOG_ACT WHERE H_ALOG_ACT_ID=".$state_id);
                $sqlo->ReadRow();
                $tmpStateName = $sqlo->RowData[0];
                if ($state_id == -2)$tmpStateName = 'flow end';
                if ($state_id == 0)$tmpStateName = 'flow start';
                if (array_key_exists($state_id, $statesFlowArr)){
                    if ($statesFlowArr[$state_id] == 2)  $tmpStateName = "<b>" . $tmpStateName . "</b>";
                    echo "<tr align=center>";
                    echo "<td align=right>".$tmpStateName."</td>";
                    foreach( $stateNames2 as $state_conn_id) {
                        $val=$statesConnArr[$state_id][$state_conn_id];
                        if ($state_id == $state_conn_id or $state_conn_id == 0 or $state_id == -2 or ($state_conn_id == -2 and $state_id ==0)){
                            echo '<td></td>';
                        } else{
                            echo '<td><input type=checkbox name=garr['.$state_id.']['.$state_conn_id.'] value=1';
                            if ($val)
                                echo " checked";
                                echo "></td>";
                        }
                    }
                   
                    echo "</tr>\n";
                }
            }
            
        }
        
        
        
        echo "</tr>\n";
        
        echo "<tr bgcolor=#C0C0FF><td colspan=".($colspan+1)." align=center>";
        echo "<img height=5 width=1><br>";
        echo "<input type=submit value=\"Save\"><br>";
        echo "<img height=5 width=1>";
        echo "</td></tr>\n";
        echo "</table>\n";
        
        echo "</form>\n";
        
        echo '</td></tr><table>';
        echo '</td></tr><table>';
        echo '</table></table>';
        echo "</form>\n";
    }
    
    /***
     * function for help
     * return -
     */
    function help($key) {
        echo "<br />\n";
        if ($key == 'flowdef')
            htmlInfoBox( "Help", "", "open", "HELP" );
            ?>
    	<ul>
    	<li>use a graph to define dependencies between states</li>
    	<li>define the connections  between states from the graph via the checkboxes</li>
    	<li>the connection defines the states, which can be choosen in the access of an object</li>
    	<li>first there are the start states to be defined (no other states are possible at the beginning)</li>
    	<li> if states are choosen, they appear as initial states</li>
    	<li>if the later selection of one state (destination state), when the initial state is set, should be possible, 
    	 activate the checkbox</li>
    	<li>the flow end defines the end states (no selection of further states will be possible)</li>
    	</ul>
    	<?
    	htmlInfoBox( "", "", "close" );
    	
    	echo "<br />\n";
    }

}
