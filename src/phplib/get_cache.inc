<?php
/**
 * get/create cache data to some global variables
 * 
 * - cache variables: $_s_i_table, $_s_i_varcol, $_s_i_app_data_type
 * - tested for 64 bit machines
 * $Header: trunk/src/phplib/get_cache.inc 59 2018-11-21 09:04:09Z $
 * @package get_cache.inc
 * @author  Mac, Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */


require_once('ErrorHandler.inc');

define ('Cache_s_i_varcol',     20020308); // this is the identifier of the shared memory variable for $_s_i_varcol
define ('Cache_s_i_table',      20010401); // this is the identifier of the shared memory variable for $_s_i_table
define ('Cache_s_i_adt',        20030502); // this is the identifier of the shared memory variable for $_s_i_app_data_type
define ('Cache_s_i_misc',       20030503); // this is the identifier of the shared memory variable for $_s_i_app_data_type
define ('Cache_renew_each',       1000001); // the cache is renewed after this number of visits 
										   // TBD:1: do we need this refresh really ???
define ('Cache_renew_var',      19771124); // DEPRECATED: this is the identifier of the shared memory variable for cache renew
if (!defined("PHP_INT_SIZE")) { define("PHP_INT_SIZE", 4); } // backward compatibility PHP < 5.0

function get_cache_calcMem(&$varx) {
	return (((strlen(serialize($varx))+ (4 * PHP_INT_SIZE)) /4 ) * 4 ) + 4;
}

/**
 * use this function every time you start the page
 * errors:
       db_access  1 no connecton to db
       db_access  2 error in query
#      init_cache ...
 * @param boolean $renew
 * @param boolean $display
 * @return: void
 */
function get_cache($renew=false, $display=true) {
    global $_s_i_varcol, $_s_i_table, $_s_i_app_data_type, $_s_i_misc, $_s_i_cache_in_globals;
  
    if (!$renew && $_s_i_cache_in_globals) return;
 
    $error = & ErrorHandler::get();
  

  if (function_exists('shm_attach')) { // the server is a unix system v one
  	
    if (empty($_SESSION['sec'])) {
        $error->set('init_cache', 12, 'You are not logged in. Maybe your session timed out.');
        return;
    }
	$CacheSEMSHM = crc32(strtolower( $_SESSION['sec']['_dbtype'].$_SESSION['sec']['db'].$_SESSION['sec']['dbuser'] ));
	if ($renew && $display)
	  printf('<tt>key of SHM &amp; SEM: 0x%x</tt>', $CacheSEMSHM);

	if (empty($CacheSEMSHM)) {
	  $error->set('init_cache', 10, 'You are not logged in. Maybe your session timed out. (Can\'t init cache.)');
	  return;
	}

	if (!($sem_id = sem_get($CacheSEMSHM, 1, 0600))) {
	  $error->set('init_cache', 2, 'Can\'t get semaphore. This should never happen.');
	  return;
	}
	if (!($shm = shm_attach($CacheSEMSHM, 1999, 0600))) { // if shared mem segment already exists size-param doesn't matter
	  $error->set('init_cache', 1, 'Can\'t attach shared memory for testing if shared memory segment already exists.');
	  return;
	}
 	if(!sem_acquire($sem_id)) { // sem_aquire waits if someone fills the shared memory right now
  	  $error->set('init_cache', 3, 'Can\'t aquire semaphore. This should never happen.');
	  return;
	}
	
	// $Cache_renew_var = @shm_get_var($shm, Cache_renew_var);

	if ($renew && $display)
	  echo '<br><tt>next automatical cache renew: manual.</tt><br>';
	

	if ( $renew ) { // check if segment filled/expired or if we hav a manual renew
	  // shared mem cache is empty or expired, so we have to fill it
	  $cacheSemHex = sprintf(' Semaphor: %x',$CacheSEMSHM);
	  
	  if (!shm_remove($shm)) { // remove existing segment because new data may not fit in old segment
		$error->set('init_cache', 8, 'Can\'t remove existing shared memory segment. This should never happen.');
		sem_release($sem_id); // release semaphore because of error
		shm_detach($shm);
		return;
	  }
  	  require_once('init_cache.inc');
  	  $CacheLib = new gInitCache();
  	  
	  $CacheLib->init_cache( $_SESSION['sec']['dbuser'], $_SESSION['sec']['passwd'], $_SESSION['sec']['db'], $_SESSION['sec']['_dbtype']);
	  if ($error->got(CCT_ERROR_READONLY)) { // do not care about warnings here
		sem_release($sem_id); // release semaphore because of error
		shm_detach($shm);
		return;
	  }

	  $_s_i_cache_in_globals = true;
	  
	  // calc memsize for shm_attach()
	  // learnt by http://www.php.net/manual/de/function.shm-attach.php # help by user: zeppelinux at comcast dot net
	  // when shm_attach() is called for the first time, PHP writes a header to the beginning of the shared memory.
	  // static value of 44 is used as HeaderSize because there are some caching problems using partisan on 64-Bit OS
	  $shmHeaderSize = 44; //if problems with the cache-size occure try to reset $shmHeaderSize = (PHP_INT_SIZE * 4) + 8;
	  $extra_size    =PHP_INT_SIZE*5;
	  $shmHeaderSize = $shmHeaderSize + $extra_size;
	  
	  //$num_of_vars_in_cache  = 4;
	  $shmVarSize              = get_cache_calcMem($_s_i_table);
	  $shmVarSize             += get_cache_calcMem($_s_i_varcol);
	  $shmVarSize             += get_cache_calcMem($_s_i_app_data_type);
	  $shmVarSize             += get_cache_calcMem($_s_i_misc);
	  // $shmVarSize             += get_cache_calcMem($Cache_renew_var);
	  
	  $shm_size = $shmHeaderSize + $shmVarSize; 
	  if ($display)
	  	printf('<tt>Size of Cache [bytes]: %d</tt><br>', $shm_size);
	  if (!($shm = shm_attach($CacheSEMSHM, $shm_size, 0600))) { // attach segment of new size
		$error->set('init_cache', 9, 'Can\'t attach shared memory segement of '.$shm_size.' bytes for storing cache.'.$cacheSemHex);
		sem_release($sem_id); // release semaphore because of error
		shm_detach($shm);
		return;
	  }
	  
	  $tempErrMesg = 'in shared memory , because there is not enough room.'.$cacheSemHex;

	  if (!shm_put_var($shm, Cache_s_i_varcol, $_s_i_varcol)) {
		$error->set('init_cache', 4, 'Can\'t save $_s_i_varcol '.$tempErrMesg);
		sem_release($sem_id); // release semaphore because of error
		shm_detach($shm);
		return;
	  }
	  if (!shm_put_var($shm, Cache_s_i_table, $_s_i_table)) {
		$error->set('init_cache', 5, 'Can\'t save $_s_i_table '.$tempErrMesg);
		sem_release($sem_id); // release semaphore because of error
		shm_detach($shm);
		return;
	  }
	  if (!shm_put_var($shm, Cache_s_i_adt, $_s_i_app_data_type)) {
		$error->set('init_cache', 11, 'Can\'t save $_s_i_app_data_type '.$tempErrMesg);
		sem_release($sem_id); // release semaphore because of error
		shm_detach($shm);
		return;
	  }
	  if (!shm_put_var($shm, Cache_s_i_misc, $_s_i_misc)) {
	      $error->set('init_cache', 11, 'Can\'t save $_s_i_misc '.$tempErrMesg);
	      sem_release($sem_id); // release semaphore because of error
	      shm_detach($shm);
	      return;
	  }
	  /* DO not needed any more
	  if (!shm_put_var($shm, Cache_renew_var, Cache_renew_each)) {
		$error->set('init_cache', 6, 'Can\'t save Cache_renew_var '.$tempErrMesg);
		sem_release($sem_id); // release semaphore because of error
		shm_detach($CacheSEMSHM);
		return;
	  }
	  */
	} else { // cache is already filled, we have to copy it to global variables
	  // $Cache_renew_var--;
	  // shm_remove_var($shm, Cache_renew_var); // remove expiry-counter first because otherwise it gets attached instead of overwritten
	  /*if (!shm_put_var($shm, Cache_renew_var, $Cache_renew_var)) {
		$error->set('init_cache', 6, 'Can\'t save $Cache_renew_var '.$tempErrMesg);
		sem_release($sem_id); // release semaphore because of error
		shm_detach($CacheSEMSHM);
		return;
	  }
	  */
	  if (($_s_i_varcol = @shm_get_var($shm, Cache_s_i_varcol)) === false) { // get from cache to global memory
		$error->set('init_cache', 7, 'Can\'t get $_s_i_varcol from shared memory, even it should be there.');
		sem_release($sem_id); // release semaphore because of error
		shm_detach($shm);
		return;
	  }
	  if (($_s_i_table = @shm_get_var($shm, Cache_s_i_table)) === false) { // get cache to global memory
		$error->set('init_cache', 7, 'Can\'t get $_s_i_table from shared memory, even it should be there.');
		sem_release($sem_id); // release semaphore because of error
		shm_detach($shm);
		return;
	  }
	  if (($_s_i_app_data_type = @shm_get_var($shm, Cache_s_i_adt)) === false) { // get from cache to global memory
		$error->set('init_cache', 7, 'Can\'t get $_s_i_app_data_type from shared memory, even it should be there.');
		sem_release($sem_id); // release semaphore because of error
		shm_detach($shm);
		return;
	  }
	  if (($_s_i_misc = @shm_get_var($shm, Cache_s_i_misc)) === false) { // get from cache to global memory
	      $error->set('init_cache', 7, 'Can\'t get $_s_i_misc from shared memory, even it should be there.');
	      sem_release($sem_id); // release semaphore because of error
	      shm_detach($shm);
	      return;
	  }
	}
	sem_release($sem_id); // we can release semaphore now because shm-access is finished
	shm_detach($shm);
	
  } else { // the server is windows or so
  	
	if (!isset($_SESSION['_s_i_varcol']) || $renew) {
	  $_SESSION['_s_i_varcol']=NULL;
	  $_SESSION['_s_i_table']=NULL;
	  $_SESSION['_s_i_app_data_type']=NULL;#
	  $_SESSION['_s_i_misc']=NULL;
	  
	  require_once('init_cache.inc');
	  $CacheLib = new gInitCache();
	  $CacheLib->init_cache( $_SESSION['sec']['dbuser'], $_SESSION['sec']['passwd'], $_SESSION['sec']['db'], $_SESSION['sec']['_dbtype']);
	}
  }
  $_s_i_cache_in_globals = true;
}
