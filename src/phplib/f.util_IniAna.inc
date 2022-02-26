<?php
/*MODULE:  f.util_IniAna.inc
  DESCR:   analyse php-ini-file
  AUTHOR:  qbi
  VERSION: 0.1 - 20071219
*/

function ini_get_bytes($option_name)
## from: piet
## Extension of buildin function 'ini_get' for
## reading the config variables 'upload_max_filesize',
## 'post_max_filesize', and 'memory_limit' more comfortably. 
##
## This function ALWAYS returns an integer value,
## If the config variable contains a string
## like '8M' or '130K', this string is converted 
## into an integer.

{ $value = ini_get($option_name);
  if ( is_null($value) ) { return ' -- unset -- '; }
  else
  { $value = trim($value);
    $last_char = strtoupper(substr($value,-1));

    if     ( $last_char == 'K' ) { $factor = 1E3; }
    elseif ( $last_char == 'M' ) { $factor = 1E6; }
    elseif ( $last_char == 'G' ) { $factor = 1E9; }
    elseif ( $last_char == 'T' ) { $factor = 1E12; }
    else                         { $factor = 1; }

    return (int) ($factor * $value);
  }##End else

}##End function

########################################################################