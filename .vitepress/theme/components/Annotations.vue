<script setup lang="ts">
import { computed, ref, onMounted, onBeforeUnmount, nextTick } from "vue";
import { onContentUpdated, useRoute } from "vitepress";
import {
  STORAGE_KEY,
  validateBackup,
  locate,
  mergeMarks,
} from "../annotations/model.mjs";
const route = useRoute();
type Mark = {
  id: string;
  path: string;
  updated: number;
  start: number;
  quote: string;
  prefix: string;
  suffix: string;
  section: string;
  heading: string;
  note: string;
};
type Block = {
  el: Element;
  text: string;
  nodes: Text[];
  section: string;
  heading: string;
};
const marks = ref<Mark[]>([]),
  path = ref(""),
  message = ref(""),
  storageError = ref(""),
  supported = ref(true);
const pending = ref<Mark | null>(null),
  editing = ref<Mark | null>(null),
  note = ref(""),
  floating = ref(false);
const floatStyle = ref({ left: "0px", top: "0px" }),
  drawer = ref<HTMLDialogElement>(),
  editor = ref<HTMLDialogElement>(),
  input = ref<HTMLTextAreaElement>();
const locations = ref<Record<string, number>>({}),
  deleted = ref<Mark | null>(null);
let blocks: Block[] = [],
  ranges = new Map<string, Range>(),
  timer: ReturnType<typeof setTimeout>,
  messageTimer: ReturnType<typeof setTimeout>,
  flashTimer: ReturnType<typeof setTimeout>;
let returnToDrawer = false,
  mounted = false;
const current = computed(() =>
  marks.value
    .filter((m) => m.path === path.value)
    .sort(
      (a, b) =>
        (locations.value[a.id] ?? Infinity) -
          (locations.value[b.id] ?? Infinity) || a.updated - b.updated,
    ),
);
function notify(s: string) {
  message.value = s;
  clearTimeout(messageTimer);
  messageTimer = setTimeout(() => (message.value = ""), 5000);
}
function read() {
  const raw = localStorage.getItem(STORAGE_KEY);
  return raw ? validateBackup(JSON.parse(raw)) : [];
}
function load() {
  try {
    marks.value = read();
    storageError.value = "";
  } catch {
    storageError.value =
      "无法读取本地记录。请先导出备份，再检查浏览器的存储设置。";
  }
}
function commit(change: (items: Mark[]) => Mark[]) {
  try {
    const next = change(read());
    validateBackup({ version: 1, marks: next });
    localStorage.setItem(
      STORAGE_KEY,
      JSON.stringify({ version: 1, marks: next }),
    );
    marks.value = next;
    storageError.value = "";
    paint();
    return true;
  } catch {
    storageError.value =
      "保存失败：浏览器存储不可用或已满。已有记录未被覆盖，请导出备份。";
    return false;
  }
}
const excluded =
  "pre,code,table,svg,math,figure,.study-diagram,.katex,.MathJax,button,a.header-anchor";
function scan() {
  blocks = [];
  const doc = document.querySelector(".vp-doc");
  if (!doc) return;
  let section = "",
    heading = "正文";
  for (const el of doc.querySelectorAll("h2,h3,p,li")) {
    if (el.matches("h2,h3")) {
      section = el.id;
      heading = (el.textContent || "").replace(/\u200b/g, "").trim();
      continue;
    }
    if (el.closest(excluded) || el.querySelector("p,li")) continue;
    const nodes: Text[] = [];
    const walker = document.createTreeWalker(el, NodeFilter.SHOW_TEXT, {
      acceptNode: (n) =>
        n.parentElement?.closest(excluded)
          ? NodeFilter.FILTER_REJECT
          : NodeFilter.FILTER_ACCEPT,
    });
    while (walker.nextNode()) nodes.push(walker.currentNode as Text);
    const text = nodes.map((n) => n.data).join("");
    if (text.trim()) blocks.push({ el, text, nodes, section, heading });
  }
}
function makeRange(b: Block, start: number, length: number) {
  const r = document.createRange();
  let offset = 0,
    started = false;
  for (const n of b.nodes) {
    const end = offset + n.length;
    if (!started && start < end) {
      r.setStart(n, start - offset);
      started = true;
    }
    if (started && start + length <= end) {
      r.setEnd(n, start + length - offset);
      return r;
    }
    offset = end;
  }
  return null;
}
function paint() {
  if (!mounted) return;
  scan();
  ranges.clear();
  const found: Record<string, number> = Object.create(null);
  for (const m of marks.value.filter((m) => m.path === path.value)) {
    const hit = locate(m, blocks);
    if (!hit) continue;
    const r = makeRange(blocks[hit.index], hit.start, m.quote.length);
    if (r) {
      ranges.set(m.id, r);
      found[m.id] = hit.index * 100000 + hit.start;
    }
  }
  locations.value = found;
  if (supported.value) {
    const api = (CSS as any).highlights;
    api.set(
      "personal-marks",
      new (window as any).Highlight(...ranges.values()),
    );
    api.set(
      "personal-notes",
      new (window as any).Highlight(
        ...marks.value
          .filter((m) => m.note && ranges.has(m.id))
          .map((m) => ranges.get(m.id)),
      ),
    );
  }
}
function clearSelection() {
  floating.value = false;
  pending.value = null;
  window.getSelection()?.removeAllRanges();
}
function capture() {
  if (editor.value?.open || drawer.value?.open) return;
  const sel = window.getSelection();
  if (!sel || sel.isCollapsed || !sel.rangeCount) {
    floating.value = false;
    return;
  }
  const r = sel.getRangeAt(0);
  if (!document.querySelector(".vp-doc")?.contains(r.commonAncestorContainer)) {
    floating.value = false;
    return;
  }
  scan();
  const b = blocks.find(
    (b) => b.el.contains(r.startContainer) && b.el.contains(r.endContainer),
  );
  if (
    !b ||
    !b.nodes.includes(r.startContainer as Text) ||
    !b.nodes.includes(r.endContainer as Text)
  ) {
    floating.value = false;
    return;
  }
  let start = 0,
    end = 0,
    offset = 0;
  for (const n of b.nodes) {
    if (n === r.startContainer) start = offset + r.startOffset;
    if (n === r.endContainer) end = offset + r.endOffset;
    offset += n.length;
  }
  const raw = b.text.slice(start, end),
    quote = raw.trim();
  start += raw.length - raw.trimStart().length;
  if (!quote || quote.length > 3000 || r.toString().trim() !== quote) {
    floating.value = false;
    return;
  }
  pending.value = {
    id: crypto.randomUUID(),
    path: path.value,
    updated: Date.now(),
    start,
    quote,
    prefix: b.text.slice(Math.max(0, start - 40), start),
    suffix: b.text.slice(start + quote.length, start + quote.length + 40),
    section: b.section,
    heading: b.heading,
    note: "",
  };
  const rect = r.getBoundingClientRect(),
    w = Math.min(260, innerWidth - 24);
  floatStyle.value = {
    left:
      Math.max(
        12,
        Math.min(innerWidth - w - 12, rect.left + rect.width / 2 - w / 2),
      ) + "px",
    top:
      Math.max(
        72,
        Math.min(
          innerHeight - 64,
          rect.top > 125 ? rect.top - 52 : rect.bottom + 8,
        ),
      ) + "px",
  };
  floating.value = true;
}
function schedule() {
  clearTimeout(timer);
  timer = setTimeout(capture, 150);
}
function existing(m: Mark) {
  return marks.value.find(
    (x) =>
      x.path === m.path &&
      x.quote === m.quote &&
      x.section === m.section &&
      x.prefix === m.prefix &&
      x.suffix === m.suffix,
  );
}
function highlight() {
  const m = pending.value;
  if (!m) return;
  if (existing(m)) {
    clearSelection();
    notify("这段文字已经标记过了");
    return;
  }
  if (commit((items) => [...items, m])) {
    clearSelection();
    notify("高亮已保存");
  }
}
async function edit(m: Mark, fromDrawer = false) {
  returnToDrawer = fromDrawer;
  drawer.value?.close();
  editing.value = { ...m };
  note.value = m.note;
  clearSelection();
  await nextTick();
  editor.value?.showModal();
  input.value?.focus();
}
function annotate() {
  if (pending.value) edit(existing(pending.value) || pending.value);
}
function save() {
  if (!editing.value) return;
  const m = { ...editing.value, note: note.value.trim(), updated: Date.now() };
  if (commit((items) => [...items.filter((x) => x.id !== m.id), m])) {
    editor.value?.close();
    notify("标记已保存");
  }
}
function remove(m: Mark) {
  if (commit((items) => items.filter((x) => x.id !== m.id))) {
    deleted.value = m;
    editor.value?.close();
    notify("标记已删除，可撤销");
  }
}
function undo() {
  if (deleted.value) {
    const m = deleted.value;
    if (commit((items) => mergeMarks(items, [m]))) {
      deleted.value = null;
      notify("已恢复标记");
    }
  }
}
function openDrawer() {
  clearSelection();
  paint();
  drawer.value?.showModal();
}
function editorClosed() {
  editing.value = null;
  if (returnToDrawer) {
    returnToDrawer = false;
    drawer.value?.showModal();
  }
}
function jump(m: Mark) {
  const r = ranges.get(m.id);
  if (!r) return;
  drawer.value?.close();
  const top = r.getBoundingClientRect().top + scrollY - 220;
  window.scrollTo({
    top,
    behavior: matchMedia("(prefers-reduced-motion: reduce)").matches
      ? "instant"
      : "smooth",
  });
  if (supported.value) {
    (CSS as any).highlights.set(
      "personal-active",
      new (window as any).Highlight(r),
    );
    clearTimeout(flashTimer);
    flashTimer = setTimeout(
      () => (CSS as any).highlights.delete("personal-active"),
      2200,
    );
  }
}
function clickText(e: MouseEvent) {
  if (
    !window.getSelection()?.isCollapsed ||
    (e.target as Element).closest("a,button,dialog,.annotation-float")
  )
    return;
  for (const [id, r] of ranges)
    for (const rect of r.getClientRects())
      if (
        e.clientX >= rect.left &&
        e.clientX <= rect.right &&
        e.clientY >= rect.top &&
        e.clientY <= rect.bottom
      ) {
        const m = marks.value.find((m) => m.id === id);
        if (m) edit(m);
        return;
      }
}
function exportAll() {
  const blob = new Blob(
    [JSON.stringify({ version: 1, marks: marks.value }, null, 2)],
    { type: "application/json" },
  );
  const url = URL.createObjectURL(blob);
  const a = document.createElement("a");
  a.href = url;
  a.download = "notes-marks-" + new Date().toISOString().slice(0, 10) + ".json";
  a.click();
  setTimeout(() => URL.revokeObjectURL(url), 1000);
  notify("已导出全部章节的标记");
}
async function importAll(e: Event) {
  const el = e.target as HTMLInputElement,
    file = el.files?.[0];
  if (!file) return;
  try {
    if (file.size > 5 * 1024 * 1024) throw Error("文件不能超过 5 MB。");
    const incoming = validateBackup(JSON.parse(await file.text()));
    if (commit((items) => mergeMarks(items, incoming)))
      notify(`已合并 ${incoming.length} 条标记，重复记录不会增加。`);
  } catch (err) {
    notify(
      err instanceof SyntaxError
        ? "文件不是有效的 JSON 备份。"
        : (err as Error).message,
    );
  } finally {
    el.value = "";
  }
}
function storage(e: StorageEvent) {
  if (e.key === STORAGE_KEY || e.key === null) {
    load();
    paint();
  }
}
function key(e: KeyboardEvent) {
  if (e.key === "Escape") floating.value = false;
}
function scrolled() {
  floating.value = false;
}
async function refresh() {
  if (!mounted) return;
  drawer.value?.close();
  returnToDrawer = false;
  editor.value?.close();
  clearSelection();
  path.value = route.path
    .replace(/^.*?(?=\/(notes|foundations)\/)/, "")
    .replace(/\.html$/, "")
    .replace(/\/$/, "");
  await nextTick();
  paint();
}
onMounted(() => {
  mounted = true;
  supported.value = !!(window as any).Highlight && !!(CSS as any).highlights;
  load();
  refresh();
  document.addEventListener("selectionchange", schedule);
  document.addEventListener("click", clickText);
  document.addEventListener("keydown", key);
  window.addEventListener("scroll", scrolled, { passive: true });
  window.addEventListener("storage", storage);
  window.addEventListener("resize", scrolled);
});
onContentUpdated(refresh);
onBeforeUnmount(() => {
  mounted = false;
  clearTimeout(timer);
  clearTimeout(messageTimer);
  clearTimeout(flashTimer);
  document.removeEventListener("selectionchange", schedule);
  document.removeEventListener("click", clickText);
  document.removeEventListener("keydown", key);
  window.removeEventListener("scroll", scrolled);
  window.removeEventListener("storage", storage);
  window.removeEventListener("resize", scrolled);
  if (supported.value)
    for (const name of ["personal-marks", "personal-notes", "personal-active"])
      (CSS as any).highlights.delete(name);
});
</script>
<template>
  <div class="annotation-bar">
    <button class="annotation-entry" @click="openDrawer" aria-haspopup="dialog">
      <span aria-hidden="true">▰</span> 本讲标记
      <span class="annotation-count">{{ current.length }}</span>
    </button>
    <span class="annotation-hint">选中文字，即可高亮或批注</span>
  </div>
  <p v-if="storageError" class="annotation-error" role="alert">
    {{ storageError }}
  </p>
  <Teleport to="body">
    <div
      v-if="floating"
      class="annotation-float"
      :style="floatStyle"
      role="toolbar"
      aria-label="标记选中文字"
      @pointerdown.prevent
    >
      <button @click="highlight"><span aria-hidden="true">▰</span> 高亮</button
      ><button @click="annotate">✎ 批注</button>
    </div>
    <div v-if="message || deleted" class="annotation-toast" role="status">
      {{ message || "标记已删除" }}
      <button v-if="deleted" @click="undo">撤销删除</button
      ><button
        aria-label="关闭提示"
        @click="
          message = '';
          deleted = null;
        "
      >
        ×
      </button>
    </div>
    <dialog
      ref="drawer"
      class="annotation-drawer"
      aria-labelledby="annotation-title"
      @click="
        (e) => {
          if (e.target === drawer) drawer?.close();
        }
      "
    >
      <div class="annotation-panel">
        <header>
          <div>
            <h2 id="annotation-title">
              本讲标记 <span>{{ current.length }}</span>
            </h2>
            <p>你的划重点与随手笔记</p>
          </div>
          <button
            class="annotation-close"
            @click="drawer?.close()"
            aria-label="关闭标记列表"
          >
            ×
          </button>
        </header>
        <div class="annotation-list">
          <p v-if="!supported" class="annotation-error">
            当前浏览器不支持原文高亮显示。标记仍可保存、查看和跳转；建议使用新版浏览器。
          </p>
          <div v-if="!current.length" class="annotation-empty">
            <span aria-hidden="true">✎</span>
            <h3>留下你的第一条标记</h3>
            <p>选中正文中的一句话，点击「高亮」或「批注」。</p>
            <small>支持单段正文与列表文字，暂不支持代码块、表格和公式。</small
            ><button @click="drawer?.close()">返回正文试一试</button>
          </div>
          <article v-for="m in current" :key="m.id" class="annotation-item">
            <span class="annotation-section">{{ m.heading }}</span>
            <button
              class="annotation-quote"
              @click="jump(m)"
              :disabled="locations[m.id] === undefined"
              :aria-label="'跳回原文：' + m.quote"
            >
              {{ m.quote }}
            </button>
            <p v-if="m.note" class="annotation-note">{{ m.note }}</p>
            <small
              v-if="locations[m.id] === undefined"
              class="annotation-missing"
              >原文位置已变化，摘录和批注仍保留。</small
            >
            <div class="annotation-item-actions">
              <button @click="edit(m, true)">
                {{ m.note ? "编辑批注" : "添加批注" }}</button
              ><button @click="remove(m)">删除</button>
            </div>
          </article>
        </div>
        <footer>
          <p>
            仅保存在当前设备的此浏览器中。清除网站数据会移除记录，建议定期备份。
          </p>
          <div>
            <button @click="exportAll">导出全部标记</button
            ><label class="annotation-import"
              >导入备份<input
                type="file"
                accept=".json,application/json"
                @change="importAll"
                aria-label="导入标记备份"
            /></label>
          </div>
          <p v-if="message" role="status">{{ message }}</p>
          <button v-if="deleted" @click="undo">撤销删除</button>
          <p v-if="storageError" class="annotation-error" role="alert">
            {{ storageError }}
          </p>
        </footer>
      </div>
    </dialog>
    <dialog
      ref="editor"
      class="annotation-editor"
      aria-labelledby="annotation-edit-title"
      @close="editorClosed"
      @click="
        (e) => {
          if (e.target === editor) editor?.close();
        }
      "
    >
      <header>
        <h2 id="annotation-edit-title">
          {{
            editing && marks.some((m) => m.id === editing?.id)
              ? "编辑标记"
              : "添加批注"
          }}
        </h2>
        <button
          class="annotation-close"
          @click="editor?.close()"
          aria-label="关闭批注"
        >
          ×
        </button>
      </header>
      <blockquote>{{ editing?.quote }}</blockquote>
      <label for="annotation-note"
        >我的批注 <span>可留空，仅保留高亮</span></label
      >
      <textarea
        id="annotation-note"
        ref="input"
        v-model="note"
        maxlength="10000"
        placeholder="写下你的理解，或留一个待解决的问题…"
        @keydown.ctrl.enter.prevent="save"
        @keydown.meta.enter.prevent="save"
      />
      <p v-if="storageError" class="annotation-error" role="alert">
        {{ storageError }}
      </p>
      <div class="annotation-editor-actions">
        <button
          v-if="editing && marks.some((m) => m.id === editing?.id)"
          @click="remove(editing!)"
        >
          删除标记</button
        ><button @click="editor?.close()">取消</button
        ><button class="annotation-primary" @click="save">保存标记</button>
      </div>
    </dialog>
  </Teleport>
</template>
