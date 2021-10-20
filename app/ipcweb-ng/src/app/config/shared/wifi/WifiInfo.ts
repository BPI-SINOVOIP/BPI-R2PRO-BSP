export class WifiInfo {
  id: number;
  ssid: string;
  mode: string;
  security: string;
  channel: number;
  signal: number;
  speed: number;
  status: string;
}

export class WifiEnabledInterface {
  iPower: boolean;
  id: number;
  sType: string;
}

export class WifiItemInterface {
  Favorite: number;
  Strength: number;
  sName: string;
  sSecurity: string;
  sService: string;
  sState: string;
  sType: string;
}

export class PutWifiInterface {
  sName: string;
  sService: string;
  sPassword: string;
  iFavorite: number;
  iAutoconnect: number;
}


