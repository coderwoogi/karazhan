# mod-login-info-filter

Filters login-time system messages that mention AzerothCore before they are
sent to the client.

## What it blocks

- `This server runs on AzerothCore`
- `AzerothCore rev. ...`

The module inspects outgoing `SMSG_MESSAGECHAT`, `SMSG_GM_MESSAGECHAT`,
and `SMSG_NOTIFICATION` packets and drops only matching messages.

## Config

Copy `conf/mod_login_info_filter.conf.dist` to your active config directory as
`mod_login_info_filter.conf` and keep:

```ini
LoginInfoFilter.Enable = 1
```
