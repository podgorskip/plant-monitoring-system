import { RoleEnum } from "./enums/RoleEnum";

export interface User {
    id: number;
    mac: string;
    firstName: string;
    lastName: string;
    password: string;
    email: string;
    phoneNumber: string;
    verified: boolean;
    role: RoleEnum
}