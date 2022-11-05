import { View } from 'electron/main';

const { ScrollView } = process._linkedBinding('electron_browser_scroll_view');

Object.setPrototypeOf(ScrollView.prototype, View.prototype);

export default ScrollView;
