# wiig - Project Configuration

## Project facts

- **Language**: C17 (`c_std=c17`, strict)
- **Build**: Meson `>= 1.1.0`
- **Core dependency**: GLib `>= 2.68` (RHEL 9 / Ubuntu 22.04 baseline)
- **License**: GPL-3.0-or-later + Commercial (contact `inquiry@cleverplant.com`)
- **No wirelog dependency, ever** — neither runtime nor test-only

## Naming convention

| Scope                              | Function/Type Prefix | Macro/Enum Prefix |
|------------------------------------|----------------------|-------------------|
| **Public API** (installed headers) | `wiig_`              | `WIIG_`           |
| **Internal** (not installed)       | `wg_`                | `WG_`             |

Public headers install under `${includedir}/wiig/` and consumers `#include
<wiig/wiig.h>`. Internal headers under `wiig/{lexer,cst,parser,format,highlight,
io,util}/` are never installed.

The library is built with `-DWIIG_BUILDING` and `gnu_symbol_visibility=hidden`;
public symbols are annotated with `WIIG_PUBLIC` (defined in `wiig-export.h`).

## Code style

- 4-space indentation, no tabs, 80-column soft cap
- `.editorconfig` is the source of truth for whitespace
- LLVM/clang-format-18-derived style enforced via `uncrustify.cfg`
  (mirrored from wirelog — wiig formats the wirelog Datalog dialect, so its
  own C code follows the same convention)
- Per-file license header is a single SPDX line:
  `/* SPDX-License-Identifier: GPL-3.0-or-later */`

## Commit hygiene (project-local + global rule)

The global commit-message rule applies in full. In particular:

- **No `Co-Authored-By:` trailers.** No AI-attribution boilerplate of any kind.
- **No emojis** in commit messages or PR descriptions.
- **No persona references** — never name `Claude`, `Codex`, `Gemini`,
  `executor`, `implementer`, `reviewer`, `critic`, `architect`, or any other
  agent identity in commit text.
- **No internal pipeline labels** — `K4.1`, `Phase E1`, `series-K4`,
  `meta-review verdict`, etc. stay inside the planning workspace.
- Reference public issues with `Closes #N` only.

## Atomic commits

- One logical change per commit. The commit must build clean and
  `meson test` green on its own.
- Test changes live in the same commit as the implementation they cover.
- Before committing:
  1. `git diff` shows only logical changes (no formatting-only drift).
  2. `meson test -C builddir` is green.
  3. `uncrustify -c uncrustify.cfg --check <changed C files>` is clean.

## Test-driven development

Where it is feasible (lexer, CST, parser, formatter, highlighter), write
tests first. Each feature/module ships with unit-test coverage in the same
commit.

## Peer review

All implementation changes get reviewed before merge. Review covers:
correctness, memory safety, performance, ABI/API stability of installed
headers, and adherence to the per-commit scope statement.
