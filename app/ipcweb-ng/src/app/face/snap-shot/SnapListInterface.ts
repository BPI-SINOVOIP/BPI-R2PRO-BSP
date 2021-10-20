export interface SnapSearch {
    matchList: SnapListInterface[];
    numOfMatches: number;
}

export interface SnapListInterface {
    id: number;
    sNote: string;
    sPicturePath: string;
    sStatus: string;
    sTime: string;
    sSnapshotName: string;
}
