export default class Logger {
    module: string;

    constructor(module: string) {
        this.module = module;
    }

    log(msg: any, append: string = '') {
        console.log('[' + this.currentTimeStr() + '][' + this.module + ']', append, msg);
    }

    error(msg: any, append: string = '') {
        console.log('[' + this.currentTimeStr() + '][' + this.module + '][E] ', append, msg);
    }

    info(msg: any, append: string = '') {
        console.log('[' + this.currentTimeStr() + '][' + this.module + '][I] ', append, msg);
    }

    debug(msg: any, append: string = '') {
        console.log('[' + this.currentTimeStr() + '][' + this.module + '][D] ', append, msg);
    }

    modify(name: string) {
        this.module = name;
    }

    currentTimeStr(): string {
        const now = new Date(Date.now());
        const year = now.getFullYear();
        const month = now.getMonth() + 1;
        const day = now.getDate();
        const hour = now.getHours();
        const min = now.getMinutes();
        const sec = now.getSeconds();
        const ms = now.getMilliseconds();
        return year + '-' + month + '-' + day + ' ' + hour + ':' + min + ':' + sec + ':' + ms;
    }
}