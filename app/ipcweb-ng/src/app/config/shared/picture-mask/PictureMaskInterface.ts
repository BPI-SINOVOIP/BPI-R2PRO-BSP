export interface PictureMaskInterface {
    normalizedScreenSize: NormalizedScreenSizeInterface;
    imageOverlay: ImageOverlayInerface;
}

export interface NormalizedScreenSizeInterface {
    iNormalizedScreenHeight: number;
    iNormalizedScreenWidth: number;
}

export interface ImageOverlayInerface {
    iImageHeight: number;
    iImageOverlayEnabled: boolean;
    iImageWidth: number;
    iPositionX: number;
    iPositionY: number;
    iTransparentColorEnabled: boolean;
}
