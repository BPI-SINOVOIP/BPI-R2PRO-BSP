import { Component, OnInit, ElementRef, Renderer2 } from '@angular/core';
import { ActivatedRoute } from '@angular/router';

@Component({
  selector: 'app-right-bottom-tips',
  templateUrl: './right-bottom-tips.component.html',
  styleUrls: ['./right-bottom-tips.component.scss']
})
export class RightBottomTipsComponent implements OnInit {

  constructor(
    private Re2: Renderer2,
    private el: ElementRef,
    private rt: ActivatedRoute
  ) { }

  tipContent: string = 'saveSuccess';
  private isShow: boolean = false;
  private closeTime: any;

  ngOnInit(): void {
    this.rt.queryParamMap.subscribe(params => {
      this.onClose();
    });
  }

  onShow(tip: string, closeTime: number = 1500) {
    this.show(tip);
    if (closeTime > 0) {
      this.waitClose(closeTime);
    } else if (closeTime < 0) {
      this.onClose();
    }
  }

  show(tip: string) {
    this.isShow = true;
    this.tipContent = tip;
    this.Re2.setStyle(this.el.nativeElement.querySelector('.modal-content'), 'display', 'block');
  }

  waitClose(second: number) {
    this.closeTime = new Date().getTime() + second;
    const that = this;
    setTimeout(() => {
      that.checkClose();
    }, second);
  }

  checkClose() {
    if (!this.isShow) {
      return;
    }
    const nowTime = new Date().getTime();
    if (nowTime >= this.closeTime) {
      this.onClose();
    } else {
      const that = this;
      const watiTime = this.closeTime - nowTime;
      setTimeout(() => {
        that.checkClose();
      }, watiTime);
    }
  }

  onClose() {
    if (!this.isShow) {
      return;
    }
    this.isShow = false;
    this.Re2.setStyle(this.el.nativeElement.querySelector('.modal-content'), 'display', 'none');
  }

}
