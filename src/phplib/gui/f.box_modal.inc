<?php
/**
 * manage modal window
 * @package f.box_modal.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @created 2021-09-28
 */
class F_box_modal {
    
    // @param string $box_id e.g. pluginInfoModal
    function __construct(string $box_id) {
        $this->box_id=$box_id;
    }

    
    /**
     * the CSS-title-field can be addressed by {$box_id}_title
     * @param string $title
     
     */
    function draw_open(string $title) {
        $box_id = $this->box_id;
        ?>
    <div id="<?echo $box_id;?>" class="y_modal">
      <!-- Modal content -->
      <div class="y_modal-content">
        <span class="close_x_button" onClick="modal = document.getElementById('<?echo $box_id;?>'); modal.style.display='none';">&times;</span>
        <h2 style="display:inline;" id="<?php echo $box_id;?>_title"><?php echo $title;?></h2>
        <div style="padding-top:10px;"></div>
        <?php 
    }
    
    function draw_close() {
        ?>
        
      </div>
    </div> 
    <?php
    }
    
    function get_click_js() {
        $out= 'modal=document.getElementById(\''.$this->box_id.'\'); modal.style.display=\'block\';';
        return $out;
    }
}
