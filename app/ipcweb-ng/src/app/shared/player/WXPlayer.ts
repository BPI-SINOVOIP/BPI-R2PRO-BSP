import { ViewChild, ElementRef} from '@angular/core';
import Logger from 'src/app/logger';
import { Subject } from 'rxjs';
/// <refrence path="assets/wxplayer/wxplayer.d.ts" />;

export default class  WXPlayerComponent {

  @ViewChild('canvasvideo', { static: true })
  canvasVideo: ElementRef<HTMLCanvasElement>;

  wxplayer: any;
  displayUrl: string;

  logger = new Logger('WXplayerComponent');

  initPlayer(options, playFunc) {
    let sub = new Subject<number>();
    WXInlinePlayer.ready(options).then((wxplayer) => {
      this.logger.log("WXInlinePlayer is ready!");
      this.wxplayer = wxplayer;
      sub.next(1);
      // this.setPlayerListener(wxplayer);
      // this.setListener(wxplayer);
    });
    return sub;
  }

  setListener(wxplayer){
    let status = document.getElementById('status');
    wxplayer.on("loadError", (e) => {
      this.logger.error(e, "loadError");
    });
    wxplayer.on("loadSuccess", () => {
      this.logger.log("loadSuccess");
    });
    wxplayer.on("mediaInfo", (mediaInfo) => {
      this.logger.log(mediaInfo, "mediaInfo");
    });
    wxplayer.on("play", () => {
      status.innerHTML = "状态: 开播了";
    });
    wxplayer.on("buffering", () => {
      status.innerHTML = "状态: 加载中";
    });
    wxplayer.on("playing", () => {
      status.innerHTML = "状态: 播放中";
    });
    wxplayer.on("paused", () => {
      status.innerHTML = "状态：已暂停";
    });
    wxplayer.on("resumed", () => {
      status.innerHTML = "状态：已继续";
    });
    wxplayer.on("stopped", () => {
      status.innerHTML = "状态：已停止";
    });
    wxplayer.on("ended", () => {
      status.innerHTML = "状态：已放完";
    });
    wxplayer.on("timeUpdate", (timestamp) => {
      //document.getElementById('time').innerHTML = " 进度:" + Math.ceil((100 * timestamp) / wxplayer.getDuration()) + "%";
      document.getElementById('duration').innerHTML = " 可播放时长:" + Math.ceil(wxplayer.getAvaiableDuration() / 1000) + "s";
      // console.log("getAvaiableDuration : " + wxplayer.getDuration());
    });
    wxplayer.on("performance", ({ averageDecodeCost, averageUnitDuration }) => {
      document.getElementById('performance').innerHTML =
        " 解码平均:" +
        Math.floor(averageDecodeCost) +
        "ms, 单元平均:" +
        Math.floor(averageUnitDuration) +
        "ms";
    });
  }

  setPlayerListener(wxplayer){
    wxplayer.on("loadError", (e) => {
      this.logger.error(e, "loadError");
    });
    wxplayer.on("loadSuccess", () => {
      this.logger.log("loadSuccess");
    });
    wxplayer.on("mediaInfo", (mediaInfo) => {
      this.logger.log(mediaInfo, "mediaInfo");
    });
    wxplayer.on("play", () => {
      this.logger.log("play");
    });
    wxplayer.on("buffering", () => {
      this.logger.log("buffering");
    });
    wxplayer.on("playing", () => {
      this.logger.log("playing");
    });
    wxplayer.on("paused", () => {
      this.logger.log("paused");
    });
    wxplayer.on("resumed", () => {
      this.logger.log("resumed");
    });
    wxplayer.on("stopped", () => {
      this.logger.log("stopped");
    });
    wxplayer.on("ended", () => {
      this.logger.log("ended");
    });
    wxplayer.on("timeUpdate", (timestamp) => {
      // this.logger.log(" 可播放时长:" + Math.ceil(wxplayer.getAvaiableDuration() / 1000) + "s");
    });
    wxplayer.on("performance", ({ averageDecodeCost, averageUnitDuration }) => {
      this.logger.log(" 解码平均:" +
        Math.floor(averageDecodeCost) +
        "ms, 单元平均:" +
        Math.floor(averageUnitDuration) +
        "ms");
    });
  }

  play() {
    this.logger.log("wxplayer.play()");
    this.wxplayer.play();
  }

  stop() {
    this.logger.log("wxplayer.stop()");
    this.wxplayer.stop();
  }

  pause() {
    this.logger.log("wxplayer.pause()");
    this.wxplayer.pause();
  }

  resume() {
    WXInlinePlayer.resume();
  }

  mute() {
    WXInlinePlayer.mute();
  }

  destroy() {
    this.logger.log("wxplayer.destroy()");
    this.wxplayer.destroy();
  }

  changeVolume(volume : number) {
    WXInlinePlayer.volume(volume);
  }
}
