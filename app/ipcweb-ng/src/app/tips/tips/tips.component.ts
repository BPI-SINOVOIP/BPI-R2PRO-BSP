import { Component, OnInit, OnDestroy, ViewChild } from '@angular/core';
import { TipsService } from '../tips.service';
import { PublicFuncService } from 'src/app/shared/func-service/public-func.service';

@Component({
  selector: 'app-tips',
  templateUrl: './tips.component.html',
  styleUrls: ['./tips.component.scss']
})
export class TipsComponent implements OnInit, OnDestroy {

  @ViewChild('rbTip', {static: true}) rbTipChild: any;
  @ViewChild('cTip', {static: true}) cTipChild: any;
  constructor(
    private tips: TipsService,
    private pfs: PublicFuncService,
  ) { }

  private rbObserver: any;
  private jsonPat = /\:/;

  ngOnInit(): void {
    this.rbObserver = this.tips.rbContent.subscribe(
      (res: string) => {
        if (this.jsonPat.test(res)) {
          const jsonContent = JSON.parse(res);
          for (const key of this.pfs.objectKeys(jsonContent)) {
            this.rbTipChild.onShow(key, jsonContent[key]);
          }
        } else {
          this.rbTipChild.onShow(res);
        }
      }
    );
  }

  ngOnDestroy() {
    this.rbObserver.unsubscribe();
  }

}
