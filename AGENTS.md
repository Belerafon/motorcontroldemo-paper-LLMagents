# Project instructions

Before building, flashing, launching OpenOCD/GDB, inspecting firmware state,
or issuing a control command, read and follow
`.opencode/skills/motorcontrol-demo/SKILL.md`.

The e2e scenarios control only the firmware's software motor model. Treat them
as ordinary tests: when the task calls for testing, run the relevant scenarios
while preserving their state checks and guaranteed `stop` cleanup.

Read the skill as UTF-8 with this PowerShell command:

```powershell
$OutputEncoding = [Console]::OutputEncoding = [System.Text.UTF8Encoding]::new()
Get-Content -Raw -Encoding UTF8 .opencode\skills\motorcontrol-demo\SKILL.md
```

Always specify `-Encoding UTF8` and set both `$OutputEncoding` and
`[Console]::OutputEncoding` before displaying the file. Do not use bare
`Get-Content`, because Windows PowerShell may decode the Russian text with the
wrong code page. Read referenced Markdown files the same way.
