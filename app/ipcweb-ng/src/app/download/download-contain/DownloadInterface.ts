export interface DownloadListInterface {
    matchList: MatchList[];
    numOfMatches: number;
}

export interface MatchList {
    fileAddress: string;
    fileId: number;
    fileName: string;
    fileSize: number;
    fileTime: string;
    isChecked: boolean;
}
