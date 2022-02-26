<?php
/**
 * password, crypt lib
 * - since 2021-06-09: password store for new users: with SALT!
 * - still supports password without SALT, but is deprecated
 * @package f.password.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class fPasswordC {
    
   
    /**
     * OLD method: without SALT
     * @param string $password
     * @return string  crypted password ready for DATABASE
     */
    static function password_OLD2crypt(string $password) {
        $passwd_crypt = md5( $password );
        return ($passwd_crypt);
    }
    
    // return OLD, NEW
    static function type_of_crypt(string $passwd_crypt) {
        $c_type='OLD';
        if (strstr($passwd_crypt,':')!=NULL) $c_type='NEW';
        return $c_type;
    }
    
    static function createPasswd() {
        
        // start with a blank password
        $password = "";
        $length   = 16;
        // define possible characters
        $possible = "0123456789bcdfghjkmnpqrstvwxyzABCDEFGHIJKLMNOPQRESTUVWXYZ";
        
        // set up a counter
        $i = 0;
        
        // add random characters to $password until $length is reached
        while ($i < $length) {
            
            // pick a random character from the possible ones
            $char = substr($possible, mt_rand(0, strlen($possible)-1), 1);
            
            // we don't want this character if it's already in the password
            if (!strstr($password, $char)) {
                $password .= $char;
                $i++;
            }
            
        }
        return $password;
        
    }
    
    static function get_nonlogin_pw() {
        $passwd_crypt = 'BAD_PW';
        return $passwd_crypt;
    }
    
    /**
     *  transform user_password into "salt:MD5": ready for DB_USER update
     */
    static function hash_pw($password) {
        $salt = random_bytes(16);
        $salt_str = substr(base64_encode($salt),0,16);
        $output = self::hash_pw_salt($password, $salt_str);
        return $output;
    }
    
    //   create hash from salt+password
    static function hash_pw_salt( $password, $salt_str) {
        $pw_hash = md5( $salt_str . $password );
        $output  = $salt_str .':'. $pw_hash;
        return $output;
    }
    
    // return 0 or 1
    static function verify_pw($pw_db, $password_in) {
        if(!is_string($password_in)) $password_in=''; // cast to string 
        $c_type='OLD';
        if (strstr($pw_db,':')!=NULL) $c_type='NEW';
        
        if ($c_type=='OLD') {
            $isok=0;
            $passwd_in_crypt = self::password_OLD2crypt($password_in);
            if ($passwd_in_crypt == $pw_db) $isok=1;
            return $isok;
        }
        $db_pw_arr = explode(":",$pw_db);
        $salt_str  = $db_pw_arr[0];
        
        $pw_hash_user = self::hash_pw_salt($password_in, $salt_str);
        
        # debug.printx( __name__, 'PW_COMP:: pw_db:'+ str(pw_db) + ' IN:'+ password_in + ' HASGED:'+pw_hash_user)
    
        $isok=0;
        if ($pw_hash_user == $pw_db) $isok=1;
        
        return $isok;
    }
    
}
