<?php

/**
 * Displays the form to select which release of a module to select
 * 
 * $modules is an array of tuples (module name, drupal version)
 */
function modulus_dlenable_modules_form($form, &$form_state, $modules, $modules_to_enable, $enable_dependencies) {  

 	
	$form['#tree'] = TRUE;
  
  $form['all'] = array (
    '#type' => 'fieldset',
    '#title' => 'Download Options',
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    '#tree' => TRUE, // enables tree datastrucutre
  );
 
  
  foreach ($modules as $module) {
    // get release history
    $drupal_version = $module['drupal_version'];
    $module = $module['module'];
    
    $form['all'][$module] = array (
         '#type' => 'fieldset',
         '#title' => $module,
         '#collapsible' => TRUE,
         '#collapsed' => FALSE, // TODO set to TRUE after debugging
    );
    
    $xml = modulus_get_release_history_xml($module, $drupal_version);
    
    if($xml == NULL) {
      $form['all'][$module]['error'] = array(
        '#markup' => 'An error occured while retrieving the release history. ' .
                     'Check the error messages for more information.',
      );
      continue;
    }
    // TODO: choose optimal solution
    
    // form for user

    // extract releases
    // Example for restricting:
    //  $releases = $xml->xpath("/project/releases/release[status='published']");
    $releases = $xml->xpath("/project/releases/release");
    $options = array('none');
    foreach($releases as $release) {
      // show information about release
      $opt =  $release->name;
      $opt .= ', Version: ' .  $release->version;
      $opt .= ', Version Extra: ' .  $release->version_extra;
      $opt .= ', Status: ' . $release->status;
      $opt .= ', ' . l('Release link',  $release->release_link);
      array_push($options, $opt);
      
      // show radios
      $form['all'][$module]['radios'] = array(
        '#type' => 'radios',
        '#title' => t('Choose the release you want to install' . $module),
        '#options' => $options,
      );
      
      // save requested drupal version
      $form['all'][$module]['drupal_version'] = array(
        '#type' => 'value',
        '#value' => $drupal_version,
      );
    }
  }

  if(!empty($modules_to_enable)) {
      $form['modules_to_enable'] = array (
        '#type' => 'fieldset',
        '#title' => 'Following modules are going to be enabled',
        '#collapsible' => FALSE,
        '#collapsed' => FALSE,
        '#tree' => TRUE, // enables tree datastrucutre
      );
    foreach($modules_to_enable as $module) {
      $form['modules_to_enable'][$module] = array(
        '#type' => 'value',
        '#value' => $module,
      );
    }

    $form['modules_to_enable']['list'] = array(
      '#markup' => implode('<br>', $modules_to_enable)
    );

    $form['modules_to_enable']['enable_dependencies'] = array(
      '#type' => 'value',
      '#value' => $enable_dependencies,
    );
  }
  
  $form['submit'] = array(
      '#type' => 'submit',
      '#value' => empty($modules_to_enable) ? t('Download') : t('Download & Enable'),
      '#submit' => array('modulus_dlenable_modules_form_submit'),
  );

  return $form;
}

function modulus_dlenable_modules_form_submit($form, &$form_state) {
  // array where all operations are saved for the batch
  $operations = array();
  $authorize = FALSE;
  
  //$output = array();
  if (!isset($form_state['values']['all'])) {
    $form_state['values']['all'] = array();
  }
  
  $projects = array(); 
  foreach($form_state['values']['all'] as $module => $info) {
  	//echo $module;
    
	  if ($module == 'submit') {
	  // TODO: check this
	  	continue;
	  }
	      
		$core = $info['drupal_version'];
	   
	  // if none was chosen, do nothing
	  $release = $form_state['values']['all'][$module]['radios'] - 1;
    
		$success = modulus_download_module($module, $core, $release = NULL, $verbose = TRUE);
		
		if ($success) {
			$real_path_project = drupal_realpath(modulus_extract_directory($module));
			$projects[$module] = array(
				'project' => $module,
     		'updater_name' => 'ModuleUpdater',
      	'local_url' => $real_path_project,
			);
		}

  }
  
  // Make sure the Updater registry is loaded
  drupal_get_updaters(); 
  module_load_include('inc', 'update', 'update.authorize');
  $filetransfer = new FileTransferLocal(DRUPAL_ROOT);
  //system_authorized_init('modulus_batch', drupal_get_path('module', 'modulus') . '/modulus.modules', array($batch), t('Installing modules'));
   
  //update_authorize_run_update($filetransfer, $projects);
  
  
  
  foreach ($projects as $module) {

  	//$extract_directory = modulus_downloads_directory(TRUE);
 	 	$project_location = modulus_extract_directory($module['project']);
        
  	// taken from update.manager.inc
  	// update_manager_install_form_submit() 
  	// check updater doc: http://api.drupal.org/api/drupal/includes!updater.inc/class/Updater/7
      
  
		try {
	  	$updater = Updater::factory($project_location);
		}
	  catch (Exception $e) {
	  	drupal_set_message('Exception catched: ' . $e->getMessage(), 'error');
			return FALSE;
		}
	    
		try {
	  	$project_title = Updater::getProjectTitle($project_location);
		}
	  catch (Exception $e) {
	  	drupal_set_message('Exception catched: ' . $e->getMessage(), 'error');
	    return FALSE;
		}
	    
		if (!$project_title) {
	  	if ($verbose) drupal_set_message(t('Unable to determine %project name.', array('%project' => $project)), 'error');
	    return FALSE;
		}

		$real_project_location = drupal_realpath($project_location);
    // call_user_func_array('update_authorize_run_install_modified', array_merge(array($filetransfer), $arguments));
    $operations[] = array(
    	'update_authorize_batch_copy_project', array(
    		$project_title,
      	get_class($updater),
        $real_project_location, // local url
        $filetransfer,
			),
    );
      
    // check if we have enough rights - otherwise ask for authorization
    if (!fileowner($real_project_location) == fileowner(conf_path())) {
	  	// Otherwise, go through the regular workflow to prompt for FTP/SSH
	    // credentials and invoke update_authorize_run_install() indirectly with
	    // whatever FileTransfer object authorize.php creates for us.
      $authorize = TRUE;
			//system_authorized_init('update_authorize_run_install', drupal_get_path('module', 'update') . '/update.authorize.inc', $arguments, t('Update manager'));
      //$form_state['redirect'] = system_authorized_get_url();
      //return;
       
    }
  }
  
  
  //if (empty($operations)) {
  //	return;
  //}
  

  
  /*
  // add operations to enable modules
  $modules_to_enable = array();
  $enable_dependencies = FALSE;
  if (isset($form_state['values']['modules_to_enable'])) {
    foreach($form_state['values']['modules_to_enable'] as $key => $value) {
      if($key != 'enable_dependencies')
        $modules_to_enable[] = $form_state['values']['modules_to_enable'][$key];
    }
    $enable_dependencies = $form_state['values']['modules_to_enable']['enable_dependencies'];
  }
  */
  $operations[] = array('modulus_batch_module_enable');
    
  
  //debug($operations);
  //return;
  
  // create batch with gathered operations and start it
  $batch = array(
    'title' => t('Installing modules'),
    'init_message' => t('Preparing to install'),
    'operations' => $operations,
    'finished' => 'update_authorize_install_batch_finished',
    'file' =>  drupal_get_path('module', 'modulus') . '/modulus.module',
    //'file' => drupal_get_path('module', 'update') . '/update.authorize.inc',
  );


  if ($authorize) {
    drupal_set_message('PHP does not have the rights to download to the modules folder', 'error');
    //return;
    system_authorized_init('modulus_batch', drupal_get_path('module', 'modulus') . '/modulus.modules', array($batch), t('Installing modules'));
    $form_state['redirect'] = system_authorized_get_url();
    return;
  }
  else {
  	//watchdog('modulus_first', print_r($batch, true), array(), WATCHDOG_NOTICE);
     batch_set($batch);
    // Invoke the batch via authorize.php.
    system_authorized_batch_process();
  }
  
}

function modulus_batch($filetransfer, $batch) {
  foreach($batch['operations'] as &$op) {
    if($op[0] != 'modulus_batch_module_enable')
      $op[1][3] = $filetransfer;
  }
  
  watchdog('modulus', print_r($batch, true), array(), WATCHDOG_NOTICE);
  
  batch_set($batch);
  system_authorized_batch_process();
}

/**
 * module_enable wrapper for batch
 */
function modulus_batch_module_enable($module_list, $enable_dependencies, &$context) {

  $context['finished'] = 1.0;
  //include('')
  module_enable($module_list);//, $enable_dependencies);
 	$context['results']['log']['Return'][] = array('message' =>  l('Click here to return.', 'admin/patterns/check/' . variable_get('patterns_pid', 0)), 'success' => 'Yuppie!');
  return;
}

////////////////////////////////////////

function modulus_is_modules_dir_writable() {
	if (!fileowner($project_real_location) == fileowner(conf_path())){};
	// TODO
}

function modulus_is_module_available($module = NULL, $list = NULL) {
  if (is_null($module)) return FALSE;
  $modules = (is_null($list)) ? system_rebuild_module_data() : $list;
  return (isset($modules[$module])) ? TRUE : FALSE;
}

function modulus_is_module_enabled($module = NULL, $list = NULL) {
  if (is_null($module)) return FALSE;
  $modules = (is_null($list)) ? module_list() : $list;
  return isset($modules[$module]) ? TRUE : FALSE;
}

function modulus_module_state($module) {
	if (modulus_is_module_enabled($module)) {
		return 'enabled';
	}
	if (modulus_is_module_available($module)) {
		return 'available';
	}
	return 'missing';
}

function modulus_build_dep_tree($module, $core, $release = NULL, $deps = array()) {
	// TODO: how to find the current core?
	
	$modules = system_rebuild_module_data();
	if (isset($modules[$module])) {
		$current_deps = $modules[$module]->info['dependencies']; 
		if (!empty($current_deps)) {
			foreach ($current_deps as $d) {
				$deps[$d] = array();
				$deps[$d]['state'] = modulus_module_state($module);
				$deps[$d]['dependencies'] = modulus_build_dep_tree($d, $core, NULL);
			}	
		}
	}
	else {
		modulus_build_dep_tree_remote($module, $core);
	}
  return $deps;
}

function modulus_build_dep_tree_remote($module, $core, $release = NULL, $deps = array()) {
	if (!modulus_download_module($module, $core, $release)) return FALSE;
	$info = modulus_get_info_from_extracted_project($module);
	$current_deps = @$info['dependencies'];
	if (!empty($current_deps)) {
		foreach ($current_deps as $d) {
			$deps[$d] = array();
			$deps[$d]['state'] = modulus_module_state($module);
			$deps[$d]['dependencies'] = modulus_build_dep_tree($d, $core, $release);
		}	
	}
	return $deps;
}


function modulus_get_info_from_extracted_project($module) {
	$dir = modulus_extract_directory($module);
  return drupal_parse_info_file($dir . '/' . $module. '.info');
}

function _modulus_find_most_recent_published_release($xml) {
	$releases = $xml->xpath("/project/releases/release");
	$found = FALSE;
	foreach($releases as $release) {
		if (!$release->xpath('status') === 'published') {
			continue;
		}
		if (!$found || $release->xpath('date') > $found->xpath('date')) {
			$found = $release;
		}
	}
	return $found;
}

function modulus_download_module($module, $core, $release = NULL, $verbose = TRUE) {

	$xml = modulus_get_release_history_xml($module, $core);
  if (is_null($xml)) {
  	if ($verbose) drupal_set_message(t('!project: Error while processing release history', array('!project' => $module)), 'error');	
    return FALSE;
	}

  if (!is_null($release)) {
  	$release = $xml->xpath("/project/releases/release[version='" . $release . "']");	
	}
	else {
  	 // if no release was specified we take the most recent one
  	$release = _modulus_find_most_recent_published_release($xml);
	}
  	
 	if (!$release) {
 		if ($verbose) drupal_set_message(t('!project: No valid release found.', array('!project' => $module)), 'error');	
    return FALSE;
 	}
  
 	
	$url = $release->download_link;
 	
  return _modulus_download_and_extract($module, $url);

}

function modulus_extract_directory($module = NULL) {
	$extract_directory = _update_manager_extract_directory();
	return (is_null($module)) ? $extract_directory : $extract_directory . '/' . $module; 
}

function _modulus_download_and_extract($module, $url, $context = NULL, $verbose = TRUE) {
	if (is_null($context)) {
		$context = array();
	 	$context['sandbox']['started'] = TRUE;
	}
 	
	update_manager_batch_project_get($module, $url, $context);
      
  debug('SUCCESS');  
  return TRUE;    
}

function modulus_move_project_to($project, $to) {
	$project_location = modulus_extract_directory($project);
	file_unmanaged_copy($project_location,  drupal_realpath('public://') . "/modulus");
	file_unmanaged_copy($project_location,  $to);
  //debug(modulus_downloads_directory() . '/' . $module);
}


 /**
 * Download the release history xml for the specified request.
 * based on drush's "_drush_pm_get_release_history_xml($request)" function
 * TODO: replace drush's error/warning messages with our own
 */
function modulus_get_release_history_xml($name, $drupal_version) {
  $url = 'http://updates.drupal.org/release-history' . '/' . $name . '/' . $drupal_version;
  
  // Some hosts have allow_url_fopen disabled.
  // TODO: alternative solution?
  if (!$xml = @simplexml_load_file($url)) {
    drupal_set_message('Could not download release history.', 'error');
//  $filename = _drush_download_file($url);
//  $xml = simplexml_load_file($filename);
//  drush_op('unlink', $filename);
  }
  if (!$xml) {
    // We are not getting here since drupal.org always serves an XML response.
    drupal_set_message(t('!project: Could not download project status information from !url', array('!url' => $url, '!project' => $name)), 'error');
    return NULL;
  }
  if ($error = $xml->xpath('/error')) {
    // Don't set an error here since it stops processing during site-upgrade.
    //drush_log($error[0], 'warning'); // 'DRUSH_PM_COULD_NOT_LOAD_UPDATE_FILE',
    drupal_set_message(t('!project: Error returned as project status of information from !url', array('!url' => $url, '!project' => $name)), 'error');
    return NULL;
  }
  // Unpublished project?
  $project_status = $xml->xpath('/project/project_status');
  if ($project_status[0][0] == 'unpublished') {
    drupal_set_message(t("Project !project is unpublished and has no releases available.", array('!project' => $request['name'])), 'warning');
  }

  return $xml;
}