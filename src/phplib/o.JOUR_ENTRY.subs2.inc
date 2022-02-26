<?php
/**
 * pure SQL methods, 
 * @author skube
 * @package o.JOUR_ENTRY.subs2.inc
 *
 */
class oJOUR_ENTRY_subs {
    
    static $SUPER_MAX=200;
    
    static function get_max_num($options) {
        $max_num = isset($options['max_num']) ? $options['max_num'] : 0;
        if ($max_num<=0) $max_num=self::$SUPER_MAX;
        if($max_num>self::$SUPER_MAX) $max_num=self::$SUPER_MAX;
        return $max_num;
    }
    
    // input: $options['owner_id']
    static function sql_get_key_query($sqlo, $options=array()) {
        $sqlsel = "x.KEYS from JOUR_ENTRY x join CCT_ACCESS a on x.CCT_ACCESS_ID=a.CCT_ACCESS_ID ".
            " where  keys is not NULL ";
        if (isset($options['owner_id'])) {
            $sqlsel .= " and a.DB_USER_ID=".$sqlo->addQuotes($options['owner_id']);
        }
        return $sqlsel;
    }
    
    /**
     * 
     * @param object $sqlo
     * @param array $options
     * @return array of
     *   KEY => count of key
     */
    static function keys_histogram($sqlo, $options=array() ) {
        
        $max_num = self::get_max_num($options);
        $sqlsel = self::sql_get_key_query($sqlo, $options);
        
        // $sqlsel .= ' order  by x.JOUR_ENTRY_ID ASC limit '.self::$SUPER_MAX;
        $sqlo->Quesel($sqlsel);
        $cnt=0;
        $keys_all=array();
        
        while ( $sqlo->ReadRow() ) {
            
            $num_keys = sizeof($keys_all); // dynamic count all keys 
            
            if ($cnt>$num_keys) break;
            $key_comb  = $sqlo->RowData[0];
            $keys_loop = explode(",",$key_comb);
            foreach($keys_loop as $one_key) {
                
                $one_key=trim($one_key);
                if ($one_key==='' ) continue;
                if (!isset($keys_all[$one_key])) $keys_all[$one_key]=0;
                $keys_all[$one_key]++;       
            }
            $cnt++;
            
        }
        return $keys_all;
        
    }
    
    /**
     * get all distinct keys from MY last objects JOUR_ENTRY
     * @param object $sqlo
     * @param int $max_num
     * @return array 
     *   max_num  --  max num of return values
     *   owner_id : only entries of USER_ID
     *   
     */
    static function get_distinct_keys($sqlo, $options=array() ) {
        
        $max_num = self::get_max_num($options);
        $sqlsel = self::sql_get_key_query($sqlo, $options);
        
        $sqlsel .= ' order  by x.JOUR_ENTRY_ID ASC limit '.self::$SUPER_MAX;
        $sqlo->Quesel($sqlsel);
        $cnt=0;
        $keys_comb_all=array();
        while ( $sqlo->ReadRow() ) {
            if ($cnt>$max_num) break;
            $val = $sqlo->RowData[0];
            if (!in_array($val, $keys_comb_all)) {
                $keys_comb_all[] = $val;
                $cnt++;
            }
        }
        //echo "DDX keys_comb_all:".print_r($keys_comb_all,1)."<br>";
        
        if (empty($keys_comb_all)) {
            return array();
        }
        
        $keys_all=array();
        $cnt=0;
        
        // one $key_comb contains many KEYs: "Hallo, du  , key"
        foreach($keys_comb_all as $key_comb) {
            
            $keys_loop = explode(",",$key_comb);
            foreach($keys_loop as $val) {
                
                $val=trim($val);
                if ($val!=='' and !in_array($val, $keys_all)) {
                    
                    if ($cnt>$max_num) break;
                    $keys_all[]=$val;
                    $cnt++;
                }
            }
            
            if ($cnt>$max_num) break;
        }
        
        sort($keys_all);
        //echo "DDX keys_all:".print_r($keys_all,1)."<br>";
        
        return $keys_all;
    }
    
    static function get_my_distinct_keys($sqlo, $max_num=0) {
        $db_user = $_SESSION['sec']['db_user_id'];
        $sea_opts = array(
            'owner_id'=>$db_user,
        );
        if ($max_num) $sea_opts['max_num'] = $max_num;
        return self::get_distinct_keys($sqlo, $sea_opts);
    }
    
    /**
     * get all parent objects of type $tablename
     * @param object $sqlo
     * @param string $tablename
     * @param int $obj_id
     * @return array
     */
    static function get_JOU_links(&$sqlo, string $tablename, int $obj_id) {
        
        // $MAXSHOW=20; // max shown elements
        $out_arr=array();

        $sqlsel = "MO_ID from S_OBJLINK where ".
            ' CH_ID='.$obj_id. ' and CH_TABLE= '.$sqlo->addquotes($tablename).
            ' and MO_TABLE='.$sqlo->addQuotes('JOUR_ENTRY').
            ' order by MO_ID';
        $sqlo->Quesel($sqlsel);
        $cnt=0;
        while ($sqlo->ReadRow()) {
          
            $mo_id = $sqlo->RowData[0];
            $out_arr[] = $mo_id;
            $cnt++;
        }
        return $out_arr;
        
    }
    
}