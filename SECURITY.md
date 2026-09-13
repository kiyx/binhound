# Security policy

## Reporting a vulnerability

Please report security issues privately, not in public issues. Use the private vulnerability
reporting feature on this repository, or send an email to giuseppep.esposito@studenti.unina.it.

Include a description of the issue, the affected version or commit, and a minimal reproduction if
possible. You can expect an initial response within a few days.

## Scope

BinHound parses untrusted input by design: a crash, a hang or a memory error on a malformed file is
a security issue for this project and is treated as such. Vulnerabilities in third-party components
found while scanning other software belong to their respective maintainers; the tool can help
document them, but does not fix them.

## Supported versions

The project is in early development; only the latest commit on `main` is supported.
