import {DomRootComponent} from "@engine/renderable/tsx/dom/domRootComponent";
import {VEngineTsxFactory} from "@engine/renderable/tsx/_genetic/vEngineTsxFactory.h";
import {MainWidget} from "../pages/main/main.widget";
import {DI} from "@engine/core/ioc";
import {BaseDialogService} from "../components/modal/dialog/dialog";
import {Router} from "../router";
import {UpdateWidget} from "../pages/update/update.widget";
import {LoginWidget} from "../pages/login/login.widget";
import {PersonalAccountWidget} from "../pages/personal-account/personal-account.widget";

@DI.Injectable()
@DI.CSS('./root.widget.css')
export class RootWidget extends DomRootComponent {

    @DI.Inject(BaseDialogService) private readonly baseDialogService: BaseDialogService;
    @DI.Inject(Router) private readonly router: Router;


    constructor() {
        super();
        this.router.setUp({
            '/':()=>({component: <MainWidget/>}),
            'update':()=>({component: <UpdateWidget/>}),
            'login':()=>({component: <LoginWidget/>}),
            'personal-account':()=>({component: <PersonalAccountWidget/>}),
        });
    }

    render(): JSX.Element {
        return (
            <>
                {this.router.getOutlet()}
                <div>
                    {this.baseDialogService.getOutlet()}
                </div>
            </>
        );
    }


}