<script setup lang="ts">
import DefaultTheme from 'vitepress/theme'
import ReadingTools from './ReadingTools.vue'
import {ref} from 'vue'
const folded=ref(false)
</script>
<template>
 <DefaultTheme.Layout>
  <template #doc-before><ReadingTools /></template>
  <template #aside-outline-before><button class="outline-toggle" :aria-expanded="!folded" @click="folded=!folded">章节导航 <span>{{folded?'展开 ＋':'收起 −'}}</span></button><span :class="{'outline-fold-marker':folded}" /></template>
  <template #home-hero-info-after><div class="cover-byline"><span class="author-mark">o.</span><span>Notes by <strong>olga</strong></span></div></template>
  <template #home-hero-image><div class="cover-art" aria-hidden="true"><div class="orbit orbit-one"/><div class="orbit orbit-two"/><div class="system-chip"><span>CSC / 3150</span><strong>OS</strong></div><span class="orbit-label label-one">PROCESS</span><span class="orbit-label label-two">MEMORY</span><span class="orbit-label label-three">I/O</span></div></template>
 </DefaultTheme.Layout>
</template>
