import type { DeviceConfig } from '../types/config';
import { DEFAULT_CONFIG } from '../types/config';

const STORAGE_KEY = 'led-sectional-config';

/**
 * Save configuration to localStorage
 */
export function saveToLocalStorage(config: Partial<DeviceConfig>): void {
  try {
    localStorage.setItem(STORAGE_KEY, JSON.stringify(config));
  } catch (error) {
    console.error('Failed to save to localStorage:', error);
  }
}

/**
 * Load configuration from localStorage
 */
export function loadFromLocalStorage(): Partial<DeviceConfig> | null {
  try {
    const saved = localStorage.getItem(STORAGE_KEY);
    if (saved) {
      return JSON.parse(saved);
    }
  } catch (error) {
    console.error('Failed to load from localStorage:', error);
  }
  return null;
}

/**
 * Clear stored configuration
 */
export function clearLocalStorage(): void {
  try {
    localStorage.removeItem(STORAGE_KEY);
  } catch (error) {
    console.error('Failed to clear localStorage:', error);
  }
}

/**
 * Encode configuration to URL-safe string (for sharing)
 * Uses a compact format: base64(JSON)
 */
export function encodeConfigToUrl(config: Partial<DeviceConfig>): string {
  try {
    // Create a minimal config for sharing (mainly airports)
    const shareableConfig = {
      airports: config.airports,
      brightness: config.brightness,
      windThreshold: config.windThreshold,
      doLightning: config.doLightning,
      doWinds: config.doWinds,
    };
    const json = JSON.stringify(shareableConfig);
    return btoa(encodeURIComponent(json));
  } catch (error) {
    console.error('Failed to encode config:', error);
    return '';
  }
}

/**
 * Decode configuration from URL string
 */
export function decodeConfigFromUrl(encoded: string): Partial<DeviceConfig> | null {
  try {
    const json = decodeURIComponent(atob(encoded));
    return JSON.parse(json);
  } catch (error) {
    console.error('Failed to decode config from URL:', error);
    return null;
  }
}

/**
 * Get current URL with config parameter
 */
export function getShareableUrl(config: Partial<DeviceConfig>): string {
  const encoded = encodeConfigToUrl(config);
  const url = new URL(window.location.href);
  url.searchParams.set('config', encoded);
  return url.toString();
}

/**
 * Load configuration from URL if present
 */
export function loadFromUrl(): Partial<DeviceConfig> | null {
  try {
    const url = new URL(window.location.href);
    const encoded = url.searchParams.get('config');
    if (encoded) {
      return decodeConfigFromUrl(encoded);
    }
  } catch (error) {
    console.error('Failed to load config from URL:', error);
  }
  return null;
}

/**
 * Merge configs with defaults
 */
export function mergeWithDefaults(config: Partial<DeviceConfig> | null): DeviceConfig {
  return { ...DEFAULT_CONFIG, ...config };
}
