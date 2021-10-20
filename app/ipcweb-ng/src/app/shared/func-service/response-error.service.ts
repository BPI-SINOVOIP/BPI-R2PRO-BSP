import { Injectable } from '@angular/core';

import { AuthService } from 'src/app/auth/auth.service';
import { TipsService } from 'src/app/tips/tips.service';
import { PublicFuncService } from 'src/app/shared/func-service/public-func.service';

@Injectable({
  providedIn: 'root'
})
export class ResponseErrorService {

  constructor(
    private auth: AuthService,
    private tips: TipsService,
    private pfs: PublicFuncService,
  ) { }

  // code, massage, todo
  errorDict = {
    401: [
      {
        pat: /token verification failed/,
        todo: 'logout',
        tip: 'loginExpire',
      },
      {
        pat: /Unauthorized/,
        todo: 'rbTip',
        tip: 'Unauthorized',
      },
    ],
    501: [
      {
        pat: /Not Implemented/,
        todo: 'rbTip',
        tip: 'responseError',
      },
    ],
    500: [
      {
        pat: /\[json\.exception\.type_error\.304\]/,
        todo: 'rbTip',
        tip: 'responseError',
      },
      {
        pat: /json\.exception\.out_of_range\.403/,
        todo: 'rbTip',
        tip: 'keyLoss',
      },
    ],
    de: {
      pat: '',
      todo: 'rbTip',
      tip: 'responseError',
    }
  };

  // analyseResp(resp: any, tip: string = '') {
  //   resp.subscribe(
  //     res => {
  //       this.analyseFunc(res, tip);
  //     }
  //   );
  //   return resp;
  // }

  analyseRes(res: any, tip: string = null, waitNav: string = null) {
    this.analyseFunc(res, tip, waitNav);
  }

  analyseFunc(res: any, tip: string, waitNav: string) {
    if (res['error']) {
      console.error('RES ERROR:', res['error']);
      const code = res['error']['code'];
      let msg = res['error']['message'];
      for (const item of this.errorDict[code]) {
        if (item.pat.test(msg)) {
          this.actionFunc(item, tip, msg, waitNav);
          return;
        }
      }
      this.actionFunc(this.errorDict.de, tip, msg, waitNav);
    }
  }

  actionFunc(item: ErrorCell, defaultTip: string, msg: any, waitNav: string) {
    switch (item.todo) {
      case 'logout':
        this.auth.logout();
        this.tips.setRbTip(item.tip);
        break;
      case 'rbTip':
        if (waitNav) {
          this.pfs.waitNavActive(20000, waitNav)
            .then(
              () => {
                this.tips.setRbTip(defaultTip ? defaultTip : item.tip);
              }
            )
            .catch(
              () => {
                console.error('res show tip fail!');
              }
          );
        } else {
          this.tips.setRbTip(defaultTip ? defaultTip : item.tip);
        }
        const err = msg.toString();
        throw err;
        break;
    }
  }

  isErrorCode(res: any) {
    if (res['error']) {
      console.error('RES ERROR:', res['error']);
      const code = res['error']['code'];
      let msg = res['error']['message'];
      return msg;
    }
    return '';
  }

}

export interface ErrorCell {
  pat: any;
  todo: string;
  tip: string;
}
