import {HttpClient, IRequestOptions} from "../httpClient";
import {DI} from "@engine/core/ioc";
import {AuthService} from "./auth.service";
import {Router} from "../router";

export class BaseService {

    @DI.Inject(AuthService) private readonly authService: AuthService;
    @DI.Inject(Router) private readonly router: Router;

    private redirectToLoginPageIfRequired(e:any) {
        if (e?.status===403) {
            this.router.navigate('login');
        }
    }

    private getTokenHeader() {
        return {'Authorization':`Bearer ${this.authService.getToken()}`};
    }

    public async post<T>(url: string, data?: any, options: IRequestOptions<T> = {}) {
        options.headers = this.getTokenHeader();
        try {
            return await HttpClient.post<T>(url,data,options);
        }
        catch (e) {
            this.redirectToLoginPageIfRequired(e);
            throw e;
        }
    }

    public async get<T>(url: string, data?: any) {
        try {
            return await HttpClient.get<T>(url,data,{headers: this.getTokenHeader()});
        }
        catch (e) {
            this.redirectToLoginPageIfRequired(e);
            throw e;
        }
    }

}