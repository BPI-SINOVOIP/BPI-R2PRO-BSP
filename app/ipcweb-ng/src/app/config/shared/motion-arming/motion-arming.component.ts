import { Component, OnInit, AfterViewInit, OnDestroy, ViewChild, ElementRef, Renderer2, HostListener } from '@angular/core';
import { FormBuilder, Validators, FormControl } from '@angular/forms';

@Component({
  selector: 'app-motion-arming',
  templateUrl: './motion-arming.component.html',
  styleUrls: ['./motion-arming.component.scss']
})
export class MotionArmingComponent implements OnInit {

  @ViewChild('table', {static: true}) tableChild: any;
  constructor() { }

  ngOnInit(): void {
  }

  resizeTable() {
    this.tableChild.reshapeCanvas();
  }
}
