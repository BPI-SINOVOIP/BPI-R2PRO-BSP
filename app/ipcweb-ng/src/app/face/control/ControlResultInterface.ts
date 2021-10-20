export interface ControlSearchResult {
    numOfMatches: number;
    matchList: ControlResultInterface[];
}

export interface ControlResultInterface {
    iAccessCardNumber: number;
    iAge: number;
    iFaceId: number;
    id: number;
    sAddress: string;
    sBirthday: string;
    sCertificateNumber: string;
    sCertificateType: string;
    sGender: string;
    sHometown: string;
    sListType: string;
    sName: string;
    sNation: string;
    sNote: string;
    sPicturePath: string;
    sRegistrationTime: string;
    sSimilarity: string;
    sSnapshotPath: string;
    sStatus: string;
    sTelephoneNumber: string;
    sTime: string;
    sType: string;
    sSnapshotName: string;
}
