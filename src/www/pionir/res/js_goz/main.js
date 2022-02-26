/* main gozilla js functions */

function open_info( url ) {
   InfoWin = window.open(url, "help", "scrollbars=yes,width=650,height=500,resizable=yes");
   InfoWin.focus();
}

function goz_error_box() {
	document.getElementById('x_error_long').style.display='block';
	document.getElementById('x_error_short').style.display='none';
}

function goz_check_form(form_name) {
	/* needs global var: goz_form_data[] = array('req'=>, 'name'=>) */
	var f = document.getElementById("id_"+form_name);
	
	if (goz_form_data==null) return;
	for (row of goz_form_data) {
		if (row['req']) {
			let x = f.elements[row['var']].value;
			  if (x == "") {
			    alert("Required parameter \""+row['nice']+"\" is missing.");
			    return false;
			  }
		 }
	}
	document.forms[form_name].submit();	
 }