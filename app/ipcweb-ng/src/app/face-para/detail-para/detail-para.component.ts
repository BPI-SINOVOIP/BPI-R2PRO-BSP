import { Component, OnInit } from '@angular/core';
import { ActivatedRoute } from '@angular/router';

import { FaceSettingOptions } from '../../face/shared/para-setting/FaceParaInterface';

@Component({
  selector: 'app-detail-para',
  templateUrl: './detail-para.component.html',
  styleUrls: ['./detail-para.component.scss']
})
export class DetailParaComponent implements OnInit {

  constructor(
    private route: ActivatedRoute,
  ) { }

  options: FaceSettingOptions = {
    isIPC: false,
  };

  selectedMenuItem: string = 'FacePara';
  activeTab: string = '';
  subMenuItems: Array<string>;
  subMenuItemsEncodeParam: Array<string> = [];
  subMenuItemsRoi: Array<string> = [];
  subMenuItemsCrop: Array<string> = [];

  ngOnInit(): void {
    this.subMenuItems = this.subMenuItemsEncodeParam;
    this.route.queryParamMap.subscribe(params => {
      if (params.get('menu') === 'FacePara') {
        this.selectedMenuItem = 'FacePara';
        this.subMenuItems = this.subMenuItemsEncodeParam;
      } else if (params.get('menu') === 'ROI') {
        this.selectedMenuItem = 'ROI';
        this.subMenuItems = this.subMenuItemsEncodeParam;
      } else {
        this.selectedMenuItem = 'FacePara';
        this.subMenuItems = this.subMenuItemsEncodeParam;
      }
    });
  }

}
