import {DI} from "@engine/core/ioc";
import {HttpClient} from "../httpClient";

@DI.Injectable()
export class AuthService {

    private token: string;

    private setToken(token: string) {
        if (token) {
            sessionStorage.setItem('token',token);
        }
    }

    public getToken() {
        if (sessionStorage.getItem('token')) {
            this.token = sessionStorage.getItem('token')!;
        }
        return this.token;
    }

    public logout() {
        sessionStorage.clear();
        this.token = '';
    }

    public async createToken(login: string, password: string) {
        const resp = await HttpClient.post<{token:string}>('/auth/createToken',{login,password});
        this.setToken(resp.token);
    }

    public async setCredentials(login: string, password: string) {
        await HttpClient.post<void>('/personal-account/creds',{login,password})
    }

}