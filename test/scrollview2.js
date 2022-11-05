const path = require("path");
const { app, BaseWindow, ScrollView, View, WebContentsView } = require("electron");

const APP_WIDTH = 600;
const GAP = 30;

const APPS = [
  "https://bitbucket.org",
  "https://github.com",
  "https://youtube.com",
  "https://figma.com/",
  "https://thenextweb.com/",
  "https://engadget.com/",
  "https://theverge.com/",
  "https://mashable.com",
  "https://www.businessinsider.com",
  "https://wired.com",
  "https://macrumors.com",
  "https://gizmodo.com",
  "https://maps.google.com/",
  "https://sheets.google.com/",
];

global.win = null;

function createWindow () {
  win = new BaseWindow({ autoHideMenuBar: true, width: 1400, height: 1000 });

  const contentView = new View();
  contentView.setBackgroundColor("#1F2937");
  contentView.setBounds({x: 0, y: 0, width: 1378, height: 600});
  win.contentView.addChildView(contentView);

  // Scroll
  const scroll = new ScrollView({ smoothScroll: true });
  scroll.setBounds({x: 0, y: 0, width: 1377, height: 600});
  scroll.setHorizontalScrollBarMode("enabled");
  scroll.setVerticalScrollBarMode("disabled");
  scroll.setTreatAllScrollEventsAsHorizontal(true);

  contentView.addChildView(scroll);

  const scrollContent = new View();
  scroll.setContentView(scrollContent);
  scroll.setContentSize({ width: APPS.length * (APP_WIDTH + GAP), height: 600 });
  scrollContent.setBackgroundColor("#FF0000");

  const addWebview = function (scrollContent, url, i) {
    const webContentsView = new WebContentsView({
      webPreferences: {
        optimizeForScroll : true,
      }
    });
    webContentsView.webContents.loadURL(url);
    webContentsView.setBackgroundColor("#ffffff");
    webContentsView.setBounds({x: 0, y: 0, width: 600, height: 540});
    const webContentView = new View();
    webContentView.setBounds({x: i*(APP_WIDTH + GAP)+GAP, y: 30, width: 600, height: 540});
    webContentView.addChildView(webContentsView);
    scrollContent.addChildView(webContentView);
  };

  var i = 0;
  APPS.forEach((app) => {
    addWebview(scrollContent, app, i++);
  });
}

// This method will be called when Electron has finished
// initialization and is ready to create browser windows.
// Some APIs can only be used after this event occurs.
app.whenReady().then(() => {
  createWindow();

  app.on('activate', function () {
    // On macOS it's common to re-create a window in the app when the
    // dock icon is clicked and there are no other windows open.
    if (BaseWindow.getAllWindows().length === 0)
      createWindow();
  });
});

// Quit when all windows are closed, except on macOS. There, it's common
// for applications and their menu bar to stay active until the user quits
// explicitly with Cmd + Q.
app.on('window-all-closed', function () {
  if (process.platform !== 'darwin')
    app.quit();
});
