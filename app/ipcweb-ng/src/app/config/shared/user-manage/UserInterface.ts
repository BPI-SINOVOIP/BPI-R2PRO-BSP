export interface UserCell {
    iAuthLevel: number;
    id: number;
    sUserName: string;
    iUserLevel: number;
}

export interface UserStatus {
    status: number;
}

export interface UserForm {
    sUserName: string;
    sPassword: string;
    newUserName: string;
    newPassword: string;
    secondNewPw: string;
    iUserLevel: number;
}
