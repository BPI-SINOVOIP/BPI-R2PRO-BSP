export interface OsdOverplaysInterface {
    attribute: AttributeInterface;
    channelNameOverlay: ChannelNameOverlay;
    // set any before making sure which type of CharacterOverlayPart
    characterOverlay: [
        CharacterOverlayPart,
        CharacterOverlayPart,
        CharacterOverlayPart,
        CharacterOverlayPart,
        CharacterOverlayPart,
        CharacterOverlayPart,
        CharacterOverlayPart,
        CharacterOverlayPart,
    ];
    dateTimeOverlay: DateTimeOverlay;
    normalizedScreenSize: NormalizedScreenSize;
}

export interface AttributeInterface {
    iBoundary: number;
    sAlignment: string;
    sOSDAttribute: string;
    sOSDFontSize: string;
    sOSDFrontColor: string;
    sOSDFrontColorMode: string;
}

export interface ChannelNameOverlay {
    iPositionX: number;
    iPositionY: number;
    sChannelName: string;
    iChannelNameOverlayEnabled: boolean;
}

export interface CharacterOverlayPart {
    iPositionX: number;
    iPositionY: number;
    id: number;
    sDisplayText: string;
    sIsPersistentText: boolean;
    iTextOverlayEnabled: boolean;
}

export interface DateTimeOverlay {
    iPositionX: number;
    iPositionY: number;
    sDateStyle: string;
    iDateTimeOverlayEnabled: boolean;
    iDisplayWeekEnabled: boolean;
    sTimeStyle: string;
}

export interface NormalizedScreenSize {
    iNormalizedScreenHeight: number;
    iNormalizedScreenWidth: number;
}
