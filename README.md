# wiig

`wiig` is a Datalog (`.dl`) formatter and syntax highlighter for the wirelog
dialect. Think `uncrustify` or `gnu-indent`, but for Datalog.

The project ships:

- `libwiig` — a C17 library exposing the lexer, CST, parser, formatter, and
  highlighter surfaces.
- `wiig` — a CLI driver wrapping `libwiig` for shell use (`wiig fmt`,
  `wiig hl`).
- Editor integrations under `examples/` (vim, vscode, tmgrammar) as they land.

## Build

```sh
meson setup builddir
meson compile -C builddir
meson test -C builddir
```

The build requires:

- a C17 compiler (GCC, Clang, or MSVC ≥ VS 2019)
- Meson `>= 1.1.0`
- GLib `>= 2.68` development headers

## License

`wiig` is dual-licensed:

- **GPL-3.0-or-later** for open-source use.
- **Commercial license** for proprietary/closed-source use. Contact
  `inquiry@cleverplant.com`.

See `LICENSE.md` for full terms.
