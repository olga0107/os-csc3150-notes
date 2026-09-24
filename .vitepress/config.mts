import { defineConfig } from 'vitepress'
import { readFileSync } from 'node:fs'
import { resolve } from 'node:path'

export default defineConfig({
  base: '/os-csc3150-notes/',
  title: 'CSC3150 Notes',
  description: 'Operating Systems lecture notes · CUHK-Shenzhen, Fall 2026',
  cleanUrls: true,
  lastUpdated: true,
  srcExclude: ['README.md', 'AGENTS.md', 'demos/**', 'node_modules/**'],

  markdown: {
    image: { lazyLoading: true },
    config(md) {
      const renderImage = md.renderer.rules.image!
      md.renderer.rules.image = (tokens, idx, options, env, self) => {
        const token = tokens[idx]
        const src = token.attrGet('src') || ''
        // Reserve local slide dimensions before lazy loading to keep anchors stable.
        if (src.startsWith('../assets/') && src.endsWith('.png')) {
          const data = readFileSync(resolve(process.cwd(), src.slice(3)))
          token.attrSet('width', String(data.readUInt32BE(16)))
          token.attrSet('height', String(data.readUInt32BE(20)))
        }
        return renderImage(tokens, idx, options, env, self)
      }
    }
  },

  themeConfig: {
    nav: [
      { text: 'Home', link: '/' },
      { text: 'Lectures', link: '/notes/lec01-introduction' }
    ],

    sidebar: [
      {
        text: 'Lectures',
        collapsed: false,
        items: [
          { text: 'Lec 01 · Introduction', link: '/notes/lec01-introduction' },
          { text: 'Lec 02 · Four Concepts', link: '/notes/lec02-concepts' },
          { text: 'Lec 03 · Threads & Processes', link: '/notes/lec03-thread-process' },
          { text: 'Lec 04 · Process API & Locks', link: '/notes/lec04-process' },
          { text: 'Lec 05 · Files & I/O', link: '/notes/lec05-files' },
          { text: 'Lec 06 · Sockets & Pipes', link: '/notes/lec06-sockets-pipes' }
        ]
      },
      {
        text: 'Foundations',
        collapsed: true,
        items: [
          { text: 'C Pointers & API Parameters', link: '/foundations/c-pointers' },
          { text: 'Stack vs Heap', link: '/foundations/stack-vs-heap' }
        ]
      }
    ],

    search: { provider: 'local' },
    outline: { level: 2, label: '本讲目录' },

    docFooter: { prev: 'Previous', next: 'Next' }
  }
})
