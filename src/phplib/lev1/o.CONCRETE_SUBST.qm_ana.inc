<?php
require_once 'o.CCT_ACCLOG.subs.inc';
require_once 'o.H_ALOG_ACT.subs.inc';
require_once 'o.CONCRETE_SUBST.subs.inc';

/**
 * QM analysis : level1
 * @package o.CONCRETE_SUBST.qm_ana.inc
 * @swreq 
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class o_CONCRETE_SUBST_qm_ana_lev1{

    function set_suc($sqlo, $suc_id) {
        $this->suc_id=$suc_id;
        
        $suc_id=$this->suc_id;
        $this->suc_features = glob_elemDataGet3( $sqlo, 'CONCRETE_SUBST',
            array('CONCRETE_SUBST_ID'=>$suc_id), array('CCT_ACCESS_ID', 'CERT_FLAG') );
        $CCT_ACCESS_ID = $this->suc_features['CCT_ACCESS_ID'];
        
        $this->auditLogLib = new oAccLogC();
        $this->auditLogLib->setObject( $sqlo, 'CONCRETE_SUBST', $suc_id, $CCT_ACCESS_ID );
    }
    
    /**
     * 
     * @param object $sqlo
     * @return array $releaseInfo
     */
    function getUpfrontInfo($sqlo) {

        $CERT_FLAG = $this->suc_features['CERT_FLAG'];
        $statusPlanned = 'released';
        if ($CERT_FLAG==2) $statusPlanned = 'D_released';
        if ($CERT_FLAG==3) $statusPlanned = 'R_released';
        
        $releaseInfo      = $this->auditLogLib->getUpfrontInfo($sqlo, $statusPlanned);
        return $releaseInfo;
    }
    
    /**
     * 
     * @param object $sqlo
     * @param array $releaseInfo
     * @return array
     *    'head'=>"QM Warnings", 'text'=>$releaseInfo['txt'].$ext_info, 'key'=> "WARN" 
     */
    function get_messages_nice($sqlo, &$releaseInfo) {
        $suc_id=$this->suc_id;
        $curr_statusName=NULL;
        $CCT_ACCLOG_stat = $this->auditLogLib->getLastLog($sqlo);
        if ($CCT_ACCLOG_stat)
            $curr_statusName = oH_ALOG_ACT_subs::statusID2Name( $sqlo, $CCT_ACCLOG_stat );
        
        $readyStateArr = array('released', 'D_released', 'R_released');
        $finishStateArr= array('rejected', 'finished');
        
        $statusAlias = 'unknown';
        if ( in_array($curr_statusName,$readyStateArr) ) $statusAlias  = 'release';
        if ( in_array($curr_statusName,$finishStateArr) ) $statusAlias = 'finish';
        
        $mess_arr=NULL;
        
        switch ($statusAlias) {
            case 'release':
                // this is a finale status
                // get short QC-info ...
                $substQclib = new oConcSubst_subC();
                $substQclib->initSubst($sqlo, $suc_id);
                $expiryInfo = $substQclib->checkExpiry( $sqlo );
                $expirDays  = $expiryInfo["days"];
                
                
                if ($expiryInfo!=NULL) {
                    echo '<br />';
                    if ($expirDays>0) {
                        $qc_info = "still <font color=green>$expirDays</font> days";
                        $mess_arr=array( 'head'=>"Expiry info", 'text'=>$qc_info, 'key'=>"INFO" );
                    } else {
                        $qc_info = "EXPIRED since:".abs($expirDays). " days. ".
                            'It is strongly recommended to "finish" this object!';
                        $qc_problem  = 1;
                        $mess_arr=array( 'head'=>"QM Warnings", 'text'=>$qc_info, 'key'=>"WARN" );
                    }
                }
                break;
            case 'finish':
                // no action
                break;
            default:
                if ($releaseInfo['txt']!=NULL) {
                    $ext_info=NULL;
                    if ( $_SESSION["userGlob"]["g.debugLevel"]>1 ) {
                        $ext_info = '; DEBUG-Info: '.print_r($releaseInfo,1);
                    }
                    $mess_arr=array( 'head'=>"QM Warnings", 'text'=>$releaseInfo['txt'].$ext_info, 'key'=> "WARN" );
                }        
        }
        
        return $mess_arr;
    }
}