<?php
/**
 * @file
 * Patterns modules page theme functions.
 */

/**
 * Lists the modules used by a particular pattern.
 *
 * @TODO Doc.
 */
//function modulus_page($pid) {
//  $pattern = patterns_utils_if_invalid_go_back($pid);
//
//  drupal_set_title($pattern->title . ' ' . t('(Pattern Modules)'));
//
//  $modules = isset($pattern->pattern['modules']) ? $pattern->pattern['modules'] : array();
//  $modules = is_array($modules) ? $modules : array($modules);
//  $modules_info = system_rebuild_module_data();
//  $modules_list = module_list();
//
//  $not_available = FALSE;
//  
//  
//  //$header = array(t('Name'), t('Available'), t('Enabled'), t('Actions'));
//  
//  $header = array(
//    'name' => array('data' => t('Name'), 'field' => 'n.name', 'sort' => 'asc'),
////    'available' => array('data' => t('Available'), 'field' => 'n.available'),
////    'enabled' => array('data' => t('Enabled'), 'field' => 'n.enabled'),
////    'actions' => array('data' => t('Actions'), 'field' => 'n.actions'),
//  );
//  
////  $header = array('name' => t('Name'), 'available' => t('Available'), 'enabled' => t('Enabled'), 'actions' => t('Actions'));
//  
//  $rows = array();
//  // Get module name, whether its to be disabled or enabled,
//  // whether the module is available or not, and whether it is
//  // currently enabled or not
//
//  foreach ($modules as $module) {
//    $module = is_array($module) ? $module['value'] : $module;
//    $available = array_key_exists($module, $modules_info);
//    $enabled = array_key_exists($module, $modules_list);
//    
//    $row = array();
//    $row['name'] = $module;
////    $row['available'] = $available ? t('Yes') : '<span class="alert">' . t('No') . '</span>';
////    $row['enabled'] = $enabled ? t('Yes') : '<span class="alert">' . t('No') . '</span>';
//
////    if ($available && ! $enabled) {
////      $row['actions'] = l(t('Enable'), 'admin/patterns/drush/enable/' . $module);
////
////    }
////    elseif (!$available) {
////      $row['actions'] = l(t('Download and Enable'), 'admin/patterns/drush/dlenable/' . $module);
////    }
////    else {
////      $row['actions'] = '-';
////    }
//
//    //$row[] = '<span id="action_results"></span>';
//    
//    //$row['#attributes'] = array('class' => array('blue-row'));
//
//    $rows[$module] = $row;
//
//    if (!$available) {
//      $not_available = TRUE;
//    }
//  }
//
//  if ($not_available) {
//    drupal_set_message(t('Some modules are not available, please download them before running this pattern.'), 'error');
//  }
//  else {
//    drupal_set_message(t('All modules required by this module are available. Click !here to run this pattern.', array('!here' => l(t('here'), 'admin/patterns/enable/' . $pid))));
//  }
//
////  if (empty($rows)) {
////    $rows = array(
////      array(
////        'data' => array(
////          array(
////            'data' => t('No extra modules are required to run this pattern.'),
////            'colspan' => '5',
////          ),
////        ),
////      ),
////    );
////
////  }
//
//  /// debug($rows);
//  
//  //return theme('table', array('header' => $header, 'rows' => $rows, 'caption' => t('Modules used for this pattern')));
//  
//  //return theme('tableselect', array('options' => $rows, 'header' => $header, 'empty' => t('no!'),));
//  
////  $options = array();
////$options[0]['title'] = "A red row";
////$options[0]['#attributes'] = array ('class' => array('red-row'));
////$options[1]['title'] = "A blue row";
////$options[1]['#attributes'] = array ('class' => array('blue-row'));
//
//  $form['modules'] = array (
//    '#type' => 'tableselect',
//    '#options' => $rows,
//    '#header' => $header,
////    '#empty' => t('no!'),
//    '#attributes' => array(),
//  );
//
//  return $form;
//
//  return drupal_render($form);
//}


/**
 * Lists the modules used by a particular pattern.
 *
 * @TODO Doc.
 */
function modulus_page($pid) {
	debug(drupal_realpath('public://') . "/modulus");
  $form_state['build_info']['args'] = array($pid);
  return drupal_render(drupal_build_form('modulus_page_form', $form_state));
}

/*
function modulus_page_form($form, &$form_state, $pid) {
  
  
  $pattern = patterns_utils_if_invalid_go_back($pid);

  drupal_set_title($pattern->title . ' ' . t('(Pattern Modules)'));
  
  $modules = isset($pattern->pattern['modules']) ? $pattern->pattern['modules'] : array();
  
  $drupal_version = isset($pattern->pattern['info']['core']) ? $pattern->pattern['info']['core'] : 'core undefined';
  
}
*/


function modulus_page_form($form, $form_state, $modules, $core = '7.x') {
		
	// check if this is the first time or if we are rebuilding the form
  if (!isset($form_state['values']['step'])) {
    $form_state['values']['step'] = 1;
  }
  else {
    // check what button was clicked and which operation to execute
    // download button?
    
    // ENABLE
    $modules_to_enable = array();
    foreach ($form_state['values']['modules'] as $module => $info) {
      if($info['enable'] == 1) {
        $modules_to_enable[] = $module;
      }
    }

    // DOWNLOAD
    if ($form_state['values']['submit_dl'] == $form_state['triggering_element']['#value'] ||
        $form_state['values']['submit_dlen'] == $form_state['triggering_element']['#value']) {
          
      // create array of tuples: (module, drupal_version)
      $modules = array();
      foreach ($form_state['values']['modules'] as $module => $info) {
        if ($info['download'] == 1) {
          $modules[] = array('module' => $module, 'drupal_version' => $info['drupal_version']);
        }
      }
      
      if ($form_state['values']['submit_dlen'] != $form_state['triggering_element']['#value']) {
        $modules_to_enable = array(); // erase array if not needed
      }
      
      $form_state2; // hack
      $form = modulus_dlenable_modules_form(NULL, $form_state2, $modules, $modules_to_enable, $form_state['values']['enable_dependencies']);
          
      return $form;
    }
    
    // ENABLE
    else if ($form_state['values']['submit_en'] == $form_state['triggering_element']['#value']) {
      if(module_enable($modules_to_enable, $form_state['values']['enable_dependencies']))
        drupal_set_message('Modules have been enabled.');
      else
        drupal_set_message('An error occured. Please check the dependencies and if everything has been downloaded.');
    }
    
    // DL AND ENABLE
    else if($form_state['values']['submit_dlen'] == $form_state['triggering_element']['#value']) {
      drupal_set_message('Not yet implemented', 'error');
    }
  }
  
  $form['information'] = array(
    '#markup' => t('Notice: Authorization credentials may be requested. '));
  
  // needed for formatting
  $form['available'] = array(
      '#markup' => t(''),
  );
	
	$modules = is_array($modules) ? $modules : array($modules);
  
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
    
    /*
    if($available) {
      $form['modules'][$module]['available'] = array(
        '#markup' => t(' available'),
      );
    }
    else {
      $form['modules'][$module]['available'] = array(
        '#markup' => t(' not available'),
      );
    }
    */
    
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
    
    if(!$available) {
      $not_available = TRUE;
    }
    
  }
  
  $form['available'] = array(
  	'#markup' => ($not_available) ? t('Some modules are not available.') : t('All modules are available.'),
  );
    
  $form['enable_dependencies'] = array(
    '#type' => 'checkbox',
    '#title' => 'Enable dependencies?',
  );
  
  // buttons
  $form['submit_dl'] = array(
    '#type' => 'submit',
    '#value' => t('Download'),
    '#submit' => array('modulus_page_submit'),
  );
  
  $form['submit_en'] = array(
    '#type' => 'submit',
    '#value' => t('Enable'),
    '#submit' => array('modulus_page_submit'),
  );
  
  $form['submit_dlen'] = array(
    '#type' => 'submit',
    '#value' => t('Download & Enable'),
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