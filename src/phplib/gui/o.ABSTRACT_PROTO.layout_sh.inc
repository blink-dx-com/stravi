<?php
require_once 'o.ABSTRACT_PROTO.layout.inc';

/**
 * layout things
 * @package o.ABSTRACT_PROTO.layout_sh.inc
 * @author  Steffen Kube (steffen@blink-dx.com)

 */
class o_ABSTRACT_PROTO_layout_sh {

    function __construct($sqlo, $pra_id, $layout_json_str=NULL) {
        
        $this->help_lib = new o_ABSTRACT_PROTO_layout($sqlo, $pra_id);
        $this->layout = json_decode($layout_json_str, TRUE);
        $this->pra_steps = $this->help_lib->get_steps_raw();
    }
    
    function get_steps_raw() {
        return $this->pra_steps;
    }
    
    function show($sqlo) {
        
        echo '<table >'."\n";
        $head = $this->layout['head'];
       
        echo '<tr style="background-color:#D0D0FF;">';
        echo "<th>#</th>"; // line number
        foreach($head as $cell) {
            echo '<th>'.$cell.'</th>';
        }
        echo '</tr>';
        
        $i=0;
        foreach($this->layout['rows'] as $row) {
            echo '<tr>';
            echo '<td>'.($i+1).'</td>';
            foreach($row as $cell) {
                
                $c_info = $this->help_lib->check_cell($cell);
                $code=$c_info['co'];
                $step=$c_info['st'];
                $cellv='';
                
                switch ($code) {
                    case 'AN':
                        $cellv= $this->pra_steps[$step]['NAME'];
                        break;
                    case 'AQ':
                        $cellv= $this->pra_steps[$step]['QUANTITY'];
                        break;
                    case 'AS':
                        $sua_id = $this->pra_steps[$step]['ABSTRACT_SUBST_ID'];
                        if ($sua_id) {
                            $nice = obj_nice_name ( $sqlo, 'ABSTRACT_SUBST', $sua_id ); 
                            $cellv = $nice .' [MAA:'.$sua_id.']';
                        } else {
                            $cellv= '?no MAA?';
                        }
                        
                        break;
                    case 'Q':
                        $cellv= '';
                        break;
                    case 'S':
                        $sua_id = $this->pra_steps[$step]['ABSTRACT_SUBST_ID'];
                        if ($sua_id) {
                            $nice = obj_nice_name ( $sqlo, 'ABSTRACT_SUBST', $sua_id );
                            $cellv = $nice .' [MAA:'.$sua_id.']';
                        } else {
                            $cellv= '?no MAA?';
                        }
                        break;
                    case 'N':
                        $cellv= '';
                        break;
                    default:
                        if ($code!='') {
                            $cellv= '?C:'.$code;
                        }
                }
                echo '<td>'.$cellv.'</td>';
            }
            echo '</tr>';
            $i++;
        }
        
        echo '</table>'."\n";
    }
}
