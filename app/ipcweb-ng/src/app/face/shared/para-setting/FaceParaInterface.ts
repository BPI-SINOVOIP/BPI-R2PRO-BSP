export interface FaceParaInterface {
    iDetectHeight: number;
    iDetectWidth: number;
    iFaceDetectionThreshold: number;
    iFaceMinPixel: number;
    iFaceRecognitionThreshold: number;
    iLeftCornerX: number;
    iLeftCornerY: number;
    iLiveDetectThreshold: number;
    iPromptVolume: number;
    id: number;
    sLiveDetect: string;
    sLiveDetectBeginTime: string;
    sLiveDetectEndTime: string;
    iNormalizedHeight: number;
    iNormalizedWidth: number;
}

export interface FaceSettingOptions {
    isIPC: boolean;
}
