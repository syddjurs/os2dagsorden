<?php
/**
 * os2dagsorden_settings
 *
 * PHP version 5
 *
 * @category OS2dagsorden
 * @package  OS2dagsorden_Settings
 * @file     Admin page file
 * @author   Stanislav Kutasevits <stan@bellcom.dk>
 * @license  http://www.gnu.org/licenses/gpl-2.0.html GNU General Public License, version 2
 * @link     http://bellcom.dk
 */

define('ACADRE_MM_IMPORT_DIR', 'private://');
define('OS2DAGSORDEN_HIDDEN_AGENGA_COMMITTEE', '28');

/**
 * Implementation of settings form specifically for OS2dagsorden system.
 *
 * @param $form       the settings form
 * @param $form_state the state of the form
 *
 * @return settings form.
 */
function os2dagsorden_settings_settings_form($form, $form_state) {
  // SOFD Integration settings
  $form['os2dagsorden_pws_sofd_group'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#title' => t('SOFD Integration settings'),
  );
  $form['os2dagsorden_pws_sofd_group']['os2dagsorden_pws_use_local_copy'] = array(
    '#type' => 'checkbox',
    '#default_value' => variable_get('os2dagsorden_pws_use_local_copy'),
    '#title' => t('Use local XML files'),
    '#description' => variable_get('os2dagsorden_pws_use_local_copy') ? 'Diselect and click save to enable the fields below' : 'Selecting and clicking save will disable the textfields below'
  );
  $form['os2dagsorden_pws_sofd_group']['os2dagsorden_pws_v_meetings_url'] = array(
    '#type' => 'textfield',
    '#default_value' => variable_get('os2dagsorden_pws_v_meetings_url'),
    '#title' => t('Meeting (planned meetings) view url:'),
    '#maxlength' => 256,
    '#disabled' => variable_get('os2dagsorden_pws_use_local_copy'),
  );
  $form['os2dagsorden_pws_sofd_group']['os2dagsorden_pws_v_meetinggroups_url'] = array(
    '#type' => 'textfield',
    '#default_value' => variable_get('os2dagsorden_pws_v_meetinggroups_url'),
    '#title' => t('Meeting groups (is member of/follows/follows+) view url:'),
    '#maxlength' => 256,
    '#disabled' => variable_get('os2dagsorden_pws_use_local_copy'),
  );
  $form['os2dagsorden_pws_sofd_group']['os2dagsorden_pws_v_acadregroups_url'] = array(
    '#type' => 'textfield',
    '#default_value' => variable_get('os2dagsorden_pws_v_acadregroups_url'),
    '#title' => t('Acadre groups (committees) url:'),
    '#maxlength' => 256,
    '#disabled' => variable_get('os2dagsorden_pws_use_local_copy'),
  );

  //cron frequencies
  $form['os2dagsorden_cron_job_freq'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#title' => t('Cron job frequencies settings'),
  );

  $options = array(
    '*/5 * * * *' => 'Every 5 min',
    '*/15 * * * *' => 'Every 15 min',
    '*/30 * * * *' => 'Every 30 min',
    '0 */1 * * *' => 'Every 1 hour',
    '0 */2 * * *' => 'Every 2 hours',
    '0 */3 * * *' => 'Every 3 hours',
    '0 */5 * * *' => 'Every 5 hours',
    '0 */10 * * *' => 'Every 10 hours',
  );
  $conf = ultimate_cron_get_settings("os2dagsorden_importer_cron");
  $form['os2dagsorden_cron_job_freq']['os2dagsorden_importer_cron'] = array(
    '#type' => 'select',
    '#options' => $options,
    '#default_value' => isset($conf['rules']) ? implode(';', $conf['rules']) : variable_get('ultimate_cron_rule', ULTIMATE_CRON_RULE),
    '#title' => t('Frequency of SOFD import cron job'),
  );

  $conf = ultimate_cron_get_settings("os2web_esdh_provider_cron");
  $form['os2dagsorden_cron_job_freq']['os2web_esdh_provider_cron'] = array(
    '#type' => 'select',
    '#options' => $options,
    '#default_value' => isset($conf['rules']) ? implode(';', $conf['rules']) : variable_get('ultimate_cron_rule', ULTIMATE_CRON_RULE),
    '#title' => t('Frequency of Acadre import cron job'),
  );

  $conf = ultimate_cron_get_settings("os2dagsorden_pdf2htmlex_cron");
  $form['os2dagsorden_cron_job_freq']['os2dagsorden_pdf2htmlex_cron'] = array(
    '#type' => 'select',
    '#options' => $options,
    '#default_value' => isset($conf['rules']) ? implode(';', $conf['rules']) : variable_get('ultimate_cron_rule', ULTIMATE_CRON_RULE),
    '#title' => t('Frequency of PDF to HTML conversion cron job'),
  );

  $conf = ultimate_cron_get_settings("os2dagsorden_adlib_api_cron");
  $form['os2dagsorden_cron_job_freq']['os2dagsorden_adlib_api_cron'] = array(
    '#type' => 'select',
    '#options' => $options,
    '#default_value' => isset($conf['rules']) ? implode(';', $conf['rules']) : variable_get('ultimate_cron_rule', ULTIMATE_CRON_RULE),
    '#title' => t('Frequency of Adlib (* to PDF) conversion cron job'),
  );
  $form['#submit'][] = 'os2dagsorden_settings_cron_job_submit';

  //expand or not expand bilag on bullet point expand
  //Meetig view settings
  $form['os2dagsorden_meeting_view_settings'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#title' => t('Meeting view settings'),
  );

  //meeting view settings
  $form['os2dagsorden_meeting_view_settings']['os2dagsorden_expand_all_bullets'] = array(
    '#type' => 'checkbox',
    '#title' => t('Expand all bullets on agenda page load'),
    '#description' => t('This decides whether all bullet points will be expanded on page load'),
    '#default_value' => variable_get('os2dagsorden_expand_all_bullets', FALSE),
  );
  $options = array(
    'true' => t('Expand'),
    'false' => t('Not expand (but keep it that way if the bilag has been expanded before)')
  );
  $form['os2dagsorden_meeting_view_settings']['os2dagsorden_expand_attachment'] = array(
    '#type' => 'radios',
    '#title' => t('Expand attachment'),
    '#default_value' => variable_get('os2dagsorden_expand_attachment'),
    '#options' => $options,
    '#description' => t('This decides whether a certain type of bullet point attachment - bilag will be expanded (activated) when functionality "Expand whole bullet point" is executed.'),
  );
  $form['os2dagsorden_meeting_view_settings']['os2dagsorden_show_meeting_enddate'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show meeting end date on meeting view'),
    '#description' => t('This decides whether a meeting end date is visible on meeting view.'),
    '#default_value' => variable_get('os2dagsorden_show_meeting_enddate', TRUE),
  );
  $form['os2dagsorden_meeting_view_settings']['os2dagsorden_show_meeting_committee'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show meeting committee on meeting view'),
    '#description' => t('This decides whether a meeting committee is visible on meeting view.'),
    '#default_value' => variable_get('os2dagsorden_show_meeting_committee', TRUE),
  );
  $form['os2dagsorden_meeting_view_settings']['os2dagsorden_show_meeting_type'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show meeting type on meeting view'),
    '#description' => t('This decides whether a meeting type field is visible on meeting view.'),
    '#default_value' => variable_get('os2dagsorden_show_meeting_type', TRUE),
  );
  $form['os2dagsorden_meeting_view_settings']['os2dagsorden_show_samlet_dokument_link'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show samlet dokument link on meeting view'),
    '#description' => t('This decides whether a samlet dokument link is visible on meeting view.'),
    '#default_value' => variable_get('os2dagsorden_show_samlet_dokument_link', TRUE),
  );
  $form['os2dagsorden_meeting_view_settings']['os2dagsorden_show_participants_afbud'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show cancelled participants'),
    '#description' => t('This decides whether a list of cancelled participants  is visible on meeting view'),
    '#default_value' => variable_get('os2dagsorden_show_participants_afbud', FALSE),
  );
  $form['os2dagsorden_meeting_view_settings']['os2dagsorden_show_print_icon'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show print icon on bullets in agenda'),
    '#description' => t('This decides whether a print button is visible on each bullet point on meeting view.'),
    '#default_value' => variable_get('os2dagsorden_show_print_icon', FALSE),
  );
  $form['os2dagsorden_meeting_view_settings']['os2dagsorden_show_bilag'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show bullet point attachment (bilag) on meeting view and bullet point attachment views'),
    '#description' => t('This decides whether a bullet point attachment converted from PDF to HTML is going to be shown in the application.<br/>Can be useful to disable display if users are known to used old browser versions (>IE8).'),
    '#default_value' => variable_get('os2dagsorden_show_bilag', TRUE),
  );
  $form['os2dagsorden_meeting_view_settings']['os2dagsorden_group_closed_bullet_points'] = array(
    '#type' => 'radios',
    '#title' => t('Group closed closed bullet points as a separate block'),
    '#default_value' => variable_get('os2dagsorden_group_closed_bullet_points', 'false'),
    '#options' => array('true' => t('Group separately'), 'false' => t('Put inline')),
    '#description' => t('This decides whether closed bullet points will be grouped separately as a additional block called "Lukket dagsorden", or put inline together the rest of the bullet points.'),
  );
  //show latest meetings in my committee or all committees
  $form['os2dagsorden_latest_meetings_view_settings'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#title' => t('Latest 8 meetings all/my committees view settings'),
  );
  $form['os2dagsorden_latest_meetings_view_settings']['os2dagsorden_latest_meetings_view_mode'] = array(
    '#type' => 'radios',
    '#title' => t('Switch beetween all/my committees meetings'),
    '#options' => array(
      'all' => t('Show meetings from all committees'),
      'mine' => t('Show meetings from my committees only')
    ),
    '#default_value' => variable_get('os2dagsorden_latest_meetings_view_mode', 'all'),
    '#description' => t('This decides whether the front page view will show 8 latest meeting from all committees or only from user committees (member of, follows, follows+)'),
  );
  // frontpage layout settings
  $form['os2dagsorden_frontpage_layout_settings'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#title' => t('Frontpage layout and settings'),
  );
  $options3 = array('false' => t('Expand'), 'true' => t('Collapse'));
  $form['os2dagsorden_frontpage_layout_settings']['os2dagsorden_collapse_menu'] = array(
    '#type' => 'radios',
    '#title' => t('Collapse/expand menu'),
    '#default_value' => variable_get('os2dagsorden_collapse_menu', TRUE),
    '#options' => $options3,
    '#description' => t('This decides whether a side menu expanded/collapsed.'),
  );
  $form['os2dagsorden_frontpage_layout_settings']['os2dagsorden_meetings_views_last_8_show_meeting_committee'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show meeting committee on frontpage'),
    '#description' => t('This decides whether a meeting committee is visible on frontpage.'),
    '#default_value' => variable_get('os2dagsorden_meetings_views_last_8_show_meeting_committee', TRUE),
  );
  $form['os2dagsorden_frontpage_layout_settings']['os2dagsorden_meetings_views_last_8_show_meeting_location'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show meeting location on frontpage'),
    '#description' => t('This decides whether a meeting location is visible on frontpage.'),
    '#default_value' => variable_get('os2dagsorden_meetings_views_last_8_show_meeting_location', TRUE),
  );
  //meeting search view settings
  $form['os2dagsorden_meeting_search_view_settings'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#title' => t('Meeting search view settings'),
  );
  $form['os2dagsorden_meeting_search_view_settings']['os2dagsorden_meetings_search_show_meeting_committee'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show meeting committee on meeting serach view'),
    '#description' => t('This decides whether a meeting committee is visible on meeting search view.'),
    '#default_value' => variable_get('os2dagsorden_meetings_search_show_meeting_committee', TRUE),
  );
  $form['os2dagsorden_meeting_search_view_settings']['os2dagsorden_meetings_search_show_meeting_status'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show meeting status on meeting serach view'),
    '#description' => t('This decides whether a meeting status is visible on meeting search view.'),
    '#default_value' => variable_get('os2dagsorden_meetings_search_show_meeting_status', FALSE),
  );
  $form['os2dagsorden_meeting_view_settings']['os2dagsorden_show_comments'] = array(
   '#type' => 'checkbox',
   '#title' => t('Show comments on meeting view'),
   '#description'=>t('This decides whether a comments field is visible on meeting view.'),
   '#default_value' => variable_get('os2dagsorden_show_comments', false),
  );
  //show or not show meeting category in the calendar
  $form['os2dagsorden_show_meeting_category_settings'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#title' => t('Show/not show meeting category in the calendar view'),
  );
  $options2 = array('true' => t('Show'), 'false' => t('Do not show'));
  $form['os2dagsorden_show_meeting_category_settings']['os2dagsorden_show_meeting_category'] = array(
    '#type' => 'radios',
    '#title' => t('Show meeting cetegories'),
    '#default_value' => variable_get('os2dagsorden_show_meeting_category'),
    '#options' => $options2,
    '#description' => t('This decides whether a meeting title in the calendar view will be prefixed with the meeting category (<b>[P]</b>olitical/<b>[O]</b>rganization)'),
  );

  //Meeting import settings
  $form['os2dagsorden_meeting_import'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#title' => t('General meeting import settings'),
  );
  $form['os2dagsorden_meeting_import']['os2dagsorden_participants_delimeter'] = array(
    '#type' => 'textfield',
    '#title' => t('Participants delimeter sign'),
    '#default_value' => variable_get('os2dagsorden_participants_delimeter', ', '),
    '#description' => t('Sign with which the participants of the meeting will be separated. Is used in the meeting view.<br/> E.g. if delimeter is <b>coma (,)</b> the list would look like:<br/>
    Jørgen Andersen, Nich Bendtsen, Poul Møller, ...<br/><br/>HTML tags (such as &lt;br/&gt;) can be used.'),
  );
  $form['os2dagsorden_meeting_import']['pdf2html_rendering_path'] = array(
    '#type' => 'textfield',
    '#title' => t('Path for HTML files to be rendered'),
    '#default_value' => variable_get('pdf2html_rendering_path', ''),
    '#description' => t('Relative path for HTML files to be rendered by os2dagsorden_pdf2htmlEX module. If empty, the path with the original PDF files will be used.<br/><b>Important!</b> You will need to create the path yourself.'),
  );
  $form['os2dagsorden_meeting_import']['os2dagsorden_send_notification_committee'] = array(
    '#type' => 'textfield',
    '#title' => 'Send notification of committees',
    '#maxlength' => 200,
    '#description' => 'Comma-separated list of committees ID changing status of which should send an email notification.',
    '#default_value' => variable_get('os2dagsorden_send_notification_committee', ''),
  );
  $form['os2dagsorden_meeting_import']['os2dagsorden_send_email_subject'] = array(
    '#type' => 'textfield',
    '#title' => 'Meeting notification email text',
    '#description' => 'The email subject, which would be send to users interested in meeting publishing - Member / Follow+ / Follow (optional)',
    '#default_value' => variable_get('os2dagsorden_send_email_subject', '!meeting_type til !committee er publiceret'),
  );
  $form['os2dagsorden_meeting_import']['os2dagsorden_send_email_body'] = array(
    '#type' => 'textarea',
    '#title' => 'Meeting notification email text',
    '#description' => 'The email body, which would be send to users interested in meeting publishing - Member / Follow+ / Follow (optional)',
    '#default_value' => variable_get('os2dagsorden_send_email_body', 'Til !user' . PHP_EOL . PHP_EOL .
      'Du abonnerer på !committee.' . PHP_EOL . 'Der er publiceret !meeting_type til !meeting_name !meeting_date.'),
  );
  $form['os2dagsorden_meeting_import']['os2dagsorden_send_email_text_available_variables'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#value' => 'Can be used both in subject and body:<br/>
    <b>!committe</b> - name of the committee<br/>
    <b>!meeting_name</b> - title of the meeting<br/>
    <b>!meeting_date</b> - date of the meeting<br/>
    <b>!meeting_type</b> - type of the meeting with acticle: <i>et referat</i> or <i>en dagsorden</i><br/>
    <b>!user</b> - name of the user that message is sent to<br/>',
    '#title' => t('Available variables'),
  );
  $form['os2dagsorden_meeting_import']['os2dagsorden_follows_committee_send_notification'] = array(
    '#type' => 'checkbox',
    '#title' => t('User that are just Follow the committee are getting notifications '),
    '#description' => t('Decide whether user that are just Follow the committee are getting notifications about new messages appeared on that committee.'),
    '#default_value' => variable_get('os2dagsorden_follows_committee_send_notification', FALSE),
  );
  $form['os2dagsorden_meeting_import']['os2dagsorden_send_timespan_of_notification'] = array(
    '#type' => 'textfield',
    '#title' => 'Meeting should be not older than:',
    '#maxlength' => 200,
    '#field_suffix' => 'Days',
    '#size' => 10,
    '#description' => 'If meeting is not older than the set amount of day, notification is sent. <br/>Otherwise notification process is skipped',
    '#default_value' => variable_get('os2dagsorden_send_timespan_of_notification', 10),
  );
  $form['os2dagsorden_meeting_import']['os2dagsorden_default_meeting_cateroty'] = array(
    '#type' => 'radios',
    '#title' => t('Default meeting category'),
    '#default_value' => variable_get('os2dagsorden_default_meeting_cateroty','pol'),
    '#options' =>  array('pol' => t('Political'), 'org' => t('Organisation')),
    '#description' => t('This decides whether a default meeting category'),
  );
  $form['os2dagsorden_meeting_import']['os2dagsorden_add_type_prefix'] = array(
    '#type' => 'radios',
    '#title' => t('Add prefix to Bilag/Sag bullet point attachments'),
    '#options' => array('true' => t('Add <i>Bilag/Sag</i> prefix'), 'false' => t('Add no prefix')),
    '#description' => t('This decides whether the will be added a prefix to Bilag or Case bullet point attachment on import.<br/>e.g. <b>Bilag:</b> Indsigelser of Bemaerkninger.'),
    '#default_value' => variable_get('os2dagsorden_add_type_prefix', 'true'),
  );
  if (module_exists('os2web_pdf_conversion_manager')) {
    $form['os2dagsorden_file_import'] = array(
      '#type' => 'fieldset',
      '#collapsible' => TRUE,
      '#title' => t('* > PDF converting configuration'),
    );
    $form['os2dagsorden_file_import']['os2web_copied_filetype'] = array(
      '#type' => 'textfield',
      '#title' => t('Type of files to be converted when importing:'),
      '#maxlength' => 200,
      '#description' => t('Comma-separated list of file types. Example: xls,doc,docx,xlsx'),
      '#default_value' => variable_get('os2web_copied_filetype', OS2WEB_COPIED_FILETYPE),
    );
    $form['os2dagsorden_file_import']['os2web_convertion_dir'] = array(
      '#type' => 'textfield',
      '#title' => t('Directory to send the files for converting:'),
      '#maxlength' => 200,
      '#description' => t('This is the path where files copy for converting'),
      '#default_value' => variable_get('os2web_convertion_dir', OS2WEB_CONVERTION_DIR),
    );
  }  
   $form['os2dagsorden_user_default_meeting_category']=array(
    '#title' => t('Default value for user meeting category'),
    '#type' => 'checkboxes',
    '#description' => t('Select default value for user meeting category.'),
    '#options' => array('pol' => t('Political'), 'org' => t('Organisation')),
    '#default_value'=>variable_get('os2dagsorden_user_default_meeting_category', array())
  );

  //hidden agenda
  $form['os2dagsorden_hidden_agenda_committee'] = array(
    '#type' => 'textfield',
    '#title' => 'List of committees which agenda is completely hidden to those without the rights',
    '#maxlength' => 200,
    '#description' => 'Comma-separated list of committees ID which agenda and the summary (samlet dokument) is hidden to those without the rights',
    '#default_value' => variable_get('os2dagsorden_hidden_agenda_committee', OS2DAGSORDEN_HIDDEN_AGENGA_COMMITTEE),
  );

    $form['os2dagsorden_pdf2htmlex_path'] = array(
        '#type' => 'textfield',
        '#title' => 'Path to pdf2htmlEX',
        '#description' => 'Path to pdf2htmlEX',
        '#default_value' => variable_get('os2dagsorden_pdf2htmlex_path', ""),
        '#element_validate' => array('_os2dagsorden_pdf2htmlEX_validate'),
    );

    return system_settings_form($form);
}

function _os2dagsorden_pdf2htmlEX_validate($element, &$form_state){
    $value = $element['#value'];
    $exec = shell_exec($value . ' pdf2htmlEX --version 2>&1');
    if(strpos($exec, 'pdf2htmlEX version') === false){
            form_error($element, "Incorrect pdf2htmlEX path");
            watchdog("os2dagsorden", t('Incorrect pdf2htmlEX path'));
        }
}

/**
 * Hook to system for submit. Stores the frequencies of the cronjob if they are different from the default provided by ultimate cron module
 *
 * @param $form       the settings form
 * @param $form_state the state of the form
 *
 * @return none.
 */
function os2dagsorden_settings_cron_job_submit($form, &$form_state) {
  $input = $form_state['input'];

  //os2dagsorden_importer_cron
  $conf = _ultimate_cron_default_settings();
  if (!empty($input['os2dagsorden_importer_cron']) && $input['os2dagsorden_importer_cron'] !== variable_get('ultimate_cron_rule', ULTIMATE_CRON_RULE)) {
    $conf['rules'][0] = $input['os2dagsorden_importer_cron'];
  }
  else {
    unset($conf['rules']);
  }
  $conf['service_group'] = "";
  ultimate_cron_set_settings('os2dagsorden_importer_cron', $conf);

  //os2web_esdh_provider_cron
  $conf = _ultimate_cron_default_settings();
  if (!empty($input['os2web_esdh_provider_cron']) && $input['os2web_esdh_provider_cron'] !== variable_get('ultimate_cron_rule', ULTIMATE_CRON_RULE)) {
    $conf['rules'][0] = $input['os2web_esdh_provider_cron'];
  }
  else {
    unset($conf['rules']);
  }
  $conf['service_group'] = "";
  ultimate_cron_set_settings('os2web_esdh_provider_cron', $conf);

  //os2dagsorden_pdf2htmlex_cron
  $conf = _ultimate_cron_default_settings();
  if (!empty($input['os2dagsorden_pdf2htmlex_cron']) && $input['os2dagsorden_pdf2htmlex_cron'] !== variable_get('ultimate_cron_rule', ULTIMATE_CRON_RULE)) {
    $conf['rules'][0] = $input['os2dagsorden_pdf2htmlex_cron'];
  }
  else {
    unset($conf['rules']);
  }
  $conf['service_group'] = "";
  ultimate_cron_set_settings('os2dagsorden_pdf2htmlex_cron', $conf);

  //os2dagsorden_adlib_api_cron
  $conf = _ultimate_cron_default_settings();
  if (!empty($input['os2dagsorden_adlib_api_cron']) && $input['os2dagsorden_adlib_api_cron'] !== variable_get('ultimate_cron_rule', ULTIMATE_CRON_RULE)) {
    $conf['rules'][0] = $input['os2dagsorden_adlib_api_cron'];
  }
  else {
    unset($conf['rules']);
  }
  $conf['service_group'] = "";
  ultimate_cron_set_settings('os2dagsorden_adlib_api_cron', $conf);
}