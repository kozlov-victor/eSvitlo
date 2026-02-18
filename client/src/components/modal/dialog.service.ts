import {AlertDialog} from "./dialogs/alert-dialog";
import {DI} from "@engine/core/ioc";
import {BaseDialogService, Dialog} from "./dialog/dialog";
import {PromptDialog} from "./dialogs/prompt-dialog";

@DI.Injectable()
export class DialogService {

    @DI.Inject(BaseDialogService) private baseDialogService: BaseDialogService;

    public async alert(text:string|JSX.Element,closeable?:false) {
        return await this.baseDialogService.open(AlertDialog,{text,closeable})
    }

    public alertSync(text:string|JSX.Element,closeable?:false) {
        return this.baseDialogService.openSync(AlertDialog,{text,closeable})
    }

    public async prompt(text:string|JSX.Element, buttons:[string,string] = ['','']) {
        return await this.baseDialogService.open(PromptDialog,{text,buttons})
    }

    public async openDialog<TOptions extends object,TResult>(clazz:new(props:any) => Dialog<TOptions, TResult>, props?: TOptions):Promise<TResult|undefined> {
        return await this.baseDialogService.open(clazz,props);
    }

}
