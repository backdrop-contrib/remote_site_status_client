# Remote Site Status Client
**Remote Site Status Client** — Keep your Backdrop CMS site visible in your fleet dashboard by automatically reporting its technical status to a centralised monitoring server.

Remote Site Status Client is a Backdrop CMS module that runs silently in the background, gathering this site's technical health data on cron and posting it to a remote `remote_site_status_server` installation. The server collects reports from all registered sites and presents the fleet in a single admin dashboard, so the operator can spot outdated modules, security releases, or stale cron jobs across many sites without logging into each one individually.

## Beta Release Notes
As a beta release, the core cron reporting, project inventory gathering, and admin settings form are fully functional. Testers should ensure their Backdrop CMS environment is running PHP 8.0+, that the companion `remote_site_status_server` module is installed and reachable, and that system caches are cleared after installation to register the new hooks and menu items. The Remote Site Status Server can be run from the same Backdrop site during development.

## Features

* **Automatic Status Reporting:** On each cron run (throttled to a configurable interval, default daily), gathers this site's status and POSTs it to the monitoring server. Reporting is fire-and-forget — a failed report logs to watchdog and retries on the next cron run; nothing in a user request path waits on it.
* **Full Project Inventory:** Collects all installed contrib projects — modules, themes, and layouts — each identified by project key, type, and installed version. Core projects are excluded. Multi-module projects are deduplicated to a single row per project.
* **Site Facts:** Reports Backdrop core version, PHP version, database driver and version, and the last cron run timestamp alongside the project inventory.
* **Per-Site API Key Auth:** Each registered site has its own API key issued by the server. The key is sent as a Bearer token and acts as the site's identity — no additional site identifier is included in the payload.
* **Connection Test:** The settings page includes a *Test connection* button that calls the server's status endpoint and displays the authentication result and server version, making it easy to verify credentials before the first cron report.
* **Send Now:** A *Send report now* button on the settings page bypasses the cron throttle and fires an immediate report, useful after initial setup or to push an updated inventory without waiting for the next cron run.
* **HTTPS Advisory:** The settings form warns if the configured server URL is not HTTPS, prompting the operator to secure the connection before deploying to production.
* **Backdrop Native:** Built exclusively for Backdrop CMS using strict PHP 8.0+ standards and Backdrop APIs throughout.

## Requirements

- Backdrop CMS 1.x
- PHP 8.0+ (While the code may technically function with PHP 7.4 at this time, we strictly require PHP 8.0+ and will not address issues related to older PHP versions.)
- A running installation of the companion [Remote Site Status Server](https://github.com/backdrop-contrib/remote_site_status_server) module

## Installation

Install this module using the official Backdrop CMS instructions at https://docs.backdropcms.org/documentation/extend-with-modules

Enable the module on each site you wish to monitor.

## Configuration

1. Navigate to **Admin → Configuration → Remote Site Status Client → Settings**.
2. Enter the **Server base URL** — the base URL of your `remote_site_status_server` installation, without a trailing slash (e.g. `https://status.example.com`). Do not include the API path; the module appends this automatically.
3. Enter the **API key** provided by your server administrator when the site was registered.
4. Select the **Report interval** — how frequently cron should post a status report (1 hour, 12 hours, 24 hours, or 7 days). The default is 24 hours.
5. Click **Test connection** to verify the server URL and API key before saving.
6. Click **Send report now** to push an immediate inventory to the server without waiting for cron.

## Issues

Bugs and feature requests should be reported in the Issue Queue: https://github.com/backdrop-contrib/remote_site_status_server/issues

## Current Maintainer(s)
- Steve Moorhouse (albanycomputers) (https://github.com/albanycomputers)
- Additional maintainers and contributors are welcome.

## Credits
- Steve Moorhouse — Zulip (DrAlbany)
- Claude Code by Anthropic assisted with development of this module.

- Current development is sponsored by [Albany Computer Services](https://www.albany-computers.co.uk), providers of computer support, [web design](https://www.albanywebdesign.co.uk), and [web hosting](https://www.albany-hosting.co.uk).

## License
This project is GPL v2 or later software. See the LICENSE.txt file in this directory for complete text.
