---
name: Bug
about: Observed behavior contradicts the spec, docs, or obvious intent
title: ""
labels: ["bug"]
---

<!--
Before submitting, set these in the sidebar, since a template cannot:
  area:*    one or more of frontend, sema, codegen, link, driver, diag, infra
  target:*  darwin, windows, or aarch64, only when the work is target-specific
  milestone the track this belongs to
  flags     critical (a miscompilation, data loss, or release blocker) or blocked, when they apply
Put relationships in the body, like "Part of #N" or "Depends on #M".
Use this template for a defect, where the behavior is wrong. Use the Problem template
when the behavior works as built but is wrong by design. The two are mutually exclusive.
-->

## What

<!-- A one or two line summary of the defect. -->

## Repro

<!-- The minimal steps or program that triggers it. -->

## Expected

<!-- What should happen, and why. Cite the spec, docs, or intent if it helps. -->

## Actual

<!-- What happens instead, with the exact diagnostic, exit, or wrong output. -->

## Environment

<!-- The triple or target, the toolchain version or commit, and the OS if it matters. -->
