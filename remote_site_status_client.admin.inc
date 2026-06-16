<?php

/**
 * @file
 * Admin settings form for Remote Site Status Client.
 */

/**
 * Settings form for the Remote Site Status Client.
 *
 * Provides fields to configure the server URL, API key and reporting interval.
 * Displays the result of the most recent connection test. Two additional
 * submit buttons — "Test connection" and "Send report now" — invoke dedicated
 * submit handlers without saving config.
 *
 * @param array $form
 *   The form array.
 * @param array $form_state
 *   The form state.
 *
 * @return array
 *   The form array.
 */
function remote_site_status_client_settings_form($form, &$form_state) {
  $config = config('remote_site_status_client.settings');

  $server_url = $config->get('server_url');
  $api_key = $config->get('api_key');

  // Non-blocking HTTPS advisory (not a validation error — operator may be on
  // a local dev environment where HTTP is intentional).
  if (!empty($server_url) && strpos($server_url, 'https://') !== 0) {
    backdrop_set_message(t('The server URL does not use HTTPS. Ensure this is intentional — API keys should not be transmitted over unencrypted connections in production.'), 'warning');
  }

  $form['#config'] = 'remote_site_status_client.settings';

  $form['connection'] = array(
    '#type' => 'fieldset',
    '#title' => t('Server connection'),
    '#collapsible' => FALSE,
  );

  $form['connection']['server_url'] = array(
    '#type' => 'textfield',
    '#title' => t('Server base URL'),
    '#description' => t('Base URL of the Remote Site Status server, e.g. <code>https://status.example.com</code>. Do not include a trailing slash or path.'),
    '#default_value' => $server_url,
    '#required' => TRUE,
    '#size' => 60,
  );

  $form['connection']['api_key'] = array(
    '#type' => 'textfield',
    '#title' => t('API key'),
    '#description' => t('The API key issued by the Remote Site Status server for this site.'),
    '#default_value' => $api_key,
    '#required' => TRUE,
    '#size' => 60,
  );

  // Reporting interval select list.
  $interval_options = array(
    3600 => t('Every hour'),
    43200 => t('Every 12 hours'),
    86400 => t('Daily (recommended)'),
    604800 => t('Weekly'),
  );

  $form['connection']['report_interval'] = array(
    '#type' => 'select',
    '#title' => t('Reporting interval'),
    '#description' => t('How often this site sends a status report to the server. Reports are sent on cron, so a high-frequency cron schedule is needed for short intervals.'),
    '#options' => $interval_options,
    '#default_value' => $config->get('report_interval') ?: 86400,
  );

  // Connection test result display.
  $last_test_time = $config->get('last_test_time');
  $last_test_status = $config->get('last_test_status');

  if ($last_test_time) {
    $ago = format_interval(REQUEST_TIME - (int) $last_test_time);
    if ($last_test_status === 'ok') {
      $test_markup = t('Last test: <strong>passed</strong> @ago ago.', array('@ago' => $ago));
      $test_class = 'ok';
    }
    else {
      $test_markup = t('Last test: <strong>failed</strong> @ago ago — @status.', array(
        '@ago' => $ago,
        '@status' => $last_test_status,
      ));
      $test_class = 'error';
    }
  }
  else {
    $test_markup = t('Connection has not been tested yet.');
    $test_class = 'warning';
  }

  $form['connection']['test_status'] = array(
    '#type' => 'item',
    '#title' => t('Connection status'),
    '#markup' => '<span class="rss-test-status rss-test-status--'
      . $test_class . '">' . $test_markup . '</span>',
  );

  $form['actions'] = array('#type' => 'actions');

  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
  );

  $form['actions']['test'] = array(
    '#type' => 'submit',
    '#value' => t('Test connection'),
    '#submit' => array('remote_site_status_client_test_connection_submit'),
    '#limit_validation_errors' => array(
      array('server_url'),
      array('api_key'),
    ),
  );

  $form['actions']['send_now'] = array(
    '#type' => 'submit',
    '#value' => t('Send report now'),
    '#submit' => array('remote_site_status_client_send_now_submit'),
    '#limit_validation_errors' => array(
      array('server_url'),
      array('api_key'),
    ),
  );

  return $form;
}

/**
 * Validates the settings form.
 *
 * @param array $form
 *   The form array.
 * @param array $form_state
 *   The form state.
 */
function remote_site_status_client_settings_form_validate($form, &$form_state) {
  $server_url = trim($form_state['values']['server_url']);
  if ($server_url && !valid_url($server_url, TRUE)) {
    form_set_error('server_url', t('The server URL must be a valid absolute URL.'));
  }
}

/**
 * Submit handler: save the settings form.
 *
 * @param array $form
 *   The form array.
 * @param array $form_state
 *   The form state.
 */
function remote_site_status_client_settings_form_submit($form, &$form_state) {
  $config = config('remote_site_status_client.settings');
  $config->set('server_url',
    rtrim(trim($form_state['values']['server_url']), '/'));
  $config->set('api_key', trim($form_state['values']['api_key']));
  $config->set('report_interval',
    (int) $form_state['values']['report_interval']);
  $config->save();
  backdrop_set_message(t('Settings saved.'));
}

/**
 * Submit handler: test the connection to the Remote Site Status server.
 *
 * GETs the /api/v1/remote-site-status/status endpoint with the configured
 * API key. Stores the result in config so the settings form can display it
 * without performing a live request on every page load. Rebuilds the form so
 * the updated status is shown immediately.
 *
 * @param array $form
 *   The form array.
 * @param array $form_state
 *   The form state.
 */
function remote_site_status_client_test_connection_submit($form, &$form_state) {
  $server_url = rtrim(trim($form_state['values']['server_url']), '/');
  $api_key = trim($form_state['values']['api_key']);

  if (empty($server_url) || empty($api_key)) {
    backdrop_set_message(t('Please enter a server URL and API key before testing.'), 'error');
    $form_state['rebuild'] = TRUE;
    return;
  }

  $endpoint = $server_url . '/api/v1/remote-site-status/status';

  $response = backdrop_http_request($endpoint, array(
    'method' => 'GET',
    'headers' => array(
      'Authorization' => 'Bearer ' . $api_key,
    ),
    'timeout' => 15,
  ));

  $code = isset($response->code) ? (int) $response->code : 0;
  $config = config('remote_site_status_client.settings');
  $config->set('last_test_time', REQUEST_TIME);

  if (!isset($response->error) && $code === 200) {
    $body = remote_site_status_client_extract_json($response->data);
    $data = json_decode($body, TRUE);
    $status = (is_array($data) && isset($data['status']))
      ? $data['status'] : 'ok';
    $config->set('last_test_status', $status);
    $config->save();
    backdrop_set_message(t('Connection test passed. Server responded with status: @status.',
      array('@status' => $status)));
  }
  else {
    $error = isset($response->error) ? $response->error : 'HTTP ' . $code;
    $config->set('last_test_status', $error);
    $config->save();
    backdrop_set_message(t('Connection test failed: @error', array('@error' => $error)), 'error');
  }

  $form_state['rebuild'] = TRUE;
}

/**
 * Submit handler: immediately send a status report outside of cron.
 *
 * Gathers and sends the current report payload on-demand, ignoring the
 * reporting interval throttle. Useful for verifying delivery after initial
 * configuration. Does not update the throttle timestamp so cron still fires
 * on schedule.
 *
 * @param array $form
 *   The form array.
 * @param array $form_state
 *   The form state.
 */
function remote_site_status_client_send_now_submit($form, &$form_state) {
  // Temporarily override the config with the submitted (unsaved) values so
  // send_report() picks up any changes the operator made before clicking.
  $server_url = rtrim(trim($form_state['values']['server_url']), '/');
  $api_key = trim($form_state['values']['api_key']);

  if (empty($server_url) || empty($api_key)) {
    backdrop_set_message(t('Please enter a server URL and API key before sending a report.'), 'error');
    $form_state['rebuild'] = TRUE;
    return;
  }

  // Save the URL and key values so send_report() can read them from config,
  // but do not save report_interval or test timestamps here.
  $config = config('remote_site_status_client.settings');
  $config->set('server_url', $server_url);
  $config->set('api_key', $api_key);
  $config->save();

  $report = remote_site_status_client_gather_report();
  $sent = remote_site_status_client_send_report($report);

  if ($sent) {
    backdrop_set_message(t('Status report sent successfully.'));
  }
  else {
    backdrop_set_message(t('Failed to send status report. Check the Recent log messages for details.'), 'error');
  }

  $form_state['rebuild'] = TRUE;
}
