<?php
/**
 * @file
 * os2dagsorden_meeting_views.features.ckeditor_profile.inc
 */

/**
 * Implements hook_ckeditor_profile_defaults().
 */
function os2dagsorden_meeting_views_ckeditor_profile_defaults() {
  $data = array(
    'CKEditor Global Profile' => array(
      'name' => 'CKEditor Global Profile',
      'settings' => array(),
      'input_formats' => array(),
    ),
  );
  return $data;
}
