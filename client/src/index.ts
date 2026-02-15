import {MainWidget} from "./main/main.widget";

const root = document.createElement('div');
root.id = 'root';
document.body.appendChild(root);

const widget = new MainWidget();
widget.mountTo(root);