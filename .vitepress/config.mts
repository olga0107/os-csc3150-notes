import { defineConfig } from 'vitepress'

export default defineConfig({
  base: '/os-csc3150-notes/',
  title: 'CSC3150 Notes',
  description: 'Operating Systems lecture notes · CUHK-Shenzhen, Fall 2026',
  cleanUrls: true,
  lastUpdated: true,
  srcExclude: ['README.md', 'AGENTS.md', 'demos/**', 'node_modules/**'],

  markdown: {
    image: { lazyLoading: true }
  },

  themeConfig: {
    nav: [
      { text: 'Home', link: '/' },
      { text: 'Lectures', link: '/notes/lec01-introduction' }
    ],

    sidebar: [
      {
        text: 'Lectures',
        items: [
          { text: 'Lec 01 · Introduction', link: '/notes/lec01-introduction' },
          { text: 'Lec 02 · Four Concepts', link: '/notes/lec02-concepts' },
          { text: 'Lec 03 · Threads & Processes', link: '/notes/lec03-thread-process' },
          { text: 'Lec 04 · Process API & Locks', link: '/notes/lec04-process' }
        ]
      },
      {
        text: 'Foundations',
        items: [
          { text: 'C Pointers & API Parameters', link: '/foundations/c-pointers' },
          { text: 'Stack vs Heap', link: '/foundations/stack-vs-heap' }
        ]
      }
    ],

    search: { provider: 'local' },
    outline: { level: [2, 3], label: 'On this page' },

    docFooter: { prev: 'Previous', next: 'Next' }
  }
})
