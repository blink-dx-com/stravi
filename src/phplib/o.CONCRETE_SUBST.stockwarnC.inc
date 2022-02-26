<?php 
require_once ('gui/f.htmlGraph2.inc');
require_once ('o.ABSTRACT_SUBST.logi.inc');
require_once ('o.CONTAINER.subs.inc');

/**
 * stock waRNINGS help class
 * @package o.CONCRETE_SUBST.stockwarnC.inc
 * @author Steffen
 */
class o_CONCRETE_SUBST_stockwarnC {
    
    function __construct() {
        $this->objShowLib = new fObjViewC();
        $this->logisticsLib = new oABSTRACT_SUBST_logi();
        
        
        $this->iconarr = array(
            'unknown' => array('key'=>'',       'img'=>'i13_ask.gif'),
            'min' => array('key'=>'store.min',  'img'=>'i13_err.gif'),
            'warn'=> array('key'=>'store.warn', 'img'=>'i13_warning.gif'),
            'norm'=> array('key'=>'store.norm', 'img'=>'i13_ok3.gif'),
            'good'=> array('key'=>'',           'img'=>'i13_ok.gif'),
            
        );
        
        $this->GRAPH_width=300;
        $this->SUA_last = 0;
        $this->SUA_BASE_info=array();
    }
    function set_GRAPH_width($width) {
        if ($width>0) $this->GRAPH_width=$width;
    }
    
    private function _getIcon($stock_status) {
        
        if ($stock_status==NULL) {
            return NULL;
        }
        
        $icon_file = $this->iconarr[$stock_status]['img'];
        
        $url='<img src="images/'.$icon_file.'"  title="'.$stock_status.'">';
        return $url;
    }
    
    private function _build_color_box($quant_sum, &$sua_logistics_data) {
        
        
        if (!$quant_sum) return NULL;
        
        $box_array=array();
        
        
        
        if ($sua_logistics_data['store.norm']>0) {
            $maxval = $sua_logistics_data['store.norm']*2.0; // give MAX + 30%
        } else {
            $maxval = $quant_sum;
        }
        
        

        $gropt=array('stripHeight'=>15);
        $graphLib = new fHtmlGraph2C($maxval,  $this->GRAPH_width, $gropt);
        
        do {
            
            # MIN
            if ($quant_sum<$sua_logistics_data['store.min']) {
                $box_array[0]=array('val'=>$quant_sum, 'color'=>'red');
                break;
            }
            $last_limit=$sua_logistics_data['store.min'];
            $box_array[0]=array('val'=>$sua_logistics_data['store.min'], 'color'=>'#b3e6ff');
            
            # WARN
            if ($quant_sum<$sua_logistics_data['store.warn']) {
                $box_array[1]=array('val'=>$quant_sum-$last_limit, 'color'=>'#ffff66');
                break;
            }
            $box_array[1]=array('val'=>$sua_logistics_data['store.warn']-$last_limit, 'color'=>'#6363f7');
            $last_limit=  $sua_logistics_data['store.warn']>0 ?  $sua_logistics_data['store.warn']  : $last_limit ;
            
            # NORM
            if ($quant_sum<$sua_logistics_data['store.norm'] ) {
                $box_array[1]=array('val'=>$quant_sum-$last_limit, 'color'=>'#6363f7');
                break;
            }
            
            $box_array[2]=array('val'=>$sua_logistics_data['store.norm']-$last_limit, 'color'=>'#1414fa');
            $last_limit= $sua_logistics_data['store.norm']>0 ? $sua_logistics_data['store.norm'] : $last_limit ;
            
            # super good
            $val_show = min($maxval, $quant_sum);
            $box_array[3]=array('val'=>$val_show-$last_limit, 'color'=>'#33cc33'); # green
            
            
        } while (0);
        
        $graph_img = $graphLib->many_boxes($box_array);
        return $graph_img;
    }
    
    /**
     * get SUA information about storage status
     * @param object $sqlo
     * @param int $suc_id
     * @param float $quant_sum
     * @return array sua_logistics_STRUCT
     *  'cnt'= $cnt_id;
        'SUA'= $sua_id;
        'QUANT' = $quant_sum;
        'ICON'  = $stock_status_icon;
        'GRAPH' = $graph_img;
        'status'= 'min', 'warn', 'norm', 'good'
        'store.min'= $sua_logistics_data['store.min';
        'store.warn'= $sua_logistics_data['store.warn';
        'store.norm'= $sua_logistics_data['store.norm';
        'unit.name' : H_UNIT name == measurement unit
     */
    function get_sua_info($sqlo, $sua_id, $quant_sum) {
        
        $datarow=array();
        
        /**
         *
         * @var unknown $stock_status
         * 'min', 'warn', 'norm', 'good'
         */
        $stock_status=NULL;
        
        if ($sua_id!=$this->SUA_last) {
            
            $this->logisticsLib->set_sua($sua_id);
            $this->SUA_BASE_info = $this->logisticsLib->get_data($sqlo);
            
            // sua features
            $h_name=NULL;
            $sua_features = glob_elemDataGet3( $sqlo, 'ABSTRACT_SUBST', array('ABSTRACT_SUBST_ID'=>$sua_id), array('H_UNIT_ID') );
            if ($sua_features['H_UNIT_ID']) {
                $h_name = glob_elementDataGet( $sqlo, 'H_UNIT', 'H_UNIT_ID', $sua_features['H_UNIT_ID'], 'NAME');
            }
           
            $this->SUA_BASE_info['unit.name']=$h_name;
            $this->SUA_last = $sua_id;
        }
        
        // deep copy
        $sua_logistics_data = &$this->SUA_BASE_info;
        
        
        
        
        do {
            if ($sua_logistics_data['store.norm']>0 and $quant_sum>=$sua_logistics_data['store.norm']) {
                $stock_status='good';
                break;
            }
            if ($sua_logistics_data['store.warn']>0 and $quant_sum>=$sua_logistics_data['store.warn']) {
                $stock_status='norm';
                break;
            }
            if ($sua_logistics_data['store.min']<=0) {
                $stock_status='unknown'; // no stock-limits set for SUA
                break;
            }
            
            if ($quant_sum>=$sua_logistics_data['store.min']) {
                $stock_status='warn';
                break;
            }
            
            // default ...
            $stock_status='min';
            
            
        } while (0);
        
        $stock_status_icon = $this->_getIcon($stock_status);
        
        $graph_img= $this->_build_color_box($quant_sum, $sua_logistics_data);
        
        // $datarow['cnt']= $cnt_id;
       
        $datarow['SUA']   = $sua_id;
        $datarow['QUANT'] = $quant_sum;
        $datarow['ICON']  = $stock_status_icon;
        $datarow['GRAPH'] = "\n".$graph_img;
        $datarow['status']= $stock_status;
        $datarow['store.min'] = $sua_logistics_data['store.min'];
        $datarow['store.warn']= $sua_logistics_data['store.warn'];
        $datarow['store.norm']= $sua_logistics_data['store.norm'];
        $datarow['unit.name'] = $sua_logistics_data['unit.name'];
        
        return $datarow;
    }
    
    /**
     * sua_logistics_data has limits ???
     * @param array $sua_logistics_data
     */
    static function sua_has_limits(&$sua_logistics_data) {
        $keys = array('store.min', 'store.warn', 'store.norm');
        $has_data=0;
        foreach($keys as $key) {
            if ($sua_logistics_data[$key]>0)  $has_data=1;
        }
        return $has_data;
    }
    
    /**
     * get SUC information about storage status
     * @param object $sqlo
     * @param int $suc_id
     * @param float $quant_sum  -QUANTITY of SUC in ONE analysed storage
     * @return array
     *  'cnt'= $cnt_id;
     'SUC'= $suc_id;
     'SUA'= $sua_id;
     'QUANT' = $quant_sum;
     'ICON'  = $stock_status_icon;
     'GRAPH' = $graph_img;
     'status'= 'min', 'warn', 'norm', 'good'
     'store.min'= $sua_logistics_data['store.min';
     'store.warn'= $sua_logistics_data['store.warn';
     'store.norm'= $sua_logistics_data['store.norm';
     */
    function get_suc_info($sqlo, $suc_id, $quant_sum) {

        $sua_id = glob_elementDataGet( $sqlo, 'CONCRETE_SUBST', 'CONCRETE_SUBST_ID', $suc_id, 'ABSTRACT_SUBST_ID');
        
        $this->logisticsLib->set_sua($sua_id);
        $sua_logistics_data = $this->logisticsLib->get_data($sqlo);
        
        if ( !$sua_logistics_data["store.ana.singlebatch"] ) {
            $quant_sum_t = oCONTAINER_many::get_one_SUA_QUANTITY($sqlo, $sua_id);
            //echo "DDD: sua_id:$sua_id quant_sum:$quant_sum t:$quant_sum_t<br>";
            $quant_sum=$quant_sum_t;
            
        }
        
        $datarow = $this->get_sua_info($sqlo, $sua_id, $quant_sum);
        $datarow['SUC']= $suc_id;
        return $datarow;
    }
    
   
}
