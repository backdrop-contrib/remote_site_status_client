# Remote Site Status Client — Changelog

## 0.1.0 (unreleased)

Initial implementation.

- Module scaffold: .info, .install (hook_uninstall only, no schema), CMI settings JSON.
- hook_cron(): throttled status reporting with configurable interval (default daily).
- remote_site_status_client_gather_report(): gathers Backdrop version, PHP
  version, DB driver and version (with MariaDB detection), last cron timestamp,
  and installed project list.
- remote_site_status_client_gather_projects(): collects enabled modules, themes,
  and layout templates. Deduplicated by project key so multi-module projects (e.g.
  devel, feeds) send one entry per project rather than one per submodule; the main
  module (name === project) is preferred over a submodule entry when both are present.
- Theme gathering uses config_get('system.core', 'theme_default/admin_theme') rather
  than system_rebuild_theme_data() status check (which is unreliable in Backdrop 1.x
  as theme enabled state lives in config, not the system table). Core themes are
  included — knowing which theme a site runs is useful fleet information. Core themes
  carry project = 'backdrop' in their .info so fall back to machine name (e.g. 'basis',
  'seven') to avoid all core themes colliding under the 'backdrop' project key.
- remote_site_status_client_send_report(): POSTs to the server's /report
  endpoint with Bearer-token auth; fire-and-forget; logs failures to watchdog;
  never acts on response content (supply-chain safety).
- remote_site_status_client_extract_json(): chunked transfer encoding
  workaround for backdrop_http_request() (adapted from webform_guard_client).
- Admin settings form: server URL, API key, report interval select (1h/12h/24h/7d),
  connection-test result display with HTTPS advisory, "Test connection" button,
  "Send report now" button.
- remote_site_status_client_test_connection_submit(): GETs server status
  endpoint, stores last_test_time and last_test_status in config, rebuilds form.
- remote_site_status_client_send_now_submit(): manual on-demand report,
  bypasses interval throttle, does not reset throttle state.
