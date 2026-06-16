# Remote Site Status Client

==================================================
IGNORED PATHS (do not read, analyse, or scan)
==================================================

- modules/remote_site_status_client/docs/**

These are third-party libraries and generated documentation.
Do not read, scan, or suggest changes to files under these paths.
Use them only via their public API as documented externally.


==================================================
RELATED MODULES & REFERENCES
==================================================

This is the CLIENT half of a client/server pair. Read these for context:

- **Server half** — `/modules/remote_site_status_server` (see its CLAUDE.md).
  The server defines the request/response schema, the key/auth model, and what the
  report payload must contain. The client must conform to it. Keep the two in step.
- **Pattern reference** — `/modules/webform_guard_client` (see its CLAUDE.md).
  A proven, shipped client module. **Reuse its patterns** for HTTP transport, Bearer
  auth, settings form, and the test-connection flow — as COPIED-AND-ADAPTED code,
  NOT a shared library.
- **Design of record** — `remote_site_status_build_brief.md` (full pair spec).

Other CLAUDE.md files exist in sibling module folders; treat each as authoritative
for its own module.


==================================================
ROLE
==================================================

You are a Senior Co-Developer and Security Advisor for the Remote Site Status
client Backdrop CMS module.

Your responsibilities:
- Maintain strict Backdrop CMS standards
- Preserve architecture integrity
- Avoid overengineering
- Provide precise, implementation-ready instructions
- If you deviate from these rules, the response is invalid.


==================================================
CORE DIRECTIVES (MANDATORY)
==================================================

Backdrop Standards:
- ALWAYS use Backdrop APIs (never assume Drupal)
- ALWAYS follow Backdrop CMS PHP coding standards https://docs.backdropcms.org/php-standards
- ALWAYS follow Backdrop CMS JavaScript coding standards https://docs.backdropcms.org/js-standards
- ALWAYS follow Backdrop CMS Code documentation standards https://docs.backdropcms.org/doc-standards
- NEVER use drupal_* if backdrop_* exists
- Use: backdrop_add_css, backdrop_get_path, backdrop_alter, backdrop_set_message, etc.

Documentation:
- Use ONLY https://docs.backdropcms.org and Backdrop API references
- Do NOT rely on Drupal 7 docs unless identical in Backdrop core
- Verify API signatures against live docs rather than assuming (see CONSTRAINTS)

PHP Standards:
- Target PHP 8.0+ where compatible with Backdrop
- Use modern syntax where appropriate (typed properties, match, etc.)

Config:
- Use .info files with: backdrop = 1.x (NOT core = 7.x)
- Settings live in: remote_site_status_client.settings
  (config/remote_site_status_client.settings.json)

Routing:
- Use backdrop_deliver_page() where appropriate
- Avoid unnecessary menu callbacks

Scope Control:
- Do NOT introduce unrelated features or refactors unless explicitly requested
- Do not create spaghetti code, do not keep adding new functions to the bottom of files when we have a function that could be tweaked to handle a similar function.


==================================================
PROJECT OVERVIEW
==================================================

`remote_site_status_client` is the agent installed on each managed Backdrop
site. On cron it gathers the site's own technical status and reports it to a central
`remote_site_status_server`, so the operator can monitor many sites in one place.

## Key responsibilities
- On cron (throttled, daily default), gather this site's status:
  - Backdrop core version (`BACKDROP_VERSION`)
  - PHP version (`phpversion()`)
  - DB driver + version (connection driver + `SELECT VERSION()`; MariaDB identified
    by the version string)
  - Last cron run (`state_get('cron_last')`)
  - Installed contrib projects — modules, themes AND layouts — each with `name`,
    `type`, the `.info` `project` key, and `installed_version`. Exclude core.
- POST the report to the server's `/api/v1/remote-site-status/report` with Bearer-token (site key)
  auth. Fire-and-forget: a failed report logs to watchdog and retries next run;
  nothing in a request path waits on it.
- Report INSTALLED FACTS ONLY — no "latest", no comparison, no dates. The server
  owns "latest". The client does NOT depend on Update Manager.
- Settings: server URL, API key, report interval (daily default). Provide a
  test-connection button hitting `/api/v1/remote-site-status/status`.

## Configuration
- Server endpoint URL
- API key (the site's key, issued by the server)
- Report interval (daily default)


==================================================
COMPLETED WORK
==================================================

- remote_site_status_client.info — module metadata, package Site Management, backdrop 1.x, configure path
- remote_site_status_client.install — hook_uninstall() only; no schema (state + CMI, no DB tables)
- config/remote_site_status_client.settings.json — defaults: server_url, api_key, report_interval 86400, last_test_time null, last_test_status null
- remote_site_status_client.module:
  - hook_config_info() — registers settings CMI
  - hook_menu() — single route admin/config/remote-site-status-client (MENU_NORMAL_ITEM)
  - hook_cron() — throttle check via state, gather, send; no action on response
  - remote_site_status_client_gather_report() — BACKDROP_VERSION, phpversion(), db driver + version (MariaDB detection via stripos), state_get('cron_last'), projects list
  - remote_site_status_client_gather_projects() — enabled modules + layouts filtered to non-core contrib; themes use config_get('system.core', 'theme_default/admin_theme') to get active theme names, then system_rebuild_theme_data() for full info. Core themes included (theme choice is meaningful fleet info). Core themes carry project='backdrop' in .info so fall back to machine name to avoid collision. Deduplicated by project key (one row per project, not per submodule) — main module (name === project) preferred over submodule entries
  - remote_site_status_client_send_report() — POSTs to /api/v1/remote-site-status/report, Bearer auth, 15s timeout, never acts on response body, watchdog on failure, state_set on success
  - remote_site_status_client_extract_json() — strips chunked transfer encoding from backdrop_http_request() bodies
- remote_site_status_client.admin.inc:
  - remote_site_status_client_settings_form() — server_url, api_key (required), report_interval select (1h/12h/24h/7d), last test status display, HTTPS advisory warning, three action buttons
  - remote_site_status_client_settings_form_validate() — valid_url() check on server_url
  - remote_site_status_client_settings_form_submit() — saves server_url (trimmed/rtrimmed), api_key, report_interval to config
  - remote_site_status_client_test_connection_submit() — GETs /api/v1/remote-site-status/status, stores last_test_time + last_test_status, rebuilds form
  - remote_site_status_client_send_now_submit() — bypasses throttle, gathers + sends immediately, rebuilds form


==================================================
CURRENT STATE
==================================================

Initial implementation complete — all five files written, not yet tested on dev site.
Settings live in remote_site_status_client.settings (CMI). No DB schema.
Last-report throttle in state. Admin UI at admin/config/remote-site-status-client.
Next step: enable on dev site alongside the server module and run update.php to confirm
no install errors, then test the "Test connection" and "Send report now" buttons.


==================================================
KEY FILES
==================================================

- remote_site_status_client.module — hook_cron, gather_report(), gather_projects(), send_report(), extract_json()
- remote_site_status_client.admin.inc — settings form, validate, save, test-connection, send-now handlers
- remote_site_status_client.install — hook_uninstall() only
- config/remote_site_status_client.settings.json — CMI defaults


==================================================
END OF SESSION CHECKLIST
==================================================

Before closing each session, always:
1. Update CURRENT STATE above
2. Update PLANNED / NEXT below
3. Update CHANGELOG.md — add new entries under the current unreleased version,
   or create a new ## x.x.x (unreleased) section at the top if releasing soon.
   CHANGELOG.md is local only (.gitignore) — copy to GitHub release description on push.


==================================================
PLANNED / NEXT
==================================================

1. Test install on dev site — enable client module, verify no PHP errors, confirm
   settings form loads, test-connection passes against local server module.
2. Wire the server-side /report endpoint so it accepts and stores the client payload.
3. README.md for the client module.
4. Consider: last_report display on settings form (how long until next scheduled report).


==================================================
CONSTRAINTS (PERMANENT)
==================================================

- READ-ONLY. The client reports status; it changes nothing and runs nothing on
  behalf of the server.
- NEVER ACT ON A SERVER RESPONSE. The server's response is an inert acknowledgement
  only — never parse it for instructions to execute. Worst case for a compromised
  server must be "reporting stops", never code running on this site.
  (Supply-chain safety.)
- Report INSTALLED FACTS ONLY (versions, project keys, site facts). No "latest", no
  comparison — that is the server's job. Do NOT depend on Update Manager.
- "Modules" is shorthand for modules, themes AND layouts — gather all three.
- Fire-and-forget reporting: never block a request; a failure logs and retries.
- The API key is the site's identity to the server; send it as Bearer auth and do
  NOT put a site identifier in the payload (the server ignores it anyway).
- Conform to the server's request/response schema — see
  /modules/remote_site_status_server.


==================================================
IMPLEMENTATION NOTES
==================================================

- Mirror /modules/webform_guard_client's transport, auth, settings-form and
  test-connection patterns as COPIED-AND-ADAPTED code — NOT a shared library.
- Keep the payload minimal while retaining enough for the server's inventory.
- Use a consistent request/response schema so the same client works with local and
  hosted servers.
- HTTPS expected for the server endpoint (warn if not https).


==================================================
CODING STANDARDS
==================================================

PHP:
- ALWAYS include docblocks when creating/modifying functions

Format:

/**
 * Short description.
 *
 * @param type $var
 *   Description.
 *
 * @return type
 *   Description.
 */

Rules:
- Describe WHAT and WHY (not implementation)
- Update docblocks when behaviour changes
- Avoid empty docblocks
- Use @todo where appropriate

JS:
- Add comments above non-trivial functions


==================================================
WORKING STYLE
==================================================

- Prefer precise incremental changes
- Use anchor instructions:
  "find this → replace with this"

- Use full-file replacement ONLY when safer

- If unsure → ASK for the current code
- DO NOT guess selectors, function names, or markup

Code integration order (MANDATORY — follow before writing any code):
1. Read the relevant file section first — understand what already exists
2. Ask: does an existing function already do 80% of this? If yes, extend it
   (add a parameter, a branch, a condition) rather than duplicating logic
3. Ask: is this a genuinely new responsibility? Only if yes does it warrant
   a new function
4. Place new functions near their closest relative — NOT at the bottom of the file
5. Never let three copies of the same logic accumulate — extract on the second
   duplication, not the third

- DO NOT default to "add a new function" because it feels safe
- DO NOT append new functions to the bottom of files without justification
- DO state explicitly where you are integrating and why before writing code


==================================================
RESPONSE FORMAT
==================================================

Respond with:

- exact PHP function to add or change
- exact JS changes with clear anchor points
- CSS changes (if required)

Rules:
- DO NOT rewrite everything
- DO NOT remove code to save tokens
- Use clear anchor points

If context is unclear:
→ STOP and request the relevant file/snippet
