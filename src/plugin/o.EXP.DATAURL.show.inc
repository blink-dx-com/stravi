<?php
require_once ("o.EXP.DATAURL.subs.inc");
require_once('f.directorySub.inc');
require_once 'f.rider.inc';

/**
 * show DATAURL details
 *
 * @package o.EXP.DATAURL.show.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq  UREQ:0003969: o.CONTACT > add related contact
 * @param  $id : EXP_ID
 * @param  $subdir : string subdir
 * @param  $f : file 
 * @param  $d : 0,1 download ???
 */
class o_EXP_DATAURL_show_XPL extends gPlugin {
    
    
    function register() {
        $tablename	= 'EXP';
        $title		  = 'Raw Data details';
        
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
    
    private function _is_image( $fullpath ) {
        
        $supported_image = array(
            'gif',
            'jpg',
            'jpeg',
            'png'
        );
        
        $is_image = 0;
        
        $ext = strtolower( pathinfo($fullpath, PATHINFO_EXTENSION));
        if (in_array($ext, $supported_image)) {
            $is_image = 1;
        } 
        return  $is_image;
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
        echo "&nbsp;&nbsp;&nbsp;<img src=\"f.image_show.php?filename=".$tmpimg_enc. $dimen_para.$optpar."\" border=0><br>\n";
    }
    
    function _path_join($path1, $path2) {
        
        $newpath = $path1;
        if ($path2!=NULL) {
            if ($newpath!=NULL) $newpath .= '/';
            $newpath .= $path2;
        } else {
            // nothing;
        }
        

        return $newpath;
    }
    
    function _show_NAV($links, $subdir, $select_key) {
        
       
        foreach($links as $key => $row) {
            
            $style='';
            if ($key!=$select_key) {
                $style=' style="filter: opacity(50%);"';
            }
            $row_rider = array(
                '<img src="images/'. $row['icon'].'" height=25 vspace=2 '.$style.' title="'.$row['txt'].'"> ',
                $this->_plug_get_URL().'&id='.$this->objid.'&subdir='.$subdir.'&'.$row['param']);
            
            echo '<a href="'.$row_rider[1].'">'.$row_rider[0].'</a>'."\n";
        }
    }
    
    function startMain() {
        global $error;
        
        $FUNCNAME='startMain';
        
        $this->objid = $_REQUEST['id'];
        $subdir      = $_REQUEST['subdir'];
        $parx        = $_REQUEST['parx'];
        
        $sqlo = &$this->_sqlo;
        $MAX_files = 1000;
        $MAX_COLS  = 5;
        
        $exp_feats = glob_elemDataGet3( $sqlo, 'EXP', array('EXP_ID'=>$this->objid), array('DATAURL') );
        $DATAURL = $exp_feats['DATAURL'];
        
        $view_opt = $_SESSION['s_formState']['o.EXP.DATAURL.show'];
        
        if ( $view_opt['view.grid']=='') {
            $view_opt['view.grid'] = 'list';
        }
        if ( $view_opt['view.zoom']<=0) {
            $view_opt['view.zoom'] = 1;
        }
        if ( $view_opt['view.zoom']>10) {
            $view_opt['view.zoom'] = 10;
        }
        
        
        // save $parx values 
        if (is_array($parx)) {
            foreach ($parx as $key=>$val) {
                if ( $parx[$key]!='') {
                    $view_opt[$key] = $parx[$key];
                }
            }
        }
        $_SESSION['s_formState']['o.EXP.DATAURL.show'] = $view_opt;

        $style_img=NULL;
        $title_img = 'Exists on network';
        
        $answer = oEXP_DATAURL_subs::onNetExists($DATAURL);
        $linux_path   = $answer['path'];
        $windows_path = $DATAURL;
        
        if ($subdir!=NULL) {
            $linux_path .= '/'.$subdir;
            $windows_path = str_replace('/', '\\', $DATAURL.'\\'.$subdir);
            
        }
        
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

        
        
        $links=array(
            'list'=> array('param'=>'parx[view.grid]=list', 'txt'=>'list view', 'icon'=>'ic.list.png'),
            'grid'=> array('param'=>'parx[view.grid]=grid', 'txt'=>'grid view', 'icon'=>'ic.grid.png'),
        );
        $view_grid_key = $view_opt['view.grid']; 
        $this->_show_NAV($links, $subdir, $view_grid_key);

        echo '&nbsp;&nbsp;&nbsp;';
        $links=array(
            'x' => array('param'=>'parx[view.zoom]='.($view_opt['view.zoom']+1),  'txt'=>'zoom in',  'icon'=>'but.40.zoom_plus.png'),
            'y'=> array('param'=>'parx[view.zoom]='.($view_opt['view.zoom']-1), 'txt'=>'zoom out', 'icon'=>'but.40.zoom_minus.png'),
            'z'=> array('param'=>'parx[view.zoom]=1', 'txt'=>'zoom reset', 'icon'=>'but.40.zoom.png'),
        );
       
        $this->_show_NAV($links, $subdir, $view_opt['view.zoom']);
    
        
        //$rider_lib = new fRiderC(); 
        // $rider_lib->riderShow( $xmodes, $view_grid_key, '' );
        
        echo "<br><br>";
        

        $directory_lib = new fDirextoryC();
        $dirs = $directory_lib->scanDir($linux_path, '', 1);
        $files= $directory_lib->scanDir($linux_path, '', 2);
        
        $subdir_base = $subdir;
        if ($subdir!=NULL) $subdir_base .='/';
        
        if ($subdir!=NULL) {
            $use_back_dir = dirname($subdir);
            if ($use_back_dir=='.') $use_back_dir='';
            echo '<a href="'.$this->_plug_get_URL().'&id='.$this->objid.'&subdir='.$use_back_dir.
              '"><img src="images/icon.PROJ.gif" height=20>  ..'."</a><br>\n";
        }
        
        foreach($dirs as $dir) {
            
            $loop_subdir_full = $subdir_base . $dir;
            echo '<a href="'.$this->_plug_get_URL().'&id='.$this->objid.'&subdir='.$loop_subdir_full.
                '"><img src="images/icon.PROJ.gif" height=20>  '.htmlspecialchars($dir)."</a><br>\n";
        }
        
        $cnt=0;
        $div_tag = '<div>';
        if ($view_grid_key=='grid') {
            $div_tag = '<div style="display:inline-block; font-size:0.8em;" >';
        }
        $col_cnt=0;
        
        foreach($files as $filex) {
            
            if ($cnt>$MAX_files) {
                echo "<br>";
                $this->_html->messageBox('Too many files ('.sizeof($files).')', '', 'WARN', 'Max files '.$MAX_files.' are allowed here for visualization.');
                break;
            }
            echo $div_tag;
            
            $is_image = $this->_is_image( $filex );
            
            $loop_file_full = $windows_path.'\\'.$filex;
            $tmp_file_enc   = urlencode($loop_file_full);
           
            echo '<img src="images/ic.document.png" height=20> <a href="p.php?mod=LAB/f.filesys_down&file='.$tmp_file_enc.'">'.
                htmlspecialchars($filex)."</a><br>\n";
            
            if ( $is_image ) {
            
                $loop_fullpath = $this->_path_join($windows_path , $filex);
                $loop_fullpath = str_replace("/", "\\", $loop_fullpath);
                $this->_show_thumb( $loop_fullpath, $view_opt['view.zoom'] );
            }
            echo '</div>'."\n";
            
            if (!$is_image or $view_grid_key!='grid') {
                echo '<p/>'."\n";
            }
            if ($is_image and $view_grid_key=='grid') {
                if ($col_cnt > $MAX_COLS) {
                    // echo '<p/>'."\n";
                    $col_cnt=0;
                }
                $col_cnt++;
            }
            
            $cnt++;
        }
    }
    
}