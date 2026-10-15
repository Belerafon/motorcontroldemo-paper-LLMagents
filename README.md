# LLM agents for motor-control software: experimental materials

[Русская версия](README.ru.md)

This repository contains code produced by LLM agents, run conditions, model-assisted trajectory analyses, and human evaluation forms from a study of agent-assisted development of [MotorControlDemo](https://gitflic.ru/project/belerafon/motorcontroldemo-vector), an embedded motor-control project.

The dataset covers **120 independent runs**: four models worked on five programming tasks under two experimental conditions, with three repetitions per model, task, and condition. Each run's final code is preserved in its own Git branch. The `master` branch is the dataset index; it is not a combined firmware release.

The original prompts, trajectory analyses, and evaluation forms are in Russian. This README explains how to navigate them in English; the original records and branch names have not been translated or renamed.

## Find a run

1. Find its `run_id` in [`run_metadata.csv`](run_metadata.csv). The row identifies the task, model, condition, and baseline commit.
2. Check out the branch with the same name, for example `LLM_test_promp1_r1`, to inspect the agent's final code.
3. Open its folder under [`Условия и анализ траекторий/`](<Условия и анализ траекторий/>). It contains the run conditions (`Условия опыта.txt`) and trajectory analysis (`Анализ_траектории.txt`).
4. If the run received human evaluation, open its folder under [`Чек-листы экспертов/`](<Чек-листы экспертов/>).

```text
README.md                         English guide
README.ru.md                      Original Russian guide
run_metadata.csv                  One row per run
Условия и анализ траекторий/
└── <run_id>/
    ├── Условия опыта.txt         Run conditions and exact prompt
    └── Анализ_траектории.txt     Analysis of the agent's actions
Чек-листы экспертов/
└── <run_id>/
    └── *.md                      One or more completed evaluation forms
```

After cloning a copy with all run branches, use `git branch -a` to list them. GitHub's branch selector can also open a run branch directly once the branches are published there.

## Run IDs and experimental design

An ID has the form `LLM_test_prompN_rM`. `N` is the task number (1–5), and `M` is the run number within that task (1–24). The spelling `promp` is part of the original branch names. Each of the eight series below contains three independent runs for every task. Use the metadata table for programmatic joins instead of inferring all attributes from a branch name.

| `M` | Series | Model | Condition | Repetitions |
|---:|:---:|---|---|:---:|
| 1–3 | C+ | GPT-5.6-Sol | `skill` | 1–3 |
| 4–6 | S+ | Step-3.7-flash | `skill` | 1–3 |
| 7–9 | G+ | GLM 5.2 | `skill` | 1–3 |
| 10–12 | D+ | DeepSeek-V4-Flash-0731 | `skill` | 1–3 |
| 13–15 | D− | DeepSeek-V4-Flash-0731 | `no_skill` | 1–3 |
| 16–18 | S− | Step-3.7-flash | `no_skill` | 1–3 |
| 19–21 | G− | GLM 5.2 | `no_skill` | 1–3 |
| 22–24 | C− | GPT-5.6-Sol | `no_skill` | 1–3 |

`skill` means the agent was given prepared project-specific tooling and instructions; `no_skill` means that support was removed. The `+` and `−` signs in series codes correspond to these conditions.

| Task | Programming assignment |
|:---:|---|
| 1 | Compensate cross-coupling in the current controllers |
| 2 | Add automatic field weakening |
| 3 | Dynamically limit current-controller outputs |
| 4 | Limit motor power |
| 5 | Implement symmetric space-vector PWM |

There are 24 branches per task. For example, `LLM_test_promp3_r1` through `LLM_test_promp3_r24` belong to task 3.

## Metadata fields

[`run_metadata.csv`](run_metadata.csv) has one row for each run. Its main columns are:

| Column | Meaning |
|---|---|
| `run_id` | Run identifier and Git branch name |
| `task`, `task_name` | Task number and original Russian task name |
| `repeat_within_series` | Repetition number within a series (1–3) |
| `series`, `series_order` | Series code and order in the experiment |
| `model_code`, `model` | Short model code and model name |
| `condition` | `skill` or `no_skill` |
| `declared_baseline_name`, `declared_baseline_commit` | Starting project version and exact commit |
| `declared_model`, `provider`, `harness` | Agent environment details, where recorded |
| `prompt_text` | Exact Russian prompt given to the agent |
| `run_folder`, `conditions_file` | Original paths in the experimental working copy |

The `run_folder` and `conditions_file` values record the original experiment layout. In this publication, locate materials by `run_id` in the two directories above. Empty metadata cells mean that a value was not recorded; they should not be inferred from other columns.

## Source project and baselines

The experiment derives from the open [MotorControlDemo project](https://gitflic.ru/project/belerafon/motorcontroldemo-vector). Its reference commit is `52a2c2a476ad70d09ed6c1988a6699b32ffecbfe`, the common ancestor of the three baselines and all run branches. The baselines form a linear Git history:

```text
MotorControlDemo 52a2c2a
└── Baseline1 2c00818
    └── Baseline2 0d71ac0
        └── Baseline3 1dd7bc1
```

| Baseline | Used for | Main changes from the source project |
|---|---|---|
| **Baseline1** `2c008185cc9e1bcb94b64e8b849e240bd25e9edd` | 15 C+ runs; branch `LLM_test` | Added `.codex` project instructions and skill, a `Makefile`, build/flash/JTAG/data-collection tools, automated tests, and supporting documentation. Selected `HW_MOTORCONTROLBOARD_SIMULATOR` and changed the first synchronous-motor model to `l_sd = l_sq = 5.0`, `j = 0.001`. Converted text sources from CP1251 to UTF-8. |
| **Baseline2** `0d71ac0189f47efc7d0b2a1503f6d77b48e136b3` | 45 S+, G+, and D+ runs; tag `baseline2` | Kept the firmware, simulator configuration, model parameters, and test setup from Baseline1. Moved the skill from `.codex` to `.opencode`, updated its paths, and required end-to-end tests for new firmware functions. |
| **Baseline3** `1dd7bc13160d35b3fc02916475d0aec06147133e` | 60 C−, S−, G−, and D− runs; tag `baseline3`, branch `LLM_test_NoSkill` | Removed the skill, `AGENTS.md`, `Makefile`, automated tests, debug scripts, experimental artifacts, and added documentation. Retained UTF-8 sources, the simulator configuration, and the same motor-model parameters so both conditions used the same target model. |

Although Baseline3 descends from Baseline2 in Git, it is not a superset of the `skill` setup: the agent tooling was deliberately removed. Much of the textual diff between the source project and a baseline is due to CP1251-to-UTF-8 conversion rather than motor-control changes.

Each `LLM_test_prompN_rM` branch contains the source state after one agent run. Compare it with the commit in that row's `declared_baseline_commit`, not with `master`:

```sh
# Changes made for the first skill-equipped baseline
git diff 52a2c2a476ad70d09ed6c1988a6699b32ffecbfe 2c008185cc9e1bcb94b64e8b849e240bd25e9edd

# Changes made by the agent in one example run
git diff 2c008185cc9e1bcb94b64e8b849e240bd25e9edd LLM_test_promp1_r1
```

## Run records and human evaluations

Every run has a folder in [`Условия и анализ траекторий/`](<Условия и анализ траекторий/>):

- `Условия опыта.txt` records the available baseline, model, agent environment, and exact task prompt. The format varies slightly because the files were captured during the experiment.
- `Анализ_траектории.txt` analyzes the agent's file reads, edits, tool calls, builds, tests, and revisions. Language models participated in preparing these analyses. They are **not complete agent logs**. References such as `[L…–L…]` point to lines in a source log that is not included in this dataset.

The [`Чек-листы экспертов/`](<Чек-листы экспертов/>) directory contains **123 completed forms for 74 runs**. Human reviewers completed these forms; some runs have forms from multiple reviewers. The forms include numeric scores and comments on code correctness, integration, preservation of existing behavior, completeness, tests, build results, regressions, function behavior, and experimental boundaries. If a run has no form, it was not evaluated by a human; a missing score must not be treated as zero. Blank form templates are not included.

## Anonymization and publication scope

The published copies remove reviewers' surnames and initials, local user-profile names, absolute filesystem paths, and internal session, project, and message identifiers. The labels `Эксперт 1`, `Эксперт 2`, and `Эксперт 3` are anonymized identifiers for the human reviewers.

Path placeholders include `<PROJECT_ROOT>`, `<PROJECT_PARENT>`, `<USER_PROFILE>`, `<USER_TEMP>`, `<VECTORIDE_ROOT>`, `<HOST_GCC_ROOT>`, `<GIT_ROOT>`, and `<PROGRAM_FILES>`. Identifiers such as `<SESSION_ID>`, `<SESSION_UUID>`, `<SESSION_SLUG>`, `<PROJECT_ID>`, `<MESSAGE_ID>`, `<REASONING_ID>`, and `<TURN_ID>` replace internal IDs. The replacements leave evaluation scores, analytical conclusions, source-log line numbers, project-relative paths, and Git commit IDs intact.

The dataset intentionally excludes complete raw agent logs, aggregate trajectory-analysis tables, backup archives, article drafts, statistical-analysis scripts, and temporary files. The run branches hold the final software states; the published records allow readers to identify the conditions and principal actions of each run and, where available, compare them with a human evaluation.

## Credit and license

The study was prepared by a research team affiliated with the [National Research University “Moscow Power Engineering Institute” (MPEI)](https://www.mpei.ru/lang/en/Pages/default.aspx). Human reviewers are identified only by the anonymized labels used in the forms. The attribution name for this dataset is **MotorControlDemo LLM Agents Study Team**; it does not designate MPEI as the copyright owner or determine the authorship of a future paper.

The team's original research materials on the `master` branch—this guide, [the Russian guide](README.ru.md), [the metadata table](run_metadata.csv), run-condition records, trajectory analyses, and expert evaluation forms—are licensed under [Creative Commons Attribution 4.0 International (CC BY 4.0)](LICENSE). When reusing them, credit the team, link to this repository and the license, and indicate changes. A suggested citation is:

> MotorControlDemo LLM Agents Study Team (2026). *LLM agents for motor-control software: experimental materials*. Repository dataset, CC BY 4.0. [Link to this repository].

This CC license does **not** apply to the firmware source, precompiled libraries, PDFs, or other third-party material in the run and baseline branches. Those branches retain their own license files and applicable third-party terms. The presence of an Apache 2.0 `LICENSE.txt` in a source branch does not place every bundled file under that license.
