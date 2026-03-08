# Project planning

This document describes how to track and organize new work items within the
`ibus-unikey` repo. It lives alongside the [`development-guidelines` skill
file](SKILL.md) so that contributors and the AI assistant can always reference
it when planning features or large changes.

## Creating a new plan

Whenever you start on a new area of work (UI change, core logic update,
setup utility modification, etc.) follow these steps:

1. **Do not create a `project_planning` directory in the project root or
   anywhere under `$PROJECT_DIR`.** All planning notes belong inside the
   `.github/skills/development-guidelines` tree so the AI assistant can find
   them. Make sure the `project_planning` directory exists under
   `.github/skills/development-guidelines`; create it if it does not:
   ```sh
   mkdir -p .github/skills/development-guidelines/project_planning
   ```

2. Inside `project_planning` create a subdirectory that reflects the category of
the task, e.g. `UI`, `Setup`, `Core`, `Macro`, etc. You can use lowercase if you
prefer, but pick something descriptive:
   ```sh
   mkdir -p .github/skills/development-guidelines/project_planning/UI
   ```
2. Inside `project_planning` create a subdirectory that reflects the category of
the task, e.g. `UI`, `Setup`, `Core`, `Macro`, etc. You can use lowercase if you
prefer, but pick something descriptive:
   ```sh
   mkdir -p .github/skills/development-guidelines/project_planning/UI
   ```
3. Add a new markdown file to the appropriate category using the current date‑
time as a prefix. The filename format should be
   ```text
   YYYYMMDD_HHmmSS_name.md
   ```
   where `YYYYMMDD` is year/month/day and `HHmmSS` is hour/minute/second from
   the system clock. For example:
   ```sh
   touch .github/skills/development-guidelines/project_planning/UI/20260308_macro_right_click.md
   ```
   You can obtain the timestamp quickly via the `date` command:
   ```sh
   ts=$(date +"%Y%m%d_%H%M%S")
   touch .github/skills/development-guidelines/project_planning/UI/${ts}_myplan.md
   ```

## Purpose of the plan files

Each markdown file should describe the scope, design decisions, and any
important details of the task at hand. They serve as a lightweight record that
can be consulted during implementation, review, or later discussion.

> **Remember to add and commit the new directories/files to git and push them**
> to the repository along with your other changes so that collaborators (and the
> AI assistant) can access the planning notes.
Including this guideline in the workflows ensures that the AI assistant and
future contributors know where planning notes live and how they should be
organized.