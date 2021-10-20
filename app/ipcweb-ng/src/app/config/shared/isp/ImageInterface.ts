export interface ImageInterface {
  id: number;
  imageAdjustment: ImageAdjustmentInterface;
  exposure: ExposureInterface;
  nightToDay: NightToDayInterface;
  BLC: BLCInterface;
  whiteBlance: WhiteBlanceInterface;
  imageEnhancement: ImageEnhancementInterface;
  videoAdjustment: VideoAdjustmentInterface;
}

export interface ImageAdjustmentInterface {
  iBrightness: number;
  iContrast: number;
  iSaturation: number;
  iSharpness: number;
  iHue: number;
}

export interface ExposureInterface {
  sIrisType: string;
  iAutoIrisLevel: number;
  sExposureTime: string;
  iExposureGain: number;
  sGainMode: string;
  sExposureMode: string;
}

export interface NightToDayInterface {
  sNightToDay: string;
  iNightToDayFilterLevel: number;
  iNightToDayFilterTime: number;
  sBeginTime: string;
  sEndTime: string;
  sDawnTime: string;
  sDuskTime: string;
  sIrcutFilterAction: string;
  sOverexposeSuppressType: string;
  sOverexposeSuppress: string;
  iDistanceLevel: number;
}

export interface BLCInterface {
  sWDR: string;
  iWDRLevel: number;
  sHLC: string;
  iHLCLevel: number;
  iDarkBoostLevel: number;
  sHDR: string;
  sBLCRegion: string;
  iBLCStrength: number;
  iBLCRegionHeight: number;
  iBLCRegionWidth: number;
  iPositionX: number;
  iPositionY: number;
}

export interface WhiteBlanceInterface {
  sWhiteBlanceStyle: string;
  iWhiteBalanceBlue: number;
  iWhiteBalanceGreen: number;
  iWhiteBalanceRed: number;
}

export interface ImageEnhancementInterface {
  sNoiseReduceMode: string;
  iDenoiseLevel: number;
  iSpatialDenoiseLevel: number;
  iTemporalDenoiseLevel: number;
  sDehaze: string;
  iDehazeLevel: number;
  sDIS: string;
  sGrayScaleMode: string;
  sFEC: string;
  sDistortionCorrection: string;
  iLdchLevel: number;
  iFecLevel: number;
}

export interface VideoAdjustmentInterface {
  sImageFlip: string;
  sSceneMode: string;
  sPowerLineFrequencyMode: string;
}

export interface ScenarioInterface {
  sScenario: string;
}
