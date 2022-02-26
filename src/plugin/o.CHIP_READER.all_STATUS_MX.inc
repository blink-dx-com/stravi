<?php
require_once ("o.CHIP_READER.logs.inc");
require_once ("visufuncs.inc");
require_once 'f.objview.inc';
require_once ("glob.obj.update.inc");

/**
 * device: STATUS_MX recalc
 * @package o.CHIP_READER.all_STATUS_MX.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq 
 * @param $_REQUEST['go'] 
 */
class o_CHIP_READER_all_STATUS_MX_XPL extends gPlugin {
    
    
    function register() {
        $tablename	= 'CHIP_READER';
        $title		= 'Berechne STATUS_MX neu';
        
        $this->infoarr			 = NULL;
        $this->infoarr['scriptID'] = '';
        $this->infoarr['title']    = $title;
        $this->infoarr['form_type']= 'list';
        $this->infoarr['design']   = 'norm';
        $this->infoarr['obj_name'] = $tablename;
        $this->infoarr["obj_cnt"]  = 1;
        
        
        
        $this->infoarr2 = array(
            'accCheck' => array('tab'=>array('read','write'), 'obj'=>array('insert') ),
            'onlyAdmin' =>1,
            'listChkSelect' =>1
        );
        
        
    }
    
    /**
     * - analyse selected substances
     * - show them in a list
     * @return
     * @param object $sqlo
     */
    function show_list(&$sqlo, &$sqlo2, $sqlAfter) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
      
     
        $main_lib = new oREA_LOG_subs();
        $tablename='CHIP_READER';
        
        $objLinkLib = new fObjViewC();
        
        
       
        $tabobj = new visufuncs();
        $headOpt = array( "title" => "Devices" );
        $headx  = array ("Device", "OLD", "NEW", "Diff?");
        $tabobj->table_head($headx,   $headOpt);

        
        $sqlsel ='x.CHIP_READER_ID, x.STATUS_MX from '.$sqlAfter;
        $sqlo2->Quesel($sqlsel);
        
        while ( $sqlo2->ReadRow() ) {
            
            $devid   = $sqlo2->RowData[0];
            $mx_old  = $sqlo2->RowData[1];
            $mx_new  = $main_lib->calc_STATUS_MX($sqlo, $devid);
            
            $diff=0;
            if ($mx_old!=$mx_new) $diff=1;
            
            if ($this->go and $diff) {
                $args= array(
                    'vals'=>array('STATUS_MX'=>$mx_new)
                );
                $UpdateLib = new globObjUpdate();
                $UpdateLib->update_meta( $sqlo, $tablename, $devid, $args );
                if ($error->Got(READONLY))  {
                    $error->set( $FUNCNAME, 1, 'Error on device:'. $devid);
                    return;
                }
                
            }
            
            
            
            $dataArr=array($objLinkLib->bo_display($sqlo, $tablename, $devid), $mx_old, $mx_new, $diff);
            $tabobj->table_row ($dataArr);
        }
       
        $tabobj->table_close();
        
    }
    
    function form1() {
        
        $initarr   = array();
        $initarr["title"]       = "Update STATUS_MX"; // title of form
        $initarr["submittitle"] = "Update"; // title of submit button
        $initarr["tabwidth"]    = "AUTO";   // table-width: AUTO
        
        $hiddenarr = NULL; // hidden form variables
        
        $formobj = $this->_html->formInit($initarr, $hiddenarr, 0);

        $formobj->close( TRUE ); // close form, sshow submit-button
    }
    
    
    function startMain() {
        global $error;
        //$FUNCNAME='startMain';
        
        $this->go   = $_REQUEST['go'];
        $sqlo = &$this->_sqlo;
        $sqlo2 = &$this->_plug_regSql2();

        $sqlAfter = $this->listLib->getSqlAfter();
        
        if (!$this->go) {
            $this->form1();
        }
        
        $this->show_list($sqlo, $sqlo2, $sqlAfter);
            
    }
    
}