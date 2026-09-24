<script setup lang="ts">
import { ref, onMounted, onBeforeUnmount, watch } from 'vue'
import { useData } from 'vitepress'
import diagrams from '../diagrams.json'
const props = defineProps<{ id: string }>()
const item = (diagrams as Record<string, {title:string;source:string;description:string;height:number}>)[props.id]
const { isDark } = useData()
const root = ref<HTMLElement>(), dialog = ref<HTMLDialogElement>()
const svg = ref(''), failed = ref(false), ready = ref(false), zoom = ref(1)
let observer: IntersectionObserver, version = 0
let uid = ''
async function render() {
 if (!ready.value) return
 const current = ++version
 try {
  const { default: mermaid } = await import('mermaid')
  if (current !== version) return
  mermaid.initialize({startOnLoad:false,securityLevel:'strict',theme:isDark.value?'dark':'base',fontFamily:'Arial, PingFang SC, Microsoft YaHei, sans-serif',themeVariables:{fontSize:'17px',primaryColor:isDark.value?'#293f3b':'#e8f2ee',primaryTextColor:isDark.value?'#e2eee9':'#24453c',primaryBorderColor:'#7da99a',lineColor:isDark.value?'#a2cabc':'#567f72',secondaryColor:isDark.value?'#303a42':'#edf1f7',tertiaryColor:isDark.value?'#33372b':'#f8f2e6'},flowchart:{htmlLabels:false,useMaxWidth:false,curve:'basis',nodeSpacing:28,rankSpacing:44},sequence:{useMaxWidth:false,wrap:true,diagramMarginX:24,diagramMarginY:24}})
  const result = await mermaid.render(uid+'-'+current,item.source)
  if(current===version){svg.value=result.svg;failed.value=false}
 } catch { if(current===version) failed.value=true }
}
function expand(){zoom.value=1;dialog.value?.showModal()}
onMounted(()=>{
 uid='diagram-'+props.id+'-'+Math.random().toString(36).slice(2,8)
 observer=new IntersectionObserver(entries=>{if(entries.some(e=>e.isIntersecting)){ready.value=true;observer.disconnect();render()}},{rootMargin:'400px'})
 if(root.value)observer.observe(root.value)
})
watch(isDark,render)
onBeforeUnmount(()=>{version++;observer?.disconnect();dialog.value?.close()})
</script>
<template>
 <figure ref="root" class="study-diagram" :data-diagram-id="id">
  <figcaption><span class="figure-label">CONCEPT MAP</span><strong>{{ item.title }}</strong><button type="button" :disabled="!svg" @click="expand" :aria-label="'放大图示：'+item.title">放大 ↗</button></figcaption>
  <div class="diagram-canvas" :style="{height:Math.min(item.height + 48, 740)+'px'}">
  <div v-if="svg" class="diagram-scroll" tabindex="0" role="img" :aria-label="item.title" v-html="svg" />
  <p v-else class="diagram-placeholder">{{ failed ? '图示暂时无法显示，可展开下方文字说明。' : item.title }}</p>
  </div>
  <details class="diagram-description" :open="failed"><summary>文字说明<span class="diagram-pan-hint">宽图可左右滑动查看</span></summary><pre>{{ item.description }}</pre></details>
  <dialog ref="dialog" class="diagram-dialog" @click="($event.target === dialog) && dialog?.close()">
   <div class="diagram-modal-head"><strong>{{ item.title }}</strong><div><button @click="zoom=Math.max(.5,zoom-.25)" aria-label="缩小">−</button><button @click="zoom=1" aria-label="恢复原始大小">{{ Math.round(zoom*100) }}%</button><button @click="zoom=Math.min(3,zoom+.25)" aria-label="放大">＋</button><button @click="dialog?.close()" aria-label="关闭图示">关闭 ✕</button></div></div>
   <div class="diagram-modal-body"><div :style="{zoom}" role="img" :aria-label="item.title" v-html="svg" /></div>
  </dialog>
 </figure>
</template>
