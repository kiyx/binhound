## What and why

<!-- One short paragraph: what changes, and why. Link the issue if there is one. -->

## Checklist

- [ ] `cmake --workflow --preset debug` passes
- [ ] `clang-tidy -p build/debug --warnings-as-errors='*'` is clean on the changed files
- [ ] `clang-format --dry-run --Werror` is clean on the changed files
- [ ] Tests added or updated for the new behavior
- [ ] No new dependency without a documented reason
