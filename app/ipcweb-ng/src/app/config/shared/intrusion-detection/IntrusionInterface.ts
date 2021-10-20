export interface IntrusionRegion {
    normalizedScreenSize: NormalizedScreenSize;
    regionalInvasion: RegionalInvasion;
}

export interface NormalizedScreenSize {
    iNormalizedScreenHeight: number;
    iNormalizedScreenWidth: number;
}

export interface RegionalInvasion {
    iEnabled: number;
    iHeight: number;
    iPositionX: number;
    iPositionY: number;
    iProportion: number;
    iSensitivityLevel: number;
    iTimeThreshold: number;
    iWidth: number;
}
