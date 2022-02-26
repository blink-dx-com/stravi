<?php
require_once('reqnormal.inc');
require_once('exception.inc');
require_once("subs/toolbar.inc");
require_once('edit.sub.inc');
require_once("edit.tmpl.inc");
require_once("sql_query_dyn.inc");

/**
 * 
 * @author skube
 * @module subs/glob.obj.superhead.inc
 * call:
    $error   = & ErrorHandler::get();
    $sqlo  = logon2( $_SERVER['PHP_SELF'] );
    
    $gui_lib = new glob_obj_superhead('EXP', $exp_id);
    $gui_lib->page_open($sqlo);
    
    ...
    
     $gui_lib->page_close($sqlo);
    
 */
class glob_obj_superhead {
    
    private $obj_features;
    
    function __construct($tablename, $id) {
        $this->tablename= $tablename;
        $this->id = $id;
    }
    
    function get_obj_features() {
        return $this->obj_features;
    }
    
    /**
     * before any smart html header ...
     * @param string $text
     */
    function error_raw($text) {
        $htmlPageLib = new gHtmlHead();
        $htmlPageLib->_PageHead ('Single object');
        
        echo "<ul><br><br>";
        htmlInfoBox( "Single object sheet Error", $text, "", "ERROR" );
        echo "<BR><br><a href=\"javascript:history.back()\">Back</a>";
        $htmlPageLib->htmlFoot();
        exit;
    }
    
    /**
     * 
     * @param object $sqlo
     * @param string $xmode
     */
    function page_open($sqlo, $xmode) {
        
        $tablename = $this->tablename;
        $id = $this->id;
        
        
        if ( $_SESSION['sec']['appuser']=="" ) {
            
            $objurl     = "edit.tmpl.php?t=".$tablename."&id=".$id;
            $urlencode  = urlencode( $objurl );
            $forwardUrl = 'index.php?forwardUrl='.$urlencode;
            echo '<br>... not logged in. automatic forward to <a href="'.$forwardUrl.'">login page</a> ...<br>';
            echo '<script language="JavaScript">' . "\n";
            echo '    location.href="'. $forwardUrl .'";' . "\n" ;
            echo '</script>' . "\n";
            
            exit;
        }
        
       
        if ($tablename=='') {
            $this->error_raw("You must provide a table name!", 0);
            
        }
        $nicename= tablename_nice2($tablename);
        if ($nicename=='') {
            $this->error_raw("table <B>$tablename</B> is nunknown.".
                " Please ask the administrator!");
        }
        
        // $moObj will be analyzed inside
        $primasid=array();  //dummy ...
        $this->editLib = new fEditLibC( $tablename, $id, $primasid );
        $this->editLib->init($sqlo);
        
        $object_is_bo = $this->editLib->object_is_bo;
        if (!$object_is_bo) {
            $this->error_raw('This tool can only be used for business objects.');
        }
        
        $pk_name = PrimNameGet2($tablename);

        $t_rights = tableAccessCheck($sqlo, $tablename);
        $o_rights = access_check($sqlo, $tablename, $id);
        $formopt  = unserialize($_SESSION['userGlob']["g.sof.opt"]);
        
        if ( $t_rights["read"] != 1 ) {
            $infoTable = getTableAccessMsg( $nicename, "read" );
            $this->error_raw($infoTable);
            exit;
        }
        
        $this->obj_features  = glob_elemDataGet3( $sqlo, $tablename, array($pk_name=>$id), array('*') );
        $CCT_ACCESS_ID = $this->obj_features['CCT_ACCESS_ID'];
        $access_data   = access_data_getai($sqlo, $CCT_ACCESS_ID);
        $this->editLib->setInfoxName($sqlo);
        $this->editLib->showHead(); // output <head> and <body> tags
        
        //$has_single_pk=1;
        $object_is_bo=1;
        htmlShowHistory();
        
        //TBD: may be not sho
        if ( $formopt["formslim"] != 1 ) {
            $menuopt=array("menushow"=> 1);
            $menuRight = $this->editLib->objHeadLib->getMenuRight($menuopt);
            // require_once ("edit.menu.inc");
            // edit_menu( $object_is_bo, $has_single_pk, $tablename , $primasid ,$id , $o_rights, $tablename_l, $menuRight );
        }
        
        $x_rights=array(); //TBD:
        
        $xmodes = array(
            'default'=> '0meta',
            'modes'=> array(
                '0meta'   =>array( 'n'=> 'meta', 'co'=>'#CEC702', 'ic'=>'ic.meta.svg' ),
                '0perm'   =>array( 'n'=> 'access', 'co'=>'#CEC702', 'ic'=>'but.access.gif' ),
                '0modlog' =>array( 'n'=> 'modification log', 'co'=>'#CEC702',  'ic'=>'ic.mod_log.svg' ),
                // 'features'=>array( 'n'=>"<b>features</b>")
            )
        );
        
        $infox2      = array('slim'=>NULL);
        $info_tab    = '';
        $this->editLib->toolbarPrep( $sqlo, $x_rights, $access_data, $formopt["formslim"], $info_tab, $infox2) ;
        $this->editLib->set_tabmode($xmode);
        $this->editLib->set_tool_button_META();
        
        if ( $formopt["formslim"] ) {
            $this->editLib->objHeadSlim($infox2, $xmodes);
        } else {
            $this->editLib->objHeadFull($sqlo, $CCT_ACCESS_ID, $infox2, $xmodes, $info_tab, $x_rights, $o_rights);
        }
        $this->editLib->historycheck();
    }
    
    function page_close($sqlo) {
        $this->editLib->pageEnd();
    }
    
}