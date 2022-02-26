<?php
/**
 * LIST of devices > booking home
 *
 *
 * @package o.CHIP_READER.li_book.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param $id project-ID
 * @param $action 
 *   'edit'
 *   'show'
 * @param $parx
 *    hourmode : ['full'], 'half'
 *    day [OPTIONAL if just shown]--- YYYY-MM-DD : show this day
 *      or TODAY
 * @param $q - hour input
 *   [DEV_ID][hour] = 0,1
 *
 */

require_once ('date_funcs.inc');
require_once ("visufuncs.inc");
require_once 'o.PROJ.subs.inc';
require_once 'o.CHIP_READER.book.inc';
require_once 'o.DB_USER.subs.inc';

class o_DEV_BOOK_table {
    
    /**
     * 
     * @var array
     *  'day'
        
     */
    private $parx;
    private $hourmode;
    
    function __construct($sqlo, $proj_id, $day, $action, $hourmode) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $this->parx=array();
        $this->parx['day'] = $day;
        $this->hourmode = $hourmode;
        
        $this->proj_id = $proj_id;
        if ($this->parx['day']=='') {
            $error->set( $FUNCNAME, 1, 'Target-Date "'.$this->parx['day'].'" not valid.' );
            return;
        }
        $this->action = $action;
    }
    
    function init($sqlo) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $this->infox=array();
        $this->defx=array();
        $this->defx['hour0'] = 8;
        $this->defx['hour1'] = 17;
        
        
        $this->my_user_id = $_SESSION['sec']['db_user_id'];
        
        
        $this->day_UNX = date_str2unix( $this->parx['day'], 5 );
        if ($this->day_UNX==NULL) {
            $error->set( $FUNCNAME, 1, 'Target-Date "'.$this->parx['day'].'" not valid.' );
            return;
        }
        $this->infox['day_name'] = date("D",$this->day_UNX);
        
        
        
        
        $this->dev_arr=array();
        $this->book_arr=array();
        
        $tablename='CHIP_READER';
        $sqlAfter = cProjSubs::getTableSQL ( $this->proj_id, $tablename );
        $sqlsel='CHIP_READER_ID from CHIP_READER where CHIP_READER_ID in ('.$sqlAfter.')';
        $sqlo->Quesel($sqlsel);
        while ( $sqlo->ReadRow() ) {
            $dev_id = $sqlo->RowData[0];
            $this->dev_arr[] = $dev_id;
        }
        
        $this->booking_lib = new o_CHIP_READER_book();
        
        // get old data of day
        foreach($this->dev_arr as $dev_id) {
            
            $this->booking_lib->set_dev($dev_id);
            $data_loop = $this->booking_lib->analyse_day($sqlo, $this->day_UNX);
            $this->book_arr[$dev_id] = $data_loop;
        }
        
        debugOut('ORI-book_arr: '.print_r($this->book_arr,1), $FUNCNAME, 1);
        
    }
    
    function get_dev_arr() {
        return $this->dev_arr;
    }
    function get_defx()  {
        return $this->defx;
    }
    
    private function _get_user($sqlo, $user_id) {
        
        if ($this->_user_cache[$user_id]) {
            return  $this->_user_cache[$user_id];
        }
        
        $username = DB_userC::getFullname($sqlo, $user_id);
        $short  = strtoupper(substr($username,0,2));
        $name_arr=explode(' ',$username);
        if (sizeof($name_arr)>=2) {
            $short  = strtoupper( substr($name_arr[0],0,1) . substr($name_arr[1],0,1) ) ;
        }
        
        $this->_user_cache[$user_id] = array('name'=>$username, 'short'=>$short);
        
        return $this->_user_cache[$user_id] ;
    }
    
    
    // can handle also half hours
    function day_hour1($hour) {
        $h_str = str_pad( $hour, 2, "0", STR_PAD_LEFT );
        if ( $this->hourmode=='half' and ($hour!=intval($hour)) ) {
            $h_str = str_pad( intval($hour), 2, "0", STR_PAD_LEFT );
            $out = $this->parx['day'].' '.$h_str.':30:00';
        } else { 
            $out = $this->parx['day'].' '.$h_str.':00:00';
        }
        return $out;
    }
    
    // can handle also half hours
    function day_hour2($hour) {
        $h_str = str_pad( $hour, 2, "0", STR_PAD_LEFT );
        if ( $this->hourmode=='half' ) {
            
            if ($hour!=intval($hour)) {
                $h_str = str_pad( intval($hour), 2, "0", STR_PAD_LEFT );
                $out = $this->parx['day'].' '.$h_str.':59:00';
            } else {
                $out = $this->parx['day'].' '.$h_str.':29:00';
            }
        } else {
            $out = $this->parx['day'].' '.$h_str.':59:00';
        }
        return $out;
    }
    
    /**
     * get entry of the hour
     * @param int $dev_id
     * @param float $hour
     * @return array
     */
    function one_dev_hour($dev_id, $hour) {
        $entries = $this->book_arr[$dev_id];
        if (empty($entries)) return;
        
        $d1 = $this->day_hour1($hour);
        $d2 = $this->day_hour2($hour);
        
        //echo "DDDX: check: d1:$d1 d2:$d2<br>";
        
        foreach($entries as $entry) {
            
            //echo " - DDDX: check: ".print_r($entry,1)."<br>";
            if ( $d1>=$entry['DATE1'] and $d2<=$entry['DATE2']) {
                return $entry;
            }
        }
    }
    
    /**
     * table of ONE day
     * @param object $sqlo
     */
    function tab_body($sqlo) {
        
        $this->_user_cache=array();
        $hour=$this->defx['hour0'];
        
        while ( $hour<=$this->defx['hour1'] ) {
            echo '<tr>'."\n";
            echo '  <td>'.$hour.'</td>';
            
            // get old data of day
            foreach($this->dev_arr as $dev_id) {
                
                $infoarr = $this->one_dev_hour($dev_id, $hour);
                $info='';
                if (!empty($infoarr)) {
                    $user_info = $this->_get_user($sqlo,$infoarr['DB_USER_ID']);
                    $info= $user_info['short'];
                }
                
                if ($this->action=='edit') {
                    
                    $edit_allow=0;
                    if (!empty($infoarr)) {
                        if ($this->my_user_id==$infoarr['DB_USER_ID']) {
                            $edit_allow=1;
                        }
                    } else {
                        $edit_allow=1;
                    }
                    if($edit_allow) {
                        $tmp_checked=NULL;
                        if (!empty($infoarr)) $tmp_checked=' checked';
                        $info='<input type=checkbox value=1 name="q['.$dev_id.']['.$hour.']" '.$tmp_checked.'>';
                    }
                }
                
                echo '  <td>'.$info.'</td>';
            }
            
            echo '</tr>'."\n";
            
            if ($this->hourmode=='half') $hour=$hour+0.5;
            else $hour++;
        }
    }
    
    function show($sqlo) {
        
        echo '<table class="xvar">'."\n";
        // get old data of day
        
        echo '<thead><tr >'."\n";
        echo '  <th class="vertical"><div>Hour</div></th>';
        foreach($this->dev_arr as $dev_id) {
            $dev_nice = obj_nice_name ( $sqlo, 'CHIP_READER', $dev_id );
            echo '  <th class="vertical">';
            echo '<div>'.   $dev_nice. '</div>';
            echo '</th>';
        }
        echo '</tr>'."\n".
          '</thead>'."\n";
        
        $this->tab_body($sqlo);
          
        echo '</table>'."\n";
            
        
    }
}


class o_CHIP_READER_li_book_XPL extends gPlugin {
    
    var $day_UNX;
    var $book_arr; // array[DEV_ID] = array(  array(DEV_BOOK entries) )

	function register() {
		
	    $title	= 'Booking of '.tablename_nice2('CHIP_READER') ;

		$this->infoarr			   = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'obj'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = 'PROJ';
		
		$this->infoarr['obj_id'] = $_REQUEST ['id'];
		$this->infoarr['checkid'] = 1;
		
		$this->infoarr['css'] ='
          
            th.vertical
            {
             height: 220px;
             min-width: 30px; /* for firefox */
             text-align: left;
            }

            th.vertical > div
            {
               height:120px;
               text-orientation: upright;
                writing-mode: vertical-rl;
            }
            
        
        ';

		
		//$this->infoarr['headIn']   = '<link href="'.$calendarCss.'" rel="stylesheet" type="text/css" />';
		//$this->infoarr['help_url'] = 'o.REA_LOG.html';
		
		
		
		$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('read'), 'obj'=>array() )
				);	
	}
	
	
	
	
	
	private function edit_save($sqlo, $q) {
	    
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    $dev_book_day_lib = new o_DEV_BOOK_table($sqlo, $this->proj_id, $this->parx['day'], 'edit', $this->parx['hourmode'] );
	    $dev_book_day_lib->init($sqlo);
	    
	    $dev_arr = $dev_book_day_lib->get_dev_arr();
	    $defx = $dev_book_day_lib->get_defx();
	    $time_slice = 1;
	    if ($this->parx['hourmode']=='half') $time_slice = 0.5;
	    
	   
	        
        // get old data of day
        foreach($dev_arr as $dev_id) {
            
            // $infoarr = $this->_one_dev_day($dev_id);
            $booking_mod_lib = new o_CHIP_READER_book_mod($sqlo, $dev_id);
            
            
            $hour_arr=array(); //
            $last_hour=-5;
            $index=-1;
            $hour=$defx['hour0'];
            
            while ( $hour<=$defx['hour1'] ) {
                
                $hour_str=strval($hour);
                $has_click = $q[$dev_id][$hour_str];
                if ($has_click) {
                    
                    $edit_allow = 0;
                    $ori_arr = $dev_book_day_lib->one_dev_hour($dev_id, $hour);
                    if (!empty($ori_arr)) {
                        if ($this->my_user_id==$ori_arr['DB_USER_ID']) {
                            $edit_allow=1;
                        }
                    } else {
                        $edit_allow=1;
                    }
                    
                }
                
                // echo "edit_save: D:".$dev_id. ' H:'.$hour. ' Click: '.$has_click.' E:'.$edit_allow ."<br>";
                
                if ($has_click and $edit_allow) {
                    $new_entry=1;
                    if (($last_hour + $time_slice)==$hour) {
                        $new_entry=0;
                    }
                    if ($new_entry) {
                        $index++;
                        $hour_arr[] = array('DATE1'=> $dev_book_day_lib->day_hour1($hour), 'DATE2'=>$dev_book_day_lib->day_hour2($hour) );
                        
                    } else {
                        $hour_arr[$index]['DATE2'] = $dev_book_day_lib->day_hour2($hour);
                    }
                    
                    $last_hour=$hour;
                }
                
                if ($this->parx['hourmode']=='half') $hour=$hour+0.5;
                else $hour++;
            }
            
            if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
                echo "DDD4:dev_id:$dev_id hour_arr:<br>";
                print_r($hour_arr);
                echo "<br>";
            }
            
            
            $booking_mod_lib->edit_day($sqlo,  $this->day_UNX , $_SESSION['sec']['db_user_id'], $hour_arr);
                
//                 foreach($hour_arr as $args) {
//                     $booking_mod_lib->edit_day($sqlo, $args);
//                     if ($error->Got(READONLY))  {
//                         $error->set( $FUNCNAME, 1, 'Error on update.' );
//                         return;
//                     }
//                 }

            

          
        }
	        
	        
	    
	    
	}
	
	function head_start() {
	    
	    $urlbase = $this->_html->_plug_get_URL().'&id='.$this->proj_id;
	    
	    $day_bef_UNX = $this->day_UNX - date_unixDay2Sec(1);
	    $this->infox['day_before'] = date_unix2datestr($day_bef_UNX, 3);
	    $this->infox['day_before_name'] = date("D",$day_bef_UNX);
	    
	    $day_aft_UNX = $this->day_UNX + date_unixDay2Sec(1);
	    $this->infox['day_after'] = date_unix2datestr($day_aft_UNX, 3);
	    $this->infox['day_after_name'] = date("D",$day_aft_UNX);
	    
	    $this->infox['day_name'] = date("D",$this->day_UNX);

	    echo ' [<a href="'.$urlbase.
	            '&parx[day]='.$this->parx['day'].'&action=edit">Edit</a>] &nbsp;'.
	           '[<a href="'.$urlbase.
	           '&parx[day]='.$this->parx['day'].'&action=show">Show</a>]';
	    
	    echo ' &nbsp; &nbsp; &nbsp;  [<a href="'.$urlbase.
    	    '&parx[day]='.$this->parx['day'].'&parx[hourmode]=full">Hours-Full</a>] &nbsp;'.
    	    '[<a href="'.$urlbase .
    	    '&parx[day]='.$this->parx['day'].'&action=show&parx[hourmode]=half">Hours-Half</a>]';
	    echo '<br>'."\n";
	    
	    


	}
	
	private function head_row($action) {
	    
	    $urlbase = $this->_html->_plug_get_URL().'&id='.$this->proj_id;
	    
	    echo '<tr valign=top>';
	    
	    echo '<td><a href="'.$urlbase.'&parx[day]='.$this->infox['day_before'].'">'.
	   	    $this->infox['day_before_name'] .' '.$this->infox['day_before'].'</a></td>';
	    echo '<td>';
	    echo '<b>'.$this->infox['day_name'].'</b> '.$this->parx['day']."<br>";
	    
	    if ($action=='edit') {
    	    echo '<input type=submit value="Save" class="yButton">'."\n";
    	    echo '<span style="padding-left:50px;">&nbsp;</span>';
    	    echo '<input type=submit value="Delete all" class="yButSmall" style="background-color: #DDAAAA;">'."\n";
	    }
	    echo '</td>';
	    echo '<td><a href="'.$urlbase.'&parx[day]='.$this->infox['day_after'].'">'.
	   	    $this->infox['day_after_name'] .' '. $this->infox['day_after'].'</a></td> </tr>';
	    
	    
	    echo '<tr>'."\n";
	}
	
	
	
	
	
	function edit($sqlo) {
	    
	    $this->head_start();
	    
	    echo '<form style="display:inline;" method="post" '.
	   	    ' name="editform"  action="'.$_SERVER['PHP_SELF'].'" >'."\n";
	    echo '<input type=hidden name="mod" value="'.$this->_mod.'">'."\n";
	    echo '<input type=hidden name="action" value="edit">'."\n";
	    echo '<input type=hidden name="go" value="1">'."\n";
	    echo '<input type=hidden name="id" value="'. $this->proj_id .'">'."\n";
	    echo '<input type=hidden name="parx[day]" value="'. $this->parx['day'] .'">'."\n";
	    
	    echo '<table>';
	    
	    $this->head_row('edit');
	    
	    echo '<td>';
	    $dev_book_day_lib = new o_DEV_BOOK_table($sqlo, $this->proj_id, $this->infox['day_before'], 'show', $this->parx['hourmode']);
	    $dev_book_day_lib->init($sqlo);
	    $dev_book_day_lib->show($sqlo);
	    echo '</td>';
	    
	    echo '<td>';

	    $dev_book_day_lib = new o_DEV_BOOK_table($sqlo, $this->proj_id, $this->parx['day'], 'edit', $this->parx['hourmode']);
	    $dev_book_day_lib->init($sqlo);
	    $dev_book_day_lib->show($sqlo);
	    
	    
	    
	    echo '</td>';
	    
	    echo '<td>';
	    $dev_book_day_lib = new o_DEV_BOOK_table($sqlo, $this->proj_id, $this->infox['day_after'], 'show', $this->parx['hourmode']);
	    $dev_book_day_lib->init($sqlo);
	    $dev_book_day_lib->show($sqlo);
	    echo '</td>';
	    
	    echo '</tr></table>'."\n";
	    
	    echo '</form>'."\n";
	}
	
	function show($sqlo) {
	    
	    $this->head_start();
	    
	    echo '<table><tr>'."\n";
	    
	    $this->head_row('show');
	    
	    echo '<td>';
	    $dev_book_day_lib = new o_DEV_BOOK_table($sqlo, $this->proj_id, $this->infox['day_before'], 'show', $this->parx['hourmode']);
	    $dev_book_day_lib->init($sqlo);
	    $dev_book_day_lib->show($sqlo);
	    echo '</td>';
	    
	    echo '<td>';
	    
	    $dev_book_day_lib = new o_DEV_BOOK_table($sqlo, $this->proj_id, $this->parx['day'], 'show', $this->parx['hourmode']);
	    $dev_book_day_lib->init($sqlo);
	    $dev_book_day_lib->show($sqlo);
	    
	    echo '</form>'."\n";
	    
	    echo '</td>';
	    
	    echo '<td>';
	    $dev_book_day_lib = new o_DEV_BOOK_table($sqlo, $this->proj_id, $this->infox['day_after'], 'show', $this->parx['hourmode']);
	    $dev_book_day_lib->init($sqlo);
	    $dev_book_day_lib->show($sqlo);
	    echo '</td>';
	    
	    echo '</tr></table>'."\n";
	}
	

	function startMain() {
		global $error;
		$FUNCNAME='startMain';
		
		if (!glob_table_exists('DEV_BOOK')) {
		    $this->htmlFoot('ERROR', 'System must be upgraded to use this function.');
		}
		
	    $tablename='CHIP_READER';
		$this->proj_id  = $_REQUEST['id'];
		$this->parx   = $_REQUEST['parx'];
		$go = $_REQUEST['go'];
		$sqlo = &$this->_sqlo;
		
		if (!is_array($_SESSION['s_formState'][$this->_mod])) $_SESSION['s_formState'][$this->_mod]=array();
		
		if ($_REQUEST['action']!=NULL) {
		    $action = $_REQUEST['action'];
		    $_SESSION['s_formState'][$this->_mod]['action'] = $action;
		}
		if ($this->parx['hourmode']!=NULL) { 
		    $_SESSION['s_formState'][$this->_mod]['hourmode'] = $this->parx['hourmode'];
		}
		
		$app_cache = $_SESSION['s_formState'][$this->_mod];
		$action = $app_cache['action'];
		if ($action==NULL) $action='show';
		
		$this->parx['hourmode'] = $app_cache['hourmode'];
		if ($this->parx['hourmode']==NULL) {
		    $this->parx['hourmode']='full';
		}
		
		if ($this->parx['day']=='TODAY') {
		    $day_unx=time();
		    $this->parx['day'] = date_unix2datestr($day_unx, 3);
		}
		
		if ($action=='edit' and $this->parx['day']==NULL) {
		    $this->htmlFoot('ERROR', 'Need a target day for edit-action.');
		}
		
		// calc $this->parx['day']
		if ($this->parx['day']==NULL) {
		    $day_unx=time();
		    $this->parx['day'] = date_unix2datestr($day_unx, 3);
		}
		$this->day_UNX = date_str2unix( $this->parx['day'], 5 );
		
		gHtmlMisc::func_hist( $this->_mod, 'EQU Booking', $this->_plug_get_URL().'&id='.$this->proj_id.'&action=show' );

		$ObjCnt = cProjSubs::getTableCnt ( $sqlo, $this->proj_id , $tablename );
		$MAX_CNT=50;
		if ($ObjCnt>$MAX_CNT) {
		    $this->htmlFoot('ERROR', 'Too many devices selected. Max '.$MAX_CNT.' allowed.');
		}

		
		if ($action=='edit') {
		    
		    if ($go>0) {
		        $this->edit_save($sqlo, $_REQUEST['q']);
		        if ($error->Got(READONLY))  {
		            $error->set( $FUNCNAME, 1, 'Error on save.' );
		            return;
		        }
		        
		        
		    }
		    
		    $this->edit($sqlo);
		}
		if ($action=='show') {
			$this->show($sqlo);
		}
	}
	
}