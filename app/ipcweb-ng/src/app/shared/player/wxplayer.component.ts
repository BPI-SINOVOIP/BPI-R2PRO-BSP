import { Component, OnInit, AfterViewInit, OnDestroy, ViewChild, ElementRef, Renderer2, HostListener, Input } from '@angular/core';
import Logger from 'src/app/logger';
/// <refrence path="assets/wxplayer/wxplayer.d.ts" />;

@Component({
  selector: 'app-wxplayer',
  templateUrl: './wxplayer.component.html',
  styleUrls: ['./wxplayer.component.scss']
})
export class WXPlayerComponent implements OnInit, AfterViewInit, OnDestroy {

  @Input() option: any;

  @ViewChild('canvasvideo', { static: true })
  canvasVideo: ElementRef<HTMLCanvasElement>;

  constructor(
    private Re2: Renderer2,
    private el: ElementRef,
  ) { }

  // isH265: boolean = false;
  // asmUrl: string = "./prod." + (this.isH265 ? "h265" : "all") + ".asm.combine.js";
  // wasmUrl: string = "./prod." + (this.isH265 ? "h265" : "all") + ".wasm.combine.js";
  // url: string;
  wxplayer: any;
  displayUrl: string;

  logger = new Logger('WXplayerComponent');

  ngOnInit() {}

  ngAfterViewInit() {}

  ngOnDestroy() {}

  bigBtnPlay() {
    this.initPlayer(this.getOptions()).then(()=>{
      this.logger.info('initPlayer');
    });
  }

  getOptions() {
    let isH265 = false;
    this.logger.info('displayUrl is ' + this.displayUrl);
    return {
      asmUrl: "./assets/wxplayer/prod." + (isH265 ? "h265" : "all") + ".asm.combine.js",
      wasmUrl: "./assets/wxplayer/prod." + (isH265 ? "h265" : "all") + ".wasm.combine.js",
      url: this.displayUrl,
      $container: document.getElementById('canvas'),
      hasVideo: true,
      hasAudio: false,
      volume: 1.0,
      muted: false,
      autoplay: true,
      loop: false,
      isLive: true,
      chunkSize: 64 * 1024, // 加载/解析块大小
      preloadTime: 500, // 预加载时间，与延时无关
      bufferingTime: 100, // 缓冲时间，与延时有关
      cacheSegmentCount: 256, // 内部最大的缓存帧数量，默认256，太小会一直在playing和buffering中切换，导致卡顿
      customLoader: null,
    };
  }

  initPlayer(options) {
    return WXInlinePlayer.ready(options).then((wxplayer) => {
      console.log("WXInlinePlayer is ready!");
      this.setPlayerUI();
      this.setListener(wxplayer);
      this.setEventForBtn(wxplayer);
    });
  }

  setPlayerUI() {
    // let chked = (<HTMLInputElement>document.getElementById('iIsLive')).checked;
    // console.log("chked is " + chked);
    // let isShow = chked ? "inline" : "none";
    // this.Re2.setStyle(document.getElementById('pauseBtn'), 'display', isShow);
    // this.Re2.setStyle(document.getElementById('resumeBtn'), 'display', isShow);
    // this.Re2.setStyle(document.getElementById('footer'), 'display', "block");
  }

  setListener(wxplayer){
    let status = document.getElementById('status');
    wxplayer.on("loadError", (e) => {
      console.error("loadError", e);
    });
    wxplayer.on("loadSuccess", () => {
      console.log("loadSuccess");
    });
    wxplayer.on("mediaInfo", (mediaInfo) => {
      console.log("mediaInfo", mediaInfo);
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

  setEventForBtn(wxplayer){
    //addEventListener()添加的匿名函数无法通过removeEventListener()移除,会造成内存泄漏
    //所以要对每个事件处理函数命名
    let fnPlay = () => {
      wxplayer.play();
    };
    document.getElementById('playBtn').addEventListener("click", fnPlay);

    // let fnPause = () => wxplayer.pause();
    // document.getElementById('pauseBtn').addEventListener("click", fnPause);

    // let fnResume = () => wxplayer.resume();
    // document.getElementById('resumeBtn').addEventListener("click", fnResume);

    // let fnMute = () => wxplayer.mute(!wxplayer.mute());
    // document.getElementById('muteBtn').addEventListener("click", fnMute);

    // let fnChange = (ev) => wxplayer.volume(ev.target.value);
    // document.querySelector("input[name=volume]").addEventListener("change", fnChange);

    let fnStop = () => wxplayer.stop();
    document.getElementById('stopBtn').addEventListener("click", fnStop);

    // let fnReset = () => {
    //   if (wxplayer) {
    //     wxplayer.stop();
    //     wxplayer = null;
    //   }
    //   //you must remove all eventListeners to prevent from memory leaking
    //   document.getElementById('playBtn').removeEventListener("click", fnPlay);
    //   document.getElementById('pauseBtn').removeEventListener("click", fnPause);
    //   document.getElementById('resumeBtn').removeEventListener("click", fnResume);
    //   document.getElementById('muteBtn').removeEventListener("click", fnMute);
    //   document.querySelector("input[name=volume]").removeEventListener("click", fnChange);
    //   document.getElementById('stopBtn').removeEventListener("click", fnStop);
    //   document.getElementById('resetBtn').removeEventListener("click", fnReset);
    //   //reload wxplayer
    //   this.initPlayer(this.getOptions());
    // };
    // document.getElementById('resetBtn').addEventListener("click", fnReset);
  }

}
