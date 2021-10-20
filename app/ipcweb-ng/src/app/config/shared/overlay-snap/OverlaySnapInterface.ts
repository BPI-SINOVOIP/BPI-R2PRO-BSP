export interface CoverInfo {
    id: number;
    iFaceEnabled: number;
    iStreamOverlayEnabled: number;
    iImageOverlayEnabled: number;
    iInfoOverlayEnabled: number;
    sTargetImageType: string;
    iWidthRatio: number;
    iFaceHeightRatio: number;
    iBodyHeightRatio: number;
    sImageQuality: string;
}

export interface OverlayInfo {
    id: number;
    iEnabled: number;
    sName: string;
    sInfo: string;
    iOrder: number;
}

export interface OverlaySnapDefaultPara {
    capability: {
        SmartCover: {
            sImageQuality: Array<string>;
            sTargetImageType: Array<string>;
        }
    };
    layout: {
        enabled: Array<string>;
        snap: Array<string>;
        infoEnabled: Array<string>;
    };
}
