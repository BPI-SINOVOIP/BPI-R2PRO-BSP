import { Component, OnInit, ViewChild, ElementRef, Renderer2, AfterContentInit, AfterViewInit } from '@angular/core';

@Component({
  selector: 'app-click-tip',
  templateUrl: './click-tip.component.html',
  styleUrls: ['./click-tip.component.scss']
})
export class ClickTipComponent implements OnInit {

  constructor(
    private el: ElementRef,
    private Re2: Renderer2,
  ) { }

  title = 'operationMenu';
  private modifyFunc: () => {};
  private deleteFunc: () => {};

  ngOnInit(): void {
  }

  setXY(x: number, y: number) {
    this.Re2.setStyle(this.el.nativeElement.querySelector('.modal'), 'top', y + 'px');
    this.Re2.setStyle(this.el.nativeElement.querySelector('.modal'), 'left', x + 'px');
  }

  onNo() {
    this.Re2.setStyle(this.el.nativeElement.querySelector('.modal'), 'display', 'none');
  }

  onShow() {
    this.Re2.setStyle(this.el.nativeElement.querySelector('.modal'), 'display', 'block');
  }

  onModify() {
    this.modifyFunc();
  }

  onDelete() {
    this.deleteFunc();
  }
}
