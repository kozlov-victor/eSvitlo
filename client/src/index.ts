import {RootWidget} from "./root/root.widget";

const root = document.createElement('div');
root.id = 'root';
document.body.appendChild(root);

const widget = new RootWidget();
widget.mountTo(root);