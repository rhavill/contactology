<?php // $Id: //sms/modules/morris/contactology/6/v2012.1/contactology.admin.inc#4 $

/*
 * Admin settings form
 */
function contactology_settings() {
  $form = array();
  $form['contactology'] = array(
    '#type' => 'fieldset',
    '#description' => '<p>' . t('The settings here configure your connection
      to the Contactology REST API. For more information, read the
      <a href="http://www.contactology.com/email-marketing-api/index.php">API Documentation</a>.') .
      '</p>',
    '#title' => 'Contactology Settings',
  );
  $form['contactology']['contactology_api_key'] = array(
    '#type' => 'textfield',
    '#title' => t('API Key'),
    '#default_value' => variable_get('contactology_api_key', ''),
    '#description' => t('The <a href="http://www.contactology.com/email-marketing-api/authentication">API key</a> used to connect to the Contactology REST web service.'),
  );
  $form['contactology']['contactology_connect_timeout'] = array(
    '#type' => 'textfield',
    '#size' => 1,
    '#maxlength' => 2,
    '#title' => t('Connection Timeout'),
    '#default_value' => variable_get('contactology_connect_timeout', 2),
    '#description' => t('The maximum number of seconds to wait for a connection to be established with the Contactology server.'),
  );
  $form['contactology']['contactology_response_timeout'] = array(
    '#type' => 'textfield',
    '#size' => 1,
    '#maxlength' => 2,
    '#title' => t('Response Timeout'),
    '#default_value' => variable_get('contactology_response_timeout', 3),
    '#description' => t('The maximum number of seconds to wait for a response from the Contactology server.'),
  );
  $form['contactology']['contactology_enable_logging'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable error logging.'),
    '#default_value' => variable_get('contactology_enable_logging', 0),
    '#description' => t('Logs errors. Should be disabled on high-traffic sites.'),
  );
  return system_settings_form($form);
}

function contactology_default_subscriptions() {
  $form = array();
  $form['contactology'] = array(
    '#type' => 'fieldset',
    '#description' => '<p>' . t('When a user creates an account, they are given the option to click checkboxes next to email lists to
      subscribe to those lists. The lists selected here will automatically have their checkboxes selected when a user creates an account.') .
      '</p>',
    '#title' => 'Default Subscriptions',
  );
  try {
    $available_lists = contactology_get_lists();
  }
  catch (Exception $e) {
    drupal_set_message(t('Error getting email lists from Contactology. ') . check_plain($e->getMessage()), 'error');
    drupal_goto('admin/user/contactology');
  }
  foreach ($available_lists as $list_id => $list_name) {
    $form['contactology']['contactology_default_subscribe'.$list_id] = array('#type' => 'checkbox',
      '#title' => t('Subscribe new users to ') . check_plain($list_name) . '.',                               
      '#default_value' => variable_get('contactology_default_subscribe'.$list_id, 0),
      '#description' => t('Checking this causes users to be subscribed to this email list by default when they register. (They must uncheck the checkbox if they want to opt out.) '),
    );
  }
  return system_settings_form($form); 
}

function contactology_custom_field_settings() {
  $form = array();
  $form['contactology'] = array(
    '#type' => 'fieldset',
    '#description' => '<p>' . t('These settings allow custom fields to be updated in Contactology when a user creates or updates
      their profile in Drupal.') . '</p>',
    '#title' => 'Custom Fields',
  );
  $form['contactology']['contactology_enable_custom_fields'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable custom field udates.'),
    '#default_value' => variable_get('contactology_enable_custom_fields', 0),
    '#description' => t('Check this box to enable custom field updates.'),
  );
  $form['contactology']['contactology_unsubscribed_contacts'] = array(
    '#type' => 'checkbox',
    '#title' => t('Allow unsubscribed contacts.'),
    '#default_value' => variable_get('contactology_unsubscribed_contacts', 0),
    '#description' => t('By default, custom field values are only copied to Contactology if a user is subscribed to at least one email list. Check this box to change the default behavior.'),
  );
  try {
    $contactology_fields = contactology_get_custom_fields();
  }
  catch (Exception $e) {
    drupal_set_message(t('Error getting custom fields from Contactology. ') . check_plain($e->getMessage()), 'error');
    drupal_goto('admin/user/contactology');
  }
  $field_mappings = variable_get('contactology_field_mappings', array());
  $drupal_fields = contactology_get_drupal_fields();
  $form['header'] = array(
    '#type' => 'value',
    '#value' => array(
      array('data' => t('Contactology field')),
      array('data' => t('Drupal field')),
      array('data' => t('Conversion')),
    ),
  );
  $options = array_merge(array('' => t('<none>')), $drupal_fields);
  $form['contactology_mappings'] = array(
    '#type' => 'fieldset',
    '#description' => '<p>' . t('Map Contactology fields to Drupal user and profile fields.') . '</p>',
    '#title' => 'Custom Field Mappings',
    '#tree' => TRUE,
    '#prefix' => '<div style="display: none;">',
    '#suffix' => '</div>',
  );
  foreach ($contactology_fields as $contactology_field_id => $field_label) {
    $form['contactology_mappings'][$contactology_field_id]['drupal_field'] = array(
      '#type' => 'select',
      '#options' => $options,
      '#default_value' => isset($field_mappings[$contactology_field_id]['drupal_field']) ? $field_mappings[$contactology_field_id]['drupal_field'] : '',
    );
    $form['contactology_mappings'][$contactology_field_id]['conversion'] = array(
      '#type' => 'select',
      '#options' => array('' => t('<none>'), 'date_to_year' => t('Date to year'), 'm_f_to_male_female' => t('M/F to Male/Female')),
      '#default_value' => isset($field_mappings[$contactology_field_id]['conversion']) ? $field_mappings[$contactology_field_id]['conversion'] : '',
    );
    $form['contactology_mappings'][$contactology_field_id]['contactology_field'] = array(
      '#value' => check_plain($field_label),
    );    
  }
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit changes'),
  );
  
  return $form;   
}

/**
 * Theme function for custom fields settings page.
 */
function theme_contactology_custom_field_settings($form) {
  $output = drupal_render($form['contactology']);

  if (isset($form['contactology_mappings']) && is_array($form['contactology_mappings'])) {
    $rows = array();
    foreach (element_children($form['contactology_mappings']) as $id) {
      if (is_int($id)) {
        $row = array();
        $row[] = drupal_render($form['contactology_mappings'][$id]['contactology_field']);
        $row[] = drupal_render($form['contactology_mappings'][$id]['drupal_field']);
        $row[] = drupal_render($form['contactology_mappings'][$id]['conversion']);
        $rows[] = $row;
      }
    }
  }
  $header = $form['header']['#value'];
  if (!$rows) {
    $rows[] = array(array('data' => t('No data in the table.'), 'colspan' => count($header)));
  }
  $output .= '<p>' . t('Map Contactology fields to Drupal user and profile fields.') . '</p>';
  $output .= theme('table', $header, $rows);
  $output .= drupal_render($form['submit']);
  $output .= drupal_render($form);

  return $output;
}

/*
 * Process the custom fields admin form.
 */
function contactology_custom_field_settings_submit($form, &$form_state) {
  $field_mappings = array();
  $contactology_fields = contactology_get_custom_fields();
  foreach ($contactology_fields as $id => $field_label) {
    if (isset($form['contactology_mappings'][$id]['drupal_field']['#value']) && $form['contactology_mappings'][$id]['drupal_field']['#value']) {
      $field_mappings[$id]['drupal_field'] = $form['contactology_mappings'][$id]['drupal_field']['#value'];
    }
    if (isset($form['contactology_mappings'][$id]['conversion']['#value']) && $form['contactology_mappings'][$id]['conversion']['#value']) {
      $field_mappings[$id]['conversion'] = $form['contactology_mappings'][$id]['conversion']['#value'];
    }
  }
  variable_set('contactology_field_mappings', $field_mappings);
  variable_set('contactology_enable_custom_fields', $form['#post']['contactology_enable_custom_fields']);
  variable_set('contactology_unsubscribed_contacts', $form['#post']['contactology_unsubscribed_contacts']);
  drupal_set_message(t('The settings have been saved.'));
}