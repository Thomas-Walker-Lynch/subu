
developer/shell/subu is what I am using to run a sub user. I haven't been maintaining any of the rest of it.



# `subu` — Sub-User Shell Launcher

## Overview

`subu` is a shell script used to launch an interactive login shell under a **sub-user** account.  
It provides an isolated session under a consistent naming scheme, while correctly configuring GUI forwarding and persistent services.

Sub-users are named using the convention:

```
Thomas-<subuser>
```

This naming scheme is required because Linux user names must be globally unique — the kernel has no built-in notion of subusers (yet!).

---

## Syntax

```bash
subu <subuser>
```

Where `<subuser>` is the symbolic sub-identity (e.g., `incommon`, `dev`, `tester`).

---

## Behavior

- Launches a shell as user `Thomas-<subuser>`
- Detects whether the current session is under **Wayland** or **X11**:
  - Under Wayland: uses `xhost +SI:localuser:Thomas-<subuser>`
  - Under X11: extracts the appropriate `.Xauthority` cookie to `$HOME/subu/<subuser>/.Xauthority`
- Sets `DISPLAY` and `XAUTHORITY` for graphical applications
- Enables lingering via `loginctl enable-linger` so sub-user services persist
- Runs the user’s shell (as listed in `/etc/passwd`) in **login mode** (`-l`)

The shell launched will source `.bash_profile`, which should in turn source `.bashrc`:

```bash
# Inside ~/.bash_profile
if [ -f "$HOME/.bashrc" ]; then
  . "$HOME/.bashrc"
fi
```

This ensures that aliases, prompts, and other interactive settings are applied.

---

## Subuser: `incommon`

The subuser named `incommon` is a shared environment used across all sub-users.
It is referenced in the PATH:

```
/home/Thomas/subu_data/incommon/executable
```

This allows tools and scripts to be shared without duplication.

---

## Example

```bash
subu incommon
```

This spawns a login shell as user `Thomas-incommon`, with GUI forwarding, shared executable access, and persistent services via systemd.

---

## Notes

- Each `<subuser>` must have a corresponding home directory:
  ```
  /home/Thomas/subu_data/<subuser>
  ```
- Ensure that the sub-user exists in `/etc/passwd` with a valid shell.
- Sub-users can run GUI applications (e.g., Emacs, Firefox) with correct X access.
- Sessions are sandboxed per sub-user, ideal for role-based separation, testing, or containment.

---
