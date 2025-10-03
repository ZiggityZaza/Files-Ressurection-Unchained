# Files Resurrection Unchained 🚣
Incremental backups management made easy.

**Files Resurrection Unchained** stores data as a sequence of compressed snapshots, keeping track of what’s new, what’s gone, and what’s changed.


#
## 🔧 How it Works
Your backups live in a **Vault**, alongside a log of changes. Each new archive (`N.tar.lrz`) only contains the differences since the previous snapshot.

**Example:**
```
Backup target:
Somewhere/
├─ Important stuff/
... ├─ dir/
    |   └─ subfile
    └─ file

Vault/
├─ Important stuff/
... ├─ 0.tar.lrz (contains)
    |   └─ Important stuff/
    |       ├─ dir/
    |       |   └─ subfile
    |       └─ file (content: "A")
    ├─ 1.tar.lrz (contains)
    |   ├─ @changes.txt
    |   └─ Important stuff/
    |       ├─ file (content: "B")
    |       └─ newfile
    ...
    └─ N.tar.lrz (contains)
```

Each incremental archive contains:
* The files that were **added** or **modified**
* A `@changes.txt` file that lists exactly what happened

Example `@changes.txt`:
```
[ADDED]: "newFile"
[DELETED]: "dir/subfile"
[MODIFIED]: "file"
```


#
## 🚀 Features
* **Incremental backups**: Store only changes, not entire directories.
* **Human-readable logs**: Track what was added, removed, or changed.
* **Space-efficient**: Uses compressed tarballs (`.tar.lrz`).
* **Portable**: Archives are just tarballs — unpack anywhere.
* **Disaster-friendly**: Rebuild any snapshot in time without headaches.


#
## 📦 Dependencies
* `tar`
* `lrzip` (for `.lrz` compression)


#
## 🖥️ Usage (Not Implemented Yet)


#
## 🌱 Roadmap
* Optional compression formats (`xz`, `zstd`)
* Encrypted vaults
* Remote sync (push to S3/SSH)

#
## ⚖️ License
MIT (do whatever you want, just don’t blame me if your cat deletes your vault.)
