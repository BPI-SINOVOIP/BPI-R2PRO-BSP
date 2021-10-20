import { Component, OnInit, OnDestroy } from '@angular/core';
import { ActivatedRoute } from '@angular/router';
import { LayoutService } from 'src/app/shared/func-service/layout.service';
import { PublicFuncService } from 'src/app/shared/func-service/public-func.service';

@Component({
  selector: 'app-member-list',
  templateUrl: './member-list.component.html',
  styleUrls: ['./member-list.component.scss']
})
export class MemberListComponent implements OnInit, OnDestroy {

  private losOb: any;
  private layoutGroup: Array<string>;
  selectedMenuItem: string;
  activeTab: string = '';
  private paramOb: any;

  constructor(
    private route: ActivatedRoute,
    private los: LayoutService,
    private pfs: PublicFuncService,
  ) { }

  memberListOp: any = {
    secondKey: 'face',
    thirdKey: 'MemberList',
    fourthKey: 'ListManagement'
  };

  batchInputOp: any = {
    secondKey: 'face',
    thirdKey: 'MemberList',
    fourthKey: 'BatchInput'
  };

  addOneOp: any = {
    secondKey: 'face-manage',
    thirdKey: 'Manage',
    fourthKey: 'AddOne'
  };

  ngOnInit(): void {

    this.losOb = this.los.FinishSignal.subscribe(
      (si: boolean) => {
        if (si) {
          this.layoutGroup = this.los.getThirdLt('face', 'MemberList');
          this.paramOb = this.route.queryParamMap.subscribe(params => {
            let menu = params.get('menu');
            if (!this.pfs.isInArrayString(this.layoutGroup, menu) && this.layoutGroup.length > 0) {
              menu = this.layoutGroup[0];
            }
            if (menu === 'ListManagement') {
              this.selectedMenuItem = 'ListManagement';
            } else if (menu === 'BatchInput') {
              this.selectedMenuItem = 'BatchInput';
            } else if (menu === 'AddOne') {
              this.selectedMenuItem = 'AddOne';
            } else {
              this.selectedMenuItem = 'ListManagement';
            }
          });
          if (this.losOb) {
            this.losOb.unsubscribe();
            this.losOb = null;
          }
        }
      }
    );
  }

  ngOnDestroy() {
    if (this.losOb) {
      this.losOb.unsubscribe();
    }
    if (this.paramOb) {
      this.paramOb.unsubscribe();
    }
  }
}
