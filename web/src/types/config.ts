// Light sensor types matching firmware enum
export enum LightSensorType {
  None = 0,
  Analog = 1,
  TSL2561 = 2,
}

// Device configuration matching firmware config structure
export interface DeviceConfig {
  version: number;
  dataPin: number;
  brightness: number;
  ledType: string;
  colorOrder: string;
  requestInterval: number;
  loopInterval: number;
  windThreshold: number;
  doLightning: boolean;
  doWinds: boolean;
  windAlternate: boolean;
  windAlternateInterval: number;
  useLightSensor: boolean;
  lightSensorType: LightSensorType;
  minBrightness: number;
  maxBrightness: number;
  minLight: number;
  maxLight: number;
  mqttEnabled: boolean;
  mqttBroker: string;
  mqttPort: number;
  mqttUsername: string;
  mqttPassword: string;
  powerOn: boolean;
  noDataColor: string;
  airports: string[];
}

// Device status from get_status command
export interface DeviceStatus {
  status: string;
  firmware_version: string;
  wifi_connected: boolean;
  wifi_ssid: string;
  ip_address: string;
  needs_wifi_config: boolean;
  heap_free: number;
  uptime: number;
  last_metar_update: number;
  metar_count: number;
  airport_count: number;
  brightness: number;
  has_lightning: boolean;
}

// Serial command types
export type SerialCommand =
  | { cmd: 'get_config' }
  | { cmd: 'set_config'; config: Partial<DeviceConfig> }
  | { cmd: 'get_status' }
  | { cmd: 'set_wifi'; ssid: string; pass: string }
  | { cmd: 'reset_wifi' }
  | { cmd: 'reboot' }
  | { cmd: 'factory_reset' }
  | { cmd: 'test_leds' }
  | { cmd: 'fetch_metars' };

// Serial response types
export interface SerialResponse {
  status: 'ok' | 'error';
  message?: string;
  config?: DeviceConfig;
}

// Special airport entries
export const SPECIAL_ENTRIES = ['VFR', 'MVFR', 'IFR', 'LIFR', 'WVFR', 'WBNK', 'NULL'] as const;
export type SpecialEntry = typeof SPECIAL_ENTRIES[number];

// Check if an airport code is a special entry
export function isSpecialEntry(code: string): code is SpecialEntry {
  return SPECIAL_ENTRIES.includes(code as SpecialEntry);
}

// Get display color class for flight category
export function getCategoryColorClass(code: string): string {
  switch (code.toUpperCase()) {
    case 'VFR': return 'cat-vfr';
    case 'MVFR': return 'cat-mvfr';
    case 'IFR': return 'cat-ifr';
    case 'LIFR': return 'cat-lifr';
    case 'WVFR': return 'cat-wvfr';
    case 'WBNK': return 'cat-wbnk';
    case 'NULL': return 'cat-null';
    default: return 'bg-gray-500';
  }
}

// Default configuration
export const DEFAULT_CONFIG: DeviceConfig = {
  version: 2,
  dataPin: 14,
  brightness: 20,
  ledType: 'WS2811',
  colorOrder: 'RGB',
  requestInterval: 900000,
  loopInterval: 5000,
  windThreshold: 25,
  doLightning: true,
  doWinds: true,
  windAlternate: false,
  windAlternateInterval: 2000,
  useLightSensor: false,
  lightSensorType: LightSensorType.None,
  minBrightness: 20,
  maxBrightness: 30,
  minLight: 16,
  maxLight: 30,
  mqttEnabled: false,
  mqttBroker: '',
  mqttPort: 1883,
  mqttUsername: '',
  mqttPassword: '',
  powerOn: true,
  noDataColor: '#000000',
  airports: [],
};
