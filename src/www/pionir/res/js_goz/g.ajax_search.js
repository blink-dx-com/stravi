/* support AJAX quick search for header.php */


/*
 * Info: I used encodeURIComponent() to support UTF-8 chars !
 * @var header_timer: time stamp for open_unten to prevent MULTIPLE SUBMIT actions, 
 *      which produce multi httpd processes and so block the server 
 *
 */
var x_search_last_str = ''; // store last user input string, used in header.php:javascript !!!
var header_timer=0; 
var header_cnt=0;
var header_lastid="";


function open_help( url )   {				
	
	InfoWin = window.open( url, "help","scrollbars=yes,width=950,height=500,resizable=yes"); 
	InfoWin.focus();				
}

function header_sleep(ms) {
    // return new Promise(resolve => setTimeout(resolve, ms));
    var start = new Date().getTime();
	for (var i = 0; i < 1e7; i++) {
	    if ((new Date().getTime() - start) > ms){
	      break;
	    }
	}
}


// submit the search form ...
function open_unten( mode ) {
    // prevent too many key strokes, which call the parent.unten ...
	MIN_SUBMIT_TIME = 3000; // minimum 3 seconds between two submits

	seatxt  = document.editform.idx.value;
	lookfor = '('+ x_search_last_str +')'; // this comes from g.ajax_search.js 
	lookfor_len = lookfor.length;
	
	if ( seatxt.substr(seatxt.length-lookfor_len)==lookfor ) {
		seatxt = seatxt.substr(0,seatxt.length-lookfor_len);  // shorten the search string
	}
	
	if (!header_timer) {
		header_timer = new Date().getTime();
	} else {
		var now = new Date().getTime();
		var distance = now - header_timer;
		rest = MIN_SUBMIT_TIME - distance;
		//document.getElementById('headerDebug').innerHTML = "cnt: "+ header_cnt + " distance:" + distance + 
		//	 " rest:" + rest;
		
		if ( distance < MIN_SUBMIT_TIME) {
			if (header_lastid==seatxt) return; // no action, because same request .....
			header_sleep(rest);
		}
		// get new time stamp
		header_timer  = new Date().getTime();
		
    }
	header_cnt = header_cnt +1;

	header_lastid = seatxt; 
    tab = document.editform.tablename.options[document.editform.tablename.options.selectedIndex].value;
    parent.unten.location.href="glob.obj.qsearch.php?idx=" + encodeURIComponent(seatxt) + "&go=" + mode + "&tablename="+tab;
}

function datalist_clear(list_obj) {

	var i;
	var lenx = list_obj.options.length;
	for (i = 0; i < lenx; i++) {
		list_obj.options[i].text='';
		var a=1;
	}
}

// perform a quick search against the database ...
function showResult(str) {
	
    list_obj = document.getElementById("search_list");
    
    var e = document.getElementById("tab_search_list");
    var tablename = e.options[e.selectedIndex].value;
    if (tablename=='') {
    	return;
    }
    
    var max_show = 25;
	  
	if (str.length<3) {
		datalist_clear(list_obj);
		x_search_last_str = ''; 
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
	      
	      x_search_last_str = str; /* store last user input string */
	      
	      if (userInfo['data']!=null) {
		      
		      data_poi =  userInfo['data']['data'];
    	      matchlist_len = data_poi.length;
    	      
    	      if ( matchlist_len) {
        	      
    	    	  upsearch = str.toUpperCase();
    	    	  
    	    	  for (i = 0; i < matchlist_len; i++) {

        	    	  if (i>=max_show) {
            	    	  break;
        	    	  }
        	    	  
        	    	  tmptxt = data_poi[i];
        	    	  uptxt  = tmptxt.toUpperCase();
        	    	  if (uptxt.search(upsearch)<0) {
        	    		  tmptxt = tmptxt + ' ('+str+')'; // if synonym found you have to add the original text
        	    	  }

    	    		  if (list_obj.options.length > i) {
    	    			  var option = list_obj.options[i];
    	    			  option.text = tmptxt;
    	    			  
    	    		  } else {
    	    		  	  var option = document.createElement("option");
    	    		  	  option.text =  tmptxt;
      	    		      list_obj.appendChild(option); 
      	    		      
    	    		  }
    	    		  
    	    	  }

    	    	  i=i-1;
    	    	  match_cnt = parseInt(userInfo['data']['cnt'], 10); /* cast to INT */
    	    	  if (matchlist_len < match_cnt) { /* more matches than shown datalist ... */
        	    	  
    	    		  if (list_obj.options.length > i) {
 
    	    			  list_obj.options[i].text = "... more hits of "+str;
    	    			 
    	    			  var dummy=1;
    	    		  } else {
    	    			  /* increase the datalist */
    	    		  	  var option = document.createElement("option");
    	    		  	  option.text =   "... more hits of "+str;
      	    		      list_obj.appendChild(option); 
      	    		      var dummy=1;
    	    		  }
    	    	  }
    	      }
	      }
	      
	      
	    }
	}
	  
	xhr.open('PUT', 'api/rest.php');
	xhr.setRequestHeader('Content-Type', 'application/json');
	xhr.send( JSON.stringify({
	        mod: 'DEF/gObj_search',
	        t: tablename,
	        q: str
	    }) );
}
