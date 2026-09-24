import { test } from "node:test";
import assert from "node:assert/strict";
import {
  locate,
  validateBackup,
  mergeMarks,
} from "../.vitepress/theme/annotations/model.mjs";
const mark = {
  id: "test",
  path: "/notes/lec01-introduction",
  updated: 1,
  start: 2,
  quote: "target",
  prefix: "a ",
  suffix: " b",
  section: "intro",
  heading: "Intro",
  note: "hello",
};
test("relocates after text inserted before the original quote", () => {
  assert.equal(
    locate(mark, [{ text: "new a target b", section: "intro" }]).start,
    6,
  );
});
test("uses context to distinguish repeated text", () => {
  assert.equal(
    locate(mark, [{ text: "x target y a target b", section: "intro" }]).start,
    13,
  );
});
test("does not guess when matching repeated text is ambiguous", () => {
  assert.equal(
    locate({ ...mark, prefix: "", suffix: "" }, [
      { text: "target target", section: "intro" },
    ]),
    null,
  );
});
test("retains missing annotations without assigning a wrong location", () => {
  assert.equal(
    locate(mark, [{ text: "edited passage", section: "intro" }]),
    null,
  );
});
test("validates full backup before import, prevents invalid paths and duplicate IDs", () => {
  assert.equal(validateBackup({ version: 1, marks: [mark] }).length, 1);
  for (const data of [
    { version: 2, marks: [mark] },
    { version: 1, marks: [mark, mark] },
    {
      version: 1,
      marks: [mark, { ...mark, id: "two", path: "javascript:alert(1)" }],
    },
  ])
    assert.throws(() => validateBackup(data));
});
test("merges by identity and keeps newer notes", () => {
  assert.deepEqual(mergeMarks([mark], [{ ...mark, updated: 2, note: "new" }]), [
    { ...mark, updated: 2, note: "new" },
  ]);
  assert.deepEqual(mergeMarks([mark], [{ ...mark, updated: 0 }]), [mark]);
});
