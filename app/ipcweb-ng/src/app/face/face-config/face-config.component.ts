import { Component, OnInit } from '@angular/core';
import { ActivatedRoute } from '@angular/router';
import { FaceSettingOptions } from '../shared/para-setting/FaceParaInterface';

@Component({
  selector: 'app-face-config',
  templateUrl: './face-config.component.html',
  styleUrls: ['./face-config.component.scss']
})
export class FaceConfigComponent implements OnInit {

  selectedMenuItem: string;
  activeTab: string = '';
  subMenuItems: Array<string>;
  subMenuItemsEncodeParam: Array<string> = [];
  subMenuItemsRoi: Array<string> = [];
  subMenuItemsCrop: Array<string> = [];

  constructor(
    private route: ActivatedRoute,
  ) { }

  options: FaceSettingOptions = {
    isIPC: true,
  };

  ngOnInit(): void {
    this.subMenuItems = this.subMenuItemsEncodeParam;
    this.route.queryParamMap.subscribe(params => {
      if (params.get('menu') === 'ParaConfig') {
        this.selectedMenuItem = 'ParaConfig';
        this.subMenuItems = this.subMenuItemsEncodeParam;
      } else {
        this.selectedMenuItem = 'ParaConfig';
        this.subMenuItems = this.subMenuItemsEncodeParam;
      }
    });
  }

}
