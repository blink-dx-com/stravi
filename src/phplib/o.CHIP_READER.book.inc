<?php
require_once('date_funcs.inc');
require_once ("f.assocUpdate.inc");

/**
 * DEVICE booking ANALYSIS
 * @package o.CHIP_READER.book.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class o_CHIP_READER_book {
    
    private $dev_id=0;
    
    function __construct() {
        
    }
    
    function set_dev($dev_id) {
        $this->dev_id = $dev_id;
    }
    
    /**
     * get all entries
     * @param int $day_UNX
     */
    function analyse_day($sqlo, $day_UNX) {
        if(!$this->dev_id) {
            sys_error_my( 'class init problem: DEV_ID missing.' );
        }
        
        if (!$day_UNX) {
            throw new Exception('Input day_UNX missing.');
        }
        
        $day_SQL = date_unix2datestr($day_UNX, 3);
        $day_end = $day_SQL.' 23:59';
        $entries=array();
        
        // like '.$sqlo->addQuotes($day_SQL.'%')
        // EXTRACT(DAY FROM DATE1)='.$sqlo->addQuotes($day_SQL).'
        $sqlsel = "* from DEV_BOOK where DEV_ID=".$this->dev_id.' and DATE1 between '.
            $sqlo->addQuotes($day_SQL).' and '.$sqlo->addQuotes($day_end).' order by DATE1';
        
        $sqlo->Quesel($sqlsel);
        while ( $sqlo->ReadArray() ) {
            $entries[] = $sqlo->RowData;
        }
        
        return $entries;
    }
    
}

class o_CHIP_READER_book_mod {
    
    function __construct($sqlo, $dev_id) {
        $this->dev_id = $dev_id;
        
        $assopt=array('noAccCheck'=>1);
        $this->assoclib = new  fAssocUpdate();
        $this->assoclib->setObj( $sqlo, 'DEV_BOOK', $this->dev_id, $assopt );
        $this->assoclib->setNoTouch(1); // no touching ...
        
        $this->booking_lib = new o_CHIP_READER_book();
        $this->booking_lib->set_dev($dev_id);
        
        
    }
    
    /**
     * 
     * @param object $sqlo
     * @param array $args
     *    all except DEV_ID, POS, DB_USER_ID
     */
    function add($sqlo, $args) {
        $args['DEV_ID'] = $this->dev_id;
        
        $sqlsel = "MAX(POS) from DEV_BOOK where DEV_ID=".$this->dev_id;
        $sqlo->Quesel($sqlsel);
        $sqlo->ReadRow();
        $last_pos = $sqlo->RowData[0];
        if (!$last_pos) $last_pos=0;
        $new_pos = $last_pos + 1;
        $args['POS'] = $new_pos;
        
        if (!$args['DB_USER_ID']) $args['DB_USER_ID']=$_SESSION['sec']['db_user_id']; 
        // echo "DDDD:add: ".print_r($args,1)."<br>";
         
        $this->assoclib->insert($sqlo, $args);
    }
    
    function del($sqlo, $pos) {
        $idarr=array('POS'=>$pos);
        $this->assoclib->delOneRow( $sqlo, $idarr );
    }
    
    /**
     * edit day for one user
     * @param  $sqlo
     * @param  $day_UNX
     * @param  $many_entries
     */
    function edit_day($sqlo, $day_UNX, $user_id, $new_entries) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        
        $data_ori = $this->booking_lib->analyse_day($sqlo, $day_UNX);
        $user_ori= array();
        
        if (!empty($data_ori)) {
            foreach ($data_ori as $entry) {
                if ($entry['DB_USER_ID']==$user_id) {
                    $user_ori[]=$entry;
                }
            }
        }
        
        $do_add_entries=1;
        $data_unchanged = 1;
        if (sizeof($user_ori)!=sizeof($new_entries)) $data_unchanged=0;
        
        if (!empty($user_ori)) {
            
            $index=0;
            
  
            foreach ($user_ori as $entry) {
                
                $new_entry=$new_entries[$index];
                if (!is_array($new_entry)) $new_entry=array();
                
                if ($new_entry['DATE1']==$entry['DATE1'] and $new_entry['DATE2']==$entry['DATE2']) {
                    //do nothing
                } else {
                    $data_unchanged=0;
                }
                
                $index++;
            }
            
            
            if ($data_unchanged) {
                $do_add_entries=0; /// !!!!
            } else {
                // delete old entrries
                foreach ($user_ori as $entry) {
                    
                    debugOut('DEL:'.print_r($entry,1), $FUNCNAME, 1);
                    if ( $_SESSION["userGlob"]["g.debugLevel"]>2 ) {
                        debugOut('no DELETE due to DEBUG', $FUNCNAME, 1);
                        continue;
                    }
                    $pos=$entry['POS'];
                    $this->del($sqlo, $pos);
                }
            }
        }
        
        debugOut('do_add_entries:'.$do_add_entries.' data_unchanged:'.$data_unchanged, $FUNCNAME, 1);
        debugOut('new_entries:'.print_r($new_entries,1), $FUNCNAME, 1);
        
        if (!empty($new_entries) and $do_add_entries) {
            // now add new entries
            foreach ($new_entries as $entry) {
                $entry['DB_USER_ID']=$user_id;
                debugOut('ADD:'.print_r($entry,1), $FUNCNAME, 1);
                if ( $_SESSION["userGlob"]["g.debugLevel"]>2 ) {
                    debugOut('no ADD due to DEBUG', $FUNCNAME, 1);
                    continue;
                }
                $this->add($sqlo, $entry);
            }
        }
    }
}