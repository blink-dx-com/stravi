<?php
require_once ("o.EXP.DATAURL.subs.inc");
require_once('f.directorySub.inc');
require_once 'o.EXP_H_IMG.subs.inc';
require_once 'o.ABS_SOCKET.subs.inc';

/**
 * show DATAURL details
 *
 * @package o.EXP.IMG_show.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param  $id : EXP_ID
 */
class o_EXP_IMG_show_XPL extends gPlugin {
    
    
    function register() {
        $tablename	= 'EXP';
        $title		  = 'Show images';
        
        $this->infoarr			 = NULL;
        $this->infoarr['title']    = $title;
        $this->infoarr['form_type']= 'obj';
        $this->infoarr['design']   = 'norm';
        $this->infoarr['obj_name'] = $tablename;
        $this->infoarr['obj_id']   = $_REQUEST['id'];
        $this->infoarr['checkid']  = 1;
        
        
        $this->infoarr2 = array(
            'accCheck' => array('tab'=>array('read'), 'obj'=>array('read') )
        );
        
    }
    
   
    
    private function _show_thumb( $fullpath, $zoom ) {
        
   
        $serialOpt=array();
        $serialOpt["zoom"] = $zoom;
        
        $dimOpt=array();
        $dimOpt["dimx"] = 150*($serialOpt["zoom"]+1);
        $dimOpt["dimy"] = 100*($serialOpt["zoom"]+1);
        
        $dimen_para = "&dim[0]=".$dimOpt["dimx"]."&dim[1]=".$dimOpt["dimy"];
        $optpar  = "&useDefParams=1";
        
        $tmpimg_enc = urlencode($fullpath);
        // echo "XXX:|$fullpath|<br>\n";
        echo "<img src=\"f.image_show.php?filename=".$tmpimg_enc. $dimen_para.$optpar."\" border=0>\n";
    }
    

    
    function _one_img( $pos, $windows_path, $filex ) {
        
        $loop_file_full = fDirextoryC::join_path(array($windows_path,$filex));
        $tmp_file_enc   = urlencode($loop_file_full);
        
        // '<img src="images/ic.document.png" height=20>'.
        echo ' <a href="p.php?mod=LAB/f.filesys_down&file='.$tmp_file_enc.'">';
        $loop_fullpath = str_replace("/", "\\", $loop_file_full);
        $this->_show_thumb( $loop_fullpath, 0 );
        echo "</a>\n";
        //echo "<br>";
    
    }
    
    //  x,y start with 0))
    private function _mtp_XY2pos($x, $y) {
        $pos = $x + $y*$this->xcnt + 1;
        return $pos;
    }
    
    function startMain() {
        global $error;
        
        $FUNCNAME='startMain';
        $tablename='EXP';
        
        $this->objid = $_REQUEST['id'];
        //$subdir      = $_REQUEST['subdir'];
        //$parx        = $_REQUEST['parx'];
        
        $sqlo = &$this->_sqlo;
        $MAX_files = 1000;
      
        $exp_img_lib = new oEXP_H_IMG_subs($this->objid);
        $img_list    = $exp_img_lib->get_img_data($sqlo);
        if (empty($img_list)) {
            $this->htmlFoot('ERROR', 'EXP: no images.' );
            return;
        }
        $view_opt=array();
        
        $exp_feats = glob_elemDataGet3( $sqlo, $tablename, array('EXP_ID'=>$this->objid), array('DATAURL', 'EXP_TMPL_ID', 'SOCKET_ID') );
    
        
        if ( $view_opt['view.zoom']<=0) {
            $view_opt['view.zoom'] = 1;
        }
        if ( $view_opt['view.zoom']>10) {
            $view_opt['view.zoom'] = 10;
        }
        
        
        if ($exp_feats['DATAURL']==NULL) {
            $this->htmlFoot('ERROR', 'EXP: DATAURL is missing.' );
            return;
        }
        if ($exp_feats['SOCKET_ID']==NULL) {
            $this->htmlFoot('ERROR', 'EXP: SOCKET_ID is missing.' );
            return;
        }
        $DATAURL=$exp_feats['DATAURL'];
        
        $cart_feats = glob_elemDataGet3( $sqlo, 'SOCKET', array('SOCKET_ID'=>$exp_feats['SOCKET_ID']), array( 'ABS_SOCKET_ID') );
        if ($cart_feats['ABS_SOCKET_ID']==NULL) {
            $this->htmlFoot('ERROR', 'EXP:SOCKET:ABS_SOCKET_ID is missing.' );
            return;
        }
        $ABS_SOCKET_ID = $cart_feats['ABS_SOCKET_ID'];
        
        $abs_cart_lib = new oABS_SOCKET_subs();
        $abs_cart_lib->init($sqlo, $ABS_SOCKET_ID);
        $H_SOCK_TYPE_data = $abs_cart_lib->get_H_SOCK_TYPE_data();
        
        
        $style_img=NULL;
        $title_img = 'Exists on network';
        
        $answer = oEXP_DATAURL_subs::onNetExists($DATAURL);
        $linux_path   = $answer['path'];
        $windows_path = $DATAURL;
        echo 'Path: '.$windows_path."<br>\n";

        if ( !$answer['exists'] ) {
            $title_img = $DATAURL. ' not exists on network';
            $style_img='style="filter: grayscale(100%);"';
        }
        $head_text  = '<img src="images/ic.network.gif" '.$style_img. '> '. $title_img;
        $head_text .= "<br>\n";
        
        
        if ( !$answer['exists']) {
            echo $head_text;
            return;
        }

        $img_pos = array();
        foreach($img_list as $row) {
            $img_pos[$row['POS']] = $row;
        }
        
        $this->xcnt = $H_SOCK_TYPE_data['xcnt'];
        $this->ycnt = $H_SOCK_TYPE_data['ycnt'];

        echo "<br><br>";

        
        echo "<table>\n";
        echo '<tr>'."\n";
        echo '<td></td>';
        for ($x=0; $x<$this->xcnt; $x++) {
            echo '<td>'.($x+1).'</td>';
        }
        echo '</tr>'."\n";
        
        for ($y=0; $y<$this->ycnt; $y++) {
            
            echo '<tr>'."\n";
            $legend_y = chr(65+$y);
            echo '<td>'.$legend_y.'</td>';
            
            for ($x=0; $x<$this->xcnt; $x++) {

                $pos = $this->_mtp_XY2pos($x, $y);
                
                $row = $img_pos[$pos];
                echo '<td>';
                if (!empty($row)) {
                    $filex=$row['URL'];
                    $pos  =$row['POS'];
                   
                    $this->_one_img($pos, $linux_path, $filex);
                }
                echo '</td>';
            }
            echo '</tr>'."\n";

        }
        echo "</table>\n";
    }
    
}