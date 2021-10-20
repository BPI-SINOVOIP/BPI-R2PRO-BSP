import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { ConfigRoutingModule } from './config-routing.module';
import { TranslateModule, TranslateLoader } from '@ngx-translate/core';
import { TranslateHttpLoader } from '@ngx-translate/http-loader';
import { HttpClient } from '@angular/common/http';
import { PlayerModule } from '../shared/player/player.module';

import { ConfigComponent } from './config.component';
import { ConfigSystemComponent } from './config-system/config-system.component';
import { ConfigNetworkComponent } from './config-network/config-network.component';
import { ConfigVideoComponent } from './config-video/config-video.component';
import { ConfigImageComponent } from './config-image/config-image.component';
import { ConfigEventComponent } from './config-event/config-event.component';
import { ConfigStorageComponent } from './config-storage/config-storage.component';
import { ConfigIntelComponent } from './config-intel/config-intel.component';
import { ConfigAudioComponent } from './config-audio/config-audio.component';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { TcpipComponent } from './shared/tcpip/tcpip.component';
import { WifiComponent } from './shared/wifi/wifi.component';
import { SmtpComponent } from './shared/smtp/smtp.component';
import { FtpComponent } from './shared/ftp/ftp.component';
import { EmailComponent } from './shared/email/email.component';
import { CloudComponent } from './shared/cloud/cloud.component';
import { ProtocolComponent } from './shared/protocol/protocol.component';
import { DdnsComponent } from './shared/ddns/ddns.component';
import { PppoeComponent } from './shared/pppoe/pppoe.component';
import { PortComponent } from './shared/port/port.component';
import { UpnpComponent } from './shared/upnp/upnp.component';
import { RoiComponent } from './shared/roi/roi.component';
import { IspComponent } from './shared/isp/isp.component';
import { NtpComponent } from './shared/ntp/ntp.component';
import { InfoComponent } from './shared/info/info.component';
import { EncoderParamComponent } from './shared/encoder-param/encoder-param.component';
import { OsdComponent } from './shared/osd/osd.component';
import { PrivacyMaskComponent } from './shared/privacy-mask/privacy-mask.component';
import { PictureMaskComponent } from './shared/picture-mask/picture-mask.component';
import { UpgradeComponent } from './shared/upgrade/upgrade.component';
import { HardDiskManagementComponent } from './shared/hard-disk-management/hard-disk-management.component';
import { RegionCropComponent } from './shared/region-crop/region-crop.component';
import { MotionDetectComponent } from './shared/motion-detect/motion-detect.component';
import { AlarmInputComponent } from './shared/alarm-input/alarm-input.component';
import { AlarmOutputComponent } from './shared/alarm-output/alarm-output.component';
import { AbnormalComponent } from './shared/abnormal/abnormal.component';
import { MotionRegionComponent } from './shared/motion-region/motion-region.component';
import { MotionArmingComponent } from './shared/motion-arming/motion-arming.component';
import { MotionLinkageComponent } from './shared/motion-linkage/motion-linkage.component';
import { TimeTableComponent } from './shared/time-table/time-table.component';
import { IntrusionDetectionComponent } from './shared/intrusion-detection/intrusion-detection.component';
import { IntrusionRegionComponent } from './shared/intrusion-region/intrusion-region.component';
import { ScreenshotComponent } from './shared/screenshot/screenshot.component';
import { UserManageComponent } from './shared/user-manage/user-manage.component';
import { AdvancedEncoderComponent } from './shared/advanced-encoder/advanced-encoder.component';
import { OverlaySnapComponent } from './shared/overlay-snap/overlay-snap.component';
import { PeripheralsComponent } from './peripherals/peripherals.component';
import { GateConfigComponent } from './shared/gate-config/gate-config.component';
import { ScreenConfigComponent } from './shared/screen-config/screen-config.component';

export function HttpLoaderFactory(http: HttpClient) {
  return new TranslateHttpLoader(http);
}

@NgModule({
  declarations: [
    ConfigComponent,
    ConfigSystemComponent,
    ConfigNetworkComponent,
    ConfigVideoComponent,
    ConfigImageComponent,
    ConfigEventComponent,
    ConfigStorageComponent,
    ConfigIntelComponent,
    ConfigAudioComponent,
    TcpipComponent,
    WifiComponent,
    SmtpComponent,
    FtpComponent,
    EmailComponent,
    CloudComponent,
    ProtocolComponent,
    DdnsComponent,
    PppoeComponent,
    PortComponent,
    UpnpComponent,
    RoiComponent,
    IspComponent,
    NtpComponent,
    InfoComponent,
    EncoderParamComponent,
    OsdComponent,
    PrivacyMaskComponent,
    PictureMaskComponent,
    UpgradeComponent,
    HardDiskManagementComponent,
    RegionCropComponent,
    MotionDetectComponent,
    AlarmInputComponent,
    AlarmOutputComponent,
    AbnormalComponent,
    MotionRegionComponent,
    MotionArmingComponent,
    MotionLinkageComponent,
    TimeTableComponent,
    IntrusionDetectionComponent,
    IntrusionRegionComponent,
    ScreenshotComponent,
    UserManageComponent,
    AdvancedEncoderComponent,
    OverlaySnapComponent,
    PeripheralsComponent,
    GateConfigComponent,
    ScreenConfigComponent,
  ],
  imports: [
    CommonModule,
    FormsModule,
    ReactiveFormsModule,
    TranslateModule.forRoot({
      loader: {
        provide: TranslateLoader,
        useFactory: HttpLoaderFactory,
        deps: [HttpClient]
      }
    }),
    ConfigRoutingModule,
    PlayerModule
  ],
})
export class ConfigModule { }
