<?php

function modulus_is_modules_dir_writable($dir = 'sites/all/modules/') {
	if (fileowner(modulus_extract_directory()) === fileowner($dir)) {
		return TRUE;
	}
	if (file_prepare_directory($dir)) {
		return TRUE;
	}
	
	return FALSE;
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


function modulus_extract_directory($module = NULL) {
	$extract_directory = _update_manager_extract_directory();
	return (is_null($module)) ? $extract_directory : $extract_directory . '/' . $module; 
}

