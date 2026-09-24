import DefaultTheme from 'vitepress/theme'
import type { Theme } from 'vitepress'
import Layout from './components/Layout.vue'
import StudyDiagram from './components/StudyDiagram.vue'
import './custom.css'
export default {
 extends: DefaultTheme,
 Layout,
 enhanceApp({ app }) { app.component('StudyDiagram', StudyDiagram) }
} satisfies Theme
