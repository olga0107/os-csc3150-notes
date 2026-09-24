<script setup lang="ts">
import { ref, watch, onMounted, onBeforeUnmount, nextTick } from 'vue'
import { onContentUpdated } from 'vitepress'
import Annotations from './Annotations.vue'
const focus=ref(false), size=ref(17), progress=ref(0), outline=ref(false)
const headings=ref<{id:string;text:string;level:number}[]>([])
let frame=0
function apply(){
 document.documentElement.classList.toggle('reading-focus',focus.value)
 document.documentElement.style.setProperty('--reading-size',size.value+'px')
 try{localStorage.setItem('os-reading',JSON.stringify({focus:focus.value,size:size.value}))}catch{}
}
function measure(){
 cancelAnimationFrame(frame)
 frame=requestAnimationFrame(()=>{const el=document.querySelector('.vp-doc') as HTMLElement;if(!el)return;const start=el.getBoundingClientRect().top+scrollY;progress.value=Math.max(0,Math.min(100,100*(scrollY-start)/(Math.max(1,el.offsetHeight-innerHeight))))})
}
async function refresh(){await nextTick();headings.value=[...document.querySelectorAll('.vp-doc h2,.vp-doc h3')].map(el=>({id:el.id,text:el.textContent?.replace(/\u200b/g,'').trim()||'',level:Number(el.tagName[1])}));outline.value=false;measure()}
onMounted(()=>{
 try{const saved=JSON.parse(localStorage.getItem('os-reading')||'{}');focus.value=saved.focus===true;if([16,17,18,19,20].includes(saved.size))size.value=saved.size}catch{}
 apply();refresh();window.addEventListener('scroll',measure,{passive:true});window.addEventListener('resize',measure)
})
watch([focus,size],apply)
onContentUpdated(refresh)
onBeforeUnmount(()=>{window.removeEventListener('scroll',measure);window.removeEventListener('resize',measure);cancelAnimationFrame(frame);document.documentElement.classList.remove('reading-focus')})
</script>
<template>
 <div class="reading-tools">
  <div class="reading-controls" aria-label="阅读设置">
   <button type="button" @click="focus=!focus" :aria-pressed="focus" class="focus-control">{{focus?'退出专注':'专注阅读'}}</button>
   <button type="button" @click="outline=!outline" :aria-expanded="outline" aria-controls="chapter-outline">本讲目录 <span>{{outline?'−':'＋'}}</span></button>
   <div class="font-controls"><button @click="size=Math.max(16,size-1)" :disabled="size===16" aria-label="减小字号">A−</button><span aria-live="polite">{{size}}</span><button @click="size=Math.min(20,size+1)" :disabled="size===20" aria-label="增大字号">A＋</button></div>
  </div>
  <nav v-if="outline" id="chapter-outline" class="chapter-outline" aria-label="本讲章节"><a v-for="h in headings" :key="h.id" :href="'#'+h.id" :class="{'subheading':h.level===3}" @click="outline=false">{{h.text}}</a></nav>
  <Annotations />
  <div class="reading-progress" role="progressbar" aria-label="阅读进度" :aria-valuenow="Math.round(progress)" :aria-valuemin="0" :aria-valuemax="100"><span :style="{width:progress+'%'}" /></div>
 </div>
</template>
