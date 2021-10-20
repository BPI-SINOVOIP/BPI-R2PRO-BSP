export interface ColorCell {
    name: string;
    color: string;
}

export interface TimeTableOption {
    isInit: boolean;
    isType: boolean;
    isEnabledTop: boolean;
    isAdvance: boolean;
    advancePara: Array<object>;
    id: number;
    paraId: number;
    pageId: string;
    advanceId: number;
}
