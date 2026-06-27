---
name: Problem
about: A problem with an existing feature's design or behavior (working as built, wrong by design)
title: ""
labels: ["problem"]
---

<!--
Before submitting, set these in the sidebar, since a template cannot:
  area:*    one or more of frontend, sema, codegen, link, driver, diag, infra
  target:*  darwin, windows, or aarch64, only when the work is target-specific
  milestone the track this belongs to
  flags     critical or blocked, when they apply
Put relationships in the body, like "Part of #N", "Depends on #M", or "Blocks #N".
Problem is exclusive with bug. Use this template when the behavior works as built but the
design is wrong, and use the Bug template for an outright defect.
-->

Part of #. Depends on #.

## What

<!-- The current design or behavior, why it is wrong, and how that was confirmed. -->

## Fix

<!-- The proper fix as a design change. List the candidate designs and the preferred one
     with its rationale. Do not propose a workaround. -->

## Acceptance

- [ ] <!-- what must be observably true, and the tests that must pass -->
