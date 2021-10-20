import { Component, OnInit, OnDestroy } from '@angular/core';
import { ActivatedRoute } from '@angular/router';

@Component({
  selector: 'app-manage-menu',
  templateUrl: './manage-menu.component.html',
  styleUrls: ['./manage-menu.component.scss']
})
export class ManageMenuComponent implements OnInit, OnDestroy {

  constructor(
    private route: ActivatedRoute,
  ) { }

  selectedMenuItem: string = 'MemberList';
  activeTab: string = '';
  private paraOb: any;

  controlOp: any = {
    secondKey: 'face-manage',
    thirdKey: 'Manage',
    fourthKey: 'Control'
  };

  memberListOp: any = {
    secondKey: 'face-manage',
    thirdKey: 'Manage',
    fourthKey: 'MemberList'
  };

  batchInputOp: any = {
    secondKey: 'face-manage',
    thirdKey: 'Manage',
    fourthKey: 'BatchInput'
  };

  snapShotOp: any = {
    secondKey: 'face-manage',
    thirdKey: 'Manage',
    fourthKey: 'SnapShot'
  };

  addOneOp: any = {
    secondKey: 'face-manage',
    thirdKey: 'Manage',
    fourthKey: 'AddOne'
  };

  ngOnInit(): void {
    this.paraOb = this.route.queryParamMap.subscribe(params => {
      if (params.get('menu') === 'MemberList') {
        this.selectedMenuItem = 'MemberList';
      } else if (params.get('menu') === 'BatchInput') {
        this.selectedMenuItem = 'BatchInput';
      } else if (params.get('menu') === 'SnapShot') {
        this.selectedMenuItem = 'SnapShot';
      } else if (params.get('menu') === 'Control') {
        this.selectedMenuItem = 'Control';
      } else if (params.get('menu') === 'AddOne') {
        this.selectedMenuItem = 'AddOne';
      } else {
        this.selectedMenuItem = 'MemberList';
      }
    });
  }

  ngOnDestroy() {
    if (this.paraOb) {
      this.paraOb.unsubscribe();
    }
  }
}
