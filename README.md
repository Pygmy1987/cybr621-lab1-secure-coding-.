# Lab 1: Comparing AI Coding Assistants

This lab compares two AI-generated Linux C file-copy programs using the identical prompt saved in `prompt.md`. Both originals were preserved, and a separate improved version was created.

## Programs

- `assistant1.c`: Generated through the Codespace Chat interface, which displayed GPT-5.6 Luna.
- `assistant2.c`: Generated using Claude Code, which displayed Sonnet 5.
- `improved.c`: A revised copy of `assistant2.c`.
- `hello.c`: A sample program used to verify the development environment.

Claude named its program `safecopy`. Its original comments and messages were retained when saving it as `assistant2.c`.

## Environment

Compilation and manual testing were performed in a Linux GitHub Codespace using GCC. Claude could not compile its output in its own environment, so its program was compiled and tested in the Codespace.

## Compilation

```bash
gcc -Wall -Wextra -pedantic assistant1.c -o assistant1
gcc -Wall -Wextra -pedantic assistant2.c -o assistant2
gcc -Wall -Wextra -pedantic improved.c -o improved
```

## Usage

Each program takes a source path and a new destination path:

```bash
./assistant1 source.txt new-copy1.txt
./assistant2 source.txt new-copy2.txt
./improved source.txt new-copy3.txt
```

The destination must not already exist. A successful copy returns exit status 0; an error returns a nonzero status.

## Observed Test Results

| Check | Assistant 1 | Assistant 2 | Improved |
|---|---|---|---|
| Compilation without displayed warnings or errors | Passed | Passed | Passed |
| Usage message when arguments are missing | Passed | Passed | Passed |
| Copy a 33-byte text file | Passed | Passed | Passed |
| Copied contents match using cmp | Passed | Passed | Passed |
| Destination permissions are 0600 | Passed | Passed | Passed |
| Existing destination refused with exit status 1 | Passed | Passed | Passed |
| Missing source returns exit status 1 | Passed | Passed | Not tested |

## Improvements

The revised Claude implementation includes three changes:

1. Removed the retry loop around `close()`. On Linux, retrying after EINTR can close a descriptor number that has been reused.
2. Added a zero-byte-write check to `write_full()`, returning an error rather than looping without progress.
3. Added a zero-byte-write check to `write_msg()`, stopping diagnostic output if no progress is made.

The close-handling comment was corrected. The original function name, `close_retry`, was retained, although the improved function calls `close()` only once.

## Limits of Testing

The manual tests used a small text file. They did not inject interrupted calls, partial writes, zero-byte writes, or close failures. The improvements to these paths were checked through code review, not demonstrated by fault-injection tests.

All versions may leave an incomplete destination after a copy failure. They do not verify that the source is a regular file. Requested permissions of 0600 can be further restricted by the process umask.

## AI Assistance

AI assistants generated the original programs. Codex provided step-by-step help with testing, reviewing the code, making the improvements, and drafting documentation. The edits and terminal tests were performed manually in the Codespace.