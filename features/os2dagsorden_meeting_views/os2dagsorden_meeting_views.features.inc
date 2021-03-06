<?php
/**
 * @file
 * os2dagsorden_meeting_views.features.inc
 */

/**
 * Implements hook_ctools_plugin_api().
 */
function os2dagsorden_meeting_views_ctools_plugin_api() {
  list($module, $api) = func_get_args();
  if ($module == "context" && $api == "context") {
    return array("version" => "3");
  }
  list($module, $api) = func_get_args();
  if ($module == "strongarm" && $api == "strongarm") {
    return array("version" => "1");
  }
}

/**
 * Implements hook_views_api().
 */
function os2dagsorden_meeting_views_views_api() {
  return array("version" => "3.0");
}
