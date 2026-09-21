# Lab 1 evidence index

These screenshots record the actual Codespace work and AI responses. Terminal excerpts in the report are crops of screenshots 13 and 22; their contents have not been rewritten. Full originals are in screenshots/.

| Screenshots | Evidence |
|---|---|
| 01 | Sample C program compiled and executed |
| 02-06 | Shared prompt, first assistant interface and generated response |
| 07-09 | Assistant 1 compilation, usage, copy comparison, permissions, overwrite refusal |
| 10 | Claude response and stated inability to compile in its generation environment |
| 11-13 | Assistant 2 compilation, usage, copy comparison, permissions, overwrite refusal |
| 14 | Missing-source tests for both original programs |
| 15-17 | Three edits to improved.c |
| 18-22 | Improved compilation, usage, copy, comparison, permissions and overwrite refusal |
| 23 | README documentation |
| 24 | Successful GitHub push of the source and README checkpoint |

environment.txt is added by running the GCC and Git version commands in the Codespace. No version values are inferred from screenshots.

The tests used a 33-byte text file. They did not inject interrupted system calls, partial writes, zero-byte writes, or close failures. The corresponding changes were based on source review.
