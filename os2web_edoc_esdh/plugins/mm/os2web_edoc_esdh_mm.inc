<?php

/**
 * @file
 * This file describes the MM plugin as implemented by eDoc
 */

/**
 * Plugin definition with callbacks
 */
$plugin = array(
  'description' => t('eDoc ESDH MM Backend'),
  'version' => 1,
  'test' => 'os2web_edoc_esdh_mm_test',
  'get_import_list' => 'os2web_edoc_esdh_mm_get_import_list',
  'import_meeting' => 'os2web_edoc_esdh_mm_import_meeting',
  'post_import_process' => '_os2web_edoc_esdh_mm_order_addenums',
);

/**
 * For plugin testing purposes.
 */
function os2web_edoc_esdh_mm_test($stuff) {
  return 'Test successful. Recieved: ' . $stuff;
}

/**
 * Returns a list of meeting_ids for import. In our case a list of uri's of xml files.
 *
 * @return array
 *   List of URIs
 */
function os2web_edoc_esdh_mm_get_import_list() {
  /*
   * For the eDoc service, we need to import meetings from xml files.
   * We will enumerate the list with the publication-id
   * as parsed from the dir name of the manifest.
   */
  $uris = array();
  foreach (file_scan_directory(variable_get('os2web_edoc_esdh_mm_path', MM_IMPORT_DIR), '/\.xml$/i', array('recurse' => TRUE)) as $xml_file) {
    $uris[] = array('id' => null, 'uri' => $xml_file->uri);
  }
#error_log(print_r($xml_file, 1 ));
  return $uris;
}


/**
 * Strip illegal characters from the XML. 
 * Ex &#xC = ASCII new line feed
 * 
 * @param object $string
 *   string to sanitise
 * 
 * $return array
 *   The string sanitised 
 */
function utf8_for_xml($string)
{
    #$replace_me = "&#xC;";
    $replace_me =  variable_get('os2web_edoc_esdh_banned_special_char');
    $str_new = str_replace($replace_me, "", $string);
    return $str_new;
}

/**
 * Imports a meeting, and return the import as a structured array.
 *
 * @param object $meeting
 *   Meeting XML object.
 *
 * @return array
 *   List of meeting_structures to save
 */
function os2web_edoc_esdh_mm_import_meeting($meeting) {
  $file = $meeting['uri'];
  if ($file === FALSE) {
    os2web_meetings_order_addenums();
  }
  $meetings = array();
  if (is_file(drupal_realpath($file))) {
    
    libxml_use_internal_errors(true);
    $manifest = simplexml_load_string(utf8_for_xml(file_get_contents(drupal_realpath($file))), 'SimpleXMLElement', LIBXML_NOWARNING); 

    libxml_clear_errors();
    libxml_use_internal_errors(false);

    if ($manifest === false) {
       error_log("Failed loading XML");
       foreach(libxml_get_errors() as $error) {
          error_log ($error->message);
       }
    }
    if (is_object($manifest)) {

      // Setting the XML file to load - don't think we need this anymore.
      $directory_name = dirname(drupal_realpath($file));

      //Collecting agendas
      $meeting_agendas = $manifest->xpath("/Meeting/MeetingAgendaTypes/MeetingAgendaType");
      foreach ($meeting_agendas as $meeting_agenda){
	// Collecting agenda data
	$committee = (string) array_shift($manifest->xpath("/Meeting/Committee"));
	$ref_state = (string) array_shift($manifest->xpath("/Meeting/MeetingState"));
	$meeting_date = (string) array_shift($manifest->xpath("/Meeting/MeetingDate"));
	//$meeting_id = $meeting_agenda->xpath('MeetingAgendaItems/MeetingAgendaItem/Document/@documentid');
	//$meeting_id = (string) $meeting_id[0][documentid];
	$meeting_id = crc32($meeting_date . ' ' . $committee);//generated meeting_id
        if ($meeting_id > 2147483647){
          $meeting_id = substr($meeting_id, 0,9);//mysql int out of range fix
        }
	$meeting_pdf = $meeting_agenda->xpath('PDFDocument');
	$meeting_pdf = (string) $meeting_pdf[0];
	$meeting_state = $meeting_agenda->xpath('AgendaType');
	$meeting_state = (string) $meeting_state[0]; 
	$meeting_type = $manifest->xpath('/Meeting/MeetingState');
	$meeting_type = (string) $meeting_type[0];
	$acadre_version = (string) array_shift(explode('.', $meeting_pdf));
      
	if (stripos($meeting_state, 'lukket') !== FALSE)
	  $agenda_access = 'Lukket';
	else 
	  $agenda_access = 'Åben';
	  
	// Collecting agenda items
	$meeting_agendas_items = $meeting_agenda->xpath('MeetingAgendaItems'); //$manifest->xpath("/Meeting/MeetingAgendaTypes/MeetingAgendaType/MeetingAgendaItems");
	
	// Check if the committee is allowed to publish.
	$publish_committee = array_map('trim', explode(',', variable_get('os2web_edoc_esdh_approved_committees', MM_DEFAULT_APPROVED)));
	if ($committee == '' || !in_array($committee, $publish_committee)) {
	  watchdog('eDoc MM', 'Ignored agenda from "%committee" .', array('%committee' => $committee));
	  return FALSE;
	}

	// Fix dates.
	$date_obj = date_create($meeting_date);
	//date_timezone_set($date_obj, timezone_open('UTC'));
	$meeting_date = date_format($date_obj, 'Y-m-d H:i');
	
	// Changing the meeting state to match ESDH Provider module
	if($meeting_type == "Referat endeligt godkendt") {
	  $meeting_type = "Referat";
	}
	else {
	  $meeting_type = "Dagsorden";
	}
	
	// Saving the meeting data in ESDH Provider format. 
	$meeting = array(
	  'system_id' => $meeting_id,
	  'meeting_id' => $meeting_id,
	  'meeting_sub_id' => $meeting_id,
	  'committee' => $committee,
	  'committee_name' => $committee,
	  'title' => $committee,
	  'uri' => strstr($file , 'eDocAgendaPublishing.xml', TRUE),
	  'meeting_date' => $meeting_date,
	  'meeting_date_start' => $meeting_date,
          'meeting_date_finish' => $meeting_date,
	  'meeting_type' => $meeting_type,
	  'type' => $meeting_type,
	  'meeting_state' => $meeting_state,
	  'meeting_pdf' => $meeting_pdf,
	  'acadre_version' => $acadre_version,
	  'agenda_access' => $agenda_access,
	);
	// Error logging to watchdog if the agenda item list is empty
	if (empty($meeting_agendas_items)) {
	  watchdog('eDoc MM', 'Empty list of import items in %file.', array('%file' => $file), WATCHDOG_WARNING);
	}

	foreach ($meeting_agendas_items as $meeting_agenda_item) {
	  // Importing each agenda item through the ESDH provider module
	  $imported = _os2web_edoc_esdh_mm_import_meeting_agenda($meeting, $meeting_agenda_item);
	  if (is_array($imported)) {
	    $imported['publish'] = TRUE;
	    $meetings[] = $imported;
	  }
	}
      }
    }
    else {
      // Error logging to watchdog if the XML failde to parse. Check if all illegal characters are in the remove list. 
      watchdog('eDoc MM', 'Failed to parse XML in %file during import.', array('%file' => $file), WATCHDOG_WARNING);
    }
  }
  return $meetings;
}

/**
 * Imports a meeting from filesystem.
 *
 * @param array $meeting
 *   Meeting array.
 *
 * @param SimpleXMLElement $agenda_xml
 *   Meeting agenda XML object.
 *
 * @return mixed
 *   Meeting_struct array for an accepted import, else FALSE
 */
function _os2web_edoc_esdh_mm_import_meeting_agenda(&$meeting, $agenda_xml) {
  $meeting['items'] = array();
  $agenda = $meeting;
  // Only add the agenda if its PDF exists.
  if (file_exists(drupal_realpath($meeting['uri'] . $meeting['pdf']))) {
    $agenda['full_doc'] = array(
      'uri' => $meeting['uri'] . $meeting['meeting_pdf'],
      'title' => $meeting['committee'],
    );
    // Import all agenda items.
    $agenda_bullet = $agenda_xml;
    foreach ($agenda_bullet->MeetingAgendaItem as $agenda_item) {
      if (($item = _os2web_edoc_esdh_mm_import_meeting_agenda_item($meeting, $agenda_item)) !== FALSE) {
        $agenda['items'][] = $item;
      }
    }
    // Apply defaults, just in case we missed something.
    $agenda += os2web_esdh_provider_default_meeting();
    return $agenda;
  }
  watchdog('eDoc MM', 'File do not exist in folder. %file', array('%file' => $xml_item_pdf), WATCHDOG_WARNING);
  return FALSE;
}

/**
 * Imports a single item for an agenda.
 *
 * @param array $meeting
 *   Meeting array.
 *
 * @param SimpleXMLElement $xml
 *   Xml segment containing the item
 *
 * @return array
 *   The imported item.
 */
function _os2web_edoc_esdh_mm_import_meeting_agenda_item($meeting, $xml) {
  $xml_item['number'] = (int) $xml->AgendaItemNumber;
  $xml_item['pdf'] = (string) $xml->PDFDocument;
  $xml_item_document = $xml->Document;
  $xml_item_bullet = $xml_item_document->Attachments;
  $xml_item['name'] = (string) $xml_item_document->NeutralTitle;
  $xml_item['document_type'] = (string) $xml_item_document->PublishingType;
  $xml_item['agenda_type'] = (string) $meeting->AgendaType;
  $xml_item['documentid'] = (string) $xml_item_document['documentid'];
  $xml_item['full_text'] = trim((string) $xml->FullText);

  $item = array(
    'id' => $xml_item['number'],
    'sysid' => crc32($meeting['committee'] . ' ' . $meeting['meeting_date'] . ' ' . $xml_item['documentid']),//generated id - because there is not unique ID for the bullet point
    'title' => 'Punkt ' . $xml_item['number'] . ' ' . $xml_item['name'],
    'access' => $xml_item['document_type'],
    'bullets' => array(),
    'enclosures' => array(),
    'doc' => array(
      'uri' => implode('/', array(
        trim($meeting['uri'], "/"),
        $xml_item['pdf'])),
     'title' => $xml_item['name'],
    ),
  );
  
  if ($item['access'] !== 'SKAL PUBLICERES' || stripos($meeting['agenda_access'], 'lukket') !== FALSE) {
    $item['access'] = 'Lukket';
  }

  if (variable_get('os2web_edoc_esdh_bullet_source', 'xml') == 'xml'){
    //Process item body
    if (!empty($xml_item['full_text'])){
      $item['bullets'][] = array(
          'title' => 'Beskrivelse',
          'body' => $xml_item['full_text'],
          'sysid' => $xml_item['documentid'] . $xml_item['number'],
      );
    }

    // Process bullets.
    foreach ($xml_item_bullet->Attachment as $bullet_xml) {
      $bullet_title = (string) $bullet_xml->NeutralTitle;
      $bullet_body = (string) trim($bullet_xml->FullText);
      $bullet_id = (string) $bullet_xml['attachmentid'];
      $bullet_access = (string) $bullet_xml->PublishingType;
      if (empty($bullet_body)) {
        $bullet_body = "Intet indhold";
      }
      else {
        if($bullet_access !== 'SKAL PUBLICERES') {
          $bullet_body = "Dette er et lukket punkt. Du kan læse selve indstillingen" . $bullet_xml['access'];
        }
      }
      if (!empty($bullet_title)) {
        $item['bullets'][] = array(
          'title' => (string) $bullet_title,
          'body' => (string) $bullet_body,
          'sysid' => $bullet_id,
        );
      }
      
      // Process enclosure, if any
      $enclosure_item['filename_in'] = (string) $bullet_xml->PDFDocument;
      $enclosure_item['access'] = (string) $bullet_xml->PublishingType;
      if (strlen($enclosure_item['filename_in']) > 0 && $enclosure_item['access'] == 'SKAL PUBLICERES') {
        $item['enclosures'][] = array(
          'uri' => implode('/', array(
          trim($meeting['uri'], "/"),
          $xml_item['pdf'])),
          'title' => trim((string) $bullet_xml->Title),
          'sysid' => 'bilag_' . $bullet_id,
          'access' => TRUE,
          'filetype' => '.pdf',
        );
      }
    }
  } else {
    //skipping content and importing just one PDF with all attachment gathered there
    $item['enclosures'][] = array(
       'uri' => implode('/', array(
       trim($meeting['uri'], "/"),
       $xml_item['pdf'])),
       'title' => variable_get('os2web_edoc_esdh_empty_bullet_title','Indhold'),
       'sysid' => 'bilag_' . $xml_item['documentid'],
       'access' => TRUE,
       'filetype' => '.pdf',
     );
  }
  
  $item += os2web_esdh_provider_default_item();
  return $item;
}

/**
 * Case insensitive fileexists(). Code from comments on php.net.
 *
 * @param string $filename
 *   File name to check.
 *
 * @return mixed
 *   False if file not found, case corrected filename if found.
 */
function _os2web_edoc_esdh_similar_file_exists($filename) {
  if (file_exists($filename)) {
    return $filename;
  }
  $lcase_filename = strtolower($filename);
  foreach (glob(dirname($filename) . '/*') as $file) {
    if (strtolower($file) == $lcase_filename) {
      return $file;
    }
  }
  return FALSE;
}

/**
 * This function will set up the addenum connections for meetings.
 * @TODO: Rid this of drupal specific code.
 */
function _os2web_edoc_esdh_mm_order_addenums($drush = FALSE) {
  if (!lock_acquire(__FUNCTION__)) {
    return;
  }

  // Debug bit set in os2web_esdh_provider settings.
  $debug = variable_get('os2web_esdh_provider_debug_info');
  $debug && timer_start('_os2web_edoc_esdh_mm_order_addenums') && error_log('MM Import - Start of addendums - ' . timer_read('_os2web_edoc_esdh_mm_order_addenums') . 'ms - ' . memory_get_usage());

  $rev_meetings = array();

  // Find all meetings and gather meeting and agenda_ids.
  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'node', '=')
        ->propertyCondition('type', 'os2web_meetings_meeting', '=');
  $nodes_result = $query->execute();

  if (!isset($nodes_result['node'])) {
    return;
  }
  $nodes = $nodes_result['node'];
  #error_log(print_r($nodes, 1));
  // Load necessary fields into the node object.
  $fields = field_info_instances('node', 'meeting');
  field_attach_load('node', $nodes, FIELD_LOAD_CURRENT, array('field_id' => $fields['field_os2web_meetings_addendum']['field_id']));
  field_attach_load('node', $nodes, FIELD_LOAD_CURRENT, array('field_id' => $fields['field_os2web_meetings_id']['field_id']));
  field_attach_load('node', $nodes, FIELD_LOAD_CURRENT, array('field_id' => $fields['field_os2web_meetings_sub_id']['field_id']));

  // Meetings as: $meetings[<meeting_id>][<system_id>][<node_id>] .
  $meetings = array();

  $nids_with_addendums = array();

  $debug && error_log('MM Import - Retrieve nodes with addendums from ' . count($nodes) . ' nodes - ' . timer_read('_os2web_edoc_esdh_mm_order_addenums') . 'ms - ' . memory_get_usage());
  // Loop through all meeting nodes, and generate an array with meeting grouped.
  foreach ($nodes as $node) {
    $addenum_to = field_get_items('node', $node, 'field_os2web_meetings_addendum');
    if (isset($addenum_to[0]['nid']) && is_numeric($addenum_to[0]['nid'])) {
      $nids_with_addendums[$node->nid] = $node->nid;
    }
    $meeting_id = field_get_items('node', $node, 'field_os2web_meetings_id');
    $meeting_sub_id = field_get_items('node', $node, 'field_os2web_meetings_sub_id');
    if (!isset($meeting_id[0]['value']) || !isset($meeting_sub_id[0]['value'])) {
      $debug && error_log('MM Import - Missing meeting_id or meeting_sub_id for nid=' . $node->nid);
      continue;
    }
    if (isset($meetings[$meeting_id[0]['value']][$meeting_sub_id[0]['value']])) {
      // Duplicate meeting detected.
      watchdog('eDoc MM', 'Duplicate import found for meeting %mid. Deleting nid=%nid.', array('%mid' => $meeting_id['value'], '%nid' => $node->nid), WATCHDOG_WARNING);
      node_delete($node->nid);
    }
    else {
      $meetings[$meeting_id[0]['value']][$meeting_sub_id[0]['value']] = $node->nid;
    }
  }

  $debug && error_log('MM Import - Update addendums on ' . count($meetings) . ' nodes - ' . timer_read('_os2web_edoc_esdh_mm_order_addenums') . 'ms - ' . memory_get_usage());
  // Run through the meeting array, to determine the addendums.
  foreach ($meetings as $m_id => $meeting) {
    // Lowest system id first.
    ksort($meeting);
    if (in_array($m_id, $rev_meetings)) {
      $meeting = array_reverse($meeting);
    }
    // Lowest system id is primary.
    $primary = $nodes[array_shift($meeting)];
    $addenum_to = field_get_items('node', $primary, 'field_os2web_meetings_addendum');
    if (!empty($addenum_to[0]['nid'])) {
      $primary->field_os2web_meetings_addendum[LANGUAGE_NONE] = array();
      node_save($primary);
    }
    while ($next = array_shift($meeting)) {
      // Set up references for addenum agendas.
      $node = $nodes[$next];
      if ($primary) {
        $addenum_to = field_get_items('node', $node, 'field_os2web_meetings_addendum');
        $addenum_to = is_array($addenum_to) ? array_shift(array_shift($addenum_to)) : FALSE;
        // Save time by only updating what is needed.
        if ($addenum_to != $primary->nid) {
           #error_log(basename(__FILE__) . ':' . __LINE__ . ' Var: $primary = ' . print_r($primary->nid, 1));
           ##error_log(basename(__FILE__) . ':' . __LINE__ . ' Var: $addenum_to = ' . print_r($addenum_to, 1));
          $node->field_os2web_meetings_addendum[LANGUAGE_NONE][0]['nid'] = $primary->nid;
          node_save($node);
        }
      }
      else {
        #error_log('MARK - ' . basename(__FILE__) . ':' . __LINE__ . ' in ' . __FUNCTION__ . '()');
        #error_log(basename(__FILE__) . ':' . __LINE__ . " jm@bellcom.dk: wanted to delete $node->nid");
        // node_delete($node->nid);
      }
      // Remove node from final nids_with_addendums.
      unset($nids_with_addendums[$node->nid]);
      if ($drush === TRUE) {
        if ($primary) {
          drush_log("Updated meeting $m_id..", 'ok');
        }
        else {
          drush_log("Removed stale addenum $m_id..", 'ok');
        }
      }
    }
  }

  $debug && error_log('MM Import - Start reset addendums on ' . count($nids_with_addendums) . ' nodes - ' . timer_read('_os2web_edoc_esdh_mm_order_addenums') . 'ms - ' . memory_get_usage());
  // Remove any addendums from nodes which hasnt been proccessed.
  foreach ($nids_with_addendums as $nid) {
    // Load the node from the local variable.
    $node = $nodes[$nid];
    $node->field_os2web_meetings_addendum = array();
    node_save($node);
    if ($drush === TRUE) {
      drush_log("Resetting $nid..", 'ok');
    }
  }
  $debug && error_log('MM Import - End of addendums - ' . timer_read('_os2web_edoc_esdh_mm_order_addenums') . 'ms - ' . memory_get_usage());
  lock_release(__FUNCTION__);
}
