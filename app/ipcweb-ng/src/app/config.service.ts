import { Injectable } from '@angular/core';
import { HttpClient, HttpHeaders, HttpErrorResponse, HttpRequest, HttpResponse,
  HttpParams, HttpEvent, HttpEventType } from '@angular/common/http';
import { Observable, bindNodeCallback, of, throwError } from 'rxjs';
import { switchMap, map, catchError, timeout, last, tap} from 'rxjs/operators';
import * as xml2js from 'xml2js';

import { NetworkInterface } from './config/NetworkInterface';
import { WifiInfo, WifiEnabledInterface, WifiItemInterface, PutWifiInterface } from './config/shared/wifi/WifiInfo';
import Logger from './logger';
import { AudioInterface } from './config/config-audio/AudioInterface';
import { VideoEncoderInterface } from './config/shared/encoder-param/VideoEncoderInterface';
import { NetworkPortInterface } from './config/shared/port/NetworkPortInterface';
import { StreamURLInterface } from './preview/StreamURLInterface';
import { environment } from '../environments/environment';
import { ImageInterface, ScenarioInterface } from './config/shared/isp/ImageInterface';
import { DeviceInfoInterface } from './config/shared/info/DeviceInfoInterface';
import { OsdOverplaysInterface } from './config/shared/osd/OsdOverplaysInterface';
import { PrivacyMaskInterface } from './config/shared/privacy-mask/PrivacyMaskInterface';
import { PictureMaskInterface } from './config/shared/picture-mask/PictureMaskInterface';
import { HardDiskManagementInterface } from './config/shared/hard-disk-management/HardDiskManagementInterface';
import { QuotaInterface } from './config/shared/hard-disk-management/QuotaInterface';
import { RoiInterface, RoiPartInterface } from './config/shared/roi/RoiInterface';
import { RegionCropInterface, RegionClipInterface } from './config/shared/region-crop/RegionCropInterface';
import { DownloadListInterface } from './download/download-contain/DownloadInterface';
import { MotionRegionInterface } from './config/shared/motion-region/MotionRegionInterface';
import { LinkageInterface } from './config/shared/motion-linkage/LinkageInterface';
import { FreeRoomInterface } from './config/shared/upgrade/UpgradeInterface';
import { NtpInfoInterface, TimeZoneInterface, NowTimeInterface } from './config/shared/ntp/NtpInterface';
import { IntrusionRegion, RegionalInvasion } from './config/shared/intrusion-detection/IntrusionInterface';
import { TimingInterface, EventShotInterface } from './config/shared/screenshot/ScreenshotInterface';
import { MemberListInterface, SearchCondition, MemberSearchResult } from './face/shared/list-management/ListMangementInterface';
import { AddMemberInterface } from './face/shared/add-member/AddMemberInterface';
import { SnapSearch } from './face/snap-shot/SnapListInterface';
import { ControlSearchResult } from './face/control/ControlResultInterface';
import { FaceParaInterface } from './face/shared/para-setting/FaceParaInterface';
import { LoginStatus } from './auth/login/LoginInterface';
import { UserCell, UserStatus, UserForm } from './config/shared/user-manage/UserInterface';
import { AdvanceEncCell, AdvanceEncPutInfo } from './config/shared/advanced-encoder/AdvancedEncoderInterface';
import { GateInterface } from './config/shared/gate-config/GateInterface';
import { ScreenCfgInterface } from './config/shared/screen-config/ScreenCfgInterface';
import { FTPInterface } from './config/shared/ftp/FTPInterface';

@Injectable({
  providedIn: 'root'
})
export class ConfigService {

  private logger: Logger = new Logger('config');

  private serverUrl: string = environment.serverUrl;

  /** To compatible with XML response */
  private xmlHttpOptions: object = {
    headers: new HttpHeaders({
      'Content-Type': 'application/xml',
      Accept: 'application/xml',
      'Response-Type': 'text'
    }),
    responseType: 'text',
  };

  private xmlParser = new xml2js.Parser({
    xmlns: false,
    explicitArray: false,
    tagNameProcessors: [xml2js.processors.stripPrefix],
    valueProcessors: [xml2js.processors.stripPrefix],
    attrNameProcessors: [xml2js.processors.stripPrefix]
  });

  constructor(
    private http: HttpClient,
  ) { }

  private handleError(error: HttpErrorResponse) {
    if (error.error instanceof ErrorEvent) {
      // A client-side or network error occurred. Handle it accordingly.
      this.logger.error('An error occurred:' + error.error.message);

    } else {
      // The backend returned an unsuccessful response code.
      // The response body may contain clues as to what went wrong,
      console.error(
       `Backend returned code ${error.status}, ` +
       `body was: ${error.error}`);
    }
    // return an observable with a user-facing error message
    return throwError(
      'Something bad happened; please try again later.');
  }

  xmlHttpGet(url: string): Observable<any> {
    return this.http
      .get(url, this.xmlHttpOptions)
      .pipe(switchMap((res) => bindNodeCallback(this.xmlParser.parseString)(res)));
  }

  xmlHttpPost(url: string, data: any): Observable<any> {
    return this.http
      .post(url, data, this.xmlHttpOptions)
      .pipe(switchMap(res => bindNodeCallback(this.xmlParser.parseString)(res)));
  }

  xmlHttpPut(url: string, data: any): Observable<any> {
    return this.http
      .put(url, data, this.xmlHttpOptions)
      .pipe(switchMap(res => bindNodeCallback(this.xmlParser.parseString)(res)));
  }

  xmlHttpDelete(url: string): Observable<any> {
    return this.http
      .delete(url, this.xmlHttpOptions)
      .pipe(switchMap(res => bindNodeCallback(this.xmlParser.parseString)(res)));
  }

  getCfgGroups(): Observable<any> {
    return this.http.get('assets/json/config.json');
  }

  getFaceGroups(): Observable<any> {
    return this.http.get('assets/json/face.json');
  }

  getFacePara() {
    return this.http.get('assets/json/face-para.json');
  }

  getDeviceInfo(): Observable<DeviceInfoInterface[]> {
    return this.http
      .get<DeviceInfoInterface[]>(this.serverUrl + environment.infoUrl + '?timestamp=' + new Date().getTime())
      .pipe(catchError(this.handleError));
  }

  setDeviceInfo(info: DeviceInfoInterface) {
    return this.http
      .put<DeviceInfoInterface>(this.serverUrl + environment.infoUrl, info)
      .pipe(catchError(this.handleError));
  }

  getLanInterface(): Observable<NetworkInterface> {
    return this.http
      .get<NetworkInterface>(this.serverUrl + environment.lanUrl + '?timestamp=' + new Date().getTime())
      .pipe(catchError(this.handleError));
  }

  setLanInterface(ni: NetworkInterface) {
    return this.http.put<NetworkInterface>(this.serverUrl + environment.lanUrl, ni);
  }

  getWLanInterface(): Observable<NetworkInterface> {
    return this.http
      .get<NetworkInterface>(this.serverUrl + environment.wlanUrl + '?timestamp=' + new Date().getTime())
      .pipe(catchError(this.handleError));
  }

  setWLanInterface(ni: NetworkInterface) {
    return this.http
      .put<NetworkInterface>(this.serverUrl + environment.wlanUrl, ni)
      .pipe(catchError(this.handleError));
  }

  getWifiList(): Observable<WifiInfo[]> {
    return this.http
      .get<WifiInfo[]>(this.serverUrl + '/wifi' + '?timestamp=' + new Date().getTime())
      .pipe(catchError(this.handleError));
  }

  turnOnWifi() {
    return this.http
      .put(this.serverUrl + environment.wifiOnUrl, null)
      .pipe(catchError(this.handleError));
  }

  turnOffWifi() {
    return this.http
      .put(this.serverUrl + environment.wifiOffUrl, null)
      .pipe(catchError(this.handleError));
  }

  scanWifiList() {
    return this.http
      .put(this.serverUrl + environment.scanWifiUrl, null)
      .pipe(catchError(this.handleError));
  }

  getWifiItemInterface(): Observable<WifiItemInterface[]> {
    return this.http
      .get<WifiItemInterface[]>(this.serverUrl + environment.wifiListUrl + '?timestamp=' + new Date().getTime())
      .pipe(catchError(this.handleError));
  }

  getWifiEnabledInterface(): Observable<WifiEnabledInterface> {
    return this.http
      .get<WifiEnabledInterface>(this.serverUrl + environment.wifiEnableUrl + '?timestamp=' + new Date().getTime())
      .pipe(catchError(this.handleError));
  }

  setPutWifiInterface(settingInfo: PutWifiInterface) {
    return this.http
      .put(this.serverUrl + environment.wifiEnableUrl, settingInfo)
      .pipe(catchError(this.handleError));
  }

  delteOneWifi(sId: string) {
    return this.http
      .delete(this.serverUrl + environment.wifiDeleteUrl + sId)
      .pipe(catchError(this.handleError));
  }

  getNetworkPortInterface(): Observable<NetworkPortInterface[]> {
    return this.http
      .get<NetworkPortInterface[]>(this.serverUrl + '/network-port' + '?timestamp=' + new Date().getTime())
      .pipe(catchError(this.handleError));
  }

  setNetworkPortInterface(port: NetworkPortInterface) {
    return this.http
      .put<NetworkPortInterface>(this.serverUrl + '/network-port/' + port.id, port)
      .pipe(catchError(this.handleError));
  }

  getAutoConnectWifi() {
    return this.http
      .get(this.serverUrl + environment.autoWifiUrl)
      .pipe(catchError(this.handleError));
  }

  getAudioInterface(): Observable<AudioInterface> {
    return this.http
      .get<AudioInterface>(this.serverUrl + '/audio/0' + '?timestamp=' + new Date().getTime())
      .pipe(catchError(this.handleError));
  }
  setAudioInterface(au: AudioInterface) {
    return this.http
      .put(this.serverUrl + '/audio/0', au)
      .pipe(catchError(this.handleError));
  }

  getIdFromStreamType(streamType: string) {
    let id = 0;
    if (streamType.match('mainStream')) {
      id = 0;
    } else if (streamType.match('subStream')) {
      id = 1;
    } else if (streamType.match('thirdStream')) {
      id = 2;
    }
    return id;
  }

  getVideoEncoderInterface(streamType: string): Observable<VideoEncoderInterface | Array<VideoEncoderInterface>> {
    if (streamType !== '') {
      return this.http
        .get<VideoEncoderInterface | Array<VideoEncoderInterface>>( this.serverUrl + '/video/' + this.getIdFromStreamType(streamType) + '?timestamp=' + new Date().getTime())
        .pipe(catchError(this.handleError));
    } else {
      return this.http
        .get<VideoEncoderInterface | Array<VideoEncoderInterface>>( this.serverUrl + '/video' + '?timestamp=' + new Date().getTime())
        .pipe(catchError(this.handleError));
    }
  }

  setVideoEncoderInterface(enc: VideoEncoderInterface): Observable<VideoEncoderInterface> {
    enc.id = this.getIdFromStreamType(enc.sStreamType);
    return this.http
      .put<VideoEncoderInterface>(this.serverUrl + '/video/' + enc.id, enc)
      .pipe(catchError(this.handleError));
  }

  getStreamURLInterface(): Observable<StreamURLInterface[]> {
    return this.http
      .get<StreamURLInterface[]>(this.serverUrl + '/stream-url' + '?timestamp=' + new Date().getTime())
      .pipe(catchError(this.handleError));
  }

  setStreamURLInterface(stream: StreamURLInterface) {
    return this.http
      .put<StreamURLInterface>(this.serverUrl + '/stream-url/' + stream.id, stream)
      .pipe(catchError(this.handleError));
  }

  // getContentType(url: string) {
  //   const httpPostOptions: {
  //     observe: 'response';
  //   } = {
  //     observe: 'response',
  //   };
  //   return this.http
  //     .get(url, httpPostOptions)
  //     .pipe(catchError(this.handleError),
  //       );
  // }

  getImageInterface(): Observable<ImageInterface> {
    return this.http
      .get<ImageInterface>(this.serverUrl + '/image/0' + '?timestamp=' + new Date().getTime())
      .pipe(catchError(this.handleError));
  }

  setImageInterface(image: ImageInterface) {
    return this.http
      .put(this.serverUrl + '/image/' + image.id, image)
      .pipe(catchError(this.handleError));
  }

  setImageInterfacePart(image: any, imageName: string, id: string | number) {
    return this.http
      .put(this.serverUrl + '/image/' + id + '/' + imageName, image)
      .pipe(catchError(this.handleError));
  }

  getScenarioInterface(): Observable<ScenarioInterface> {
    return this.http
      .get<ScenarioInterface>(this.serverUrl + environment.scenarioUrl + '?timestamp=' + new Date().getTime())
      .pipe(catchError(this.handleError));
  }

  setScenarioInterface(scenario: ScenarioInterface) {
    return this.http
      .put<ScenarioInterface>(this.serverUrl + environment.scenarioUrl, scenario)
      .pipe(catchError(this.handleError));
  }

  getOsdOverplaysInterface(): Observable<OsdOverplaysInterface> {
    return this.http
      .get<OsdOverplaysInterface>(this.serverUrl + environment.osdUrl + '?timestamp=' + new Date().getTime())
      .pipe(catchError(this.handleError));
  }

  setOsdOverplaysInterface(osdOverplays: OsdOverplaysInterface): Observable<OsdOverplaysInterface> {
    return this.http
      .put<OsdOverplaysInterface>(this.serverUrl + environment.osdUrl, osdOverplays)
      .pipe(catchError(this.handleError));
  }

  getPrivacyMaskInterface(): Observable<PrivacyMaskInterface> {
    return this.http
      .get<PrivacyMaskInterface>(this.serverUrl + environment.privacyMaskUrl + '?timestamp=' + new Date().getTime())
      .pipe(catchError(this.handleError),
        );
  }

  setPrivacyMaskInterface(privacyMask: PrivacyMaskInterface) {
    return this.http
      .put(this.serverUrl + environment.privacyMaskUrl, privacyMask)
      .pipe(catchError(this.handleError));
  }

  getPictureMaskInterface(): Observable<PictureMaskInterface> {
    return this.http
      .get<PictureMaskInterface>(this.serverUrl + environment.picMaskUrl + '?timestamp=' + new Date().getTime())
      .pipe(catchError(this.handleError));
  }

  setPictureMaskInterface(picMask: PictureMaskInterface) {
    return this.http
      .put(this.serverUrl + environment.picMaskUrl, picMask)
      .pipe(catchError(this.handleError));
  }

  postImage(img: any) {
    const httpPostOptions = {headers: new HttpHeaders({})};
    let fd = new FormData();
    fd.append('file', img);
    return this.http
      .post(this.serverUrl + environment.pictureMaskUrl, fd, httpPostOptions)
      .pipe(catchError(this.handleError));
  }

  putRebootSignal() {
    return this.http
      .put(this.serverUrl + environment.rebootUrl, null)
      .pipe(timeout(1000), catchError(this.handleError));
  }

  putPreFactoryResetSignal() {
    return this.http
      .put(this.serverUrl + environment.preFactoryResetUrl, null)
      .pipe(timeout(1000), catchError(this.handleError));
  }

  putFactoryResetSignal() {
    return this.http
      .put(this.serverUrl + environment.factoryResetUrl, null)
      .pipe(timeout(1000), catchError(this.handleError));
  }

  getSysLogUrl() {
    return this.http
      .put(this.serverUrl + environment.sysLogUrl, null)
      .pipe(catchError(this.handleError));
  }

  getSysDbUrl() {
    return this.http
      .put(this.serverUrl + environment.sysDbUrl, null)
      .pipe(catchError(this.handleError));
  }

  putDeviceInfo(dbFile: any) {
    const httpPostOptions = {headers: new HttpHeaders({})};
    let fd = new FormData();
    fd.append('file', dbFile);
    return this.http
      .put(this.serverUrl + environment.infoDbUrl, fd, httpPostOptions)
      .pipe(timeout(1000), catchError(this.handleError));
  }

  reboot4DB() {
    return this.http
      .put(this.serverUrl + environment.reboot4DbUrl, null)
      .pipe(timeout(1000), catchError(this.handleError));
  }

  getUpgradeNum() {
    const httpPutOptions: {
      observe: 'response';
  } = {
      observe: 'response',
    };
    return this.http
      .put(this.serverUrl + environment.upgradeNumUrl, null, httpPutOptions)
      .pipe(catchError(this.handleError));
  }

  putUpgradeInfo(id: string | number, data: any, headval: string) {
    const httpPostOptions: {
      headers?: HttpHeaders | {
        [header: string]: string | string[];
      };
      observe: 'body';
      responseType?: 'json';
    } = {
      headers: new HttpHeaders({'Content-Range': headval}),
      observe: 'body',
      responseType: 'json',
    };
    return this.http
      .put(this.serverUrl + environment.upgradeUrl + id, data, httpPostOptions)
      .pipe(catchError(this.handleError));
  }

  putUpgradeEndSignal(order: any) {
    return this.http
      .put(this.serverUrl + environment.upgradeEndUrl + order, null)
      .pipe(timeout(1000), catchError(this.handleError));
  }

  getFreeRoom(): Observable<FreeRoomInterface> {
    return this.http
      .get<FreeRoomInterface>(this.serverUrl + environment.freeRoomUrl + '?timestamp=' + new Date().getTime())
      .pipe(catchError(this.handleError));
  }

  getQuotaInterface(id: number): Observable<QuotaInterface> {
    return this.http
      .get<QuotaInterface>(this.serverUrl + environment.quotaUrl + id + '?timestamp=' + new Date().getTime())
      .pipe(catchError(this.handleError));
  }

  setQuotaInterface(id: number, qi: QuotaInterface) {
    return this.http
      .put<QuotaInterface>(this.serverUrl + environment.quotaUrl + id, qi)
      .pipe(catchError(this.handleError));
  }

  getHardDiskManagementInterface(): Observable<HardDiskManagementInterface[]> {
    return this.http
      .get<HardDiskManagementInterface[]>(this.serverUrl + environment.hddListUrl + '?timestamp=' + new Date().getTime())
      .pipe(catchError(this.handleError));
  }

  formatHardDisk(id: number) {
    return this.http
      .post(this.serverUrl + environment.formatUrl + id, null)
      .pipe(catchError(this.handleError));
  }

  getDownloadList(data: any): Observable<DownloadListInterface> {
    return this.http
      .put<DownloadListInterface>(this.serverUrl + environment.searchUrl, data)
      .pipe(catchError(this.handleError));
  }

  deleteReview(data: any) {
    return this.http
      .post(this.serverUrl + environment.reviewDeleteUrl, data)
      .pipe(catchError(this.handleError));
  }

  getTTLRoiList(): Observable<RoiInterface> {
    return this.http
      .get<RoiInterface>(this.serverUrl + environment.ttlRoiUrl + '?timestamp=' + new Date().getTime())
      .pipe(catchError(this.handleError));
  }

  getPartRoiList(type: string, region: number): Observable<RoiPartInterface> {
    return this.http
      .get<RoiPartInterface>(this.serverUrl + environment.ttlRoiUrl + '/' + type + '/' + region + '?timestamp=' + new Date().getTime())
      .pipe(catchError(this.handleError));
  }

  putPartRoiList(type: string, region: number, data: any) {
    return this.http
      .put(this.serverUrl + environment.ttlRoiUrl + '/' + type + '/' + region, data)
      .pipe(catchError(this.handleError));
  }

  getRegionCropInterface(): Observable<RegionCropInterface> {
    return this.http
      .get<RegionCropInterface>(this.serverUrl + environment.regionCropUrl + '?timestamp=' + new Date().getTime())
      .pipe(catchError(this.handleError));
  }

  putRegionCropInterface(data: RegionClipInterface) {
    return this.http
      .put(this.serverUrl + environment.regionCropUrl, data)
      .pipe(catchError(this.handleError));
  }

  getMotionRegionInterface(): Observable<MotionRegionInterface> {
    return this.http
      .get<MotionRegionInterface>(this.serverUrl + environment.motionRegionUrl + '?timestamp=' + new Date().getTime())
      .pipe(catchError(this.handleError));
  }

  putMotionRegionInterface(data: MotionRegionInterface) {
    return this.http
      .put(this.serverUrl + environment.motionRegionUrl, data)
      .pipe(catchError(this.handleError));
  }

  getLinkageInterface(path: string): Observable<LinkageInterface> {
    return this.http
      .get<LinkageInterface>(this.serverUrl + environment.motionLinkageUrl + path + '?timestamp=' + new Date().getTime())
      .pipe(catchError(this.handleError));
  }

  putLinkageInterface(data: LinkageInterface, path: string) {
    return this.http
      .put(this.serverUrl + environment.motionLinkageUrl + path, data)
      .pipe(catchError(this.handleError));
  }

  getTimZoneInfo(): Observable<TimeZoneInterface[]> {
    return this.http
      .get<TimeZoneInterface[]>(this.serverUrl + environment.timezoneUrl + '?timestamp=' + new Date().getTime())
      .pipe(catchError(this.handleError));
  }

  getNtpInfo(): Observable<NtpInfoInterface> {
    return this.http
      .get<NtpInfoInterface>(this.serverUrl + environment.ntpInfoUrl + '?timestamp=' + new Date().getTime())
      .pipe(catchError(this.handleError));
  }

  putNtpInfo(data: NtpInfoInterface) {
    return this.http
      .put(this.serverUrl + environment.ntpInfoUrl, data)
      .pipe(catchError(this.handleError));
  }

  getNowTime(): Observable<NowTimeInterface> {
    return this.http
      .get<NowTimeInterface>(this.serverUrl + environment.ntpTimUrl + '?timestamp=' + new Date().getTime())
      .pipe(catchError(this.handleError));
  }

  putNowTime(data: NowTimeInterface) {
    return this.http
      .put(this.serverUrl + environment.ntpTimUrl, data)
      .pipe(catchError(this.handleError));
  }

  getMemberListInterface(): Observable<MemberListInterface[]> {
    return this.http
      .get<MemberListInterface[]>(this.serverUrl + environment.memberListUrl + '?timestamp=' + new Date().getTime())
      .pipe(catchError(this.handleError));
  }

  deleteMemberFace(id: string) {
    return this.http
      .delete(this.serverUrl + environment.memberFaceUrl + id)
      .pipe(catchError(this.handleError));
  }

  putAddMemberInterface(data: AddMemberInterface, id: string = ''): Observable<MemberListInterface> {
    if (id) {
      return this.http
      .post<MemberListInterface>(this.serverUrl + environment.memberFaceUrl + '/' + id, data)
      .pipe(catchError(this.handleError));
    } else {
      return this.http
      .post<MemberListInterface>(this.serverUrl + environment.memberFaceUrl, data)
      .pipe(catchError(this.handleError));
    }
  }

  getAddMemberInterface(id: string | number): Observable<MemberListInterface> {
    return this.http
      .get<MemberListInterface>(this.serverUrl + environment.memberFaceUrl + '/' + id + '?timestamp=' + new Date().getTime())
      .pipe(catchError(this.handleError));
  }

  putAddMemberImage(img: any, path: string) {
    let fd = new FormData();
    fd.append('file', img);
    return this.http
      .post(this.serverUrl + environment.memberAvatarUrl + path, fd)
      .pipe(catchError(this.handleError));
    const req = new HttpRequest('PUT', this.serverUrl + environment.memberAvatarUrl + path, fd, {
      reportProgress: true
    });
    return this.http.request(req).pipe(
      last(),
      catchError(this.handleError)
    );
  }

  private getEventMessage(event: HttpEvent<any>, file: File) {
    switch (event.type) {
      case HttpEventType.Sent:
        console.log(new Date(), `Uploading file "${file.name}" of size ${file.size}.`);
        return `Uploading file "${file.name}" of size ${file.size}.`;

      case HttpEventType.UploadProgress:
        // Compute and show the % done:
        const percentDone = Math.round(100 * event.loaded / event.total);
        console.log(new Date(), `File "${file.name}" is ${percentDone}% uploaded.`, event);
        return `File "${file.name}" is ${percentDone}% uploaded.`;

      case HttpEventType.Response:
        console.log(new Date(), `File "${file.name}" was completely uploaded!`);
        return `File "${file.name}" was completely uploaded!`;

      default:
        console.log(new Date(), `File "${file.name}" surprising upload event: ${event.type}.`);
        return `File "${file.name}" surprising upload event: ${event.type}.`;
    }
  }

  putMemberSearchCondition(data: SearchCondition): Observable<MemberSearchResult> {
    return this.http
      .put<MemberSearchResult>(this.serverUrl + environment.memberSearchUrl, data)
      .pipe(catchError(this.handleError));
  }

  resetFaceMember() {
    return this.http
      .post(this.serverUrl + environment.resetFaceUrl, null)
      .pipe(timeout(360000000), catchError(this.handleError));
  }

  resetFaceSnapshot() {
    return this.http
      .post(this.serverUrl + environment.resetSnapUrl, null)
      .pipe(timeout(360000000), catchError(this.handleError));
  }

  resetFaceControl() {
    return this.http
      .post(this.serverUrl + environment.resetControlUrl, null)
      .pipe(timeout(360000000), catchError(this.handleError));
  }

  checkFaceFeature(id: number | string): Observable<Array<object>> {
    return this.http
      .post<Array<object>>(this.serverUrl + environment.checkFaceFail + '?id=' + id, null)
      .pipe(catchError(this.handleError));
  }

  getLastFaceId() {
    return this.http
      .get(this.serverUrl + environment.lastFaceUrl + '?timestamp=' + new Date().getTime())
      .pipe(catchError(this.handleError));
  }

  putSnapSearch(data: any): Observable<SnapSearch> {
    return this.http
      .put<SnapSearch>(this.serverUrl + environment.snapSearchUrl, data)
      .pipe(catchError(this.handleError));
  }

  getObject(url: string) {
    return this.http
      .get(url + '?timestamp=' + new Date().getTime())
      .pipe(catchError(this.handleError));
  }

  putSnapImg(path: string, data: any) {
    return this.http
      .put(this.serverUrl + environment.snapImgUrl + path, data)
      .pipe(catchError(this.handleError));
  }

  putMemberNameSearch(data: any): Observable<MemberSearchResult> {
    return this.http
      .put<MemberSearchResult>(this.serverUrl + environment.memberSearchByNameUrl, data)
      .pipe(catchError(this.handleError));
  }

  getControlSearchResult(data: any): Observable<ControlSearchResult> {
    return this.http
      .put<ControlSearchResult>(this.serverUrl + environment.controlConditionSearchUrl, data)
      .pipe(catchError(this.handleError));
  }

  getControlNameSearchResult(data: any): Observable<ControlSearchResult> {
    return this.http
      .put<ControlSearchResult>(this.serverUrl + environment.controlNameSearchUrl, data)
      .pipe(catchError(this.handleError));
  }

  deleteControlList(deleteList: Array<string>) {
    const options: {
      params?: HttpParams | {
          [param: string]: string | string[];
      };
    } = {
      params: {
        delete: deleteList
      }
    };
    return this.http
      .delete(this.serverUrl + environment.controlUrl, options)
      .pipe(catchError(this.handleError));
  }

  deleteSnapList(deleteList: Array<string>) {
    const options: {
      params?: HttpParams | {
          [param: string]: string | string[];
      };
    } = {
      params: {
        delete: deleteList
      }
    };
    return this.http
      .delete(this.serverUrl + environment.snapSearchUrl, options)
      .pipe(catchError(this.handleError));
  }

  getFaceParaInterface(): Observable<FaceParaInterface> {
    return this.http
      .get<FaceParaInterface>(this.serverUrl + environment.faceParaUrl + '?timestamp=' + new Date().getTime())
      .pipe(catchError(this.handleError));
  }

  putFaceParaInterface(data: FaceParaInterface): Observable<FaceParaInterface> {
    return this.http
      .put<FaceParaInterface>(this.serverUrl + environment.faceParaUrl, data)
      .pipe(catchError(this.handleError));
  }

  postTakePhotoSignal() {
    return this.http
      .post(this.serverUrl + environment.takePhotoUrl, null)
      .pipe(catchError(this.handleError));
  }

  getIntrusionRegionInfo(): Observable<IntrusionRegion> {
    return this.http
      .get<IntrusionRegion>(this.serverUrl + environment.intrusionRegionUrl + '?timestamp=' + new Date().getTime())
      .pipe(catchError(this.handleError));
  }

  putIntrusionRegionInfo(data: RegionalInvasion): Observable<IntrusionRegion> {
    return this.http
      .put<IntrusionRegion>(this.serverUrl + environment.intrusionRegionUrl, data)
      .pipe(catchError(this.handleError));
  }

  getScreenshotTimingInfo(): Observable<TimingInterface> {
    return this.http
      .get<TimingInterface>(this.serverUrl + environment.planTimingUrl + '?timestamp=' + new Date().getTime())
      .pipe(catchError(this.handleError));
  }

  getScreenshotEventInfo(): Observable<EventShotInterface> {
    return this.http
      .get<EventShotInterface>(this.serverUrl + environment.planEventUrl + '?timestamp=' + new Date().getTime())
      .pipe(catchError(this.handleError));
  }

  putScreenshotTimingInfo(data: any): Observable<TimingInterface> {
    return this.http
      .put<TimingInterface>(this.serverUrl + environment.planTimingUrl, data)
      .pipe(catchError(this.handleError));
  }

  putScreenshotEventInfo(data: any): Observable<EventShotInterface> {
    return this.http
      .put<EventShotInterface>(this.serverUrl + environment.planEventUrl, data)
      .pipe(catchError(this.handleError));
  }

  // getDefaultPara(id: number|string) {
  //   const id2path = {
  //     isp: 5,
  //   };
  //   const path = id2path[id] ? id2path[id] : id;
  //   return this.http
  //     .get(this.serverUrl + environment.parameterUrl + path)
  //     .pipe(catchError(this.handleError));
  // }

  getDefaultPara(id: number|string) {
    const id2path = {
      0: 'screenshot',
      1: 'screenshot-schedule',
      2: 'video-plan-schedule',
      3: 'smart-cover',
      4: 'video-encoder',
    };
    const path = id2path[id] ? id2path[id] : id;
    return this.http
      .get(this.serverUrl + environment.parameterUrl + path + '?timestamp=' + new Date().getTime())
      .pipe(catchError(this.handleError));
  }

  getSchedule(id: number | string) {
    const id2path = {
      0: 'motion',
      1: 'intrusion',
      2: 'video-plan',
      3: 'screenshot',
    };
    const path = id2path[id] ? id2path[id] : id;
    return this.http
      .get(this.serverUrl + environment.scheduleUrl + path + '?timestamp=' + new Date().getTime())
      .pipe(catchError(this.handleError));
  }

  setSchedule(id: number | string, data: any) {
    const id2path = {
      0: 'motion',
      1: 'intrusion',
      2: 'video-plan',
      3: 'screenshot',
    };
    const path = id2path[id] ? id2path[id] : id;
    return this.http
      .put(this.serverUrl + environment.scheduleUrl + path, data)
      .pipe(catchError(this.handleError));
  }

  putLoginInfo(data: any, para: string = ''): Observable<LoginStatus> {
    return this.http
      .put<LoginStatus>(this.serverUrl + environment.loginUrl + para, data)
      .pipe(catchError(this.handleError));
  }

  getUserList(): Observable<UserCell[]> {
    return this.http
      .get<UserCell[]>(this.serverUrl + environment.loginUrl + '?timestamp=' + new Date().getTime())
      .pipe(catchError(this.handleError));
  }

  modifyUser(data: UserForm): Observable<UserStatus> {
    return this.http
      .put<UserStatus>(this.serverUrl + environment.modifyLoginUrl, data)
      .pipe(catchError(this.handleError));
  }

  addUser(data: UserForm): Observable<UserStatus> {
    return this.http
      .put<UserStatus>(this.serverUrl + environment.addLoginUrl, data)
      .pipe(catchError(this.handleError));
  }

  deleteUser(data: UserForm): Observable<UserStatus> {
    return this.http
    .put<UserStatus>(this.serverUrl + environment.deleteLoginUrl, data)
    .pipe(catchError(this.handleError));
  }

  getStoragePath() {
    return this.http
      .get(this.serverUrl + environment.storagePathUrl + '?timestamp=' + new Date().getTime())
      .pipe(catchError(this.handleError));
  }

  getAdvanceEnco(): Observable<AdvanceEncCell> {
    return this.http
      .get<AdvanceEncCell>(this.serverUrl + environment.advancedEncUrl + '?timestamp=' + new Date().getTime())
      .pipe(catchError(this.handleError));
  }

  putAdvanceEnco(data: AdvanceEncPutInfo) {
    const urlPath = data.sFunction;
    return this.http
      .put(this.serverUrl + environment.advancedEncUrl + '/' + urlPath, data)
      .pipe(catchError(this.handleError));
  }

  getBatchInputBuffer() {
    return this.http
      .get(this.serverUrl + environment.batchInputBufferUrl + '?timestamp=' + new Date().getTime())
      .pipe(catchError(this.handleError));
  }

  sendRecordSignal(recording: boolean) {
    if (recording) {
      return this.http
        .put(this.serverUrl + environment.startRecordUrl, null)
        .pipe(catchError(this.handleError));
    } else {
      return this.http
        .put(this.serverUrl + environment.stopRecordUrl, null)
        .pipe(catchError(this.handleError));
    }
  }

  getOverlaySnap() {
    return this.http
      .get(this.serverUrl + environment.overlaySnapUrl + '?timestamp=' + new Date().getTime())
      .pipe(catchError(this.handleError));
  }

  setOverlaySnap(data: any) {
    return this.http
      .put(this.serverUrl + environment.overlaySnapUrl, data)
      .pipe(catchError(this.handleError));
  }

  getRecordStatus() {
    return this.http
      .get(this.serverUrl + environment.recordStatusUrl + '?timestamp=' + new Date().getTime())
      .pipe(catchError(this.handleError));
  }

  getPlanAdvancePara(id: number | string) {
    return this.http
      .get(this.serverUrl + environment.planAdvancePara + '/' + id + '?timestamp=' + new Date().getTime())
      .pipe(catchError(this.handleError));
  }

  setPlanAdvancePara(id: number | string, data: any) {
    return this.http
      .put(this.serverUrl + environment.planAdvancePara + '/' + id, data)
      .pipe(catchError(this.handleError));
  }

  getGatePeripherals(): Observable<GateInterface> {
    return this.http
      .get<GateInterface>(this.serverUrl + environment.gatePeripheralsUrl + '?timestamp=' + new Date().getTime())
      .pipe(catchError(this.handleError));
  }

  setGatePeripherals(data: GateInterface): Observable<GateInterface> {
    return this.http
      .put<GateInterface>(this.serverUrl + environment.gatePeripheralsUrl, data)
      .pipe(catchError(this.handleError));
  }

  getFillLightPeripherals(): Observable<ScreenCfgInterface> {
    return this.http
      .get<ScreenCfgInterface>(this.serverUrl + environment.fillLightpheralsUrl + '?timestamp=' + new Date().getTime())
      .pipe(catchError(this.handleError));
  }

  setFillLightPeripherals(data: ScreenCfgInterface): Observable<ScreenCfgInterface> {
    return this.http
      .put<ScreenCfgInterface>(this.serverUrl + environment.fillLightpheralsUrl, data)
      .pipe(catchError(this.handleError));
  }

  getFtpConfig(): Observable<FTPInterface> {
    return this.http
      .get<FTPInterface>(this.serverUrl + environment.ftpUrl + '?timestamp=' + new Date().getTime())
      .pipe(catchError(this.handleError));
  }

  setFtpCofig(data: FTPInterface): Observable<FTPInterface> {
    return this.http
      .put<FTPInterface>(this.serverUrl + environment.ftpUrl, data)
      .pipe(catchError(this.handleError));
  }

  testFtp(data: FTPInterface) {
    return this.http
      .put(this.serverUrl + environment.ftpTestUrl, data)
      .pipe(catchError(this.handleError));
  }
}
