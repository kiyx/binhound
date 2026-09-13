#!/bin/sh
set -e

root="$(git rev-parse --show-toplevel)"
cp "$root/scripts/pre-push" "$root/.git/hooks/pre-push"
chmod +x "$root/.git/hooks/pre-push"
echo "pre-push hook installed (bypass with: git push --no-verify)"
