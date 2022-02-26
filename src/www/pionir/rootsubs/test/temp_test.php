<?php 
session_start(); 

?>
<html>
<head>
<script>


function datalist_clear(list_obj) {

	var i;
	var lenx = list_obj.options.length;
	for (i = 0; i < lenx; i++) {
		list_obj.options[i].text='';
		var a=1;
	}
}

function showResult(str) {
	
    list_obj = document.getElementById("search_list");
    var max_show = 5;
	  
	if (str.length<3) {
		datalist_clear(list_obj);
	    return;
	}

	if ( !isNaN(str) ) {
		/* do not search for numbers ...*/
		return;
	}

	  
	var xhr = new XMLHttpRequest();
	  
	xhr.onreadystatechange=function() {
		
	if (this.readyState==4 && this.status==200) {
		
	      var userInfo = JSON.parse(xhr.responseText);
	      datalist_clear(list_obj);
	      
	      if (userInfo['data']!=null) {
		      
		      data_poi =  userInfo['data']['data'];
    	      matchlist_len = data_poi.length;
    	      
    	      if ( matchlist_len) {
        	      
    	    	  for (i = 0; i < matchlist_len; i++) {

        	    	  if (i>=max_show) {
            	    	  break;
        	    	  }

    	    		  if (list_obj.options.length > i) {
    	    			  var option = list_obj.options[i];
    	    			  option.text = data_poi[i];
    	    			  
    	    		  } else {
    	    		  	  var option = document.createElement("option");
    	    		  	  option.text = new String( "x"+ i.toString() ); /* data_poi[i]; */
      	    		      list_obj.appendChild(option); 
      	    		      
    	    		  }
    	    		  
    	    	  }

    	    	  list_obj_len = list_obj.options.length;
				  if (list_obj_len <= max_show) {
        	    	  var option = document.createElement("option");
        		  	  option.text =  ""; // empty field ...
    	    		  list_obj.appendChild(option); 
				  }
    	    	  
    	    	  match_cnt = parseInt(userInfo['data']['cnt'], 10); /* cast to INT */
    	    	  if (matchlist_len < match_cnt) { /* more matches than shown datalist ... */
        	    	  
    	    		  if (list_obj.options.length > i) {

    	    			  tmpval =  "... more hits of "+str;
    	    			  list_obj.options[i].text = tmpval;

    	    			  /*
    	    			  tmpval =  data_poi[i];
    	    			  tmpval = tmpval.substring(0, 2);
    	    			  list_obj.options[i].text = tmpval;
    	    			  */
    	    			 
    	    			  var dummy=1;
    	    		  } else {
    	    			  /* increase the datalist */
    	    		  	  var option = document.createElement("option");
    	    		  	  option.text =  '... more ...';
      	    		      list_obj.appendChild(option); 
      	    		      var dummy=1;
    	    		  }
    	    	  }
    	      }
	      }
	      
	      
	    }
	}
	  
	xhr.open('PUT', '../../api/rest.php');
	xhr.setRequestHeader('Content-Type', 'application/json');
	xhr.send( JSON.stringify({
	        mod: 'DEF/gObj_search',
	        t: 'ABSTRACT_SUBST',
	        q: str
	    }) );
}


</script>
</head>
<?php




echo "TEST:<br>";

?>


<form>
<input id="txt_search_list" list="search_list" type="text" size="30" autoComplete="off" onkeyup="showResult(this.value)">
<div id="livesearch"></div>
<datalist id="search_list">
</datalist>
</form>


END:<br>


</body></html>
<?php

