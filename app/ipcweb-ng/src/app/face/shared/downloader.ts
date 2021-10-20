export class Downloader {

    isFirstInFunc: boolean = true;
    jsonDict: any = {};
    oneTimeNumber: number;
    searchTime: number;
    page: number = 1;
    failTime: number = 0;
    limitTime: number = 5;
    ttlNumber: number;

    constructor(
        oneTimeNumber: number,
        ttlNumber: number,
    ) {
        this.oneTimeNumber = oneTimeNumber;
        this.ttlNumber = ttlNumber;
        this.searchTime = Math.ceil(ttlNumber / oneTimeNumber);
    }

    get beginPosition(): number {
        return (this.page - 1) * this.oneTimeNumber;
    }

    get endPosition(): number {
        if (this.page !== 1) {
            return Math.min((this.page) * this.oneTimeNumber - 1, this.ttlNumber);
          } else {
            return this.page * this.oneTimeNumber - 1;
          }
    }

    doneOneSearch() {
        this.failTime = 0;
        this.page += 1;
    }

    searchFail() {
        this.failTime += 1;
    }

    isSearchError() {
        if (this.failTime >= this.limitTime) {
            return true;
        }
        return false;
    }

    isSearchEnd() {
        if (this.page > this.searchTime) {
            return true;
        }
    }
}
