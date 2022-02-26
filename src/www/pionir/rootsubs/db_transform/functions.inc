<?php
########################################################################
# $Header: trunk/src/www/pionir/rootsubs/db_transform/functions.inc 59 2018-11-21 09:04:09Z $
########################################################################
# Functions.
#
# Piet, CCT Jena
########################################################################

function fget_line (&$fileHandle)
{ ## Read one line from an ASCII file where lines
  ## are delimited by newline character ("\n"):
  $blockSize = 1000; 
  $line = '';

  ## Repeated reads are needed for lines longer than $blockSize:
  while( substr($line,-1,1)!="\n" and !feof ($fileHandle) )
  { $line .= fgets($fileHandle, $blockSize); }

  return $line;

}##End function

########################################################################

function is_line_empty ($line)
{ ## Test if line contains only whitespace characters ("\s"):
 
  return preg_match("/^\s*$/",$line);

}##End function

########################################################################

function is_line_comment ($line)
{ ## Test if line begins with '#' or '//':
 
  if( preg_match("/^\s*#/",$line) )    { return 1; }
  if( preg_match("/^\s*\/\//",$line) ) { return 1; }

  return 0;

}##End function

########################################################################

function get_comment ($line)
{ ## Extract comment from a comment line.
  ## Comment lines begin with '#' or '//':
 
  preg_match("/^\s*(#|\/\/)\s*(.*)$/",$line,$matches);

  return $matches[2];

}##End function

########################################################################

function script_info ()
{ ## Print name and date of current script:

  $scriptName  = $_SESSION['SCRIPT_FILENAME'];
  $scriptNameT = $_SESSION['PATH_TRANSLATED'];
  clearstatcache();
  $scriptDate = date("j M Y H:i",filemtime($scriptName));

  print "<font size=-1>\n";
  print "Script name:&nbsp;   '" . basename($scriptName) . "'";
  print "<br>Script dir:&nbsp;   '"  . dirname($scriptNameT) . "'";
  print "<br>Last modified:&nbsp; $scriptDate";
  print "</font>\n"; 

  return;

}##End function

########################################################################

function open_HTML_page ($title)
{ #### Open a HTML page.

  print "<html>\n";
  print "<head>\n";
  print "  <title>$title</title>\n";
  print "</head>\n";

  print "<body bgcolor=\"#FFFFFF\">\n";
  print "<h1>$title</h1>\n";
  flush();

}##End function

########################################################################

function close_HTML_page ()
{ #### Close a HTML page.

  print "</body>\n";
  print "</html>\n";

}##End function

########################################################################

function footline ()
{ #### Close a HTML page.

  print "<hr noshade size=1>\n";
  print "<table>\n";
  print "<tr><td><font size=-1>Script:&nbsp;&nbsp;&nbsp;&nbsp;</font></td>\n";
  print "    <td><font size=-1>'".$_SERVER[PHP_SELF]."'</td></font></tr>\n";
  print "<tr><td><font size=-1>Author:&nbsp;&nbsp;&nbsp;&nbsp;</font></td>\n";
  print "    <td><font size=-1><a href=\"mailto:piet@clondiag.com\">Peter Slickers</a>, CCT Jena</font></td></tr>\n";
  print "</table>\n";

}##End function

########################################################################

function getmicrotime()
{ #### detemine time in sec with three decimals
  list($usec, $sec) = explode(" ",microtime()); 
  return ((float)$usec + (float)$sec); 
}##End function 

########################################################################
#eof
?>
