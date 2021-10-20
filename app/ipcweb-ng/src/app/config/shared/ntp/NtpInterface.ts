export interface NtpInfoInterface {
    iAutoDst: number;
    iAutoMode: number;
    iRefreshTime: number;
    sNtpServers: string;
    sTimeZone: string;
    sTimeZoneFile: string;
    sTimeZoneFileDst: string;
}

export interface TimeZoneInterface {
    id: number;
    sTimeZone: string;
    sTimeZoneFile: string;
    sTimeZoneFileDst: string;
}

export interface NowTimeInterface {
    time: string;
}
