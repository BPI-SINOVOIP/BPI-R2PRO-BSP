export interface NetworkInterface {
  link: {
    sAddress: string,
    sInterface: string,
    sNicSpeed: string,
    sDNS1: string;
    sDNS2: string;
    iDuplex: number;
    iPower: number;
    iNicSpeed: number;
    sNicSpeedSupport: string;
  };
  ipv4: {
    sV4Address: string,
    sV4Gateway: string,
    sV4Method: string,
    sV4Netmask: string,
  };
}
