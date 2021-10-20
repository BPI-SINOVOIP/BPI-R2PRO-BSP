export interface FTPInterface {
    iEnabled: number;
    iPathDepth: number;
    iPicArchivingInterval: number;
    iPortNo: number;
    id: number;
    sAddressType: string;
    sIpAddress: string;
    sPicNameRuleType: string;
    sPicNamePrefix: string;
    sSubDirName: string;
    sTopDirName: string;
    sUserName?: string;
    sPassword?: string;
    sPasswordConfirm?: string;
    iAnonymous: number;
    sSubDirNameRule: string;
    sTopDirNameRule: string;
}
