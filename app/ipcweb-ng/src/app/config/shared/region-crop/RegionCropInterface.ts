export interface RegionCropInterface {
    normalizedScreenSize: NormalizedScreenSizeInterface;
    regionClip: RegionClipInterface;
}

export interface RegionClipInterface {
    iRegionClipEnabled: number;
    iHeight: number;
    iWidth: number;
    iPositionX: number;
    iPositionY: number;
    sresolution: string;
}

export interface NormalizedScreenSizeInterface {
    iNormalizedScreenHeight: number;
    iNormalizedScreenWidth: number;
}
