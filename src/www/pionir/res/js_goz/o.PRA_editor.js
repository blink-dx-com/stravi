/* 
 * special code for obj.abstract_proto.xmode.edi.inc
 * produce o.ABSTRACT_PROTO.mod.inc:PRA_steps_form_STRUCT
 * version: 2020-05-10 18:00
 * @package o.PRA_editor.js
 **/

var $TABLE = $('#table');
var $BTN = $('#export-btn');
var $EXPORT = $('#export');
var $ABSTRACT_PROTO_ID = 0;
var $TAB_COLID_DEVICE=2; /* column ID of device */
var $TAB_COLID_SUA=3;   /* column ID of substance */

/* Drag-Drop-Support for step orders, needs jquery-ui */
$("#table tbody").sortable({
    items: "> tr",
    appendTo: "parent",
    helper: "clone",
    cancel: ':input,button,.contenteditable'
}).disableSelection();


function x_set_proto_id(pra_id) {
	$ABSTRACT_PROTO_ID = pra_id;
}

/* replace form vars
 * @param object $clone 
 * @param string $key: x or y
 **/
function repl_form_vars($clone, $key) {

  var dblink_colnum=0;
  if ($key=='x') {
	// DEVICE
	dblink_colnum=$TAB_COLID_DEVICE;
	$search_key  = "fyqx";
	$replace_key = "fyx"; // was fyqx
  }
  if ($key=='y') {
	// SUBSTANCE
	dblink_colnum=$TAB_COLID_SUA;
	$search_key  = "fyqy";
	$replace_key = "fyy"; // was fyqy
  }

  var temp = "";
  var rowcnt=0
  var $td = "";
  
  
  $td = $clone.find('td');
  temp = $td.eq(dblink_colnum).html();
  
  // just count the rows ...
  var $rows = $TABLE.find('tr');
  $rows.each(function () {
	   rowcnt=rowcnt+1;
  })
 

  // replace all occurences ...
  while (1) {
	old_html = temp
    temp = temp.replace($search_key, $replace_key + rowcnt);
	if (old_html==temp) break;
  } 
  
  $td.eq(dblink_colnum).html(temp);

}

// ADD row 
$('.table-add').click(function () {

  var $clone = $TABLE.find('tr.hide').clone(true).removeClass('hide');
  
  var $rows = $TABLE.find('tr');
  // var $clone = $trs.eq(0).clone(true);
  
  $TABLE.find('table').append($clone);
  $clone.show();
  
  // get new TR
  var $rows2 = $TABLE.find('tr');
  var rowcnt = $rows.length;
  

 
  repl_form_vars($clone, 'x');
  repl_form_vars($clone, 'y');
  
  /* alert("ROW:"+rowcnt) */
});

$('.table-remove').click(function () {
  $(this).parents('tr').detach();
});

$('.table-up').click(function () {
  var $row = $(this).parents('tr');
  if ($row.index() === 1) return; // Don't go above the header
  $row.prev().before($row.get(0));
});

$('.table-down').click(function () {
  var $row = $(this).parents('tr');
  $row.next().after($row.get(0));
});

// A few jQuery helpers for exporting only
jQuery.fn.pop = [].pop;
jQuery.fn.shift = [].shift;


$BTN.click(function () {
  var $rows = $TABLE.find('tr:not(:hidden)');
  var headers = [];
  var data = {};

  // Get the headers (add special header logic here)
  $($rows.shift()).find('th:not(:empty)').each(function () {
    headers.push($(this).text().toLowerCase());
  });
  
  /* define header content here !
   * mind to change $TAB_COLID_DEVICE and other const vars */
  var headers2= [];
  // headers2.push('hide1');
  headers2.push('none'); 
  headers2.push('name');
  headers2.push('dev');
  headers2.push('sua');
  headers2.push('q');
  headers2.push('unit');
  headers2.push('notes');
  headers2.push('mv');
  headers2.push('is_sam');
  
  var headers3= [];
  // types: none, text, link, select, checkbox
  headers3.push({'type': 'none'});
  headers3.push({'type': 'text'});
  headers3.push({'type': 'link'});
  headers3.push({'type': 'link'});
  headers3.push({'type': 'text'});
  headers3.push({'type': 'select'});
  headers3.push({'type': 'text'});
  headers3.push({'type': 'text'});
  headers3.push({'type': 'checkbox'});
  
  const CONST_STEP_NR_COL=0; // column ID of stepnr column
  
  

  // Turn all existing rows into a loopable array
  var cnt=0;
  var form_is_valid=1;
  
  $rows.each(function () {
    var $td = $(this).find('td');
    var h = {};

    var temp_type = '';
    var temp_select = '';
    var tmp_str='';
    
    // Use the headers from earlier to name our hash keys
    headers2.forEach(function (colname, i) {
    
      temp_type = headers3[i]['type'];
      if (temp_type=='text')  {
    	tmp_str = $td.eq(i).text();
    	h[colname] = unescape( encodeURIComponent( tmp_str ) ); // convert unicode to UTF-8
    	
    	if (colname=='q')  {
  	      // check numeric
    	  tmp_str = tmp_str.trim();
  		  if (isNaN(tmp_str)) {
  			$td.eq(i).css('border', '1px solid red');
  			form_is_valid=0;
  		  }
  	  }
       
      } 
      if (colname=='is_sam')  {
    	  temp_select = 0;
          if ( $td.eq(i).find('input').is(":checked") ) temp_select=1;
          h[colname] = temp_select;
        } 
      if (temp_type=='select')  {
        temp_select = $td.eq(i).find('select').val();
        h[colname] = temp_select;
      } 
	  if (temp_type=='link')  {
        temp_select = $td.eq(i).find('input').val();
        h[colname] = temp_select;
      } 
	  
	   
	  
      
    });
    
    h['stepno'] = $td.eq(CONST_STEP_NR_COL).find('input').val(); // secret value of TD (column=1)

    data[cnt]=h;
    cnt=cnt+1;
  });
  
  /*
  var tmp = data['0']['name'];
  var hex = '';
  for(var i=0;i<tmp.length;i++) {
	  hex += ':' + tmp.charCodeAt(i).toString(16);
  }
  var tmpenc = document.characterSet;
  $EXPORT.text(JSON.stringify(tmp)+ ' HEX: ' + hex + ' ENC:'+tmpenc);
  */
  // Output the result
  //$EXPORT.text(JSON.stringify(data));

  if (!form_is_valid) {
	  alert("Some data is not valid!");
	  return;
  }

  var redirect = "p.php?mod=DEF/o.ABSTRACT_PROTO.mod";
  var form = '';
          
  form += '<input type="hidden" name="id" value="'+ $ABSTRACT_PROTO_ID +'">';
  form += '<input type="hidden" name="data" value="'+ window.btoa(JSON.stringify(data)) +'">';
          
  $('<form action="'+redirect+'" method="POST">'+form+'</form>').appendTo('body').submit();

  
  
});

