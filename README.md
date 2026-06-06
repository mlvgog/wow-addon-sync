# wow-addon-sync

Syncs WoW addon folders (`WTF` and `Interface`) between a local machine and a Samba network share. Compares the newest file modification time recursively across each tree, so individual file edits are caught — not just top-level directory timestamps.

Ships with a raylib GUI showing per-folder progress bars, and a headless console build for scripted use.

---

## Prerequisites

- [MSYS2](https://www.msys2.org/) with the **UCRT64** toolchain
- raylib installed via pacman:

```
pacman -S mingw-w64-ucrt-x86_64-raylib
```

---

## Building

Open the **MSYS2 UCRT64** terminal and navigate to the project root.

```
make          # builds gui.exe (default)
make console  # builds console.exe (no GUI, no raylib dependency)
make clean    # removes compiled output
```

---

## Configuration

Copy `config.example.txt` to `config.txt` and fill in your paths:

```
local_wtf=C:\Program Files (x86)\World of Warcraft\_retail_\WTF
local_interface=C:\Program Files (x86)\World of Warcraft\_retail_\Interface
remote_wtf=\\192.168.x.x\share\WOW_ADDON_BACKUP\WTF
remote_interface=\\192.168.x.x\share\WOW_ADDON_BACKUP\Interface
```

`config.txt` is gitignored — keep your server IP out of version control.

---

## Usage

**GUI:**
```
./gui.exe
```
Click **Sync Now**. Progress bars track each folder independently. Click **Sync Again** to re-run without restarting.

**Console:**
```
./console.exe
```
Reads `config.txt`, prints progress to stdout, and reports total time on completion.

Any config value can be overridden at runtime:
```
./console.exe --local_wtf="D:\WTF" --remote_wtf="\\server\share\WTF"
```

---

## Behaviour

| Condition | Action |
|---|---|
| Remote folder missing | First run — copies local → remote |
| Local newer than remote | Copies local → remote |
| Remote newer than local | No action |
| In sync | No action |

Timestamps are compared using the newest `mtime` found anywhere in the directory tree, not the directory entry itself.
