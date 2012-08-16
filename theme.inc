<?php
/**
 * @file
 * Patterns modules page theme functions.
 */

/**
 * Display information about the status of the modules passed as parameter
 *
 */
function modulus_page($options = array()) {
	$info = array();
    
 	$info['Path'][] = t('Temporary download directory:');
  $info['Path'][] = drupal_realpath(modulus_extract_directory());
  $info['modules_system_dir'][] = t('System modules directory');
  $info['modules_system_dir'][] = DRUPAL_ROOT . 'sites/all/modules';
      
  $directory = DRUPAL_ROOT . 'sites/all/modules';
  
  if (modulus_is_modules_dir_writable()) {
  	$write_dir = t('sites/all/modules/ is writable. You will be able to copy downloaded modules here automatically. Important: this is NOT the recommended setting for production web sites.');
  }
  else {
  	$write_dir = t('sites/all/modules/ is not writable. You will NOT be able to copy downloaded modules here automatically. Important: this is the recommended setting for production web sites.');
  }
   
  $info['modules_system_dir_write'][] = t('Modules system write permissions');
  $info['modules_system_dir_write'][] = $write_dir;
   
      

  $out = '<h6>Info</h6>';
  $out .= theme('table', array('rows' => $info));
  $form_state['build_info']['args'] = array($options);
  
  $out .= drupal_render(drupal_build_form('modulus_page_form', $form_state));
  
  return $out;
}

function modulus_page_form($form, $form_state, $modules, $core = '7.x', $options = array()) {
	
	drupal_add_js(drupal_get_path('module', 'modulus') . '/modulus.js');
	// check if this is the first time or if we are rebuilding the form
  if (!isset($form_state['values']['step'])) {
    $form_state['values']['step'] = 1;
  }
  else {
    // check what button was clicked and which operation to execute
    // download button?
    
    // ENABLE
    $modules_to_just_enable = array();
    $modules_to_enable_after_dl = array();
		$modules_to_dl = array();
    
    foreach ($form_state['values']['modules'] as $module => $info) {
    	if ($info['download'] == 1) {
    		$modules_to_dl[$module] = array(
      		'module' => $module, 
        	'drupal_version' => $info['drupal_version'],
    		);
    	}
    	
    	if ($info['enable'] == 1 && $info['download'] != 1) {
    		$modules_to_just_enable[$module] = $module;
    	}
    	else if ($info['enable'] == 1) {
    		$modules_to_enable_after_dl[$module] = $module; 
    	}
    	
    }
    
    if (count($modules_to_dl) > 0) {
    	$form_state2; // hack
      $form = modulus_dlenable_modules_form(NULL, $form_state2, $modules_to_dl, ($modules_to_just_enable + $modules_to_enable_after_dl));    
      return $form;
    }
    else if (count($modules_to_just_enable) > 0) {
    	 $result = module_enable($modules_to_just_enable);
    	 if ($result) {
    	 		drupal_set_message(t('All modules were succesfully enabled'));
    	 }
    	 else {
    	 		drupal_set_message(t('One or more modules could not be enabled'));
    	 }
    }
  }
  
	
  $modules_info = system_rebuild_module_data();
  
  $modules_list = module_list();

  $not_available = FALSE;
  
  $form['modules'] = array(
    '#prefix' => '<table border=1>
                    <tr>
                      <td><h1>Module</h1></td>
                      <td><h1>Enable</h1></td>
                      <td><h1>Download</h1></td>
                    </tr>',
    '#suffix' => '</table>',
  );
  $form['#tree'] = TRUE;
  $form['step'] = array(
    '#type' => 'value',
    '#value' => $form_state['values']['step'],
  );
  
  //variable_set('patterns_pid', $pid);
  
  // Get module name, whether it is to be disabled or enabled,
  // whether the module is available or not, and whether it is
  // currently enabled or not
  foreach ($modules as $module) {
    $module = is_array($module) ? $module['value'] : $module;
    $available = array_key_exists($module, $modules_info);
    $enabled = array_key_exists($module, $modules_list);
    //$drupal_version = isset($pattern->pattern['info']['core']) ? $pattern->pattern['info']['core'] : 'core undefined';
    
    $form['modules'][$module] = array(
      '#prefix' => '<tr>',
      '#suffix' => '</tr>',
    );
    
    $form['modules'][$module]['m'] = array(
      '#markup' => $module,
      '#prefix' => '<td>',
      '#suffix' => '</td>',
    );
      
    $form['modules'][$module]['module'] = array(
      '#type' => 'value',
      '#value' => $module,
    );
    
    $form['modules'][$module]['drupal_version'] = array(
      '#type' => 'value',
      '#value' => $core,
    );
    
    // enable checkbox
    $form['modules'][$module]['enable'] = array(
      '#type' => 'checkbox',
      '#title' => $enabled ? t('Already enabled') :  t('Enable'),
      '#disabled' => $enabled,
      '#prefix' => '<td>',
      '#suffix' => '</td>',
    );
    
    // download checkbox
    $form['modules'][$module]['download'] = array(
      '#type' => 'checkbox',
      '#title' => $available ? t('Already downloaded') : t('Download'),
      '#disabled' => $available,
      '#prefix' => '<td>',
      '#suffix' => '</td>',
    );
    
  }
  
  // buttons
  $form['submit_dl'] = array(
    '#type' => 'submit',
    '#value' => t('Go!'),
    '#submit' => array('modulus_page_submit'),
  );
  
  $form['#redirect'] = 'modulus_page_submit';
  return $form;
}

  

// needed for redirecting to another form
function modulus_page_submit($form, &$form_state) {
  $form_state['values']['step'] = $form_state['values']['step'] + 1;
  $form_state['rebuild'] = TRUE;  
}