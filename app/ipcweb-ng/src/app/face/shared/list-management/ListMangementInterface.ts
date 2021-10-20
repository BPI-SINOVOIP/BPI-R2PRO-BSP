export interface MemberSearchResult {
    matchList: MemberListInterface[];
    numOfMatches: number;
}



export interface MemberListInterface {
    sTelephoneNumber: number;
    iAccessCardNumber: number;
    id: number;
    sAddress: string;
    iAge: string;
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
    sType: string;
    iLoadCompleted: number;
}

export interface SearchCondition {
    beginTime: string;
    endTime: string;
    type: string;
    gender: string;
    minAge: number;
    maxAge: number;
    accessCardNumber: number;
    beginPosition: number;
    endPosition: number;
}
