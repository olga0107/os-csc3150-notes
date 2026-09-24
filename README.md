# CSC3150 Operating Systems · Lecture Notes

Study and revision notes for **CSC3150 Operating Systems** (CUHK-Shenzhen, Fall 2026).

Read online: **https://olga0107.github.io/os-csc3150-notes/**

Style: English technical terms and code, with Chinese explanations that break difficult mechanisms into small steps. Each note follows the lecture's own logic. Sections open with the core question being solved, concepts are explained through execution traces and worked examples, and notes end with self-check questions for active recall. Supporting slide diagrams are extracted as images under `assets/<lecture>/`.

## Index

| # | Topic | Note |
|---|-------|------|
| 1 | What is an OS? Three roles (referee / illusionist / glue), evaluation criteria, OS in the AI age | [lec01-introduction](notes/lec01-introduction.md) |
| 2 | Four fundamental concepts: thread, address space, process, dual mode, base & bound protection | [lec02-concepts](notes/lec02-concepts.md) |
| 3 | Threads & processes: control transfers, interrupt vector, PCB & scheduler, pthread API, race condition | [lec03-thread-process](notes/lec03-thread-process.md) |
| 4 | Interleaving & race conditions, locks & critical sections, fork / exec / wait / exit, signals, the shell pattern | [lec04-process](notes/lec04-process.md) |
| 5 | File streams and descriptors, copy loops, buffering, open file descriptions and fork | [lec05-files](notes/lec05-files.md) |
| 6 | Sockets and pipes, TCP streams, connection setup, concurrent servers and EOF | [lec06-sockets-pipes](notes/lec06-sockets-pipes.md) |

## Course links

- Syllabus: https://yunmingxiao.github.io/courses/csc3150-26fall/syllabus.html
- Textbooks: *Operating System Concepts* · *OSTEP* · *The RISC-V Reader*

## Personal annotations

Select text within a paragraph or list item to highlight it or add a note. The chapter toolbar opens saved annotations, with links back to the original text and JSON backup import/export. Records stay in this browser's local storage; there is no account or server sync. Code, tables and formulas are excluded.

Annotation matching and backup validation checks: `node --test tests/annotations.test.mjs`.
