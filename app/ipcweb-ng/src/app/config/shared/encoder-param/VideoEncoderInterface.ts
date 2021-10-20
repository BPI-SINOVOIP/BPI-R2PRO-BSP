import { MenuGroup } from '../../MenuGroup';

export interface VideoEncoderInterface {
  id: number;
  iGOP: number;
  iTargetRate: number;
  iMaxRate: number;
  iMinRate: number;
  iStreamSmooth: number;
  sFrameRate: string;
  sFrameRateIn: string;
  sRCQuality: string;
  sOutputDataType: string;
  sRCMode: string;
  sH264Profile: string;
  sResolution: string;
  sSVC: string;
  sSmart: string;
  sStreamType: string;
  sVideoType: string;
}

export interface VideoDefaultPara {
  static: any;
  disabled: Array<DisabledOp>;
  dynamic: any;
  layout: any;
}

export interface DisabledOp {
  name: string;
  options: any;
}
