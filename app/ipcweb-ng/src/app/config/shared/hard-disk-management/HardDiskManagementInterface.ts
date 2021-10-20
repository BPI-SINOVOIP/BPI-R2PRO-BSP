export interface HardDiskManagementInterface {
    selected: boolean;
    id: number;
    iFormatStatus: number;
    iFormatProg: number;
    iFreeSize: number;
    iMediaSize: number;
    sStatus: string;
    iTotalSize: number;
    sAttributes: string;
    sDev: string;
    sMountPath: string;
    sName: string;
    sType: string;
}
