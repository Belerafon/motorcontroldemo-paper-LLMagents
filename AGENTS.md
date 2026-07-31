# Project instructions

Before building, flashing, launching OpenOCD/GDB, inspecting firmware state,
or issuing a control command, read and follow
`.opencode/skills/motorcontrol-demo/SKILL.md`.

The e2e scenarios control only the firmware's software motor model. Treat them
as ordinary tests: when the task calls for testing, run the relevant scenarios
while preserving their state checks and guaranteed `stop` cleanup.

For any newly added firmware function, write new e2e tests and run them on
the microcontroller as described in `.opencode/skills/motorcontrol-demo/SKILL.md`.
Do not consider the work complete until e2e tests pass and JTAG-based
verification has been performed.

Read the skill as UTF-8 with this PowerShell command:

```powershell
$OutputEncoding = [Console]::OutputEncoding = [System.Text.UTF8Encoding]::new()
Get-Content -Raw -Encoding UTF8 .opencode\skills\motorcontrol-demo\SKILL.md
```

Always specify `-Encoding UTF8` and set both `$OutputEncoding` and
`[Console]::OutputEncoding` before displaying the file. Do not use bare
`Get-Content`, because Windows PowerShell may decode the Russian text with the
wrong code page. Read referenced Markdown files the same way.
