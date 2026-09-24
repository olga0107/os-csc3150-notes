export const STORAGE_KEY = "os-annotations-v1";
export function validateBackup(data) {
  if (
    !data ||
    data.version !== 1 ||
    !Array.isArray(data.marks) ||
    data.marks.length > 5000
  )
    throw Error("备份格式不正确或版本不支持。");
  const ids = new Set();
  return data.marks.map((m) => {
    if (
      !m ||
      typeof m.id !== "string" ||
      !/^[\w-]{1,100}$/.test(m.id) ||
      ids.has(m.id) ||
      typeof m.path !== "string" ||
      !/^\/(notes|foundations)\/[a-z0-9-]+$/.test(m.path) ||
      !Number.isSafeInteger(m.updated) ||
      m.updated < 0 ||
      !Number.isSafeInteger(m.start) ||
      m.start < 0 ||
      !["quote", "prefix", "suffix", "section", "heading", "note"].every(
        (k) => typeof m[k] === "string",
      ) ||
      !m.quote.trim() ||
      m.quote.length > 3000 ||
      m.note.length > 10000 ||
      m.prefix.length > 60 ||
      m.suffix.length > 60 ||
      m.section.length > 300 ||
      m.heading.length > 500
    )
      throw Error("备份中有无效记录，未导入任何内容。");
    ids.add(m.id);
    return Object.fromEntries(
      [
        "id",
        "path",
        "updated",
        "start",
        "quote",
        "prefix",
        "suffix",
        "section",
        "heading",
        "note",
      ].map((k) => [k, m[k]]),
    );
  });
}
// A repeated quote is accepted only when surrounding text identifies one occurrence.
export function locate(mark, blocks) {
  const candidates = [];
  for (const [index, b] of blocks.entries()) {
    let start = b.text.indexOf(mark.quote);
    while (start !== -1) {
      const before = b.text.slice(
        Math.max(0, start - mark.prefix.length),
        start,
      );
      const after = b.text.slice(
        start + mark.quote.length,
        start + mark.quote.length + mark.suffix.length,
      );
      const context =
        Number(!!mark.prefix && before === mark.prefix) +
        Number(!!mark.suffix && after === mark.suffix);
      candidates.push({
        index,
        start,
        score: context * 10 + Number(b.section === mark.section) * 3,
      });
      start = b.text.indexOf(mark.quote, start + 1);
    }
  }
  candidates.sort((a, b) => b.score - a.score);
  if (
    !candidates.length ||
    (candidates.length > 1 && candidates[0].score === candidates[1].score)
  )
    return null;
  return candidates[0];
}
export function mergeMarks(current, incoming) {
  const map = new Map(current.map((m) => [m.id, m]));
  for (const m of incoming)
    if (!map.has(m.id) || map.get(m.id).updated < m.updated) map.set(m.id, m);
  return [...map.values()];
}
