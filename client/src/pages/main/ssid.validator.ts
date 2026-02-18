import {DI} from "@engine/core/ioc";
import {ISsid} from "./model";

@DI.Injectable()
export class SsidValidator {

    public validate(ssid:ISsid):{success:boolean, message:string} {
        if (!ssid.ssid) {
            return {success: false, message: 'WIFI мережа не задана'};
        }
        if (!ssid.password) {
            return {success: false, message: 'Пароль не задано'};
        }
        if (!ssid.id) {
            return {success: false, message: 'ID не задано'};
        }
        if (!ssid.ep) {
            return {success: false, message: 'URL сервера не задано'};
        }
        if (!ssid.ep.startsWith('http')) {
            return {success: false, message: 'URL сервера невірна'};
        }
        if (ssid.ep.endsWith('/')) {
            return {success: false, message: 'URL сервера не може закінчуватись на /'};
        }
        if (!ssid.time) {
            return {success: false, message: 'Періодичність пінгу не задана'};
        }
        if (isNaN(+ssid.time)) {
            return {success: false, message: 'Невірний час'};
        }
        if (ssid.time<30 || ssid.time>60*5) {
            return {success: false, message: 'Невірний діапазон часу'};
        }
        return {success: true, message: ''};
    }

}