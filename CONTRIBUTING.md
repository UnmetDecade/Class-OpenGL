# Contributing

<div align="left"><b>English</b> · <a href="CONTRIBUTING.zh-CN.md">简体中文</a></div>

Thanks for taking the time to contribute. This project is a teaching-oriented
OpenGL engine: small, explicit and readable code matters more than clever
abstractions.

## Before you start

- Open an issue for a new example or a non-trivial engine change, so the scope
  can be agreed before code is written.
- Keep one capability per example. Do not rewrite earlier examples to support a
  later one.
- Read [ARCHITECTURE.md](ARCHITECTURE.md); its rules are not negotiable.

## Prerequisites

- CMake ≥ 3.16 and a C++17 compiler (Apple clang, GCC, Clang, MSVC).
- Linux: `sudo apt-get install xorg-dev libgl1-mesa-dev`.
- macOS and Windows need no extra packages.
- GLFW and glm are fetched automatically by CMake; the first configure needs
  network access to `github.com`.

## Build and test

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DENGINE_WARNINGS_AS_ERRORS=ON
cmake --build build -j
ctest --test-dir build --output-on-failure
```

The examples need a display and a GPU. Unit tests under `tests/` are headless
and must stay that way: no window, no GL context, no asset files outside the
repository.

## Code conventions

- C++17, 4-space indentation, braces on their own line.
- Every GL resource is RAII: delete copies, default moves, release in the
  destructor.
- Every GL call goes through `GLCall`; do not call GL directly.
- Comments and log output are written in English only. This also applies to
  error strings and `LOG_*` messages.
- Public headers document the contract, not the implementation.
- Keep parsing and math code free of GL headers so it stays unit-testable.

## Tests

- Add a test for new pure logic (parsing, math, ordering) under `tests/`.
- Tests use the small runner in `tests/TestFramework.h`; no external framework.
- Do not lower the thresholds in `scripts/check_coverage.py` to make a change
  pass. Add tests instead.

## Commits and pull requests

- Use a short imperative subject, optionally with a conventional prefix:
  `feat:`, `fix:`, `docs:`, `test:`, `build:`, `ci:`.
- Keep commits focused. Do not mix formatting-only changes with behaviour.
- Before opening a PR, run the Debug build with `-DENGINE_WARNINGS_AS_ERRORS=ON`
  and the full test suite.
- Update `CHANGELOG.md` and `CHANGELOG.zh-CN.md` for user-visible changes.
- Update both language versions of user-facing documentation.
- Never commit credentials, tokens, local absolute paths or build outputs.

## Reporting problems

- Bugs and feature requests: use the issue forms.
- Security vulnerabilities: follow [SECURITY.md](SECURITY.md) and report
  privately. Do not open a public issue.
