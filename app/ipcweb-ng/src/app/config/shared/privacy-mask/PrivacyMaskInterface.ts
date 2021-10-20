export interface PrivacyMaskInterface {
    normalizedScreenSize: NormalizedScreenSizeInterface;
    privacyMask: [
        MaskInterface,
        MaskInterface,
        MaskInterface,
        MaskInterface,
    ];
}

export interface NormalizedScreenSizeInterface {
    iNormalizedScreenHeight: number;
    iNormalizedScreenWidth: number;
}

export interface MaskInterface {
    iMaskHeight: number;
    iMaskWidth: number;
    iPositionX: number;
    iPositionY: number;
    id: number;
    iPrivacyMaskEnabled: boolean;
}

