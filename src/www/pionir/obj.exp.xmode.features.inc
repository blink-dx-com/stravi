<?php
/**
 * experiment single page extension
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @app_type_info will be overwritten by type:2021_abbott
 */

require_once("glob.image.inc");
require_once ("f.objview.inc");

require_once("o.EXP.subs.inc");
require_once("o.EXP.RESULT.inc");
require_once 'o.EXP_H_IMG.subs.inc';
require_once 'o.EXP_TMPL.subs.inc';
require_once "o.EXP.DATAURL.subs.inc";
require_once 'o.EXP.proto.inc';
require_once 'o.SOCKET.subs.inc';
require_once 'o.PROTO.steps.inc';



class oEXP_xedit {
    
    private $argu;
    
    function __construct($id, &$argu, $editAllow) {
        $this->id   = $id;
        $this->argu = &$argu;
        $this->editAllow = $editAllow;
        
        $this->page_ul  = "<ul style='margin-top:3px;'>\n";
    }
    
    function get_page_ul() {
        return $this->page_ul;
    }
    
    function _tab_in($title, $text) {
        
        echo "<table border=0 cellpadding=0 ceppspacing=0><tr>";
        if ($text!="") {
            echo "<td width=150 nowrap>";
            echo "<img src=\"images/greenball.gif\"> <B>".$title. ":</B></td><td>".$text."</td>";
        } else {
            echo "<td nowrap colspan=2>";
            echo "<img src=\"images/greenball.gif\"> <B>".$title. ":</B></td>";
        }
        echo "</tr></table>\n";
    }
    
    function _tab_in2($title, $text) {
        echo "<img src=\"images/greenball.gif\"> <B>".$title. ":</B> &nbsp;&nbsp;&nbsp;".$text;
    }
    
    /**
     * show diagram header
     * @param $tmp_globvar
     * @param $icon
     * @param int $def_value
     *    -1 -- inactive
     *     1 -- active
     * 
     */
    function _exp_diagram_header($tmp_globvar, $icon, $title, $def_value=NULL) {
        $val_now = $_SESSION['userGlob'][$tmp_globvar];
        if ($val_now===NULL and $def_value!==NULL) {
            $val_now=$def_value;
        }
        
        $output='';
        if ( $val_now<0 ) {
            $output .= '<a href="preferences.php?variable='.$tmp_globvar.'&val=1">';
            if ($icon!='') $output .= '<img src="images/'.$icon.'" border="0" TITLE="show Graphics">';
            $output .= ' <img src="images/ic.eye.gif" TITLE="show Graphics" border=0></a> ';
        } else {
            if ($icon!='') $output .= '<img src="images/'.$icon.'">';
            $output .= " <a href=\"preferences.php?variable=".$tmp_globvar."&val=-1\" >";
            $output .= "<img src=\"res/img/x.svg\" TITLE=\"Close Graphics\" height=20></a> ";
        }
        if ($title) {
            $output .= '<span class="yGgray">['.$title.']</span> ';
        }
        
        return $output;
    }
    
    /**
     * show IMG
     * @param object $sql
     */
    function sh_IMG($sqlo) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $id = $this->id;
        $editAllow = $this->editAllow;
        $cart_KEY='';
        
        if ($this->argu['EXP_TMPL_ID']) {
            $exp_tmp_lib = new oEXP_TMPL_subs($this->argu['EXP_TMPL_ID']);
            $cart_KEY = $exp_tmp_lib->get_cart_SOCK_TYPE_KEY($sqlo);
        }
        
        debugOut("cart_KEY: $cart_KEY", $FUNCNAME, 1);
        
        if ($cart_KEY=='07') {
            // only this type has images ...
            
            $exp_img_lib = new oEXP_H_IMG_subs($id);
            $img_list    = $exp_img_lib->get_img_data($sqlo);
    
            $this->_tab_in2("Images",'&nbsp;');
            
            if (empty($img_list)) {
                if ( $editAllow ) {
                    $imgText .= ' [<a href="p.php?mod=LAB/o.EXP.IMG_07_create&id='.$id.'"><img src="images/but9.new.gif"> Import images</a>]'; 
                } else {
                    $imgText = '<span style="color:gray;">no image.</span>';
                }
               
                
            } else {
                $imgText = "";
                $image_obj_txt = "";
                $imageExists = FALSE;
    
                //$image_obj_txt .= " [<a href=\"obj.img.series_show.php?id=".$image_id."&showimages=1\">".
                //        "<img src=\"images/ic.oIMG.series.gif\" border=0> Show all thumbnails</a>]".
    
                
                if ( $imageExists ) {
                    if ( $_SESSION['userGlob']["o.IMG.noThumbshow"]>=1 ) 
                        $imgText .=  "&nbsp;<a href=\"preferences.php?var=o.IMG.noThumbshow&val=\">".
                            "<img src=\"images/ic.eye.gif\" border=0> Show image thumbnail</a>&nbsp;";
                    else $imgText .= "&nbsp;<a href=\"preferences.php?var=o.IMG.noThumbshow&val=1\">".
                       "<img src=\"images/ic.eyeNo.gif\" border=0 title=\"Hide image thumbnail\"></a>&nbsp;";  
                }
                $imgText .= '<a href="p.php?mod=DEF/o.EXP.IMG_show&id='.$id.'">Show images ('.sizeof($img_list).')</a>';
                $imgText .= $image_obj_txt;
                
                //if ( $editAllow )  $imgText .= " [<a href=\"".$tmpImgLinkNew."\"><font color=gray>Add an other Image</font></A>]" ." ";
                //if ( $img_morethanone && $editAllow ) $imgText .= $tmpseries;

            }
            echo $imgText."<br>";
        }
        
    }
    
    /**
     * show cartridge
     * @param object $sql
     * @param int $cart_id
     */
    function sh_SOCKET( &$sqlo, $cart_id) {
        
        $objLinkLib = new fObjViewC();
        $cart_link_html=NULL;
        
        if ($cart_id) {
            $cart_link_html = $objLinkLib->bo_display( $sqlo, 'SOCKET', $cart_id );
        }
        
        $tmptxt =" &nbsp;&nbsp;".$cart_link_html;
        
        //$icon='but.expres2d.gif';
        $tmp_globvar = "o.EXP.CartShow";
        $tmptxt .= " &nbsp;&nbsp;&nbsp; " . $this->_exp_diagram_header( $tmp_globvar, '', '', 1 );

        $this->_tab_in("Cartridge view",$tmptxt);
        
        
        if ( $_SESSION['userGlob'][$tmp_globvar]>=0 ) {
            
            echo $this->get_page_ul();
            
            if (!$cart_id) {
                echo "&nbsp;&nbsp;&nbsp;";
                echo 'no cartridge on experiment.'."\n";
            } else {
           
                echo "<br>\n";
                $filename =  $_SESSION['s_sessVars']['AppLabLibDir'] . '/lablib/o.SOCKET.showcart.inc';
                if (file_exists($filename)) {
                    require_once $filename;
                    
                    $sh_opt=array('suc.distinct.colors'=>1);
                    $show_lib = new oSOCKET_showcart($sqlo, $cart_id);
                    $show_lib->show($sqlo, $sh_opt);
                }
            }
            
            echo "<br>\n";
            echo "</ul>\n";
        }
       
    }
    
    /**
     * show raw data 
     * @param object $sql
     * @param string $DATAURL
     */
    function sh_DATAURL($sql, $DATAURL) {
        

        
        $style_img=NULL;
        $title_img = 'Raw data directory exists on network';
        
        $answer = oEXP_DATAURL_subs::onNetExists($DATAURL);
        if ( !$answer['exists'] ) {
            $title_img = 'Not exists on network';
            $style_img='style="filter: grayscale(100%);"';
        } 
        //$head_text  = ' <img src="images/ic.network.gif" '.$style_img. ' title="'.$title_img.'">';
        $head_text = ' <a href="p.php?mod=DEF/o.EXP.DATAURL.show&id='.$this->id.'">'.
            '<img src="images/ic.proj.zoom.png" height=25 '.$style_img. ' title="Browse details">'.
            '&nbsp;&nbsp;&nbsp;'.$title_img."</a> \n";
        $head_text .= "<br>\n";
        
        $this->_tab_in("Raw Data",$head_text);
        
        if ( !$answer['exists'] ) return;
        
        
        
    }
}

// ------------------





class obj_edit_ext extends gObj_edit_feat_ABS {
    
    function rightWindow( &$sqlo, &$sqlo2) {
        $id = $this->id;
        
        $argu  = &$this->argu;
        $o_rights = &$this->o_rights;
        $viewmode = $this->viewmode;
        
        $editAllow = $o_rights["write"];
        if ($viewmode!= "edit" ) $editAllow = 0;
        
        $helplib = new oEXP_xedit($id, $argu, $editAllow);
        
        if ($argu['DATAURL']!=NULL) {
        $helplib->sh_DATAURL($sqlo, $argu['DATAURL']);
        }
        
        $helplib->sh_IMG($sqlo);
        
        
        $cart_id = $argu['SOCKET_ID'];
        if ($cart_id) {
          $helplib->sh_SOCKET($sqlo, $cart_id);
        }
        
        /*
        $helplib->_tab_in("Result views","");
        echo $helplib->get_page_ul();
        
        $tmp_globvar = "o.EXP.MxShowArray";
        $helplib->_exp_diagram_header( $tmp_globvar, 'but.expres2d.gif', '2D-array' );
        echo "bioarray geometry [<a href='obj.exp.mxshow.php?exp_id=$id'>Customized</a>]<br>\n";	
        if ( $_SESSION['userGlob'][$tmp_globvar] ) { 
        echo "&nbsp;&nbsp;&nbsp; TBD ...";
        echo "<br><br>\n";
        }
        */
        
        echo '</ul>';
    }

}


/**
 * show samples : Dealing with 'IS_SAMPLE'-flag:
 * @param object $sql
 * @param int $id
 * @return array
 */
function x_assoc_show( &$sql, $id ) {
    $dataarr=array();

    $samplesIDs = oEXP_subs::getSamples($sql, $id);
    if ( sizeof($samplesIDs) ) { 
        $colname  = "sample";
        $icon     = "images/icon.SAMPLE.gif";
        $tmpval   = "";  
        $samples  = oEXP_subs::getSampleNamesOfArr($sql, $samplesIDs); 
        foreach($samples as $sample_id => $sample_name) {
            if ( empty($sample_name) ) $sample_name = "sample without name";
            $tmpval .= "<a class=xl href=\"edit.tmpl.php?t=CONCRETE_SUBST&id=$sample_id\"><B>$sample_name</B></a><br>\n";
        }  
        $dataarr[0] = array( $icon, $colname, $tmpval, "samples of experiment" );
    }
    return ($dataarr);
}

