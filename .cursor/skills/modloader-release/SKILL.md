---
name: modloader-release
description: Draft changelog from PRs, confirm with you, bump version on master, tag and push.
disable-model-invocation: true
---

# Mod Loader release

End-to-end release workflow for [thelink2012/modloader](https://github.com/thelink2012/modloader). Follow phases in order. **Stop and wait for explicit user approval** at each gate marked **GATE**.

Do **not** read code diffs for changelog text. Use commit subjects, PR metadata, and PR comments only.

## Files touched on release

| File | Change |
|------|--------|
| `doc/CHANGELOG.md` | Prepend new version section |
| `include/modloader/modloader.h` | `MODLOADER_VERSION_*` and `MODLOADER_VERSION_ISDEV` |

Do **not** edit `doc/readme/Readme.md` or `doc/readme/Leia-me.md` — credits and readme text are human-written. Only touch them if the user explicitly asks outside this workflow.

Tag name matches version: `v0.3.9` ↔ `0.3.9`.

## Version rules

- Version lives in `include/modloader/modloader.h`:
  - `MODLOADER_VERSION_MAJOR`, `MODLOADER_VERSION_MINOR`, `MODLOADER_VERSION_REVISION`
- Release tag: `v{major}.{minor}.{revision}` (e.g. `v0.3.9`).
- Default next version: increment revision from the latest tag; set all three macros to match the chosen release.
- Patch releases use the third number (`0.3.8`). Do not invent `0.3.7.1`-style tags.
- Release `modloader.h` values:

```c
/* match release v0.3.9 */
#define MODLOADER_VERSION_MAJOR         0
#define MODLOADER_VERSION_MINOR         3
#define MODLOADER_VERSION_REVISION      9
#ifdef NDEBUG
#define MODLOADER_VERSION_ISDEV         0
#else
#define MODLOADER_VERSION_ISDEV         1
#endif
```

## Changelog format

- Section header: `v0.3.9 (May 16 2026)` then a line of dashes.
- Bullets: leading space + `* ` + one line per user-visible change.
- Merged PR: ` * … [#135 — contributed by @login]`
- GitHub `@login`, not display name.
- Do not credit `@thelink2012` (project maintainer); omit “— contributed by …” for maintainer-only changes.
- Do not paste PR bodies verbatim; summarize in one line. Prefer “Fixed …” / “Added …” over implementation detail.

## Phase 1 — Collect changes (read-only)

### Preflight (stop if any check fails)

Do not fix preflight problems yourself (no `git checkout`, `git pull`, `git merge`, `git reset`, etc.). Report what you found and **ask the user** what to do.

```bash
git fetch origin master --tags
```

1. **On `master`**
   ```bash
   git branch --show-current
   ```
   Must print `master`. If not, stop — ask the user to switch to `master` or merge release work into `master` before continuing. Do not release from another branch.

2. **In sync with `origin/master`**
   ```bash
   git rev-parse HEAD
   git rev-parse origin/master
   git status -sb
   ```
   - **Behind** `origin/master`: stop; ask the user to update local `master`, then re-run preflight.
   - **Ahead** of `origin/master`: stop; ask the user — unpushed commits may be intended for the release, but they must confirm before tagging.
   - **Diverged**: stop; ask the user to reconcile with `origin/master` before continuing.

3. Latest tag:
   ```bash
   git tag -l --sort=-version:refname | head -1
   ```
4. Commits since tag (skip merge commits):
   ```bash
   git log <TAG>..HEAD --no-merges --format="%h %s"
   ```
5. For each `(#NNN)` in a subject, fetch PR metadata and discussion:
   ```bash
   gh pr view NNN --repo thelink2012/modloader --json title,body,author,comments
   gh pr view NNN --repo thelink2012/modloader --comments
   ```
   `comments` in JSON are issue comments on the PR. `--comments` prints the full thread (including review replies). Use both when the title/body is thin but the thread explains user-facing impact.
   Contributor handle: `author.login` → `@login`.
6. Commits without a PR: use the subject; attribute only external contributors (not `@thelink2012`).
7. **Omit** from the changelog unless the user asks otherwise:
   - Pure merge commits
   - Reverted work (and the revert pair)
   - Any commit whose subject is a build/toolchain fix (e.g. “Fix build”, “VS2019”, premake/CI-only)
   - `chore:` that does not affect players

### Commit → bullet examples

| Commit subject | Draft bullet |
|----------------|--------------|
| `feat: create std.dmaudio for III/VC audios (#135)` | ` * Added audio support for GTA III and Vice City [#135 — contributed by @CookiePLMonster]` |
| `fix: improve ZMenu compatibility through CreateDirectory hook (#122)` | ` * Improved ZMenu compatibility [#122 — contributed by @TheComputerGuy96]` |
| `fix priority limit in menu` | ` * Fixed mod priority limit in the in-game menu` |
| `chore: update premake5 settings` | (omit) |
| `Fix build to VS2019` | (omit) |

## Phase 2 — Prepare files for review **GATE**

All release file edits happen here. The user reviews everything in this phase (e.g. `git diff`) before any commit.

Prepend this shape to `doc/CHANGELOG.md` (newest section on top; leave older sections below untouched):

```markdown
v0.3.9 (May 16 2026)
-----------------------
 * Added audio support for GTA III and Vice City [#135 — contributed by @CookiePLMonster]
 * Fixed mod priority limit in the in-game menu
```

1. **Write** the new changelog section. Use today’s date in `Mon DD YYYY` form.
2. **Update** `include/modloader/modloader.h` (version macros and `MODLOADER_VERSION_ISDEV` per **Version rules**).
3. Tell the user both files are ready for review. Do **not** paste file contents in chat unless they ask — they review via the editor or `git diff`.
4. State the planned version and tag (e.g. **0.3.9** / **v0.3.9**) so they can verify `modloader.h`.
5. **Do not proceed until they confirm** changelog and version files are final (they may edit on disk).

## Phase 3 — Confirm commit, tag, and push **GATE**

After Phase 2 approval, check the tag does not already exist (`git tag -l vX.Y.Z`, `git ls-remote --tags origin vX.Y.Z`). If it exists, stop and ask the user.

Restate the plan and ask once more before any git write:

> Release: **0.3.9**, tag **v0.3.9**, commit **"Bump to v0.3.9"**, push to **origin/master**?

**Do not commit, tag, or push until the user confirms.**

## Phase 4 — Commit, tag, push

Only after Phase 3 approval:

```bash
git add doc/CHANGELOG.md include/modloader/modloader.h
git commit -m "Bump to vX.Y.Z"
git tag vX.Y.Z
git push origin master
git push origin vX.Y.Z
```

## Phase 5 — Post-release

Remind the user to check the GitHub Actions release workflow for the new tag.

## Checklist

```
Release progress:
- [ ] Preflight: on master, synced with origin/master
- [ ] Tag baseline identified
- [ ] Commits/PRs/comments collected (no code diffs)
- [ ] Changelog + modloader.h written — Phase 2 GATE passed
- [ ] Commit/tag/push confirmed — Phase 3 GATE passed
- [ ] Commit created
- [ ] Tag created
- [ ] Pushed to origin (if approved)
- [ ] CI release workflow checked
```

## Safety

Never force-push, rewrite published history, delete branches/tags, or run other destructive git operations. Never change git config. Never skip hooks unless the user asks. Do not run `git checkout`, `git pull`, `git merge`, or `git reset` to fix preflight — always ask the user. When unsure, stop and ask.
