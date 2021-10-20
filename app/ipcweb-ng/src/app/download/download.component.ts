import { Component, OnInit, ElementRef, OnDestroy } from '@angular/core';
import { LastNavService } from 'src/app/shared/func-service/last-nav.service';
import { LayoutService } from 'src/app/shared/func-service/layout.service';
import { PublicFuncService } from 'src/app/shared/func-service/public-func.service';

@Component({
  selector: 'app-download',
  templateUrl: './download.component.html',
  styleUrls: ['./download.component.scss']
})
export class DownloadComponent implements OnInit, OnDestroy {

  constructor(
    private lns: LastNavService,
    private el: ElementRef,
    private los: LayoutService,
    private pfs: PublicFuncService,
  ) { }

  private navGroup: string = 'download';
  private losOb: any;

  video: any = {
    type: 'videoRecord',
    title: 'Download.downloadList1',
    delete: false,
  };

  picture: any = {
    type: 'pictureRecord',
    title: 'Download.downloadList2',
    delete: false,
  };

  subMenuItems: Array<string>;
  subMenuItemsBasic: Array<string> = [
    'videoRecord',
    'pictureRecord'
  ];

  ngOnInit(): void {
    this.losOb = this.los.getSecondLayout('download', false).subscribe(
      (menu: string) => {
        this.subMenuItems = [];
        const second = JSON.parse(menu);
        for (const item of second) {
          if (this.pfs.isInArrayString(this.subMenuItemsBasic, item)) {
            this.subMenuItems.push(item);
            const deleteLt = this.los.getThirdLt('download', item);
            let deleteOp = false;
            if (deleteLt.length > 0 && deleteLt[0] === 'delete') {
              deleteOp = true;
            }
            switch (item) {
              case 'videoRecord':
                this.video.delete = deleteOp;
                break;
              case 'pictureRecord':
                this.picture.delete = deleteOp;
                break;
            }
          }
        }
        this.losOb.unsubscribe();
        this.losOb = null;
      }
    );
    this.lns.getLastNav(this.navGroup, this.el);
  }

  ngOnDestroy() {
    if (this.losOb) {
      this.losOb.unsubscribe();
    }
  }

  setActiveTab(tab: string): void {
    this.lns.setLastNav(this.navGroup, tab + 'Tab');
  }
}
