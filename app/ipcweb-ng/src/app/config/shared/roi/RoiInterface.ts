export interface RoiInterface {
    normalizedScreenSize: NormalizedScreenSizeInterface;
    ROIRegionList: RoiPartInterface[];
}

export interface RoiPartInterface {
    sStreamType: string;
    iStreamEnabled: number;
    iROIId: number;
    iROIEnabled: number;
    sName: string;
    iQualityLevelOfROI: number;
    iHeight: number;
    iWidth: number;
    iPositionX: number;
    iPositionY: number;
}

export interface NormalizedScreenSizeInterface {
    iNormalizedScreenHeight: number;
    iNormalizedScreenWidth: number;
}
