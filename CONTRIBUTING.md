# Contributing

Thanks for considering a contribution. The project is built in public and welcomes bug reports,
small fixes, tests and documentation improvements.

## Before you start

- Open an issue for anything larger than a small fix, so the design can be discussed first.
- Follow the conventions in [AGENTS.md](AGENTS.md) and the [Code of Conduct](CODE_OF_CONDUCT.md).

## Development setup

Requirements: CMake 3.28 or newer, Ninja, and a C++20 compiler (GCC 13+ or Clang 18+).

```bash
git clone https://github.com/kiyx/binhound.git
cd binhound
cmake --workflow --preset debug
```

The same checks run in CI on Linux, Windows and macOS.

## Making a change

1. Create a branch named `feat/short-description`, `fix/short-description` or similar.
2. Keep the change focused: one logical change per pull request.
3. Add or update tests; the tree must build and pass tests at every commit.
4. Run the quality gates:

   ```bash
   cmake --workflow --preset debug
   clang-tidy -p build/debug --warnings-as-errors='*' <changed files>
   clang-format --dry-run --Werror <changed files>
   ```

5. Commit with a conventional message (see [AGENTS.md](AGENTS.md)) and open a pull request.

## What we look for

- Small, reviewable pull requests with a clear description of the change and its motivation.
- Tests for new behavior; a regression test that fails before the fix.
- No new dependencies without a documented reason.

## License

By contributing you agree that your contribution is licensed under the Apache License 2.0, the
same license as this project (inbound equals outbound).

## Security

Do not report security issues in public issues; see [SECURITY.md](SECURITY.md).
