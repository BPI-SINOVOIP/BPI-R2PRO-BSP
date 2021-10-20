export interface GateInterface {
    relay: RelayData;
    weigen: WeigenData;
}

export interface RelayData {
    iDuration: number;
    iEnable: number;
    iIOIndex: number;
    iValidLevel: number;
    id: number;
}

export interface WeigenData {
    iDuration: number;
    iEnable: number;
    iWiegandBit: number;
    id: number;
}
