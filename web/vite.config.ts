import { defineConfig } from 'vite'
import react from '@vitejs/plugin-react'

// https://vite.dev/config/
export default defineConfig({
  plugins: [react()],
  // Use '/led-sectional/' for GitHub Pages, '/' for other hosts
  // Override with: npm run build -- --base=/
  base: process.env.VITE_BASE_PATH || '/led-sectional/',
})
